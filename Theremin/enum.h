/* enum.h
 * Ernie Bodle
 * Created: 2017 - 03 - 13
 * 
 * This is made to keep the interfaces of SSM and UI clean.
 */

#define RPI 0

//enum for the state/setting manager 
enum State {STARTUP, 
    MAIN_MENU, THEREMIN, ALTERNATE, RECORDED, LOOPING, SETTINGS, 
    KILL}; 

enum Setting {NONE, TONE, FOUR_SINE, JOE, BARK, PIANO, 
    REC, PLAY_REC, REVERB, ECHO, ONE_K, PLAY_LOOP, PAUSE_LOOP};


State find_state(int i){
    if(i == 0){
	return STARTUP;
    } else if(i == 1){
	return MAIN_MENU;
    }

    //default
    return MAIN_MENU;
}





