// C includes
#include <raylib.h>
#include <stdlib.h>
#include <unistd.h>

// Game-specific includes and stuff
// TODO: not do this ../ nonsense?
#include "../include/player.h"  // That works.
#include "../include/dot.h"
#include "../include/potion.h"
#include "../include/graphics-logic.h"


// Setting variables for game logic
Player dotty{50, 50, 96, 96};      // The main player.
Player dupedotty{50, 50, 96, 96};  // The second player (or the clone)

float velX;  // By how much the player should move horizontally
float velY;  // Ditto, vertically
float vel;   // Base velocity speed, will then be applied to the above.
             // TODO: this could be optimized better. use boolean to determine if X or Y.
float multiplier;  // by how much do we multiply/divide the velocity speed

bool left;   // are we going left?
bool right;  // are we going right?
bool up;     // are we.. you get the idea
bool down;   // TODO: all of this could be an enum; the game allows only one direction.

// bool old_left;  // fuck are you doing???
// bool old_right;
// bool old_up;
// bool old_down;

bool has_moved = false;  // has the player moved more than once? used for the reminder on controls
bool nightmare = false;  // enable this to experience pain
int hasnt_moved_counter = 0;  // how many times has the player not moved? used to taunt the user in game over screen.

bool has_saved = false;  // did we already save? if so, don't save again every frame.

int screenWidth  = platformScreenWidth;   // defined by graphics-logic.h
int screenHeight = platformScreenHeight;

Dot dots[8] = {};    // array of dots. TODO: can we make this unlimited?
int dot_amount = 1;  // how many actual dots are in the game?
Potion potion;       // the potion used to increase the amount of dots.
Potion dupepotion;   // ditto; Dotty clone.
Potion doompotion;   // ditto; death potion.
bool dupe = false;   // is clone Dotty active?

unsigned int highscoreInt;  // our highscore, as an int (unsigned = cannot go below 0)
unsigned int eaten = 0;     // our current score, how many dots have been eaten?

typedef enum Actions { NOTHING, DOT_OBTAINED, DUPE_DOTS, DUPE_DOTTY, DUPE_LOST, OUCHIE } Actions;
// enum of possible.. not "collisions" but "things that can happen by bumping into them" (so actions)
Actions collision_type = NOTHING;  // current action that is happening
Failures failure;                  // current failure, or reason for game over

// Actions old_collision_type;  // unbelievably stupid fucking idiot, me! why the fuck would you do this?!

bool pressed_pause_to_continue = false;  // this prevents a bug where the user presses Space to start the game
                                         // and immediately pauses the game.
//--------------------------------------------------------------------------------------

float get_velocity(bool isVertical = false) {
    // by how much should the protagonist move, relative to the frame time?

    // return (
    //     (
    //         (((float)screenWidth / 265) + ((float)screenHeight / 150)) / 2
    //     ) + ((eaten * 2) * multiplier)
    //     );

    int baseSpeed = 3;                                  // Dotty's base speed at its base resolution.
    int points = baseSpeed + (eaten * 2 * multiplier);  // The base speed + eaten dots
    float targetDelta = 0.016667f;                      // 60fps time delta

    // adjust velocity based on frame time
    float adjustedSpeed = points * (GetFrameTime() / targetDelta);
    // Adjusted Speed = Base Speed * (Current Delta / Target Delta)

    return adjustedSpeed;
};

void reset_dots(const int screenWidth, const int screenHeight){
    for (int i = 0; i < 8; i++) dots[i].remove();
    for (int i = 0; i < dot_amount; i++) dots[i].updatePosition(screenWidth, screenHeight);
}

void reset_dotty(const int screenWidth, const int screenHeight){
    // reinitialize everything
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
    highscoreInt = get_highscore();

    reset_dots(screenWidth, screenHeight);
    potion.remove();
    potion.setAvailable(true);
    dupepotion.remove();
    dupepotion.setAvailable(true);
    doompotion.remove();
};

void fix_stuff_outside_window(const int screenWidth, const int screenHeight, const int deadzone = 40){
    // TODO: this is probably broken
    for (int i = 0; i < dot_amount; i++) if(dots[i].getX() + deadzone > screenWidth or dots[i].getY() + deadzone > screenHeight) dots[i].updatePosition(screenWidth, screenHeight);
    if((potion.getX() + deadzone > screenWidth) or
       (potion.getY() + deadzone > screenHeight))
        potion.randomize_position(screenWidth, screenHeight);
    if((dupepotion.getX() + deadzone > screenWidth) or
       (dupepotion.getY() + deadzone > screenHeight))
        dupepotion.randomize_position(screenWidth, screenHeight);
    if((doompotion.getX() + deadzone > screenWidth) or
       (doompotion.getY() + deadzone > screenHeight))
        doompotion.randomize_position(screenWidth, screenHeight);
};

bool check_collision_line(double point1, double point2){
    // TODO: make this use vectors or tuples?
    //       by the way, take methamphetamine and fix this code.

    // point1 = point1.getX()
    // point2 = point2.getX()

    // point1 = dotty's X or Y position
    // point2 = object's X/Y pos

    // return (
    //     point1 > point2 && // we are within the object from its left; we are to its right
    //     point1 < point2    // however we are not outside of its boundaries to the right
    // );

    return (point1 + 48 > point2 && point1 < point2 + 48);
};

bool check_collision_2d(double point1x, double point1y, double point2x, double point2y){
    // point1 = dotty
    // point2 = dot

    return (check_collision_line(point1x, point2x) && check_collision_line(point1y, point2y));
};

bool check_dotty_collision(double x, double y){
    // Checks if either the main Dotty or clone Dotty collides against an object using specified coordinates.
    return (check_collision_2d(dotty.getX(), dotty.getY(), x, y)       // Main Dotty collided with something
    or check_collision_2d(dupedotty.getX(), dupedotty.getY(), x, y));  // Dupe Dotty collided
};

bool check_player_window_collision(){
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
            // bonk. hehe
        }
    }
}
//--------------------------------------------------------------------------------------


int main(void)
{
    // initialize the game
    // this function will set up the window, the game loop, and the game state
    // it will also set up the game's variables and functions
    // it will also set up the game's objects and functions
    // it will also set up the game's sounds and music
    initialize_game();

    float * spentTime = new float;  // used to show title screen after 2 seconds

    GameScreen currentScreen = SPLASH;

    reset_dotty(screenWidth, screenHeight);

    while (!should_exit_game())  // run until user presses either Xbox/PS/Home/Steam button on controller or closes window
                                 // check the actual function for more info on "what does it do and how does it do"
    {
        int baseWidth = 800;
        int baseHeight = 450;
        screenWidth =  get_screen_width();
        screenHeight = get_screen_height();

        float horizontalScalingFactor = float(screenWidth) / baseWidth;
        float verticalScalingFactor = float(screenHeight) / baseHeight;

        vel = get_velocity();
        if(nightmare) vel *= 10;

        switch(currentScreen)
        {
            case SPLASH:
            {
                *spentTime += GetFrameTime();  // this doesn't count the time spent drawing something, does it?

                if (*spentTime > 1.5f or press_start()){
                    currentScreen = TITLE;
                    delete spentTime;
                }
            } break;
            case TITLE:
            {
                if (press_start()){
                    currentScreen = GAMEPLAY;
                    if(do_pause()) pressed_pause_to_continue = true;
                    // Whoops. Not only does it do it on the pause menu but also on the title screen.
                }
            } break;
            case GAMEPLAY:
            {
                TraceLog(LOG_DEBUG, TextFormat("Player velocity currently is %f", get_velocity()));
                TraceLog(LOG_DEBUG, TextFormat("FPS is at %d", GetFPS()));

                failure = NONE_YET;

                left = false; right = false; up = false; down = false;
                // this part is necessary otherwise movement will be janky and unresponsive.

                // add control stuff
                Directions movement = get_movement();
                switch (movement){
                    case LEFT:  left = true;
                    case RIGHT: right = true;
                    case UP:    up = true;
                    case DOWN:  down = true;
                    default: break;
                }

                fix_stuff_outside_window(get_screen_width(), get_screen_height());

                if (start_pause_menu()){
                    currentScreen = PAUSE;
                    break;
                }

                if(is_window_resized()){
                    // is the window or play field being resized?
                    if(check_player_window_collision()){  // is the field being resized AND have we collided?
                        dotty.setX(50);
                        dotty.setY(50);
                        left = false;
                        right = true;
                    }
                }

                if (movement != NONE){
                    velX = 0;
                    velY = 0;
                    has_moved = true;
                }

                // applying velocity integers
                if      (left  or velX < 0) velX = vel * -1;
                else if (right or velX > 0) velX = vel;
                else if (up    or velY < 0) velY = vel * -1;
                else if (down  or velX > 0) velY = vel;

                // check if player is colliding with the window borders
                if (check_player_window_collision()){
                    collision_type = OUCHIE;
                    failure = SCREEN_EDGE;
                }

                // applying movements with scaling factors in mind
                dotty.setX(dotty.getX() + (velX * horizontalScalingFactor));
                dotty.setY(dotty.getY() + (velY * verticalScalingFactor));
                if (dupe){
                    dupedotty.setX(screenWidth  - (dotty.getX() + 64));
                    dupedotty.setY(screenHeight - (dotty.getY() + 64));
                }

                // player collision
                checkPlayerCollision(screenWidth, screenHeight);
                if (collision_type != NOTHING){
                    switch (collision_type){
                        case DOT_OBTAINED: play_sound(OBTAINED); break;
                        case DUPE_LOST:    play_sound(CLONE_OUCHIE); break;
                        case DUPE_DOTS:    play_sound(POTION_OBTAINED); break;
                        case DUPE_DOTTY:   play_sound(POTION_OBTAINED); break;
                        case OUCHIE: {
                            // game over
                            play_sound(OWIE);
                            currentScreen = GAMEOVER;
                            has_saved = false;
                            break;
                        }
                        default: break;
                    }
                    collision_type = NOTHING;
                }

                // potion randomizer stuff
                if (rand() % 2000 == 1){
                    potion.update(screenWidth, screenHeight);
                }
                if (rand() % 2500 == 1){
                    dupepotion.update(screenWidth, screenHeight);
                }
                if (rand() % 500 == 1){
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
                }
                if (eaten > highscoreInt){
                    highscoreInt = eaten;
                    #if defined(PLATFORM_DESKTOP)
                        if(not has_saved){
                            set_highscore(highscoreInt);
                            has_saved = true;
                        };
                    #endif
                };

                if (press_start())
                {
                    if(not has_moved) hasnt_moved_counter+=1;
                    if(hasnt_moved_counter == 22) nightmare = true;
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

        frame_start();

        switch(currentScreen)
            {
                case SPLASH: draw_scene(SPLASH, screenWidth, screenHeight); break;
                case TITLE: draw_scene(TITLE, screenWidth, screenHeight); break;
                case GAMEPLAY:
                {
                    draw_scene(GAMEPLAY);

                    for (int i = 0; i < 8; i++) draw_sprite(DOT, dots[i].getX(), dots[i].getY());

                    draw_sprite(POTION_DOT,  potion.getX(),     potion.getY());
                    draw_sprite(POTION_DUPE, dupepotion.getX(), dupepotion.getY());
                    draw_sprite(POTION_DOOM, doompotion.getX(), doompotion.getY());

                    draw_sprite(DOTTY_BASE, dotty.getX(), dotty.getY());
                    if (velX > 0){
                        draw_sprite(DOTTY_RIGHT, dotty.getX(), dotty.getY());
                    }else if (velX < 0){
                        draw_sprite(DOTTY_LEFT, dotty.getX(), dotty.getY());
                    }else if (velY > 0){
                        draw_sprite(DOTTY_DOWN, dotty.getX(), dotty.getY());
                    }else if (velY < 0){
                        draw_sprite(DOTTY_UP, dotty.getX(), dotty.getY());
                    }else{
                        // couldn't determine in which position they were going
                        draw_sprite(DOTTY_FRONT, dotty.getX(), dotty.getY());
                    }

                    if(dupe){
                        draw_sprite(DOTTY_BASE, dupedotty.getX(), dupedotty.getY());
                        draw_sprite(DOTTY_CLONE, dupedotty.getX(), dupedotty.getY());
                        if (velX > 0){
                            draw_sprite(DOTTY_LEFT, dupedotty.getX(), dupedotty.getY());
                        }else if (velX < 0){
                            draw_sprite(DOTTY_RIGHT, dupedotty.getX(), dupedotty.getY());
                        }else if (velY > 0){
                            draw_sprite(DOTTY_UP, dupedotty.getX(), dupedotty.getY());
                        }else if (velY < 0){
                            draw_sprite(DOTTY_DOWN, dupedotty.getX(), dupedotty.getY());
                        }else{
                            // couldn't determine in which position they were going
                            draw_sprite(DOTTY_FRONT, dupedotty.getX(), dupedotty.getY());
                        }
                    }

                    draw_score(eaten, highscoreInt);
                } break;
                case GAMEOVER:
                {
                    draw_scene(GAMEOVER, screenWidth, screenHeight);

                    const char* gameover_message = "What happened? How did you die?";
                    switch (failure){
                        case SCREEN_EDGE:     gameover_message = "You hit the window borders."; break;
                        case CLONE_COLLISION: gameover_message = "You entered in collision with your clone."; break;
                        case DOOM_POTION:     gameover_message = "You drank the death potion."; break;
                        default: break;
                    }
                    // if (((get_screen_height() - dotty.getY()) > 64 and (dotty.getY() > 0) and vertically_collided) and failure == SCREEN_EDGE) gameover_message = "You really shouldn't play around with the window like that.";
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
                                case 21: {
                                    gameover_message = "";
                                } break;
                                case 22: gameover_message = "â”¬â”€â”¬ ãƒŽ( ã‚œ-ã‚œãƒŽ)"; break;  // what did this come from?
                                default: break;
                            }
                        }
                    }
                    // if (not has_moved) gameover_message = "lmao fat fuck";
                    draw_gameover(failure, gameover_message, screenWidth, screenHeight);
                } break;
                case PAUSE:
                {
                    draw_scene(PAUSE, screenWidth, screenHeight);
                } break;
                default: break;
            }
            frame_end();

         //----------------------------------------------------------------------------------

    }

    close_game();

    return 0;
}