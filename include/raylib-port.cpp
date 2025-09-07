#include "graphics-logic.h"
#include "raylib.h"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <stdio.h>
#ifdef PLATFORM_DESKTOP
#include <dirent.h>
#endif

// int currentGesture = GESTURE_NONE;
// int lastGesture = GESTURE_NONE;
// bool gesture_movement = false;

int platformScreenWidth = 800;
int platformScreenHeight = 450;

int currentGesture = GESTURE_NONE;
int lastGesture = GESTURE_NONE;
bool gesture_movement;

bool continuedUsingPause;
bool alreadyPressedPause;

const char* bottom_text;

// texture variables
Sound ouchie;
Sound obtained;
Sound potion_sound;
Sound clone_ouchie;
Texture2D splash_logo;
Texture2D dotty_base;
Texture2D dotty_front;
Texture2D dotty_clone;
Texture2D dotty_sleep;
Texture2D dotty_down;
Texture2D dotty_up;
Texture2D dotty_right;
Texture2D dotty_left;
Texture2D dot;
Texture2D potion_dot;
Texture2D potion_dupe;
Texture2D potion_doom;
Texture2D oopsies_screenedge;
Texture2D oopsies_clonecollision;
Texture2D oopsies_what;
Texture2D oopsies_doom;
// -------------------------------

// int get_highscore(){return LoadFileData(0);};
// void set_highscore(int highscore){
//     SaveStorageValue(0, highscore);  // stores highscore integer in storage.data in an Int32 format
// }

unsigned int get_highscore() {
    int dataSize = 0;
    unsigned char * fileData = LoadFileData(SAVE_FILE, &dataSize);

    if(fileData == NULL){
        set_highscore(0);        // create save file
        return get_highscore();  // reload said save file
    }

    return ((unsigned int *) fileData)[0];
}
void set_highscore(unsigned int highscore){
    SaveFileData(SAVE_FILE, &highscore, sizeof(int));
    return;
}

void initialize_game(){
    #if defined(PLATFORM_DESKTOP)
        if (!opendir("resources")) {
            printf("Could not find the resources/ folder. This will render the game unplayable.\n");
            InitWindow(600, 400, "My resources are unavailable! >:(");

            while(
                !(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) || GetKeyPressed() != KEY_NULL || WindowShouldClose())
                ){
                BeginDrawing();

                ClearBackground(RAYWHITE);
                DrawText("My \"resources\" folder is gone!", 8, 8, 32, BLACK);
                DrawText("FIX THIS!!!", 8, 48, 64, BLACK);
                DrawText("The game is broken otherwise!", 8, 128, 32, BLACK);
                DrawText("Re-download the game, or put it", 8, 160, 32, BLACK);
                DrawText("back where it belongs!", 8, 192, 32, BLACK);

                DrawRectangle(0, 280, 600, 200, BLACK);
                DrawText("Signed,", 32, 300, 24, WHITE);
                DrawText("Angry Faceless Dotty.", 32, 330, 48, WHITE);

                EndDrawing();
            }

            CloseWindow();
            exit(1);
            return;
        }
    #endif

    try {
        InitWindow(platformScreenWidth, platformScreenHeight, "Dotty");  // 70 megabytes of RAM
    } catch (...) {
        // FIXME: figure out how to catch a segfault with raylib's InitWindow
        printf(
            "\n\nHey, buddy! Do you even have a GUI? Raylib couldn't initialize the window! What is this, the 1980s? DOS??\nSigned, Confused Dotty."
        );
    }
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(platformScreenWidth, platformScreenHeight);

    // Initialize audio
      InitAudioDevice();  // 3 more megabytes of RAM
      ouchie       = LoadSound("resources/sounds/ouchie.ogg");
      obtained     = LoadSound("resources/sounds/obtained.ogg");
      potion_sound = LoadSound("resources/sounds/potion.ogg");
      clone_ouchie = LoadSound("resources/sounds/clone-ouchie.ogg");

    // Initialize textures.
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    // NOTE: Otherwise you'll get a "Segmentation fault" error.

    // Splash logo
      splash_logo = LoadTexture("resources/raylib/images/splash.png");
    // Basic textures
      dotty_base = LoadTexture("resources/raylib/images/dotty/base.png");
      dotty_front = LoadTexture("resources/raylib/images/dotty/front.png");
      dotty_clone = LoadTexture("resources/raylib/images/dotty/clone.png");
      dotty_sleep = LoadTexture("resources/raylib/images/dotty/sleep.png");
    // Up-down textures
      Image dotty_down_image = LoadImage("resources/raylib/images/dotty/down.png");
      Image *dotty_up_image = &dotty_down_image;
      dotty_down = LoadTextureFromImage(dotty_down_image);
      ImageFlipVertical(dotty_up_image);
      dotty_up = LoadTextureFromImage(dotty_down_image);
      UnloadImage(dotty_down_image);
    // Side textures
      Image dotty_side = LoadImage("resources/raylib/images/dotty/side.png");
      Image *dotty_side_flipped = &dotty_side;
      dotty_right = LoadTextureFromImage(dotty_side);
      ImageFlipHorizontal(dotty_side_flipped);
      dotty_left = LoadTextureFromImage(dotty_side);
      UnloadImage(dotty_side);
    // Power-up textures
      dot = LoadTexture("resources/raylib/images/items/dot.png");
    // Potion textures
      potion_dot = LoadTexture("resources/raylib/images/items/potion-dots.png");
      potion_dupe = LoadTexture("resources/raylib/images/items/potion-dupe.png");
      potion_doom = LoadTexture("resources/raylib/images/items/smallpotion-doom.png");
    // Power-up textures
      oopsies_screenedge = LoadTexture("resources/raylib/images/oopsies/screen-edge.png");
      oopsies_clonecollision = LoadTexture("resources/raylib/images/oopsies/clone-collision.png");
      oopsies_what = LoadTexture("resources/raylib/images/oopsies/what-just-happened.png");
      oopsies_doom = LoadTexture("resources/raylib/images/oopsies/doom-drank.png");
    // Likely unused textures
    //   dotty_blink = LoadTexture("resources/raylib/images/dotty/blink.png");
    //   dotty_half = LoadTexture("resources/raylib/images/dotty/half-blink.png");
    
    SetTargetFPS(60);
}

void close_game(){
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
}

// Controller/keyboard input mechanisms
bool check_left(void){
    bool go_left = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (IsGamepadButtonPressed(a, 4)){
                go_left = true;
            }
            else if (GetGamepadAxisMovement(a, 0) < -0.50){
                go_left = true;
            }
            
        }
    }
    if (IsKeyPressed(KEY_LEFT) or IsKeyPressed(KEY_A)){
        go_left = true;
    }
    return (go_left);
}

bool check_right(void){
    bool go_right = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (IsGamepadButtonPressed(a, 2)){
                go_right = true;
            }
            else if (GetGamepadAxisMovement(a, 0) > 0.50){
                go_right = true;
            }
        }
    }
    if (IsKeyPressed(KEY_RIGHT) or IsKeyPressed(KEY_D)){
        go_right = true;
    }
    return (go_right);
}

bool check_up(void){
    bool go_up = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (IsGamepadButtonPressed(a, 1)){
                go_up = true;
            }
            else if (GetGamepadAxisMovement(a, 1) < -0.50){
                go_up = true;
            }
        }
    }
    if (IsKeyPressed(KEY_UP) or IsKeyPressed(KEY_W)){
        go_up = true;
    }
    return (go_up);
}

bool check_down(void){
    bool go_down = false;
    int a;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (IsGamepadButtonPressed(a, 3)){
                go_down = true;
            }
            else if (GetGamepadAxisMovement(a, 1) > 0.50){
                go_down = true;
            }
        }
    }
    if (IsKeyPressed(KEY_DOWN) or IsKeyPressed(KEY_S)){
        go_down = true;
    }
    return (go_down);
}

Directions get_movement(void){
    if(check_left())  return LEFT;
    if(check_right()) return RIGHT;
    if(check_up())    return UP;
    if(check_down())  return DOWN;

    currentGesture = GetGestureDetected();
    if (currentGesture >= 16 and currentGesture <= 128) gesture_movement = true;

    if (gesture_movement){
        switch (currentGesture){
            case GESTURE_SWIPE_RIGHT: return RIGHT; break;
            case GESTURE_SWIPE_LEFT:  return LEFT; break;
            case GESTURE_SWIPE_UP:    return UP; break;
            case GESTURE_SWIPE_DOWN:  return DOWN; break;
            default: break;
        }
    }
    return NONE;
}

bool exit_game(void){
    bool exit = false;
    int a;
    if(WindowShouldClose()) return true;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if ((IsGamepadButtonPressed(a, 14) or (IsGamepadButtonDown(a, 13) and IsGamepadButtonDown(a, 15))) and IsWindowFocused()){
                exit = true;
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
            }
            if ((GetGamepadAxisMovement(a, 4) >= 0.5) && (GetGamepadAxisMovement(a, 5) >= 0.5)){
                pause = true;
            }
        }
    }
    if (GetGamepadButtonPressed() == 5 || GetGamepadButtonPressed() == 8){
        pause = true;
    }
    if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_SPACE) || IsGestureDetected(GESTURE_DOUBLETAP)){
        pause = true;
    }
    continuedUsingPause = false;
    if ((alreadyPressedPause and pause) or continuedUsingPause){
        pause = false;
    }else{
        alreadyPressedPause = pause;
    }
    return (pause);
}
bool start_pause_menu(){
    return(do_pause() || !IsWindowFocused());
}
bool is_window_resized(){
    return(IsWindowResized());
}

const int get_screen_width(){
    return GetScreenWidth();
}
const int get_screen_height(){
    return GetScreenHeight();
}

void frame_start(){
    BeginDrawing();

    // if(IsWindowResized()){
    //     if (screenWidth < 800)  SetWindowSize(800, GetScreenHeight());
    //     if (screenHeight < 450) SetWindowSize(GetScreenWidth(), 450);
    // }
    // SetWindowMinSize() is a thing. I'm dumb.
}

void draw_sprite(Sprite sprite, const int x, const int y){
    switch(sprite){
        case DOTTY_BASE:  DrawTexture(dotty_base,   x, y, WHITE); break;
        case DOT:         DrawTexture(dot,          x, y, WHITE); break;
        case POTION_DOT:  DrawTexture(potion_dot,   x, y, WHITE); break;
        case POTION_DUPE: DrawTexture(potion_dupe,  x, y, WHITE); break;
        case POTION_DOOM: DrawTexture(potion_doom,  x, y, WHITE); break;
        case DOTTY_RIGHT: DrawTexture(dotty_right,  x, y, WHITE); break;
        case DOTTY_LEFT:  DrawTexture(dotty_left,   x, y, WHITE); break;
        case DOTTY_UP:    DrawTexture(dotty_up,     x, y, WHITE); break;
        case DOTTY_DOWN:  DrawTexture(dotty_down,   x, y, WHITE); break;
        case DOTTY_FRONT: DrawTexture(dotty_front,  x, y, WHITE); break;  // hi :)
        case DOTTY_CLONE: DrawTexture(dotty_clone,  x, y, WHITE); break;
        default: break;
    }
}

void play_sound(Sounds sound){
    switch(sound){
        case OBTAINED:        PlaySound(obtained); break;
        case CLONE_OUCHIE:    PlaySound(clone_ouchie); break;
        case POTION_OBTAINED: PlaySound(potion_sound); break;
        case OWIE:            PlaySound(ouchie); break;
        default: break;
    }
}

void draw_scene(GameScreen scene, const int screenWidth, const int screenHeight){
    switch(scene){
        case SPLASH: {
            ClearBackground(DARKGRAY);
            DrawTexture(dotty_sleep, (screenWidth - dotty_sleep.width) / 2, (screenHeight  - dotty_sleep.height) / 2.25, WHITE);
            DrawText("made with raylib", 20, screenHeight - 40, 20, WHITE);
        }; break;
        case TITLE: {
            ClearBackground(GRAY);
            DrawTexture(splash_logo, (screenWidth / 2) - splash_logo.width / 2, (screenHeight / 2) - splash_logo.height * 0.75, GRAY);
            DrawText("press any key", (screenWidth / 2) - (219 / 2), screenHeight * 0.75, 30, BLACK);
        }; break;
        case GAMEPLAY: ClearBackground(RAYWHITE); break;
        case GAMEOVER: {
            ClearBackground(DARKGRAY);
            DrawText("GAME OVER", 20, 20, 40, WHITE);
        }; break;
        case PAUSE: {
            // DrawRectangle(0, 0, screenWidth, screenHeight, GRAY);
            ClearBackground(LIGHTGRAY);
            int displayHeight = screenHeight / 2;
            DrawText("PAUSED", (screenWidth / 2) - (164 / 2), displayHeight - 36, 40, BLACK);                   // PAUSED's height is 28
            DrawText("pause again to continue", (screenWidth / 2) - (242 / 2), displayHeight + 26, 20, BLACK);  // this text's height is 18
            // it's 36 and 34 here to add 36 pixels of spacing between the two texts.. does this make sense? please tell me it does.
        }; break;
        default: break;
    }
}

void draw_gameover(Failures failure, const char* gameover_message, const int screenWidth, const int screenHeight){
    Texture2D &oopsie_texture = oopsies_what;
    switch (failure){
        case SCREEN_EDGE:     oopsie_texture = oopsies_screenedge;     break;
        case CLONE_COLLISION: oopsie_texture = oopsies_clonecollision; break;
        case DOOM_POTION:     oopsie_texture = oopsies_doom;           break;
        default: break;
    }
    DrawText(gameover_message, 20, 80, 20, WHITE);
    DrawTexture(oopsie_texture, screenWidth - 512, screenHeight - 256, DARKGRAY);
    DrawText("press any key to continue", 20, screenHeight - 40, 20, WHITE);
}

void draw_score(int score, int highscore){
    char eaten_cchar[32];
    char bottom_text[32];
    sprintf(eaten_cchar, "%d", score);
    #if defined(PLATFORM_DESKTOP)
        sprintf(bottom_text, "BEST: %d", highscore);
        DrawText(eaten_cchar, 20, get_screen_height() - 80, 20, DARKGRAY);
    #else
        bottom_text = eaten_cchar;
    #endif
    DrawText(bottom_text, 20, get_screen_height() - 40, 20, DARKGRAY);
}

void frame_end(){
    EndDrawing();
}

void crash_game(){
    CloseWindow();
}
