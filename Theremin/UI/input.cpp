
#include "input.h"

void setup_input(input *in){
    for(int i = 0; i < 255; i++){
	in->keystate[i] = 0;
	in->keydown[i] = 0;
	in->prevkeystate[i] = 0;
    }

    in->mouse_r = in->mouse_l = false;
    in->prev_mr = in->prev_ml = false;
    in->mouse_r_down = in->mouse_l_down = false;
    in->quit = false;
}

void poll(input *in){
    while(SDL_PollEvent(&in->event)) {
	if (in->event.type == SDL_QUIT)
	    in->quit = true;
	if(in->event.type == SDL_KEYDOWN)
	    setstate(in, in->event.key.keysym.scancode, SDL_PRESSED);
	if(in->event.type == SDL_KEYUP)
	    setstate(in, in->event.key.keysym.scancode, SDL_RELEASED);
    }
    mouse_check(in);
    SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
    key_down_check(in);
}

//getters
bool get_keystateInt(input *in, int i){
    if(in->keystate[i] == SDL_PRESSED)
	return true;
    return false;
}

bool get_keystateChar(input *in, char c){
    int i = (int) c - 93;
    if(in->keystate[i] == SDL_PRESSED)
	return true;
    return false;
}

bool get_keydown(input *in, int i){
    if(in->keydown[i] == SDL_PRESSED)
	return true;
    return false;
}

bool get_keydown(input *in, char c){
    int i = (int) c - 93;
    if(in->keydown[i] == SDL_PRESSED)
	return true;
    return false;
}

//private
void setstate(input *in, SDL_Keycode key, bool state){
    in->keystate[key] = state;
}

bool isPressed(input *in, SDL_Keycode key){
    return in->keystate[key];
}

void key_down_check(input *in){
    for(int i = 0; i < 255; i++){
	if(in->keystate[i] == SDL_PRESSED && in->prevkeystate[i] == SDL_RELEASED){
	    in->keydown[i] = SDL_PRESSED;
	    //cout<<"[keystate] "<<i<<endl;
	} else {
	    in->keydown[i] = SDL_RELEASED;
	}
    }

    for(int i = 0; i < 255; i++)
	in->prevkeystate[i] = in->keystate[i];
}

void mouse_check(input *in){
    in->mouse_r = in->mouse_l = false;
    if(in->event.type == SDL_MOUSEBUTTONDOWN){
	if(in->event.button.button == SDL_BUTTON_RIGHT)
	    in->mouse_r = true;
	if(in->event.button.button == SDL_BUTTON_LEFT)
	    in->mouse_l = true;
    }

    in->mouse_r_down = in->mouse_l_down = false;
    if(in->mouse_r == true && in->prev_mr == false)
	in->mouse_r_down = true;
    if(in->mouse_l == true && in->prev_ml == false)
	in->mouse_l_down = true;

    in->prev_ml = in->mouse_l;
    in->prev_mr = in->mouse_r;
}


