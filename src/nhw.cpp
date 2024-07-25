#include "nhw.h"
#include "stdio.h"
#include <filesystem>
#include <fstream>
#include "GLFW/glfw3.h"

nhwInstanceInfo InstanceInfo = {};

void CreateInstance(nhwInstanceInfo createInfo)
{
	if (!glfwInit()) Mayday("Failed to init GLFW");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	InstanceInfo = createInfo;
}

void DestroyInstance()
{
	glfwTerminate();
}

void Log(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	if (InstanceInfo.logCallback) {
		InstanceInfo.logCallback(msg, args);
	}
	va_end(args);
}

void ResetLog()
{
	if (InstanceInfo.logResetCallback) {
		InstanceInfo.logResetCallback();
	}
}

void Mayday(const char* msg)
{
	Log(msg);

	// FORCED SEGFAULT
	int* a = 0;
	int b = *a;
}


bool FileExists(const char* fileName) {
	return std::filesystem::exists(fileName);
};
bool DirectoryExists(const char* dirPath) {
	return std::filesystem::exists(dirPath);
}
bool IsFileExtension(const char* fileName, const char* ext) {
	return !strcmp(GetFileExtension(fileName), ext);
};
int GetFileLenght(const char* fileName) {
	return std::filesystem::file_size(std::string(fileName));
};

const char* GetFileExtension(const char* fileName) {
	static std::string outstr;
	outstr = std::filesystem::path(fileName).extension().string();
	return outstr.c_str();
};

const char* GetFileName(const char* filePath) {
	static std::string outstr;
	outstr = std::filesystem::path(filePath).filename().string();
	return outstr.c_str();
};
const char* GetFileNameWithoutExt(const char* filePath) {
	static std::string outstr;
	outstr = std::filesystem::path(filePath).stem().string();
	return outstr.c_str();
};
const char* GetDirectoryPath(const char* filePath) {
	static std::string outstr;
	outstr = std::filesystem::path(filePath).parent_path().string();
	return outstr.c_str();
};

const char* GetPrevDirectoryPath(const char* dirPath) {
	static std::string outstr;
	outstr = std::filesystem::path(dirPath).parent_path().string();
	return outstr.c_str();
};
const char* GetWorkingDirectory(void) {
	static std::string outstr;
	outstr = std::filesystem::current_path().string().c_str();
	return outstr.c_str();
};
const char* GetApplicationDirectory(void) { return "NOT IMPLEMENTED"; };


bool ChangeDirectory(const char* dir) {
	if (DirectoryExists(dir)) {
		std::filesystem::current_path(dir);
		return true;
	}
	return false;
};
long GetFileModTime(const char* fileName) {

};



unsigned char* LoadFileData(const char* fileName) {

	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return nullptr;
	}

	size_t fileSize = (size_t)file.tellg();
	unsigned char* data = (unsigned char*)malloc(fileSize);

	file.seekg(0);
	file.read((char*)data, fileSize);

	file.close();

	return data;
};

unsigned char* LoadFileData(const char* fileName, uint32_t* fileSizeOut) {

	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return nullptr;
	}

	size_t fileSize = (size_t)file.tellg();
	*fileSizeOut = fileSize;
	unsigned char* data = (unsigned char*)malloc(fileSize);

	file.seekg(0);
	file.read((char*)data, fileSize);

	file.close();

	return data;
};

void UnloadFileData(unsigned char* data) {
	free(data);
};