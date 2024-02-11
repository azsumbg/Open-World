#pragma once

#ifndef D2BMPLOADER
#define D2BMPLOADER

#ifdef D2BMPLOADER_EXPORTS
#define D2BMPLOADER_API _declspec(dllexport)
#else
#define D2BMPLOADER_API _declspec(dllimport)
#endif

#include <d2d1.h>
#include <wincodec.h>

#pragma comment (lib,"d2d1.lib")
#pragma comment (lib,"windowscodecs.lib")

extern "C" D2BMPLOADER_API ID2D1Bitmap * Load(LPCWSTR _filename, ID2D1HwndRenderTarget* _render_target);
extern "C" D2BMPLOADER_API void Release();

#endif