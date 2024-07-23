#pragma once
#include <stdint.h>
#include <stdarg.h>


//  main file 
// defines essential functions that will be used everywhere and function to work properly


// Debug message
void Log(const char* msg, ...);

// Reset debug messanger
void ResetLog();

void Mayday(const char* msg);


// instance



typedef void (*LogCallback)(const char* msg, va_list args);
typedef void (*ResetLogCallback)();

typedef struct {
	LogCallback logCallback;
	ResetLogCallback logResetCallback;
} nhwInstanceInfo;

void CreateInstance(nhwInstanceInfo createInfo);
void DestroyInstance();

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
unsigned char* LoadFileData(const char* fileName);
unsigned char* LoadFileData(const char* fileName, uint32_t* fileSizeOut);
void UnloadFileData(unsigned char* data);