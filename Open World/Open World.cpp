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
int minutes = 0;
int seconds = 0;

CELL Grid[10][10];

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

template<typename COMOBJ> BOOL ReleaseCOM(COMOBJ** what)
{
    if ((*what))
    {
        (*what)->Release();
        (*what) = nullptr;
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
    minutes = 0;
    seconds = 0;


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

INT_PTR CALLBACK bDlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(main_icon));
        return true;
        break;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            {
                int size = GetDlgItemText(hwnd, IDC_NAME, current_player, 15);
                if (size < 1)
                {
                    if (sound)MessageBeep(MB_ICONEXCLAMATION);
                    MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                    wcscpy_s(current_player, L"ONE PLAYER");
                    EndDialog(hwnd, IDCANCEL);
                    break;
                }
                EndDialog(hwnd, IDOK);
            }
            break;
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK bWinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        {
            srand((unsigned int)(time(0)));
            SetTimer(hwnd, bTimer, 1000, NULL);
            RECT clR = { 0 };
            GetClientRect(hwnd, &clR);
            cl_width = static_cast<float>(clR.right);
            cl_height = static_cast<float>(clR.bottom);

            bBar = CreateMenu();
            bMain = CreateMenu();
            bStore = CreateMenu();

            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

            AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
            AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bMain, MF_STRING, mExit, L"Изход");

            AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
            AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
            AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");
            SetMenu(hwnd, bBar);
            InitGame();
        }
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        if (MessageBox(hwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Изход", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(0, 0, 0)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_TIMER:
        if (pause)break;
        seconds++;
        minutes = seconds / 60;
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }

            if (cur_pos.y <= 50)
            {
                if (!name_set)
                {
                    if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                    {
                        if (!b1Hglt)
                        {
                            b1Hglt = true;
                            if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        }
                    }
                    else if (b1Hglt)
                    {
                        b1Hglt = false;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                }
            
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        b2Hglt = true;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                }
                else if (b2Hglt)
                {
                    b2Hglt = false;
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                }

                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        b3Hglt = true;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                }
                else if (b3Hglt)
                {
                    b3Hglt = false;
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                }
            
                SetCursor(out_cursor);
                return true;
            }
            else
            {
                if (b1Hglt || b2Hglt || b3Hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b1Hglt = false;
                    b2Hglt = false;
                    b3Hglt = false;
                }
            }
            SetCursor(main_cursor);
            return true;
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)MessageBeep(MB_ICONEXCLAMATION);
            if (MessageBox(hwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;


        }
        break;


    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void SystemInit()
{
    int result = 0;
    int window_x = 0;
    int window_y = 0;
    
    if (!bIns)ErrExit(eClass);
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream tmp(Ltmp_file);
        tmp << L"Ne barai be, rAbotim !";
        tmp.close();
    }

    window_x = GetSystemMetrics(SM_CXSCREEN) / 2 - 250;
    window_y = 40;

    if (GetSystemMetrics(SM_CXSCREEN) < window_x + scr_width 
        || GetSystemMetrics(SM_CYSCREEN) < window_y + scr_height)ErrExit(eScreen);

    main_icon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 128, 128, LR_LOADFROMFILE));
    if (!main_icon)ErrExit(eIcon);

    main_cursor = LoadCursorFromFile(L".\\res\\bcursor.ani");
    out_cursor = LoadCursorFromFile(L".\\res\\out.ani");
    if (!main_cursor || !out_cursor)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &bWinProc;
    bWin.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    bWin.hIcon = main_icon;
    bWin.hCursor = main_cursor;
    bWin.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindowW(bWinClassName, L"Световни приключения", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, window_x, window_y,
        static_cast<int>(scr_width), static_cast<int>(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else ShowWindow(bHwnd, SW_SHOWDEFAULT);

    HRESULT hr = S_OK;

    D2D1_GRADIENT_STOP gStop[2] = {0};
    ID2D1GradientStopCollection* gsCol = nullptr;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
    if (hr != S_OK)
    {
        LogError(L"Error creating iFactory ");
        ErrExit(eD2D);
    }

    hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(bHwnd, D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
    if (hr != S_OK)
    {
        LogError(L"Error creating Draw ");
        ErrExit(eD2D);
    }

    gStop[0].position = 0;
    gStop[0].color = D2D1::ColorF(D2D1::ColorF::DarkBlue);
    gStop[1].position = 1.0f;
    gStop[1].color = D2D1::ColorF(D2D1::ColorF::AliceBlue);

    hr = Draw->CreateGradientStopCollection(gStop, 2, &gsCol);
    if (hr != S_OK)
    {
        LogError(L"Error creating GradientStopCollection ");
        ErrExit(eD2D);
    }

    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
        D2D1::Point2F(0, 0), scr_width / 2, 25.0f), gsCol, &ButBckgBrush);
    if (hr != S_OK)
    {
        LogError(L"Error creating GradientBrush for button background ");
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkMagenta), &TxtBrush);
    if (hr != S_OK)
    {
        LogError(L"Error creating TextBrush ");
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &ButHgltBrush);
    if (hr != S_OK)
    {
        LogError(L"Error creating ButHgltBrush ");
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &ButInactBrush);
    if (hr != S_OK)
    {
        LogError(L"Error creating ButInactBrush ");
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &FieldBrush);
    if (hr != S_OK)
    {
        LogError(L"Error creating ButFieldBrush ");
        ErrExit(eD2D);
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
    if (hr != S_OK)
    {
        LogError(L"Error creating iWriteFactory ");
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"", &nrmText);
    if (hr != S_OK)
    {
        LogError(L"Error creating nrmText ");
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_NORMAL, 64.0f, L"", &bigText);
    if (hr != S_OK)
    {
        LogError(L"Error creating bigText ");
        ErrExit(eD2D);
    }

    bmpTile = Load(L".\\res\\img\\tile.png", Draw);
    if (!bmpTile)
    {
        LogError(L"Error loading bmpTile ");
        ErrExit(eD2D);
    }

    bmpRockTile = Load(L".\\res\\img\\rock_tile.png", Draw);
    if (!bmpRockTile)
    {
        LogError(L"Error loading bmpRockTile ");
        ErrExit(eD2D);
    }

    bmpTreeTile = Load(L".\\res\\img\\tree_tile.png", Draw);
    if (!bmpTreeTile)
    {
        LogError(L"Error loading bmpTreeTile ");
        ErrExit(eD2D);
    }

    bmpEndTile = Load(L".\\res\\img\\end_tile.png", Draw);
    if (!bmpEndTile)
    {
        LogError(L"Error loading bmpEndTile ");
        ErrExit(eD2D);
    }

    bmpKill = Load(L".\\res\\img\\hollyroger.png", Draw);
    if (!bmpKill)
    {
        LogError(L"Error loading bmpKill ");
        ErrExit(eD2D);
    }

    for (int i = 0; i < 22; i++)
    {
        wchar_t path[75] = L".\\res\\img\\hero_l\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpHeroL[i] = Load(path, Draw);
        if (!bmpHeroL[i])
        {
            LogError(L"Error loading bmpHeroL ");
            ErrExit(eD2D);
        }
    }

    for (int i = 0; i < 22; i++)
    {
        wchar_t path[75] = L".\\res\\img\\hero_r\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpHeroR[i] = Load(path, Draw);
        if (!bmpHeroR[i])
        {
            LogError(L"Error loading bmpHeroR ");
            ErrExit(eD2D);
        }
    }

    for (int i = 0; i < 12; i++)
    {
        wchar_t path[75] = L".\\res\\img\\bad\\ground_l\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpWalkL[i] = Load(path, Draw);
        if (!bmpWalkL[i])
        {
            LogError(L"Error loading bmpWalkL ");
            ErrExit(eD2D);
        }
    }

    for (int i = 0; i < 12; i++)
    {
        wchar_t path[75] = L".\\res\\img\\bad\\ground_r\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpWalkR[i] = Load(path, Draw);
        if (!bmpWalkR[i])
        {
            LogError(L"Error loading bmpWalkR ");
            ErrExit(eD2D);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        wchar_t path[75] = L".\\res\\img\\bad\\slime_l\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpCreepL[i] = Load(path, Draw);
        if (!bmpCreepL)
        {
            LogError(L"Error loading bmpCreepL ");
            ErrExit(eD2D);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        wchar_t path[75] = L".\\res\\img\\bad\\slime_r\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpCreepR[i] = Load(path, Draw);
        if (!bmpCreepR)
        {
            LogError(L"Error loading bmpCreepR ");
            ErrExit(eD2D);
        }
    }

    for (int i = 0; i < 28; i++)
    {
        wchar_t path[75] = L".\\res\\img\\bad\\fly\\";
        wchar_t name[5] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpFly[i] = Load(path, Draw);
        if (!bmpFly)
        {
            LogError(L"Error loading bmpFly ");
            ErrExit(eD2D);
        }
    }

    wchar_t first_text[34] = L"ОПАСНО ПРИКЛЮЧЕНИЕ !\n\ndev. Daniel";
    wchar_t show[34] = L"\0";

    for (int i = 0; i < 34; i++)
    {
        mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
        show[i] = first_text[i];
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Azure));
        if (TxtBrush && nrmText)
            Draw->DrawText(show, i, nrmText, D2D1::RectF(50.0f, cl_height / 2 - 50.0f, cl_width, cl_height), TxtBrush);
        Draw->EndDraw();
        Sleep(30);
    }
    Sleep(2500);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;

    SystemInit();
    
    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;

            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::Azure));
            if (TxtBrush && bigText)
                Draw->DrawText(L"ПАУЗА", 6, bigText, D2D1::RectF(100.0f, cl_height / 2 - 50.0f, cl_width, cl_height), TxtBrush);
            Draw->EndDraw();
            continue;
        }
        /////////////////////////////////////////////////////////////////





        // DRAW THINGS *******************************
        Draw->BeginDraw();
        
        




        Draw->EndDraw();

    }

    std::remove(tmp_file);
    ReleaseResources();
    exit(1);
    return (int) bMsg.wParam;
}