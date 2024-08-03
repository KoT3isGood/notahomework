#pragma once
#include <stdint.h>
#include <stdarg.h>


//  main file 
// defines essential functions that will be used everywhere and function to work properly


// Debug message
void Log(const char* msg, ...);

// Reset debug messanger
// unused
void ResetLog();

// Crashes application when code fails to execute correctly
void Mayday(const char* msg);


// Instance
// Log callback
// Called when Log is getting called
typedef void (*LogCallback)(const char* msg, va_list args);
// Called to reset log
typedef void (*ResetLogCallback)();

typedef struct {
	LogCallback logCallback;
	ResetLogCallback logResetCallback;
} nhwInstanceInfo;

// Initializes instance
// Inits GLFW
// Does not init vulkan and openal
// Look in draw/nhwdraw.h and audio/nhwaudio.h for them
void CreateInstance(nhwInstanceInfo createInfo);

// Destroys instance
void DestroyInstance();

// Filesystem

bool FileExists(const char* fileName);
bool DirectoryExists(const char* dirPath);
bool IsFileExtension(const char* fileName, const char* ext);
int GetFileLenght(const char* fileName);
const char* GetFileExtension(const char* fileName);
const char* GetFileName(const char* filePath);
const char* GetFileNameWithoutExt(const char* filePath);
const char* GetDirectoryPath(const char* filePath);
const char* GetPrevDirectoryPath(const char* dirPath);
const char* GetWorkingDirectory(void);
const char* GetApplicationDirectory(void);
bool ChangeDirectory(const char* dir);
long GetFileModTime(const char* fileName);

// Reading file
unsigned char* LoadFileData(const char* fileName);
unsigned char* LoadFileData(const char* fileName, uint32_t* fileSizeOut);
void UnloadFileData(unsigned char* data);

// Dynamic Libraries
void* LoadDynamicLibrary(const char* file);
void UnloadDynamicLibrary(void* lib);
void* GetFunction(void* lib, const char* func);

// Time related
float GetTime();