#pragma once

#ifdef ERRH_EXPORTS
#define ERRH_API _declspec(dllexport)
#else 
#define ERRH_API _declspec(dllimport)
#endif

constexpr int eScreen = 10001;
constexpr int eButton = 10002;
constexpr int eImage = 10003;
constexpr int eStarted = 10004;
constexpr int eIcon = 10005;
constexpr int eCursor = 10006;
constexpr int eClass = 10007;
constexpr int eWindow = 10008;
constexpr int eMsg = 10009;
constexpr int eGdi = 10010;
constexpr int eD2D = 10011;

extern wchar_t error_message[300];

extern "C" ERRH_API LPCWSTR ErrHandle(int whicherror);