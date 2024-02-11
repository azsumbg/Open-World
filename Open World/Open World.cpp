#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include "framework.h"
#include "Objects.h"
#include "Open World.h"
#include <chrono>
#include <ctime>
#include <d2d1.h>
#include <dwrite.h>
#include <fstream>
#include <mmsystem.h>
#include <vector>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "fcheck.lib")
#pragma comment (lib, "objects.lib")

#define bWinClassName L"MyOpenWorld"

#define tmp_file ".\\res\\data\\temp.dat"
#define Ltmp_file L".\\res\\data\\temp.dat"
#define sound_file L".\\res\\snd\\main.wav"
#define save_file L".\\res\\data\\save.dat"
#define record_file L".\\res\\data\\record.dat"
#define help_file L".\\res\\data\\help.dat"

#define mNew 1001
#define mExit 1002
#define mSave 1003
#define mLoad 1004
#define mHoF 1005

#define record 2001
#define first_record 2002
#define no_record 2003

WNDCLASS bWin = { 0 };
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
HDC PaintDC = nullptr;
HCURSOR main_cursor = nullptr;
HCURSOR out_cursor = nullptr;
HICON main_icon = nullptr;
PAINTSTRUCT bPaint = { 0 };
POINT cur_pos = { -1,-1 };

MSG bMsg = { 0 };
BOOL bRet = 0;
UINT bTimer = 0;

D2D1_RECT_F b1Rect = { 0,0,100.0f,50.0f };
D2D1_RECT_F b2Rect = { 200.0f,0,300.0f,50.0f };
D2D1_RECT_F b3Rect = { 400.0f,0,500.0f,50.0f };

bool pause = false;
bool show_help = false;
bool in_client = true;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

wchar_t current_player[16] = L"ONE WARRIOR";

float cl_width = 0;
float cl_height = 0;
int score = 0;
int speed = 1;

GRID Grid;

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* ButBckgBrush = nullptr;
ID2D1SolidColorBrush* TxtBrush = nullptr;
ID2D1SolidColorBrush* ButHgltBrush = nullptr;
ID2D1SolidColorBrush* ButInactBrush = nullptr;
ID2D1SolidColorBrush* FieldBrush = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmText = nullptr;
IDWriteTextFormat* bigText = nullptr;

ID2D1Bitmap* bmpTile = nullptr;
ID2D1Bitmap* bmpRockTile = nullptr;
ID2D1Bitmap* bmpTreeTile = nullptr;
ID2D1Bitmap* bmpEndTile = nullptr;
ID2D1Bitmap* bmpKill = nullptr;

ID2D1Bitmap* bmpFly[28] = { nullptr };
ID2D1Bitmap* bmpWalkL[12] = { nullptr };
ID2D1Bitmap* bmpWalkR[12] = { nullptr };
ID2D1Bitmap* bmpCreepL[4] = { nullptr };
ID2D1Bitmap* bmpCreepR[4] = { nullptr };

ID2D1Bitmap* bmpHeroL[22] = { nullptr };
ID2D1Bitmap* bmpHeroR[22] = { nullptr };

//////////////////////////////////////////////////////////

template<typename COM> BOOL ReleaseCOM(COM** what)
{
    if ((*COM))
    {
        (*COM)->Release();
        (*COM) = nullptr;
        return DL_OK;
    }
    return DL_FAIL;
}
void LogError(LPCWSTR txt)
{
    std::wofstream err(L".\\res\\data\\error.log");
    err << txt << L", at: " << std::chrono::system_clock::now() << std::endl;
    err.close();
}
void InitGame()
{
    wcscpy_s(current_player, L"ONE WARRIOR");
    speed = 1;
    score = 0;
    InitGrid(0, 50.0f, Grid);
}
void ReleaseResources()
{
    if (ReleaseCOM(&iFactory) == DL_FAIL)LogError(L"Error releasing iFactory");
    if (ReleaseCOM(&Draw) == DL_FAIL)LogError(L"Error releasing Draw");
    if (ReleaseCOM(&iWriteFactory) == DL_FAIL)LogError(L"Error releasing iWriteFactory");
    if (ReleaseCOM(&nrmText) == DL_FAIL)LogError(L"Error releasing nrmText");
    if (ReleaseCOM(&bigText) == DL_FAIL)LogError(L"Error releasing bigText");
    if (ReleaseCOM(&ButBckgBrush) == DL_FAIL)LogError(L"Error releasing ButBckgBrush");
    if (ReleaseCOM(&ButHgltBrush) == DL_FAIL)LogError(L"Error releasing ButHgltBrush");
    if (ReleaseCOM(&ButInactBrush) == DL_FAIL)LogError(L"Error releasing ButInactBrush");
    if (ReleaseCOM(&TxtBrush) == DL_FAIL)LogError(L"Error releasing TxtBrush");
    if (ReleaseCOM(&FieldBrush) == DL_FAIL)LogError(L"Error releasing FieldBrush");

    if (ReleaseCOM(&bmpTile) == DL_FAIL)LogError(L"Error releasing bmpTile");
    if (ReleaseCOM(&bmpRockTile) == DL_FAIL)LogError(L"Error releasing bmpRockTile");
    if (ReleaseCOM(&bmpTreeTile) == DL_FAIL)LogError(L"Error releasing bmpTreeTile");
    if (ReleaseCOM(&bmpEndTile) == DL_FAIL)LogError(L"Error releasing bmpEndTile");
    if (ReleaseCOM(&bmpKill) == DL_FAIL)LogError(L"Error releasing bmpKill");

    for (int i = 0; i < 28; i++) if(ReleaseCOM(&bmpFly[i])==DL_FAIL)LogError(L"Error releasing bmpFly");

    for (int i = 0; i < 4; ++i)if (ReleaseCOM(&bmpCreepL[i]) == DL_FAIL)LogError(L"Error releasing bmpCreepL");
    for (int i = 0; i < 4; ++i)if(ReleaseCOM(&bmpCreepR[i])==DL_FAIL)LogError(L"Error releasing bmpCreepR");

    for (int i = 0; i < 12; ++i)if (ReleaseCOM(&bmpWalkL[i]) == DL_FAIL)LogError(L"Error releasing bmpWalkL");
    for (int i = 0; i < 12; ++i)if (ReleaseCOM(&bmpWalkR[i]) == DL_FAIL)LogError(L"Error releasing bmpWalkR");

    for (int i = 0; i < 22; ++i)if (ReleaseCOM(&bmpHeroL[i]) == DL_FAIL)LogError(L"Error releasing bmpHeroL");
    for (int i = 0; i < 22; ++i)if (ReleaseCOM(&bmpHeroR[i]) == DL_FAIL)LogError(L"Error releasing bmpHeroR");
}
void ErrExit(int which)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(which), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(tmp_file);
    ReleaseResources();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);





    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{








    std::remove(tmp_file);
    ReleaseResources();
    exit(1);
    return (int) bMsg.wParam;
}