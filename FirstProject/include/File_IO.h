#pragma once
/* Author: Ben Weatherall
 * Description: Helper function that allows the contents of a folder to be loaded in 
 * both windows and linux.
*/
#if _WIN64 || _WIN32
	#include <Windows.h>
#elif __linux__
	#include <sys/types.h>
	#include <dirent.h>
#endif // _WIN32 || _WIN64

#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> DirectoryContents(std::string dir);

// Taken from viewer.cc (in interest of time)
std::string GetBaseDir(const std::string &filepath);

bool FileExists(const std::string &abs_filename);