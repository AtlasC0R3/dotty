// C includes I think
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

// Game-specific includes and sh- stuff
#include "../include/player.h"  // That works.
#include "../include/dot.h"
#include "../include/potion.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

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
bool dupe = false;

int dot_amount = 1;
int eaten = 0;
int collision_type = 0;
double multiplier = 0.1;
//--------------------------------------------------------------------------------------

void reset_dots(void){
    for (int i = 0; i < 8; i++) dots[i].remove();
    for (int i = 0; i < dot_amount; i++) dots[i].updatePosition();
}

void reset_dotty(void){
    dotty = {50, 50, 96, 96};
    vel = 3;
    velX = 3;
    velY = 0;
    eaten = 0;
    dupe = false;
    dot_amount = 1;

    reset_dots();
    potion.remove();
    potion.setAvailable(true);
    dupepotion.remove();
    dupepotion.setAvailable(true);
};

void checkPlayerCollision()
{
    for (int i = 0; i < 8; i++)
    {
        if ((dotty.getRelativeX() > dots[i].getX() && dotty.getRelativeX() < dots[i].getX() + 64 && dotty.getRelativeY() > dots[i].getY() and dotty.getRelativeY() < dots[i].getY() + 64) or
            (dupe and (dupedotty.getRelativeX() > dots[i].getX() && dupedotty.getRelativeX() < dots[i].getX() + 64 && dupedotty.getRelativeY() > dots[i].getY() && dupedotty.getRelativeY() < dots[i].getY() + 64)))
        {
            // dot obtained
            dots[i].updatePosition();
            vel+=multiplier;
            if (velX > 0) velX = vel;       // welcome: unreadable code that will take 30 seconds to decode for a human being!
            if (velX < 0) velX = vel * -1;
            if (velY > 0) velY = vel;
            if (velY < 0) velY = vel * -1;
            eaten+=1;
            collision_type = 1;
        }
    }
    if ((dotty.getRelativeX() > potion.getX() && dotty.getRelativeX() < potion.getX() + 64 && dotty.getRelativeY() > potion.getY() and dotty.getRelativeY() < potion.getY() + 64) or 
       (dupe and (dupedotty.getRelativeX() > potion.getX() && dupedotty.getRelativeX() < potion.getX() + 64 && dupedotty.getRelativeY() > potion.getY() && dupedotty.getRelativeY() < potion.getY() + 64))){
        collision_type = 2;
        if (not (dot_amount == 8)){
            dot_amount+=1;
            dots[dot_amount].updatePosition();
        } else{
            potion.setAvailable(false);
        }
        potion.remove();
    }if (dotty.getRelativeX() > dupepotion.getX() && dotty.getRelativeX() < dupepotion.getX() + 64 && dotty.getRelativeY() > dupepotion.getY() && dotty.getRelativeY() < dupepotion.getY() + 64){
        collision_type = 2;
        dupepotion.setAvailable(false);
        dupepotion.remove();
        dupe = true;
    }
    if (dupe){
        if (dotty.getRelativeX() > dupedotty.getX() && dotty.getX() < dupedotty.getRelativeX() + 32 && dotty.getY() > dupedotty.getY() && dotty.getRelativeY() < dupedotty.getY() + 64){
            reset_dotty();
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
//--------------------------------------------------------------------------------------

bool cont_exit(void){
    bool exit = false;
    int a;
    if(WindowShouldClose()) return true;
    for( a = -1; a < 9; a = a + 1 ){
        if (IsGamepadAvailable(a)){
            // there's a gamepad!
            if (IsGamepadButtonPressed(a, 14)){
                exit = true;
                if (debug_prints) printf("Home button pressed on controller name %s: exiting...\n", GetGamepadName(a));
            }
        }
    }
    return exit;
}


int main(void)
{
    if (extreme_debug_prints) debug_prints = true;

    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Dotty");

    // Initialize audio
      InitAudioDevice();
      Sound ouchie   = LoadSound("resources/sounds/ouchie.wav");
      Sound obtained = LoadSound("resources/sounds/obtained.wav");
      Sound potion_sound = LoadSound("resources/sounds/potion.wav");

    // Initialize textures.
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    // NOTE: Otherwise you'll get a "Segmentation fault" error.

    // Basic textures
      Texture2D dotty_base = LoadTexture("resources/images/dotty/base.png");
      Texture2D dotty_front = LoadTexture("resources/images/dotty/front.png");
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
    // Likely unused textures
    //   Texture2D dotty_blink = LoadTexture("resources/images/dotty/blink.png");
    //   Texture2D dotty_half = LoadTexture("resources/images/dotty/half-blink.png");
    //   Texture2D dotty_sleep = LoadTexture("resources/images/dotty/sleep.png");
    //--------------------------------------------------------------------------------------

    reset_dotty();

    SetTargetFPS(60);

    while (!cont_exit())  // run until user presses either Xbox/PS/Home/Steam button on controller, ESC key or closes window
                          // check the actual function for more info on "what does it do and how does it do"
    {
        // Draw
        //----------------------------------------------------------------------------------
        // add control stuff
        left = check_left();
        right = check_right();
        up = check_up();
        down = check_down();

        if (left or right or up or down){
            velX = 0;
            velY = 0;
        }

             if (left)  velX = vel * -1;
        else if (right) velX = vel;
        else if (up)    velY = vel * -1;
        else if (down)  velY = vel;

        if (((dotty.getX() + 64 >= screenWidth)  or (dotty.getX() <= 0)) or 
            ((dotty.getY() + 64 >= screenHeight) or (dotty.getY() <= 0))){
            reset_dotty(); // gaem over?!?!?!??!?!?!??!?!??!?!?!??!?!?! yes
            PlaySound(ouchie);
        }
            
        dotty.setX(dotty.getX() + velX);
        dotty.setY(dotty.getY() + velY);
        dupedotty.setX(screenWidth  - (dotty.getX() + 64));
        dupedotty.setY(screenHeight - (dotty.getY() + 64));

        checkPlayerCollision();
        if (collision_type != 0){
            if (collision_type == 1){
                PlaySound(obtained);
            }
            else if (collision_type == 2){
                PlaySound(potion_sound);
            }
            collision_type = 0;
        }

        BeginDrawing();
        
            ClearBackground(RAYWHITE);

            for (int i = 0; i < 8; i++){
                DrawTexture(dot, dots[i].getX(), dots[i].getY(), WHITE);
            }

            DrawTexture(potion_dot, potion.getX(), potion.getY(), WHITE);
            DrawTexture(potion_dupe, dupepotion.getX(), dupepotion.getY(), WHITE);

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

            if ((rand() % 2000 == 1) and (potion.getX() < 0)){
                potion.update();
            }
            if ((rand() % 2500 == 1) and (dupepotion.getX() < 0)){
                dupepotion.update();
            }
            
            char eaten_cchar[100000];
            sprintf(eaten_cchar, "%d", eaten);
            DrawText(eaten_cchar, 20, GetScreenHeight() - 40, 20, DARKGRAY);

        EndDrawing();

         //----------------------------------------------------------------------------------

    }

    CloseWindow();

    return 0;
}