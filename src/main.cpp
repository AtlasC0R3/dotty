// C includes
#include "raylib.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// Game-specific includes and stuff
#include "../include/player.h"  // That works.
#include "../include/dot.h"
#include "../include/potion.h"

#include <unistd.h>

bool debug_prints = true;
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
Dot dots[8] = {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}};
Potion potion = 0;
Potion dupepotion = 1;
Potion doompotion = 2;
bool dupe = false;
int highscoreInt = LoadStorageValue(0);
char highscore[32];

int dot_amount = 1;
int eaten = 0;
typedef enum Actions { NOTHING, DOT_OBTAINED, DUPE_DOTS, DUPE_DOTTY, DUPE_LOST, OUCHIE } Actions;
typedef enum Failures { ALIVE, SCREEN_EDGE, CLONE_COLLISION, DOOM_POTION } Failures;  // no, being alive is not a failure. you're amazing alive. keep being you.
Actions collision_type = NOTHING;
Actions old_collision_type;
Failures failure;
double multiplier = 0.1;

int currentGesture = GESTURE_NONE;
int lastGesture = GESTURE_NONE;
bool gesture_movement = false;
typedef enum GameScreen { SPLASH, TITLE, GAMEPLAY, PAUSE, GAMEOVER } GameScreen;
bool pauseButtonAlreadyPressed;
bool disableEscapeQuit = false;  // set it to "true" and it'll pause the game when the user presses "escape"
//--------------------------------------------------------------------------------------

void reset_dots(void){
    for (int i = 0; i < 8; i++) dots[i].remove();
    for (int i = 0; i < dot_amount; i++) dots[i].updatePosition();
}

void reset_dotty(void){
    dotty = {50, 50, 96, 96};
    dupedotty = {-5000, -5000, 96, 96};
    vel = 3;
    velX = 3;
    velY = 0;
    eaten = 0;
    dupe = false;
    dot_amount = 1;
    highscoreInt = LoadStorageValue(0);

    reset_dots();
    potion.remove();
    potion.setAvailable(true);
    dupepotion.remove();
    dupepotion.setAvailable(true);
    doompotion.remove();
};

void checkPlayerCollision()
{
    for (int i = 0; i < 8; i++)
    {
        if (((dotty.getX() + 48 > dots[i].getX() && dotty.getX() < dots[i].getX() + 48) && (dotty.getY() + 48 > dots[i].getY() && dotty.getY() < dots[i].getY() + 48)) or
            (dupe and ((dupedotty.getX() + 48 > dots[i].getX() && dupedotty.getX() < dots[i].getX() + 48) && (dupedotty.getY() + 48 > dots[i].getY() && dupedotty.getY() < dots[i].getY() + 48))))
            // welcome: unreadable code that will take 2 minutes to decode for a human being!
        {
            // dot obtained
            dots[i].updatePosition();
            vel+=multiplier;
            if (velX > 0) velX = vel;
            if (velX < 0) velX = vel * -1;
            if (velY > 0) velY = vel;
            if (velY < 0) velY = vel * -1;
            eaten+=1;
            collision_type = DOT_OBTAINED;
        }
    }
    if ((dotty.getRelativeX() > potion.getX() && dotty.getRelativeX() < potion.getX() + 64 && dotty.getRelativeY() > potion.getY() and dotty.getRelativeY() < potion.getY() + 64) or 
       (dupe and (dupedotty.getRelativeX() > potion.getX() && dupedotty.getRelativeX() < potion.getX() + 64 && dupedotty.getRelativeY() > potion.getY() && dupedotty.getRelativeY() < potion.getY() + 64))){
        // dupe dot potion obtained
        collision_type = DUPE_DOTS;
        if (not (dot_amount == 8)){
            dot_amount+=1;
            dots[dot_amount].updatePosition();
        } else{
            potion.setAvailable(false);
        }
        potion.remove();
    }if (dotty.getRelativeX() > dupepotion.getX() && dotty.getRelativeX() < dupepotion.getX() + 64 && dotty.getRelativeY() > dupepotion.getY() && dotty.getRelativeY() < dupepotion.getY() + 64){
        // dupe dotty potion obtained
        collision_type = DUPE_DOTTY;
        dupepotion.setAvailable(false);
        dupepotion.remove();
        dupe = true;
    }
    if (dotty.getRelativeX() > doompotion.getX() && dotty.getRelativeX() < doompotion.getX() + 64 && dotty.getRelativeY() > doompotion.getY() && dotty.getRelativeY() < doompotion.getY() + 64){
        // main dotty obtained doom potion
        collision_type = OUCHIE;
        failure = DOOM_POTION;
    }
    if (dupe and (dupedotty.getRelativeX() > doompotion.getX() && dupedotty.getRelativeX() < doompotion.getX() + 64 && dupedotty.getRelativeY() > doompotion.getY() && dupedotty.getRelativeY() < doompotion.getY() + 64)){
        // dupe dotty obtained doom potion
        collision_type = DUPE_LOST;
        dupe = false;
        dupepotion.setAvailable(true);
        doompotion.remove();
    }
    if (dupe){
        // dotty.getY() > dupedotty.getY() and dotty.getY() < dupedotty.getY() + 48
        if ((dotty.getX() > dupedotty.getX() - 48 && dotty.getX() < dupedotty.getX() + 48) and
            (dotty.getY() > dupedotty.getY() - 48 && dotty.getY() < dupedotty.getY() + 48)){
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
            if (IsGamepadButtonPressed(a, 14) and IsWindowFocused()){
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
    if (pauseButtonAlreadyPressed and pause){
        pause = false;
    }else{
        pauseButtonAlreadyPressed = pause;
    }
    return (pause);
}
//--------------------------------------------------------------------------------------


int main(void)
{
    // TODO: make a splash screen saying who made the game (me)

    if (!opendir("resources")) {
        printf("Could not find the resources/ folder. This will render the game unplayable.\n");
        exit(1);
    }
    
    if (extreme_debug_prints) debug_prints = true;


    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Dotty");

    SetWindowIcon(LoadImage("resources/images/logo/icon.ico"));

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
    //   Texture2D dotty_sleep = LoadTexture("resources/images/dotty/sleep.png");
    //--------------------------------------------------------------------------------------

    GameScreen currentScreen = SPLASH;

    reset_dotty();

    SetTargetFPS(60);

    while (!cont_exit())  // run until user presses either Xbox/PS/Home/Steam button on controller or closes window
                          // check the actual function for more info on "what does it do and how does it do"
    {
        if (disableEscapeQuit){
            SetExitKey(-420); // i could've just put 0 but, no.
        }
        switch(currentScreen)
        {
            case SPLASH: 
            {
                currentScreen = TITLE; break;
                if (press_start())
                {
                    currentScreen = GAMEPLAY;
                }
            } break;
            case TITLE: 
            {
                if (press_start())
                {
                    currentScreen = GAMEPLAY;
                }
            } break;
            case GAMEPLAY:
            {
                sprintf(highscore, "BEST: %d", highscoreInt);

                failure = ALIVE;
                gesture_movement = false;
                // add control stuff
                left = check_left();
                right = check_right();
                up = check_up();
                down = check_down();
        
                lastGesture = currentGesture;
                currentGesture = GetGestureDetected();

                if (do_pause() or !IsWindowFocused()){
                    currentScreen = PAUSE;
                    break;
                }

                if (currentGesture >= 16 and currentGesture <= 128) gesture_movement = true;
        
                if (left or right or up or down or gesture_movement){
                    velX = 0;
                    velY = 0;
                }
        
                if (gesture_movement){
                    switch (currentGesture){
                        case GESTURE_SWIPE_RIGHT: right = true; break;
                        case GESTURE_SWIPE_LEFT: left = true; break;
                        case GESTURE_SWIPE_UP: up = true; break;
                        case GESTURE_SWIPE_DOWN: down = true; break;
                        default: break;
                    }
                }
        
                
                     if (left)  velX = vel * -1;
                else if (right) velX = vel;
                else if (up)    velY = vel * -1;
                else if (down)  velY = vel;
        
                if (((dotty.getX() + 64 >= screenWidth)  or (dotty.getX() <= 0)) or 
                    ((dotty.getY() + 64 >= screenHeight) or (dotty.getY() <= 0))){
                    collision_type = OUCHIE;
                    failure = SCREEN_EDGE;
                }
                    
                dotty.setX(dotty.getX() + velX);
                dotty.setY(dotty.getY() + velY);
                if (dupe){
                    dupedotty.setX(screenWidth  - (dotty.getX() + 64));
                    dupedotty.setY(screenHeight - (dotty.getY() + 64));
                }
        
                checkPlayerCollision();
                old_collision_type = collision_type;
                collision_type = NOTHING;
                if (old_collision_type != NOTHING){
                    switch (old_collision_type){
                        case DOT_OBTAINED: PlaySound(obtained); break;
                        case DUPE_LOST: PlaySound(clone_ouchie); break;
                        case DUPE_DOTS: PlaySound(potion_sound); break;
                        case DUPE_DOTTY: PlaySound(potion_sound); break;
                        case OUCHIE: {
                            // game over
                            PlaySound(ouchie);
                            currentScreen = GAMEOVER;
                            break;
                        }
                        default: break;
                    }
                }
                
                if ((rand() % 2000 == 1) and (potion.getX() < 0)){
                    potion.update();
                }
                if ((rand() % 2500 == 1) and (dupepotion.getX() < 0)){
                    dupepotion.update();
                }
                if (rand() % 500 == 1){
                    if (doompotion.getX() < 0){
                        // not already in screen, we have to add it
                        doompotion.update();
                    } else{
                        // we have to remove it
                        doompotion.remove();
                    }
                }
            } break;
            case GAMEOVER: 
            {
                if (eaten > highscoreInt) highscoreInt = eaten;

                SaveStorageValue(0, highscoreInt);  // stores highscore integer in storage.data in an Int32 format

                if (press_start())
                {
                    reset_dotty();
                    currentScreen = GAMEPLAY;
                }  
            } break;
            case PAUSE: 
            {
                if (do_pause())
                {
                    currentScreen = GAMEPLAY;
                }  
            } break;
            default: break;
        }

        BeginDrawing();

        switch(currentScreen) 
            {
                case TITLE: 
                {
                    DrawRectangle(0, 0, screenWidth, screenHeight, GRAY);
                    DrawTexture(splash_logo, (screenWidth / 2) - splash_logo.width / 2, (screenHeight / 2) - splash_logo.height * 0.75, GRAY);
                    DrawText("press any key", 290, screenHeight * 0.75, 30, BLACK);
                    
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
                    
                    char eaten_cchar[100000];
                    sprintf(eaten_cchar, "%d", eaten);
                    DrawText(eaten_cchar, 20, GetScreenHeight() - 80, 20, DARKGRAY);
                    DrawText(highscore,   20, GetScreenHeight() - 40, 20, DARKGRAY);

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
                    DrawText(gameover_message, 20, 80, 20, WHITE);
                    DrawTexture(oopsie_texture, screenWidth - 512, screenHeight - 256, DARKGRAY);
                    DrawText("press any key to continue", 20, screenHeight - 40, 20, WHITE);
                } break;
                case PAUSE: 
                {
                    // DrawRectangle(0, 0, screenWidth, screenHeight, GRAY);
                    ClearBackground(LIGHTGRAY);
                    DrawText("PAUSED", (screenWidth / 2) - 75, (screenHeight / 2.75), 40, BLACK);
                    DrawText("pause again to continue", 282, (screenHeight / 2), 20, BLACK);
                } break;
                default: break;
            }

        EndDrawing();

         //----------------------------------------------------------------------------------

    }

    CloseWindow();

    return 0;
}