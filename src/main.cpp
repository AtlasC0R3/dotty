// C includes
#include <stdio.h>
#include <stdlib.h>

// Game-specific includes and stuff
#include "../include/player.h"  // That works.
#include "../include/dot.h"
#include "../include/potion.h"
#include "../include/graphics-logic.h"

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

int screenWidth  = platformScreenWidth;
int screenHeight = platformScreenHeight;
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
Actions collision_type = NOTHING;
Actions old_collision_type;
Failures failure;
double multiplier;

bool pauseButtonAlreadyPressed;

bool pressed_pause_to_continue = false;  // often times, people would press the Space key to play again, after a "game over" screen
                                         // activating the pause menu from within the game
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
    highscoreInt = get_highscore();

    reset_dots(screenWidth, screenHeight);
    potion.remove();
    potion.setAvailable(true);
    dupepotion.remove();
    dupepotion.setAvailable(true);
    doompotion.remove();
};

void fix_stuff_outside_window(const int screenWidth, const int screenHeight, const int deadzone = 40){
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
//--------------------------------------------------------------------------------------


int main(void)
{
    initialize_game();

    int framesCounter = 0;  // used to show title screen after 2 seconds

    GameScreen currentScreen = SPLASH;

    reset_dotty(screenWidth, screenHeight);

    while (!exit_game())  // run until user presses either Xbox/PS/Home/Steam button on controller or closes window
                          // check the actual function for more info on "what does it do and how does it do"
    {
        screenWidth =  get_screen_width();
        screenHeight = get_screen_height();

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
                    if(do_pause()) pressed_pause_to_continue = true;
                    // Whoops. Not only does it do it on the pause menu but also on the title screen.
                }
            } break;
            case GAMEPLAY:
            {
                sprintf(highscore, "BEST: %d", highscoreInt);

                failure = ALIVE;

                left = false; right = false; up = false; down = false;
                // yes this part is necessary otherwise movement will be janky and unresponsive.

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
                    if(check_player_window_collision()){
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
                }

                // potion randomizer stuff
                if (rand() % 2000 == 1){
                    potion.update(screenWidth, screenHeight);
                    if(debug_prints) play_sound(CLONE_OUCHIE);
                }
                if (rand() % 2500 == 1){
                    dupepotion.update(screenWidth, screenHeight);
                    if(debug_prints) play_sound(CLONE_OUCHIE);
                }
                if (rand() % 500 == 1){
                    if(debug_prints) play_sound(CLONE_OUCHIE);
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
                            set_highscore(highscoreInt);
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
                    if (((get_screen_height() - dotty.getY()) > 64 and (dotty.getY() > 0) and vertically_collided) and failure == SCREEN_EDGE) gameover_message = "You really shouldn't play around with the window like that.";
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
                                            set_highscore(1);
                                        #endif
                                        crash_game();
                                    }
                                } break;
                                case 22: gameover_message = "â”¬â”€â”¬ ãƒŽ( ã‚œ-ã‚œãƒŽ)"; break;
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