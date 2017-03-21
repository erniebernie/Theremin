/* Theresim.cpp
 * Ernie Bodle 2017 - 02 - 05
 * Made to write to a file which the theremin program 
 * takes in.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int main(){

    int delay = 5000000; //idk how to compute this
    int end = 2600; 	//from start to to end
    int start = 200; 	//^what he said
    int delta = 5; 	//change of freq
    string amp; //const

    int x = 350; //frequency outputted
    int y = 2500; //amp 
    int temp; //temp value
    stringstream ss;
    string str; //string of the freq
    string nel = "\n"; //just a newline
    ofstream file; //out stream to file

    int back = 1; //0 forward, 1 back

    while(1){
	if(x < start){
	    back = 0;
	} else if(x > end){
	    back = 1;
	}

	if(back == 0){
	    x += delta;
	    y -= delta;
	} else {
	    x -= delta;
	    y += delta;
	}

	ss.str(""); //clears the stream
	ss << x;
	str = ss.str();

	//y = 225 - (x/6) + 25; 

	ss.str(""); //clears the stream
	ss << y;
	amp = ss.str();

	file.open ("freq.txt", ios::out | ios::trunc);
	if(file.is_open()){
	    //cout<<str<<" "<<amp<<endl<<endl;
	    
	    file.write(str.c_str(), str.size());
	    file.write(nel.c_str(), nel.size());
	    file.write(amp.c_str(), amp.size());
	    
	    file.close();
	}

	int temp = delay;
	while(temp-- > 0) ; //delay

    }

    return 0;
}



