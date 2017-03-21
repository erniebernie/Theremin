/*Tone.cpp
 * Ernie Bodle * Created on 2017 - 01 - 26
 * Last Update on 2017 - 02 - 20
 * Used to create a tone for the Digital Theremin
 */

#include <iostream>
#include <SDL2/SDL.h> 
#include <math.h> 
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>

#include <SDL2/SDL_mixer.h> 
#include "../enum.h"

//LED
#if RPI
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// define our pins : 
#define DATA        0 // GPIO 17 (WiringPi pin num 0)  header pin 11
#define CLOCK       3 // GPIO 22 (WiringPi pin num 3)   header pin 15
#define LOAD        4 // GPIO 23 (WiringPi pin num 4)   header pin 16

// The Max7219 Registers :
#define DECODE_MODE   0x09                       
#define INTENSITY     0x0a                        
#define SCAN_LIMIT    0x0b                        
#define SHUTDOWN      0x0c                        
#define DISPLAY_TEST  0x0f                         
//END LED
#endif

using namespace std;

#define LEN 4096 //audio buffer length (?) 
#define FORMAT_FREQ 44100

//for raw ADC data
#define LINEAR (MAX_DATA - MIN_DATA)
#define MAX_DATA 2500
#define MIN_DATA 350

//for tone
#define MAX_TONE_FREQ 1500 //min is 0
#define MAX_TONE_VOLUME 15000 //min is 0

//for alt sound
#define MAX_FILE 15 //15 differnt files; 0 to 14

//globals for myAudioCallback
float tone_freq;
float tone_vol;

//obtaining data from files
void get_file_data(string str, int &x, int& y);
int cutfv(int data);

//tone
SDL_AudioDeviceID setup_tone();
//play_tone(); //It plays automatically
//clean_tone();

//alt
Mix_Chunk** setup_alt(int &num_files, Setting setting);
void play_alt(Mix_Chunk** sound, int num_files, 
	int frequency_data, int volume_data);
void clean_alt(Mix_Chunk** sound, int num_files);

//loop
Mix_Chunk* setup_loop(Setting setting);
void play_loop(Mix_Chunk* sound);
void clean_loop(Mix_Chunk* sound);

//LED
void LED0(int data);
void LED1(int data);
#if RPI
static void MAX7219Send (unsigned char reg_number, unsigned char dataout);
static void Send16bits (unsigned short output);
#endif

//Make more than one of these?
//Make it work like a LUT
//For theremin sounds
void MyAudioCallback(void* userdata, Uint8* stream, int len){
    float audio_frequency = (tone_freq - MIN_DATA) * (MAX_TONE_FREQ) / (MAX_DATA - MIN_DATA);
    float audio_volume = (tone_freq - MIN_DATA) * (MAX_TONE_VOLUME) / (MAX_DATA - MIN_DATA);

    //cout<<tone_freq<<" vs "<<audio_frequency<<endl;
    //cout<<tone_vol<<" vs "<<audio_volume<<endl<<endl;
    audio_frequency = audio_frequency/FORMAT_FREQ;
    len /= 2; //because we are converting to 16 bits

    static float last_freq = 0; 
    static unsigned int audio_position; //X
    float prod = audio_frequency * 2 * M_PI;    

    if(audio_frequency != 0){
	audio_position = last_freq*audio_position/audio_frequency; 
    } //if af != 0

    Sint16* buf = (Sint16*)stream; //8bits to 16bits
    double temp = 0;

    buf = (Sint16*)stream; //8bits to 16bits
    for(int i = 0; i < len; i++){
	buf[i] = audio_volume * sin(audio_position * prod)/3;
	audio_position++; //basically X++
    } //end for i < len
    last_freq = audio_frequency;

    return;
} //MyAudioCallback

int main(int argc, char** argv){
    //Variables
    State current_state = STARTUP;
    State next_state = MAIN_MENU;

    Setting current_setting = NONE;
    Setting next_setting = NONE;

    //raw data from file
    int state_data = 0;
    int setting_data = 0;

    //raw, uncut data
    int fd = 0;
    int vd = 0;

    int frequency_data = 0;
    int volume_data = 0;

    //starts up the AUDIO
    SDL_Init(SDL_INIT_AUDIO); 

    //tone    
    SDL_AudioDeviceID tone;

    //alt
    int num_files = 0;
    Mix_Chunk** sound = NULL;

#if RPI
    //LED
    //----------------------------------------
    if (wiringPiSetup () == -1) exit (1);

    //We need 3 output pins to control the Max7219: Data, Clock and Load
    pinMode(DATA, OUTPUT);  
    pinMode(CLOCK, OUTPUT);
    pinMode(LOAD, OUTPUT);  
    MAX7219Send(SCAN_LIMIT, 3); // scanning digit 0, 1, 2, 3

    //other
    MAX7219Send(DECODE_MODE, 0); // Set BCD decode mode on
    MAX7219Send(DISPLAY_TEST, 0); // Disable test mode
    MAX7219Send(INTENSITY, 1); // set brightness 0 to 15
    MAX7219Send(SHUTDOWN, 1); // come out of shutdown mode	
    //----------------------------------------
    //END LED
#endif

    //loop variables 
    int loop_state = 0;
    int prev_loop_state = 1;
    int loop_setting = 0;
    Mix_Chunk* loop_sound = NULL;

    while(1){ 
	//check files
	//get_file_data("../range.txt", max_range, min_range);
	get_file_data("loop.txt", loop_state, loop_setting);
	get_file_data("../state.txt", state_data, setting_data);
	get_file_data("freq.txt", fd, vd);

	if((State) state_data == KILL){
	    break;
	}


	//looping
	//--------------------------------------
	if(loop_state != 0 && loop_state != prev_loop_state){
	    cout<<"loop_state = "<<loop_state<<endl;
	    cout<<"prev_loop_state = "<<prev_loop_state<<endl<<endl;

	    prev_loop_state = loop_state;

	    if(loop_state == 2){
		cout<<"Play loop!"<<endl;
		loop_sound = setup_loop(current_setting);
		play_loop(loop_sound);
	    } else if(loop_state == 1){
		cout<<"Pause loop!"<<endl;
		clean_loop(loop_sound);
	    }
	}
	//--------------------------------------

	//cutting the frequency so it's not too high/low
	if(!(fd == 0 && vd == 0 && vd > 2600 && fd > 2600)){ //and not greater than 2600
	    frequency_data = cutfv(fd);
	    volume_data = cutfv(vd);

	    //globals
	    tone_freq = frequency_data;
	    tone_vol = volume_data;
	}

	LED0(frequency_data);
	LED1(volume_data);

	//if state/setting changes then change
	next_state = (State) state_data;
	next_setting = (Setting) setting_data;

	if(next_state != current_state || next_setting != current_setting){
	    //clean up old state	
	    if(current_state == THEREMIN){
		//SDL_PauseAudioDevice(tone, 1); 
		SDL_CloseAudioDevice(tone);
	    } else if(current_state == ALTERNATE){
		//clean_alt(sound, num_files);

		//ERNIE
		//Make this reopen after closing

		//SDL_CloseAudio();   
	    } else if(current_state == RECORDED){
		if(current_state != REC){	
		    //clean_alt(sound, num_files);
		}
		//SDL_CloseAudio();   
	    }

	    //set up new state
	    if(next_state == THEREMIN){
		if(next_setting == TONE){
		    SDL_Init(SDL_INIT_AUDIO); 
		    tone = setup_tone();
		    SDL_PauseAudioDevice(tone, 0); 
		}
	    } else if(next_state == ALTERNATE){
		//SDL_Init(SDL_INIT_AUDIO); 
		sound = setup_alt(num_files, next_setting); 
	    } else if(next_state == RECORDED){
		//SDL_Init(SDL_INIT_AUDIO); 
		if(next_setting == PLAY_REC){
		    sound = setup_alt(num_files, next_setting); 
		}
	    }

	    current_setting = next_setting;
	    current_state = next_state;	    
	}

	//play sound/tone depending on state/setting
	if(current_state == THEREMIN){
	    //this is done automatically
	} else if(current_state == ALTERNATE){
	    if(current_setting != NONE){	    
		play_alt(sound,num_files,frequency_data,volume_data);
	    }
	} else if(current_state == RECORDED){
	    if(current_setting == PLAY_REC){	
		play_alt(sound,num_files,frequency_data,volume_data);
	    }
	}
    } //end while(1)

#if RPI
    //RPI turning all off
    MAX7219Send(1,0); //volume 0-8	 
    MAX7219Send(2,0); //volume 9-10
    MAX7219Send(3,0); //pitch 0-8		 
    MAX7219Send(4,0); //pitch 9-10	 
#endif

    clean_loop(loop_sound);
    SDL_CloseAudio();   

    cout<<"Good Bye from Tone :)"<<endl;

    return 0;    
} //end main


void play_alt(Mix_Chunk** sound, int num_files, int frequency_data, int volume_data){

    static int last_freq = -1;
    float temp = (LINEAR*100)/frequency_data;

    int lin_max = LINEAR*100 / MIN_DATA; 
    int lin_min = LINEAR*100 / MAX_DATA;

    int small_max = 14;

    int num = (temp - lin_min) / ((lin_max - lin_min) / small_max);

    if(num < 0)
	num = 0;
    if(num > (MAX_FILE-1))
	num = MAX_FILE-1;

    //int vol = MIX_MAX_VOLUME * volume_data/(MAX_DATA);
    //ERNIE THIS SHOULD BE LINEAR TOO
    int vol = (volume_data - MIN_DATA) * (MIX_MAX_VOLUME) / (MAX_DATA - MIN_DATA); 

    if(last_freq != num){
	//cout<<"freq: "<<frequency_data<<" vol "<<volume_data<<endl;
	//cout<<"Play range "<<num<<" vol "<<vol<<"/128"<<endl<<endl;;
	if(sound != NULL){ 
	    if(sound[num] != NULL){
		Mix_VolumeChunk(sound[num], vol);	
		Mix_PlayChannel(-1, sound[num], 0);
	    }
	}
	last_freq = num;
    } //end if last = num
}

//This function cuts the freq/vol data 
//so they are not to high/low.
int cutfv(int data){
    if(data > MAX_DATA)
	data = MAX_DATA;
    if(data < MIN_DATA)
	data = MIN_DATA;
    return data;
}

Mix_Chunk** setup_alt(int &num_files, Setting setting){
    cout<<"Setting up AltSound"<<endl;
    stringstream ss;
    string str = "";
    string sound_file;
    Mix_Chunk** sound;

    //SDL_Init(SDL_INIT_AUDIO); 
    //how can I open this in a differnt way than the TONE
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048) < 0 ){ //2, 2048) < 0){
	cout<<"SDL_GetError()"<<endl;
	//return 1;
    }

    //make these into functions where I just put the string
    if(setting == BARK){	
	num_files = 15; //actually 15    
	sound_file = "Sound/bark/bark";
    } else if(setting == JOE){
	num_files = 15;    
	sound_file = "Sound/joe/joe";
    } else if(setting == PLAY_REC){
	num_files = 15;    
	sound_file = "Sound/split/split";
    } //end elseif chain

    if(setting != NONE){	
	sound = new Mix_Chunk*[num_files];

	for(int i = 0; i < num_files; i++){
	    sound[i] = new Mix_Chunk;
	    ss.str("");
	    ss<<(i);
	    str = sound_file;
	    str += ss.str();
	    str += ".wav";

	    cout<<"Loading: "<<str<<endl;

	    sound[i] = Mix_LoadWAV(str.c_str());
	    if(sound[i] == NULL){
		cout<<"Failed to load: "<<str<<endl;	
	    } //end if sound[i] == NULL
	} //end for
	return sound;
    } //end if setting != NONE

    return NULL;
    }

    void clean_alt(Mix_Chunk** sound, int num_files){
	cout<<"Cleaning alt sound"<<endl;
	if(sound != NULL){
	    for(int i = 0; i < num_files; i++){
		if(sound[i] != NULL){
		    delete sound[i];
		}
	    }
	    delete [] sound;
	    sound = NULL;
	}
    } //end clean_alt

    SDL_AudioDeviceID setup_tone(){
	SDL_AudioDeviceID dev; 
	SDL_AudioSpec have;
	SDL_AudioSpec want;

	SDL_memset(&want, 0, sizeof(want));

	want.freq = FORMAT_FREQ; //44100;
	want.format = AUDIO_S32SYS; //AUDIO_S16;
	want.channels = 1;
	want.samples = LEN;

	want.callback = MyAudioCallback;

	dev = SDL_OpenAudioDevice(NULL, 0, &want, 
		&have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

	if(dev == 0){
	    SDL_Log("Failed to open audio: %s", SDL_GetError());
	}

	if(have.format != want.format){
	    SDL_Log("Not the format I want");
	}

	return dev;
    }

    void get_file_data(string file, int &x, int& y){
	string line;
	ifstream myfile;

	myfile.open(file.c_str());
	if(myfile.is_open()){
	    getline(myfile, line);
	    x = atoi(line.c_str());

	    getline(myfile, line);
	    y = atoi(line.c_str());

	    myfile.close();
	} 
    }

    //loop
    Mix_Chunk* setup_loop(Setting setting){
	cout<<"Setting up Looping!"<<endl;

	Mix_Chunk* sound = NULL; 
	string str = "Sound/loop.wav";

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048) < 0 ){ 
	    cout<<"SDL_GetError()"<<endl;
	}

	cout<<"Loading: "<<str<<endl;
	sound = Mix_LoadWAV(str.c_str());

	if(sound == NULL){
	    cout<<"Failed to load: "<<str<<endl;	    
	} //end if sound == NULL
	return sound;
    } //end setup_loop

    void play_loop(Mix_Chunk* sound){
	//Have the user be able to change the volume
	//and sound file to be played
	if(sound != NULL){
	    Mix_VolumeChunk(sound, 128/4);		
	    Mix_PlayChannel(-1, sound, -1); //play forever
	}
    } //end play_loop

    void clean_loop(Mix_Chunk* sound){
	if(sound != NULL){
	    delete sound;
	    sound = NULL;
	}
    } //end clean_alt

    //LED0
    void LED0(int data){
	static int last_num0 = 0;
	//static int last_num2 = 0;

	float temp = (LINEAR*100)/data;

	int lin_max = LINEAR*100 / MIN_DATA; 
	int lin_min = LINEAR*100 / MAX_DATA;
	int small_max = 10;

	int num = (temp - lin_min) / ((lin_max - lin_min) / small_max);
	if(data == 350){
	    return;
	}

	if(last_num0 != num){ 
	    cout<<"Num0 = "<<num<<" vs "<<last_num0<<endl;
	    last_num0 = num;

	    int A = 0;
	    int B = 0;

	    for(int i = 0; i < 8; i++){
		if(num <= 0)
		    break;
		A =  A | 1<<(7 - i);
		num--;
	    } //end for 8

	    for(int i = 0; i < 2; i++){
		if(num <= 0)
		    break;
		//B =  B | 1<<(7 - i);
		B =  B | 1<<i;
		num--;
	    } //end for 2

	    cout<<"0"<<endl;
	    cout<<"A = "<<bitset<8>(A)<<endl;
	    cout<<"B = "<<bitset<8>(B)<<endl<<endl;

	    //RPI
#if RPI
	    MAX7219Send(1,A); //volume 0-8	 
	    MAX7219Send(2,B); //volume 9-10
#endif
	} //if last_num != num
    } //end LED(


    //LED
    void LED1(int data){
	static int last_num1 = 0;

	float temp = (LINEAR*100)/data;

	int lin_max = LINEAR*100 / MIN_DATA; 
	int lin_min = LINEAR*100 / MAX_DATA;
	int small_max = 10;

	int num = (temp - lin_min) / ((lin_max - lin_min) / small_max);
	if(data == 350){
	    return;
	}

	if(last_num1 != num){ 
	    cout<<"Num1 = "<<num<<" vs "<<last_num1<<endl;
	    last_num1 = num;

	    int A = 0;
	    int B = 0;

	    for(int i = 0; i < 8; i++){
		if(num <= 0)
		    break;
		A =  A | 1<<(7 - i);
		num--;
	    } //end for 8

	    for(int i = 0; i < 2; i++){
		if(num <= 0)
		    break;
		//B =  B | 1<<(7 - i);
		B =  B | 1<<i;
		num--;
	    } //end for 2

	    cout<<"1"<<endl;
	    cout<<"A = "<<bitset<8>(A)<<endl;
	    cout<<"B = "<<bitset<8>(B)<<endl<<endl;

#if RPI //RPI
	    MAX7219Send(3,A); //pitch 0-8		 
	    MAX7219Send(4,B); //pitch 9-10	 
#endif
    } //if last_num != num
} //end LED(


#if RPI
// Take a reg numer and data and send to the max7219
static void MAX7219Send (unsigned char reg_number, unsigned char dataout){
    digitalWrite(LOAD, 1);  // set LOAD 1 to start
    Send16bits((reg_number << 8) + dataout);   // send 16 bits ( reg number + dataout )
    digitalWrite(LOAD, 0);  // LOAD 0 to latch
    digitalWrite(LOAD, 1);  // set LOAD 1 to finish
}

static void Send16bits (unsigned short output){
    unsigned char i;

    for (i=16; i>0; i--){
	unsigned short mask = 1 << (i - 1); // calculate bitmask
	digitalWrite(CLOCK, 0);  // set clock to 0
	// Send one bit on the data pin
	if(output & mask){   
	    digitalWrite(DATA, 1);          
	} else {                              
	    digitalWrite(DATA, 0);     
	}
	digitalWrite(CLOCK, 1);  // set clock to 1	 
    }
}
#endif


