// C includes
#include "raylib.h"

#ifdef PLATFORM_DESKTOP
#include <dirent.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// Game-specific includes and stuff
#include "../include/player.h"  // That works.
#include "../include/dot.h"
#include "../include/potion.h"

#include <unistd.h>

bool debug_prints = false;
bool extreme_debug_prints = false;

// Setting variables for game logic
Player dotty{50, 50, 96, 96};
Player dupedotty{50, 50, 96, 96};
double velX;
double velY;
double vel;
int game_status = 0;
bool left;
bool right;
bool up;
bool down;
bool old_left;
bool old_right;
bool old_up;
bool old_down;
bool has_moved = false;
bool vertically_collided = false;
bool nightmare = false;  // enable this to experience pain
int hasnt_moved_counter = 0;
int random_counter;

bool has_saved = false;

int screenWidth = 800;
int screenHeight = 450;
int screenWidthBeforeFullscreen;
int screenHeightBeforeFullscreen;

Dot dots[8] = {};
Potion potion;
Potion dupepotion;
Potion doompotion;
bool dupe = false;

int highscoreInt;
char highscore[32];

int dot_amount = 1;
int eaten = 0;
typedef enum Actions { NOTHING, DOT_OBTAINED, DUPE_DOTS, DUPE_DOTTY, DUPE_LOST, OUCHIE } Actions;
typedef enum Failures { ALIVE, SCREEN_EDGE, CLONE_COLLISION, DOOM_POTION } Failures;  // no, being alive is not a failure. you're amazing alive. keep being you.
Actions collision_type = NOTHING;
Actions old_collision_type;
Failures failure;
double multiplier;

int currentGesture = GESTURE_NONE;
int lastGesture = GESTURE_NONE;
bool gesture_movement = false;
typedef enum GameScreen { SPLASH, TITLE, GAMEPLAY, PAUSE, GAMEOVER } GameScreen;
bool pauseButtonAlreadyPressed;
bool disableEscapeQuit = false;  // set it to "true" and it'll pause the game when the user presses "escape"

bool pressed_pause_to_continue = false;  // often times, people would press the Space key to play again, after a "game over" screen
//--------------------------------------------------------------------------------------

double get_velocity() {return ((((screenWidth / 265) + (screenHeight / 150)) / 2) + ((eaten * 2) * multiplier));};

void reset_dots(const int screenWidth, const int screenHeight){
    for (int i = 0; i < 8; i++) dots[i].remove();
    for (int i = 0; i < dot_amount; i++) dots[i].updatePosition(screenWidth, screenHeight);
}

void reset_dotty(const int screenWidth, const int screenHeight){
    dotty = {50, 50, 96, 96};
    dupedotty = {-5000, -5000, 96, 96};
    dot_amount = 1;
    multiplier = 0.1;
    vel = get_velocity();
    velX = 3;
    velY = 0;
    eaten = 0;
    dupe = false;
    has_moved = false;
    vertically_collided = false;
    #if defined(PLATFORM_DESKTOP)
        highscoreInt = LoadStorageValue(0);
    #endif

    reset_dots(screenWidth, screenHeight);
    potion.remove();
    potion.setAvailable(true);
    dupepotion.remove();
    dupepotion.setAvailable(true);
    doompotion.remove();
};

bool check_collision_line(double point1, double point2){
    // point1 = point1.getX()
    // point2 = point2.getX()
    // point1 = dotty
    // point2 = dot

    return (point1 + 48 > point2 && point1 < point2 + 48);
};

bool check_collision_2d(double point1x, double point1y, double point2x, double point2y){
    // point1 = dotty
    // point2 = dot

    return (check_collision_line(point1x, point2x) && check_collision_line(point1y, point2y));
};

bool check_dotty_collision(double x, double y){
    // Checks if either the main Dotty or clone Dotty collides against an object using specified coordinates.
    return (check_collision_2d(dotty.getX(), dotty.getY(), x, y)       // Main Dotty collided
    or check_collision_2d(dupedotty.getX(), dupedotty.getY(), x, y));  // Dupe Dotty collided
};

bool check_player_window_collision(){
    if(dotty.getY() + 64 >= screenHeight) vertically_collided = true;
    return (((dotty.getX() + 64 >= screenWidth) or (dotty.getX() <= 0)) or ((dotty.getY() + 64 >= screenHeight) or (dotty.getY() <= 0)));
};

void checkPlayerCollision(const int screenWidth, const int screenHeight)
{
    for (int i = 0; i < 8; i++)
    {
        if (check_dotty_collision(dots[i].getX(), dots[i].getY())){
            // dot obtained
            dots[i].updatePosition(screenWidth, screenHeight);
            if (velX > 0) velX = vel;
            if (velX < 0) velX = vel * -1;
            if (velY > 0) velY = vel;
            if (velY < 0) velY = vel * -1;
            eaten+=1;
            collision_type = DOT_OBTAINED;
        }
    }
    if (check_dotty_collision(potion.getX(), potion.getY())){
        // dupe dot potion obtained
        collision_type = DUPE_DOTS;
        if (not (dot_amount == 8)){
            dot_amount+=1;
            dots[dot_amount].updatePosition(screenWidth, screenHeight);
        } else{
            potion.setAvailable(false);
        }
        potion.remove();
    }if (check_collision_2d(dotty.getX(), dotty.getY(), dupepotion.getX(), dupepotion.getY())){
        // dupe dotty potion obtained
        collision_type = DUPE_DOTTY;
        dupepotion.setAvailable(false);
        dupepotion.remove();
        dupe = true;
    }
    if (check_collision_2d(dotty.getX(), dotty.getY(), doompotion.getX(), doompotion.getY())){
        // main dotty obtained doom potion
        collision_type = OUCHIE;
        failure = DOOM_POTION;
    }
    if (check_collision_2d(dupedotty.getX(), dupedotty.getY(), doompotion.getX(), doompotion.getY())){
        // dupe dotty obtained doom potion
        collision_type = DUPE_LOST;
        dupe = false;
        dupepotion.setAvailable(true);
        doompotion.remove();
    }
    if (dupe){
        // dotty.getY() > dupedotty.getY() and dotty.getY() < dupedotty.getY() + 48
        // if ((dotty.getX() > dupedotty.getX() - 48 && dotty.getX() < dupedotty.getX() + 48) and
        //     (dotty.getY() > dupedotty.getY() - 48 && dotty.getY() < dupedotty.getY() + 48)){
        if (check_collision_2d(dotty.getX(), dotty.getY(), dupedotty.getX(), dupedotty.getY())){
            collision_type = OUCHIE;
            failure = CLONE_COLLISION;
            // the duplicate Dotty's entered in collision
            // i would say "bonk" but the British definition of the verb prevents me from doing so
        }
    }
}
// Controller/keyboard input mechanisms
bool check_left(void){
    bool go_left = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (extreme_debug_prints) printf("Controller %s detected\n", GetGamepadName(a));  // debugging controllers.
            if (IsGamepadButtonPressed(a, 4)){
                go_left = true;
                if (debug_prints) printf("Left D-pad on controller name %s\n", GetGamepadName(a));
            }
            else if (GetGamepadAxisMovement(a, 0) < -0.50){
                go_left = true;
                if (debug_prints) printf("Left joystick on controller name %s\n", GetGamepadName(a));
            }
            
        }
    }
    if (IsKeyPressed(KEY_LEFT) or IsKeyPressed(KEY_A)){
        go_left = true;
        if (debug_prints) printf("Left key pressed on keyboard\n");
    }
    return (go_left);
}

bool check_right(void){
    bool go_right = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (extreme_debug_prints) printf("Controller %s detected\n", GetGamepadName(a));  // debugging controllers.
            if (IsGamepadButtonPressed(a, 2)){
                go_right = true;
                if (debug_prints) printf("Right D-pad on controller name %s\n", GetGamepadName(a));
            }
            else if (GetGamepadAxisMovement(a, 0) > 0.50){
                go_right = true;
                if (debug_prints) printf("Right joystick on controller name %s\n", GetGamepadName(a));
            }
        }
    }
    if (IsKeyPressed(KEY_RIGHT) or IsKeyPressed(KEY_D)){
        go_right = true;
        if (debug_prints) printf("Right key pressed on keyboard\n");
    }
    return (go_right);
}

bool check_up(void){
    bool go_up = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (extreme_debug_prints) printf("Controller %s detected\n", GetGamepadName(a));  // debugging controllers.
            if (IsGamepadButtonPressed(a, 1)){
                go_up = true;
                if (debug_prints) printf("Up D-pad on controller name %s\n", GetGamepadName(a));
            }
            else if (GetGamepadAxisMovement(a, 1) < -0.50){
                go_up = true;
                if (debug_prints) printf("Up joystick on controller name %s\n", GetGamepadName(a));
            }
        }
    }
    if (IsKeyPressed(KEY_UP) or IsKeyPressed(KEY_W)){
        go_up = true;
        if (debug_prints) printf("Up key pressed on keyboard\n");
    }
    return (go_up);
}

bool check_down(void){
    bool go_down = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (extreme_debug_prints) printf("Controller %s detected\n", GetGamepadName(a));  // debugging controllers.
            if (IsGamepadButtonPressed(a, 3)){
                go_down = true;
                if (debug_prints) printf("Down D-pad on controller name %s\n", GetGamepadName(a));
            }
            else if (GetGamepadAxisMovement(a, 1) > 0.50){
                go_down = true;
                if (debug_prints) printf("Down joystick on controller name %s\n", GetGamepadName(a));
            }
        }
    }
    if (IsKeyPressed(KEY_DOWN) or IsKeyPressed(KEY_S)){
        go_down = true;
        if (debug_prints) printf("Down key pressed on keyboard\n");
    }
    return (go_down);
}

bool cont_exit(void){
    bool exit = false;
    int a;
    if(WindowShouldClose()) return true;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if ((IsGamepadButtonPressed(a, 14) or (IsGamepadButtonDown(a, 13) and IsGamepadButtonDown(a, 15))) and IsWindowFocused()){
                exit = true;
                if (debug_prints) printf("Home button pressed on controller name %s: exiting...\n", GetGamepadName(a));
            }
        }
    }
    return exit;
}

bool press_start(void){
    bool start = false;
    int pressed_key = GetKeyPressed();
    if (GetGamepadButtonPressed() > 0){
        start = true;
    }
    if ((pressed_key != 0 &&                                                                                // check if there's actually a key pressed
       !(pressed_key >= 262 && pressed_key <= 265) &&                                                       // ignore arrow keys
       !(pressed_key >= 290 && pressed_key <= 301) &&                                                       // ignore F* keys
       !(pressed_key == KEY_W || pressed_key == KEY_A || pressed_key == KEY_S || pressed_key == KEY_D)) ||  // ignore WASD keys
       IsGestureDetected(GESTURE_TAP)){                                                                     // check if the user tapped on the window or screen
        start = true;
    }
    int a;
    for( a = 0; a <= 8; a+=1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            int b;
            for (b=0;b <= GetGamepadAxisCount(a);b+=1){
                if (GetGamepadAxisMovement(a, b) == 1){
                    start = true;
                }
            }
        }
    }
    return (start);
}

bool do_pause(void){
    bool pause = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (IsGamepadButtonPressed(a, 15)){
                pause = true;
                if (debug_prints) printf("Pause button pressed on controller name %s\n", GetGamepadName(a));
            }
            if ((GetGamepadAxisMovement(a, 4) >= 0.5) && (GetGamepadAxisMovement(a, 5) >= 0.5)){
                pause = true;
                if (debug_prints) printf("Both triggers pressed on controller name %s\n", GetGamepadName(a));
            }
        }
    }
    if (GetGamepadButtonPressed() == 5 || GetGamepadButtonPressed() == 8){
        pause = true;
    }
    if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_SPACE) || IsGestureDetected(GESTURE_DOUBLETAP)){
        pause = true;
    }
    pressed_pause_to_continue = false;
    if ((pauseButtonAlreadyPressed and pause) or pressed_pause_to_continue){
        pause = false;
    }else{
        pauseButtonAlreadyPressed = pause;
    }
    return (pause);
}
//--------------------------------------------------------------------------------------


int main(void)
{
    #if defined(PLATFORM_DESKTOP)
        if (!opendir("resources")) {
            printf("Could not find the resources/ folder. This will render the game unplayable.\n");
            exit(1);
        }
        SetWindowIcon(LoadImage("resources/images/logo/icon.ico"));
    #endif
    
    if (extreme_debug_prints) debug_prints = true;

    InitWindow(screenWidth, screenHeight, "Dotty");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    int framesCounter = 0;  // used to show title screen after 2 seconds

    // Initialize audio
      InitAudioDevice();
      Sound ouchie       = LoadSound("resources/sounds/ouchie.wav");
      Sound obtained     = LoadSound("resources/sounds/obtained.wav");
      Sound potion_sound = LoadSound("resources/sounds/potion.wav");
      Sound clone_ouchie = LoadSound("resources/sounds/clone-ouchie.wav");

    // Initialize textures.
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    // NOTE: Otherwise you'll get a "Segmentation fault" error.

    // Splash logo
      Texture2D splash_logo = LoadTexture("resources/images/splash.png");
    // Basic textures
      Texture2D dotty_base = LoadTexture("resources/images/dotty/base.png");
      Texture2D dotty_front = LoadTexture("resources/images/dotty/front.png");
      Texture2D dotty_clone = LoadTexture("resources/images/dotty/clone.png");
      Texture2D dotty_sleep = LoadTexture("resources/images/dotty/sleep.png");
    // Up-down textures
      Image dotty_down_image = LoadImage("resources/images/dotty/down.png");
      Image *dotty_up_image = &dotty_down_image;
      Texture2D dotty_down = LoadTextureFromImage(dotty_down_image);
      ImageFlipVertical(dotty_up_image);
      Texture2D dotty_up = LoadTextureFromImage(dotty_down_image);
      UnloadImage(dotty_down_image);
    // Side textures
      Image dotty_side = LoadImage("resources/images/dotty/side.png");
      Image *dotty_side_flipped = &dotty_side;
      Texture2D dotty_right = LoadTextureFromImage(dotty_side);
      ImageFlipHorizontal(dotty_side_flipped);
      Texture2D dotty_left = LoadTextureFromImage(dotty_side);
      UnloadImage(dotty_side);
    // Power-up textures
      Texture2D dot = LoadTexture("resources/images/items/dot.png");
    // Potion textures
      Texture2D potion_dot = LoadTexture("resources/images/items/potion-dots.png");
      Texture2D potion_dupe = LoadTexture("resources/images/items/potion-dupe.png");
      Texture2D potion_doom = LoadTexture("resources/images/items/smallpotion-doom.png");
    // Power-up textures
      Texture2D oopsies_screenedge = LoadTexture("resources/images/oopsies/screen-edge.png");
      Texture2D oopsies_clonecollision = LoadTexture("resources/images/oopsies/clone-collision.png");
      Texture2D oopsies_what = LoadTexture("resources/images/oopsies/what-just-happened.png");
      Texture2D oopsies_doom = LoadTexture("resources/images/oopsies/doom-drank.png");
    // Likely unused textures
    //   Texture2D dotty_blink = LoadTexture("resources/images/dotty/blink.png");
    //   Texture2D dotty_half = LoadTexture("resources/images/dotty/half-blink.png");
    //--------------------------------------------------------------------------------------




    GameScreen currentScreen = SPLASH;

    reset_dotty(screenWidth, screenHeight);

    SetTargetFPS(60);

    while (!cont_exit())  // run until user presses either Xbox/PS/Home/Steam button on controller or closes window
                          // check the actual function for more info on "what does it do and how does it do"
    {
        if (disableEscapeQuit){
            SetExitKey(-420); // i could've just put 0 but, no.
        }
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        if(IsWindowResized()){
            if (screenWidth < 800)  SetWindowSize(800, GetScreenHeight());
            if (screenHeight < 450) SetWindowSize(GetScreenWidth(), 450);
        }

        #if defined(PLATFORM_DESKTOP)
            if(IsKeyPressed(KEY_F9)){
                if(IsWindowFullscreen()){
                    // fullscreen to windowed

                    ToggleFullscreen();
                    ClearWindowState(FLAG_VSYNC_HINT);
                    SetWindowSize(screenWidthBeforeFullscreen, screenHeightBeforeFullscreen);
                    // Window ready

                    screenWidth = GetScreenWidth();
                    screenHeight = GetScreenHeight();

                    dotty = {50, 50, 96, 96};
                    reset_dots(screenWidthBeforeFullscreen, screenHeightBeforeFullscreen);
                    {
                        if((potion.getX() + 96 > screenWidthBeforeFullscreen) or
                           (potion.getY() + 96 > screenHeightBeforeFullscreen))
                            potion.randomize_position(screenWidthBeforeFullscreen, screenHeightBeforeFullscreen);
                        if((dupepotion.getX() + 96 > screenWidthBeforeFullscreen) or
                           (dupepotion.getY() + 96 > screenHeightBeforeFullscreen))
                            dupepotion.randomize_position(screenWidthBeforeFullscreen, screenHeightBeforeFullscreen);
                        if((doompotion.getX() + 96 > screenWidthBeforeFullscreen) or
                            doompotion.getY() + 96 > screenHeightBeforeFullscreen)
                            doompotion.randomize_position(screenWidthBeforeFullscreen, screenHeightBeforeFullscreen);
                    }
                    // otherwise stuff will still be outside of the window and cause a game over.
                } else{
                    // windowed to fullscreen
                    screenWidthBeforeFullscreen = screenWidth;
                    screenHeightBeforeFullscreen = screenHeight;
                    SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
                    SetWindowState(FLAG_VSYNC_HINT);

                    ToggleFullscreen();
                    // This applies fullscreen as we all know it.

                    // SetWindowState(FLAG_WINDOW_UNDECORATED); SetWindowPosition(0, 0);
                    // This does borderless windowed. This has been disabled since I can't find a way to re-enable the window decorations.
                    // It also tends to bend window reality.
                }
            }
        #endif

        vel = get_velocity();
        if(nightmare) vel *= 10;

        switch(currentScreen)
        {
            case SPLASH: 
            {
                framesCounter++;    // Count frames

                if (framesCounter > 90 or press_start()){
                    currentScreen = TITLE;
                }
            } break;
            case TITLE: 
            {
                if (press_start()){
                    currentScreen = GAMEPLAY;
                }
            } break;
            case GAMEPLAY:
            {
                sprintf(highscore, "BEST: %d", highscoreInt);

                failure = ALIVE;
                gesture_movement = false;

                // add control stuff
                left  = check_left();
                right = check_right();
                up    = check_up();
                down  = check_down();

                lastGesture = currentGesture;
                currentGesture = GetGestureDetected();

                if (do_pause() or !IsWindowFocused()){
                    currentScreen = PAUSE;
                    break;
                }

                if (currentGesture >= 16 and currentGesture <= 128) gesture_movement = true;

                if(IsWindowResized()){
                    if(check_player_window_collision()){
                        dotty.setX(50);
                        dotty.setY(50);
                        left = false;
                        right = true;
                    }
                }

                if (left or right or up or down or gesture_movement){
                    velX = 0;
                    velY = 0;
                    has_moved = true;
                }

                // gesture movements
                if (gesture_movement){
                    switch (currentGesture){
                        case GESTURE_SWIPE_RIGHT: right = true; break;
                        case GESTURE_SWIPE_LEFT: left = true; break;
                        case GESTURE_SWIPE_UP: up = true; break;
                        case GESTURE_SWIPE_DOWN: down = true; break;
                        default: break;
                    }
                }

                // applying velocity integers
                     if (left  or velX < 0) velX = vel * -1;
                else if (right or velX > 0) velX = vel;
                else if (up    or velY < 0) velY = vel * -1;
                else if (down  or velX > 0) velY = vel;

                // check if player is colliding with the window borders
                if (check_player_window_collision()){
                    collision_type = OUCHIE;
                    failure = SCREEN_EDGE;
                }

                // applying movements
                dotty.setX(dotty.getX() + velX);
                dotty.setY(dotty.getY() + velY);
                if (dupe){
                    dupedotty.setX(screenWidth  - (dotty.getX() + 64));
                    dupedotty.setY(screenHeight - (dotty.getY() + 64));
                }

                // player collision
                checkPlayerCollision(screenWidth, screenHeight);
                old_collision_type = collision_type;
                collision_type = NOTHING;
                if (old_collision_type != NOTHING){
                    switch (old_collision_type){
                        case DOT_OBTAINED: PlaySound(obtained); break;
                        case DUPE_LOST:    PlaySound(clone_ouchie); break;
                        case DUPE_DOTS:    PlaySound(potion_sound); break;
                        case DUPE_DOTTY:   PlaySound(potion_sound); break;
                        case OUCHIE: {
                            // game over
                            PlaySound(ouchie);
                            currentScreen = GAMEOVER;
                            has_saved = false;
                            break;
                        }
                        default: break;
                    }
                }

                // potion randomizer stuff
                if (rand() % 2000 == 1){
                    potion.update(screenWidth, screenHeight);
                    if(debug_prints) PlaySound(clone_ouchie);
                }
                if (rand() % 2500 == 1){
                    dupepotion.update(screenWidth, screenHeight);
                    if(debug_prints) PlaySound(clone_ouchie);
                }
                if (rand() % 500 == 1){
                    if(debug_prints) PlaySound(clone_ouchie);
                    while(true){
                        doompotion.update(screenWidth, screenHeight);
                        if(!((dotty.getX() + 96 > doompotion.getX() && dotty.getX() < doompotion.getX() + 96) && (dotty.getY() + 96 > doompotion.getY() && dotty.getY() < doompotion.getY() + 96)) or
                            ((dupedotty.getX() + 96 > doompotion.getX() && dupedotty.getX() < doompotion.getX() + 96) && (dupedotty.getY() + 96 > doompotion.getY() && dupedotty.getY() < doompotion.getY() + 96))){
                               break;
                           }
                    }
                }
            } break;
            case GAMEOVER: 
            {
                if (nightmare){  // The user didn't listen.
                    has_moved = false;
                    hasnt_moved_counter = 21;
                    random_counter = 1;
                }
                if (eaten > highscoreInt){
                    highscoreInt = eaten;
                    #if defined(PLATFORM_DESKTOP)
                        if(not has_saved){
                            SaveStorageValue(0, highscoreInt);  // stores highscore integer in storage.data in an Int32 format
                            has_saved = true;
                        };
                    #endif
                };

                if (press_start())
                {
                    if(not has_moved) hasnt_moved_counter+=1;
                    if(hasnt_moved_counter == 22) nightmare = true;
                    if(hasnt_moved_counter == 20)
                    random_counter = rand() % 3 + 1;
                    reset_dotty(screenWidth, screenHeight);
                    if(do_pause()) pressed_pause_to_continue = true;
                    currentScreen = GAMEPLAY;
                }  
            } break;
            case PAUSE: 
            {
                if (do_pause()) currentScreen = GAMEPLAY;
            } break;
            default: break;
        }

        BeginDrawing();

        switch(currentScreen) 
            {
                case SPLASH: 
                {
                    ClearBackground(DARKGRAY);
                    DrawTexture(dotty_sleep, (screenWidth - dotty_sleep.width) / 2, (screenHeight  - dotty_sleep.height) / 2.25, WHITE);
                    DrawText("made with raylib", 20, screenHeight - 40, 20, WHITE);
                    
                } break;
                case TITLE: 
                {
                    ClearBackground(GRAY);
                    DrawTexture(splash_logo, (screenWidth / 2) - splash_logo.width / 2, (screenHeight / 2) - splash_logo.height * 0.75, GRAY);
                    DrawText("press any key", (screenWidth / 2) - (219 / 2), screenHeight * 0.75, 30, BLACK);
                    
                } break;
                case GAMEPLAY:
                {
                    ClearBackground(RAYWHITE);

                    for (int i = 0; i < 8; i++){
                        DrawTexture(dot, dots[i].getX(), dots[i].getY(), WHITE);
                    }
        
                    DrawTexture(potion_dot,  potion.getX(),     potion.getY(),     WHITE);
                    DrawTexture(potion_dupe, dupepotion.getX(), dupepotion.getY(), WHITE);
                    DrawTexture(potion_doom, doompotion.getX(), doompotion.getY(), WHITE);

                    DrawTexture(dotty_base, dotty.getX(), dotty.getY(), WHITE);
                    if (velX > 0){
                        DrawTexture(dotty_right, dotty.getX(), dotty.getY(), WHITE);
                    }else if (velX < 0){
                        DrawTexture(dotty_left, dotty.getX(), dotty.getY(), WHITE);
                    }else if (velY > 0){
                        DrawTexture(dotty_down, dotty.getX(), dotty.getY(), WHITE);
                    }else if (velY < 0){
                        DrawTexture(dotty_up, dotty.getX(), dotty.getY(), WHITE);
                    }else{
                        // couldn't determine in which position they were going
                        DrawTexture(dotty_front, dotty.getX(), dotty.getY(), WHITE);
                    }
        
                    if(dupe){
                        DrawTexture(dotty_base, dupedotty.getX(), dupedotty.getY(), WHITE);
                        DrawTexture(dotty_clone, dupedotty.getX(), dupedotty.getY(), WHITE);
                        if (velX > 0){
                            DrawTexture(dotty_left, dupedotty.getX(), dupedotty.getY(), WHITE);
                        }else if (velX < 0){
                            DrawTexture(dotty_right, dupedotty.getX(), dupedotty.getY(), WHITE);
                        }else if (velY > 0){
                            DrawTexture(dotty_up, dupedotty.getX(), dupedotty.getY(), WHITE);
                        }else if (velY < 0){
                            DrawTexture(dotty_down, dupedotty.getX(), dupedotty.getY(), WHITE);
                        }else{
                            // couldn't determine in which position they were going
                            DrawTexture(dotty_front, dupedotty.getX(), dupedotty.getY(), WHITE);
                        }
                    }
                    
                    char eaten_cchar[512];
                    sprintf(eaten_cchar, "%d", eaten);
                    const char* bottom_text;
                    #if defined(PLATFORM_DESKTOP)
                        bottom_text = highscore;
                        DrawText(eaten_cchar, 20, screenHeight - 80, 20, DARKGRAY);
                    #else
                        bottom_text = eaten_cchar;
                    #endif
                    DrawText(bottom_text, 20, screenHeight - 40, 20, DARKGRAY);
                } break;
                case GAMEOVER: 
                {
                    const char* gameover_message = "What happened? How did you die?";
                    ClearBackground(DARKGRAY);
                    Texture2D &oopsie_texture = oopsies_what;
                    DrawText("GAME OVER", 20, 20, 40, WHITE);
                    switch (failure){
                        case SCREEN_EDGE:     gameover_message = "You hit the window borders.";               oopsie_texture = oopsies_screenedge;     break;
                        case CLONE_COLLISION: gameover_message = "You entered in collision with your clone."; oopsie_texture = oopsies_clonecollision; break;
                        case DOOM_POTION:     gameover_message = "You drank the death potion.";               oopsie_texture = oopsies_doom;           break;
                        default: break;
                    }
                    if (not has_moved){
                        if(hasnt_moved_counter >= 0 and hasnt_moved_counter <= 1) gameover_message  = "You hit the window borders.\nYou can move using the arrow or WASD keys.\n\nYou can also move using the left thumbstick\nor D-Pad if you are using a controller.";
                        else{
                            switch (hasnt_moved_counter){
                                case 2: gameover_message  = "Hey, you're supposed to move!"; break;
                                case 3: gameover_message  = "MOVE USING THE ARROW KEYS OR THE WASD KEYS!"; break;
                                case 4: gameover_message  = "You're just messing with me, aren't you."; break;
                                case 5: gameover_message  = "Stop that."; break;
                                case 6: gameover_message  = "I said, stop that."; break;
                                case 7: gameover_message  = "ENOUGH!"; break;
                                case 8: gameover_message  = "You're pissing me off..!"; break;
                                case 9: gameover_message  = "HEY. STOP THAT. YOU'RE HURTING A POOR, INNOCENT LITTLE SQUARE."; break;
                                case 10: gameover_message = "You're a masochist..."; break;
                                case 11: gameover_message = "Come on, man. Give Dotty a break."; break;
                                case 12: gameover_message = "Are you just doing this to see what I have to say?"; break;
                                case 13: gameover_message = "Please get help."; break;
                                case 14: gameover_message = "Dots. Dotty hungry. Dot is Dotty food. Dotty want food.\nDotty want dot. You control Dotty. You give Dotty dot."; break;
                                case 15: gameover_message = "Is the pizza here yet?"; break;
                                case 16: gameover_message = "Man, I feel bad for Dotty."; break;
                                case 17: gameover_message = "Okay, I'm not joking. Stop."; break;
                                case 18: gameover_message = "You're forcing my hand."; break;
                                case 19: gameover_message = "Stop, I'm serious. This is your last chance."; break;
                                case 20: gameover_message = "OLOLO POOLOA"; break;
                                case 21: {
                                    gameover_message = "You made me do this. :)";
                                    if(random_counter == 1){
                                        // You're fricked :)
                                        #if defined(PLATFORM_DESKTOP)
                                            SaveStorageValue(0, 1);
                                        #endif
                                        CloseWindow();
                                    }
                                } break;
                                case 22: gameover_message = "â”¬â”€â”¬ ãƒŽ( ã‚œ-ã‚œãƒŽ)"; break;
                                default: break;
                            }
                        }
                    }
                    // if (not has_moved) gameover_message = "lmao fat fuck";
                    if (((GetScreenHeight() - dotty.getY()) > 64 and (dotty.getY() > 0) and vertically_collided) and failure == SCREEN_EDGE) gameover_message = "You really shouldn't play around with the window like that.";
                    DrawText(gameover_message, 20, 80, 20, WHITE);
                    DrawTexture(oopsie_texture, screenWidth - 512, screenHeight - 256, DARKGRAY);
                    DrawText("press any key to continue", 20, screenHeight - 40, 20, WHITE);
                } break;
                case PAUSE: 
                {
                    // DrawRectangle(0, 0, screenWidth, screenHeight, GRAY);
                    ClearBackground(LIGHTGRAY);
                    int displayHeight = screenHeight / 2;
                    DrawText("PAUSED", (screenWidth / 2) - (164 / 2), displayHeight - 36, 40, BLACK);                   // PAUSED's height is 28
                    DrawText("pause again to continue", (screenWidth / 2) - (242 / 2), displayHeight + 26, 20, BLACK);  // this text's height is 18
                    // it's 36 and 34 here to add 36 pixels of spacing between the two texts.. does this make sense? please tell me it does.
                } break;
                default: break;
            }
            EndDrawing();

         //----------------------------------------------------------------------------------

    }

    // Unload stuff
    UnloadTexture(splash_logo);
    UnloadTexture(dotty_base);
    UnloadTexture(dotty_front);
    UnloadTexture(dotty_clone);
    UnloadTexture(dotty_down);
    UnloadTexture(dotty_up);
    UnloadTexture(dotty_right);
    UnloadTexture(dotty_left);
    UnloadTexture(dot);
    UnloadTexture(potion_dot);
    UnloadTexture(potion_dupe);
    UnloadTexture(potion_doom);
    UnloadTexture(oopsies_screenedge);
    UnloadTexture(oopsies_clonecollision);
    UnloadTexture(oopsies_what);
    UnloadTexture(oopsies_doom);
    UnloadSound(ouchie);
    UnloadSound(obtained);
    UnloadSound(potion_sound);
    UnloadSound(clone_ouchie);
    // ... is this going to cause a memory leak?
    

    CloseWindow();

    return 0;
}