#pragma once

#ifdef FCHECK_EXPORTS 
#define FCHECK_API _declspec(dllexport)
#else
#define FCHECK_API _declspec(dllimport)
#endif

#ifndef FCHECK_INCLUDED
#define FCHECK_INCLUDED


#include <fstream>

constexpr int FILE_EXIST = 20001;
constexpr int FILE_NOT_EXIST = 20002;

extern "C" FCHECK_API LPCWSTR CheckFile(LPCWSTR file_name, int* return_msg);

#endif