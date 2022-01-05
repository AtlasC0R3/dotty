#ifndef GRAPHICS_LOGIC
#define GRAPHICS_LOGIC

void initialize_game(void);
void close_game(void);

int get_highscore(void);
void set_highscore(int highscore);

extern int platformScreenWidth;
extern int platformScreenHeight;

enum Directions{ NONE, LEFT, RIGHT, UP, DOWN };
enum Sprite{ DOTTY_BASE, DOT, POTION_DOT, POTION_DUPE, POTION_DOOM, DOTTY_RIGHT, DOTTY_LEFT, DOTTY_UP, DOTTY_DOWN, DOTTY_FRONT, DOTTY_CLONE };
enum Sounds{ OBTAINED, CLONE_OUCHIE, POTION_OBTAINED, OWIE };
typedef enum Failures { ALIVE, SCREEN_EDGE, CLONE_COLLISION, DOOM_POTION } Failures;  // no, being alive is not a failure. you're amazing alive. keep being you.
typedef enum GameScreen { SPLASH, TITLE, GAMEPLAY, PAUSE, GAMEOVER } GameScreen;
Directions get_movement(void);

bool exit_game(void);
bool press_start(void);
bool do_pause(void);
bool start_pause_menu(void);
bool is_window_resized(void);

const int get_screen_width(void);
const int get_screen_height(void);

void frame_start(void);
void frame_end(void);

void draw_scene(GameScreen scene, const int screenWidth = platformScreenWidth, const int screenHeight = platformScreenHeight);
void draw_gameover(Failures failure, const char* gameover_message, const int screenWidth, const int screenHeight);

void draw_score(int score, int highscore);

void draw_sprite(Sprite sprite, const int x, const int y);
void play_sound(Sounds sound);

void crash_game(void);

#endif