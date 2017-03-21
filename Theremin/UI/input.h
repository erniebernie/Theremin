/* input(.h/.cpp) - Ernie Bodle - 2016/01/16
* This controls the functions for user input
* This also holds the struct that contains data from user input
* This is derived from my input class that I made a while ago
*/

/* Side note:
 * Key down or anything that says once is basically
 * saying "rising edge" (or falling edge, idk how
 * these keyboards work)
 *
 */

#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

struct input{
	SDL_Event event;
	
	bool mouse_r, mouse_l;
	bool prev_mr, prev_ml;
	bool mouse_r_down, mouse_l_down;
	int mouse_x, mouse_y;

	bool keydown[256];
	bool keystate[256];
	bool prevkeystate[256];

	int quit;
};

void setup_input(input *in);
//void clean_input(input *in); //do I even need this?

void poll(input *in); //This is important. Use this one in main

//these were in public
bool get_keystateInt(input *in, int i);
bool get_keystateChar(input *in, char c);
bool get_keydown(input *in, int i);
bool get_keydown(input *in, char c);
//bool get_mouse(right or left, all or down);

//these were in private
void key_down_check(input *in);
void mouse_check(input *in);
void setstate(input *in, SDL_Keycode key, bool state);
bool isPressed(input *in, SDL_Keycode key);


#endif


