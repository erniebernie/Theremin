/* UI.cpp 
 * Ernie Bodle 
 * Modified shape.cpp created on 2016 - 09 - 04 
 * Started on 2016 - 12 - 23
 * Last Updated on 2017 - 03 - 20
 * User Interface for the Digital Theremin
 */

/* To do:
 * 1. Change file I/O to threading
 *
 * 2. Add in range settings 
 *
 * AFTER
 * 1. Add in pong
 * 2. Add in Theremin Hero
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../enum.h"
#include "input.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define BUTTON_WIDTH 115
#define BUTTON_HEIGHT 115

#define DELTA 2 //for the text shadow
#define SEC 3 //how many seconds to record

using namespace std;

//holds data for each button
struct button {		
    int x, y; //position on the screen
    int w, h; //width and the height
    int R, B, G; //colors
    string text; //display
    Setting next_setting; //setting enum
    State next_state; //states enum
};

//holds the buttons
struct buttons{
    button *arry;
    int num;
    string text;
};

//set ups the button struct
void setup_state(State state, buttons &butt); 
void create_buttons(buttons &butt, int num, State state);
void clean_buttons(buttons &butt); 

//differnt menu layers
void create_MAIN_MENU(buttons &butt);
void create_ALTERNATE(buttons &butt);
void create_THEREMIN(buttons &butt);
void create_RECORDED(buttons &butt);
void create_LOOPING(buttons &butt);
void create_SETTINGS(buttons &butt);

//button helper functions
int check_button(input inpu, button b); 

//file I/O
void state_setting(string name, int state, int setting);

//SDL2 Stuff
void print_text(string str, int x, int y, 
	int pt, SDL_Renderer* rend, int fixed);
void print_rect(Setting setting, button &butt, 
	SDL_Renderer* rend, SDL_Surface* surf);
void render_screen(SDL_Renderer* rend, SDL_Surface* surf, 
	buttons &butt, Setting setting);

int main(int argc, char* argv[]){
    SDL_Window* window = NULL;
    SDL_Renderer* rend = NULL;
    SDL_Surface* surf = NULL;

    //For FPS
    //int ticks = 0;
    //int prev_ticks = 0;

    //setting up buttons
    buttons butt;
    butt.num = 0;
    butt.arry = NULL;

    State state = STARTUP; //setting the state to main
    State next_state = MAIN_MENU; //setting the next_state

    Setting setting = NONE; //system setting
    Setting next_setting = NONE; //system setting

    //my input struct
    input inpu;
    setup_input(&inpu);

    //looping
    int loopy = 0; //indicates if looping or nah

    //---------------------------------------------------
    //setup
    TTF_Init(); //fonts

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
	cout<<SDL_GetError()<<endl;
	return 1;
    } 

    //window
    window = SDL_CreateWindow("Shapes", SDL_WINDOWPOS_UNDEFINED, 
	    SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 
	    SDL_WINDOW_SHOWN);
    if(window == NULL){ 
	cout<<SDL_GetError()<<endl;
	return 1;
    }

#if RPI
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    //SDL_ShowCursor(SDL_DISABLE);
#endif

    rend = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    if(rend == NULL){
	cout<<SDL_GetError()<<endl;
	return 1;
    } //rend == NULL

    //rendering the backgroud
    surf = SDL_GetWindowSurface(window);
    SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, 0, 0, 0));
    //end setup
    //---------------------------------------------------

    //buttons
    setup_state(state, butt); //set up state to main by default

    //main program loop!
    while(!inpu.quit){
	poll(&inpu); //polls for input

	//------------------------------------------------------
	//checks buttons for state/setting
	for(int i = 0; i < butt.num; i++){
	    if(check_button(inpu, butt.arry[i])){
		next_state = butt.arry[i].next_state;
		next_setting = butt.arry[i].next_setting;
		if(next_setting == setting)
		    next_setting = NONE;
	    } //if
	} //for loop
	//end checks buttons for state/setting
	//------------------------------------------------------

	//------------------------------------------------------
	if(state == KILL){
	    state_setting("../state.txt", 0, 0);
	    state_setting("../Tone/loop.txt", 0, 0);
	    //RPI
#if RPI
	    system("sudo reboot");
	    butt.text = "Goodbye :-)";
	    render_screen(rend, surf, butt, setting);
#endif	    
	    break;
	}
	//------------------------------------------------------

	//------------------------------------------------------
	if(state == LOOPING){ 
	    if(next_setting == PLAY_LOOP && setting != PLAY_LOOP){
		loopy = 2;
		state_setting("../Tone/loop.txt", 2, 0);
	    } else if(next_setting == PAUSE_LOOP && setting != PAUSE_LOOP){
		loopy = 1;
		state_setting("../Tone/loop.txt", 1, 0);
	    } //if setting == PLAY_LOOP chain

	    if(loopy == 2){
		next_setting = PLAY_LOOP;
	    } else if(loopy == 1){
		next_setting = PAUSE_LOOP;		
	    }

	} //if state == LOOPING
	//------------------------------------------------------

	if(setting == REC){
	    //ERNIE put below in a function
	    //-----------------------------------------------
	    butt.text = "Speak now!";
	    render_screen(rend, surf, butt, setting);
	    
	    int n = SEC; //how many seconds I want to record	    

	    //recording sound 
	    string str = "rec -c 2 ../Tone/Sound/split.wav trim 0 ";
	    stringstream ss;
	    ss.str("");
	    ss<<n;
	    str += ss.str();
	    system(str.c_str()); 

	    //show that it's done 
	    //ERNIE there is a bug here!
	    //It will not update the screen until sound splitting is done
	    next_setting = NONE;
	    render_screen(rend, surf, butt, setting);

	    //sound splitting
	    int num = 14;
	    int delta = 171;
	    int shift = num/2 * -delta;

	    //sound split
	    for(int i = 0; i < (num+1); i++){
		ss.str("");
		ss<< (i);
		str = "sox ../Tone/Sound/split.wav ../Tone/Sound/split/split";
		str += ss.str();
		str += ".wav pitch ";
		ss.str("");
		ss<<(shift + (i)*delta);
		str += ss.str();
		system(str.c_str());
	    }
	    //ERNIE render
	    //-----------------------------------------------
	} //setting == REC


	//------------------------------------------------------
	//change the state/settings
	if(setting != next_setting || state != next_state){
	    if(state != next_state)
		setting = NONE;

	    setting = next_setting;	
	    state = next_state;
	    setup_state(next_state, butt); 
	    state_setting("../state.txt", state, setting);

	    //cout<<"New State = "<<state<<endl;
	    //cout<<"New Setting = "<<setting<<endl<<endl;

	    //comment this out if I want to go for FPS
	    render_screen(rend, surf, butt, setting);
	} //if next sate/setting is current
	//end change the state/setting
	//------------------------------------------------------

	/* This will keep track of how many miliseconds have passed
	   ticks += SDL_GetTicks() - prev_ticks; 
	   prev_ticks = SDL_GetTicks();
	   if(ticks > 500){ //render every 1/2 of a second
	   ticks = 0;
	   render_screen(rend, surf, butt, setting);
	   } //if ticks		    
	   */

    } //while

    clean_buttons(butt);

    SDL_FreeSurface(surf);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

    cout<<"Good Bye from UI :-)"<<endl;

    return 0;
} //end main

//Renders everything!
void render_screen(SDL_Renderer* rend, SDL_Surface* surf, 
	buttons &butt, Setting setting){
    SDL_RenderClear(rend); //clears screen	    
    for(int i = 0; i < butt.num; i++){ 
	//printing rectangles
	print_rect(setting, butt.arry[i], rend, surf);

	//printing text
	int x = butt.arry[i].x + butt.arry[i].w/2;
	int y = butt.arry[i].y + butt.arry[i].h/2;
	string str = butt.arry[i].text;
	print_text(str.c_str(), x, y, 22, rend, 1);
    } //for loop
    //prints the header text
    print_text(butt.text, 0, 0, 30, rend, 0);
    SDL_RenderPresent(rend); //renders all
}

//cleans the butt array and resets the struct
void clean_buttons(buttons &butt){
    if(butt.arry != NULL){
	delete [] butt.arry;
    }
    butt.num = 0;
    butt.arry = NULL;
}

//maybe make these buttons in file I/O
void create_buttons(buttons &butt, int num, State state){
    butt.num = num;
    butt.arry = new button[num];
    for(int i = 0; i < butt.num; i++){
	butt.arry[i].w = BUTTON_WIDTH;
	butt.arry[i].h = BUTTON_HEIGHT;
	butt.arry[i].next_state = state;
	butt.arry[i].next_setting = NONE;
    }
}

void create_back_button(button &b){
    b.text = "Back"; 
    b.x = 6; //SCREEN_WIDTH - 121; //<Test on Theremin
    b.y = SCREEN_HEIGHT - 121;
    b.next_state = MAIN_MENU;  
}

//configures buttons 0 to n in a row
void button_row(buttons &butt, int n, int y){
    int space = (SCREEN_WIDTH-BUTTON_WIDTH*n)/(n+1);
    for(int i = 0; i < n; i++){
	butt.arry[i].x = space*(i+1) + BUTTON_WIDTH*i;
	butt.arry[i].y = y;
    }
}

//sets up main menu
void create_MAIN_MENU(buttons &butt){
    cout<<"MAIN_MENU"<<endl;
    butt.text = "Main Menu";

    create_buttons(butt, 6, MAIN_MENU);

    for(int i = 0; i < butt.num; i++){
	butt.arry[i].R = i*5 + 10; 
	butt.arry[i].B = i*20 + 40;
	butt.arry[i].G = i*30 + 60;
    } //for loop

    button_row(butt, 4, 50); //makes a row of buttons

    butt.arry[0].text = "Theremin"; 
    butt.arry[0].next_state = THEREMIN; 
    butt.arry[1].text = "Alternate"; 
    butt.arry[1].next_state = ALTERNATE; 
    butt.arry[2].text = "Recorded"; 
    butt.arry[2].next_state = RECORDED; 
    butt.arry[3].text = "Looping"; 
    butt.arry[3].next_state = LOOPING; 

    //butt.arry[4].x = 6;
    //butt.arry[4].y = SCREEN_HEIGHT - 121;
    create_back_button(butt.arry[butt.num-2]);
    butt.arry[4].text = "Settings"; 
    butt.arry[4].next_state = SETTINGS; 

    butt.arry[5].x = SCREEN_WIDTH - 121;
    butt.arry[5].y = SCREEN_HEIGHT - 121;
    butt.arry[5].text = "Exit";
    butt.arry[5].next_state = KILL; 
}


void create_ALTERNATE(buttons &butt){
    cout<<"ALTERNATE"<<endl;
    butt.text = "Alternate Sound";

    create_buttons(butt, 4, ALTERNATE);

    for(int i = 0; i < butt.num; i++){
	butt.arry[i].text = "Alternate"; 
	butt.arry[i].R = i*30 + 60; 
	butt.arry[i].B = i*10 + 20;
	butt.arry[i].G = i*20 + 40;
    } //for loop

    button_row(butt, 3, 50); //makes a row of buttons

    butt.arry[0].text = "Bark"; 
    butt.arry[0].next_setting = BARK;

    butt.arry[1].next_setting = JOE;
    butt.arry[1].text = "Joe"; 

    butt.arry[2].next_setting = NONE;
    butt.arry[2].text = "Add_in"; 

    create_back_button(butt.arry[butt.num-1]);
}

void create_THEREMIN(buttons &butt){
    cout<<"THEREMIN"<<endl;
    //butt.text = "Theremin";
    butt.text = "Working on it!";

    create_buttons(butt, 5, THEREMIN);

    for(int i = 0; i < butt.num; i++){
	butt.arry[i].text = "Theremin"; 
	butt.arry[i].R = 40;
	butt.arry[i].B = 30*i + 30;
	butt.arry[i].G = 20;
    } //for loop

    button_row(butt, 4, 50); //makes a row of buttons

    butt.arry[0].text = "Tone";
    butt.arry[0].next_setting = TONE;
    butt.arry[1].text = "4Sine"; 
    butt.arry[1].next_setting = FOUR_SINE;
    butt.arry[2].text = "1kHz"; 
    butt.arry[2].next_setting = ONE_K;
    butt.arry[3].text = "EFFECT"; 
    butt.arry[3].next_setting = REVERB;

    create_back_button(butt.arry[butt.num-1]);
}

void create_RECORDED(buttons &butt){
    cout<<"RECORDED"<<endl;
    butt.text = "Recording";

    create_buttons(butt, 3, RECORDED);

    for(int i = 0; i < butt.num; i++){
	butt.arry[i].text = "Recorded"; 
	butt.arry[i].R = 200;
	butt.arry[i].B = 20;
	butt.arry[i].G = 22*i + 22;
    } //for loop

    button_row(butt, 2, 50); //makes a row of buttons

    butt.arry[0].text = "REC"; 
    butt.arry[0].next_setting = REC; 
    butt.arry[1].text = "Play"; 
    butt.arry[1].next_setting = PLAY_REC; 

    create_back_button(butt.arry[butt.num-1]);
}

void create_LOOPING(buttons &butt){
    cout<<"LOOPING"<<endl; 
    butt.text = "Looping";

    create_buttons(butt, 3, LOOPING);

    for(int i = 0; i < butt.num; i++){
	butt.arry[i].text = "Looping"; 
	butt.arry[i].R = 51 + i * 50;
	butt.arry[i].B = 153 + i*20;
	butt.arry[i].G = 255;
    } //for loop

    button_row(butt, butt.num-1, 50); //makes a row of buttons

    butt.arry[0].text = "Play Loop";
    butt.arry[0].next_setting = PLAY_LOOP;  
    butt.arry[1].text = "Pause";
    butt.arry[1].next_setting = PAUSE_LOOP;  

    //add in button that will shuffel through sound options
    //this will control the loop_setting and that will control
    //the sound being played 

    create_back_button(butt.arry[butt.num-1]);
}

void create_SETTINGS(buttons &butt){
    cout<<"SETTINGS"<<endl; 
    //butt.text = "Settings";
    butt.text = "Working on it!!!";

    create_buttons(butt, 4, SETTINGS);

    for(int i = 0; i < butt.num; i++){
	butt.arry[i].text = "1/3 Range"; 

	butt.arry[i].R = 10*i + 60;
	butt.arry[i].B = 20*i + 20;
	butt.arry[i].G = 30*i + 50;

	butt.arry[i].y = 50;
    } //for loop

    int n = 3;
    int space = (SCREEN_WIDTH - n*BUTTON_WIDTH)/(n+1);

    butt.arry[0].x = space;
    butt.arry[0].text = "1/3 Range"; 

    butt.arry[1].x = space*2 + BUTTON_WIDTH;
    butt.arry[1].text = "2/3 Range"; 

    butt.arry[2].x = space*3 + BUTTON_WIDTH*2;
    butt.arry[2].text = "Full Range"; 


    create_back_button(butt.arry[butt.num-1]);
}

void setup_state(State state, buttons &butt){
    //tear down states
    clean_buttons(butt); //cleans the buttons

    cout<<"State: ";
    if(state == MAIN_MENU){
	create_MAIN_MENU(butt);
    } else if(state == ALTERNATE){
	create_ALTERNATE(butt);
    } else if(state == THEREMIN){
	create_THEREMIN(butt);
    } else if(state == RECORDED){
	create_RECORDED(butt);
    } else if(state == LOOPING){
	create_LOOPING(butt);
    } else if(state == SETTINGS){
	create_SETTINGS(butt);
    } else {
	butt.arry = NULL; //if no state is selected then it will be NULL
	butt.num = 0; //if no state is selected then it will be 0
    }
} //setup_state()

int check_button(input inpu, button butt){
    if(inpu.mouse_l_down){
	if(inpu.mouse_x > butt.x && inpu.mouse_x < (butt.x + butt.w))
	    if(inpu.mouse_y > butt.y && inpu.mouse_y < (butt.y + butt.h)){
		//cout<<"y: "<<inpu.mouse_y<<endl;
		//cout<<"x: "<<inpu.mouse_x<<endl;
		return 1;
	    }
    }
    return 0;
} //check_button


void state_setting(string name, int state, int setting){
    //--------------------
    ofstream file;
    file.open(name.c_str());
    if(file.is_open()){
	file <<state<<"\n"<<setting;
	file.close();
    } //file is open?
    //----------------
}

void print_rect(Setting setting, button &butt, SDL_Renderer* rend, SDL_Surface* surf){

    SDL_Texture* color = NULL;
    SDL_Rect rect; //rectangle that will be rendered 

    rect.x = butt.x;
    rect.y = butt.y;
    rect.w = butt.w;
    rect.h = butt.h;


    if(butt.next_setting != setting || setting == NONE){ //or whatever
	SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, butt.R, butt.B, butt.G));
    } else {
	SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, (255 - butt.R), (255 - butt.B), (255 - butt.G)));
    }

    color = SDL_CreateTextureFromSurface(rend, surf); 
    SDL_RenderCopy(rend, color, 0, &rect); //plops rect onto render

    SDL_DestroyTexture(color);
}

void print_text(string str, int x, int y, int pt, SDL_Renderer* rend, int fixed){
    SDL_Texture* words = NULL; 
    TTF_Font *font = NULL; 
    SDL_Surface* surf = NULL; 

    SDL_Rect rect; //rectangle that will be rendered 
    SDL_Color white = {255, 255, 255};
    SDL_Color black = {0, 0, 0};

    //font
    font = TTF_OpenFont("./fonts/hemi-head-426/hemi_head_bd_it.ttf",pt);
    if(font == NULL){
	cout<<"Font Error"<<endl;
	cout<<TTF_GetError()<<endl;
    }

    for(int i = 0; i < 2; i++){
	//surf
	surf=TTF_RenderText_Blended(font,str.c_str(),(i?white:black));

	if(surf != NULL){
	    words = SDL_CreateTextureFromSurface(rend, surf);
	} //end if surf != NULL

	//rect
	TTF_SizeText(font, str.c_str(), &rect.w, &rect.h);
	rect.x = i?x:(x+DELTA); //i==0 is white viseversa	
	rect.y = i?y:(y+DELTA);	

	if(fixed){
	    rect.x -= rect.w/2;
	    rect.y -= rect.h/2;
	}

	//render
	SDL_RenderCopy(rend, words, NULL, &rect);

	//clean up
	SDL_DestroyTexture(words);
	SDL_FreeSurface(surf);
    }

    TTF_CloseFont(font);
}

