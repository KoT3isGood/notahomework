#pragma once

// nhw main file 
// defines essential functions that will be used everywhere and function to work properly


// debug
void nhwLog(const char* msg);

// reset debug (mostly log)
void nhwResetLog();



// instance


typedef void (*nwhLogCallback)(const char* msg);
typedef void (*nwhResetLogCallback)();

struct NhwInstanceInfo {
	nwhLogCallback logCallback = nullptr;
	nwhResetLogCallback logResetCallback = nullptr;
};

void nhwCreateInstance(NhwInstanceInfo createInfo);
void nhwDestroyInstance();

