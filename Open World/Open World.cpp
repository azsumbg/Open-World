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

bool win_game = false;

wchar_t current_player[16] = L"ONE WARRIOR";

float cl_width = 0;
float cl_height = 0;
int score = 0;
int speed = 1;
int minutes = 0;
int seconds = 0;

CELL Grid[10][10] = { 0 };

cre_ptr Hero = nullptr;
dirs hero_prev_dir = dirs::stop;

std::vector<cre_ptr> vBadArmy;

prot_ptr Potion = nullptr;
prot_ptr Castle = nullptr;

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
ID2D1Bitmap* bmpHeal = nullptr;
ID2D1Bitmap* bmpCastle = nullptr;

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
    seconds = 120;
    name_set = false;

    ReleaseCOM(&Hero);
    ReleaseCOM(&Potion);
    ReleaseCOM(&Castle);

    if (!vBadArmy.empty())
        for (int i = 0; i < vBadArmy.size(); i++)ReleaseCOM(&vBadArmy[i]);
    vBadArmy.clear();
    

    InitGrid(1.0f, 51.0f, Grid);

    for (int j = 0; j < 10; j++)
    {
        bool found_first_col = false;
        int start_col = -1;
        int max_num_cols = 0;

        while (!found_first_col)
        {
            start_col++;
            if (start_col > 7)start_col = 0;
            if (rand() % 3 == 1)found_first_col = true;
        }

        max_num_cols = 9 - start_col;
        max_num_cols += start_col - rand()% 2;
        if (max_num_cols <= start_col)max_num_cols = start_col + 1;

        for (int i = start_col; i <= max_num_cols; i++)
        {
            switch (rand() % 10)
            {
            case 0:
                Grid[i][j].type = grids::rock;
                break;

            case 3:
                Grid[i][j].type = grids::tree;
                break;

            default:
                Grid[i][j].type = grids::empty;
            }
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (Grid[i][9].type == grids::empty)Hero = CreatureFactory(creatures::hero, Grid[i][9].x + 10.0f, Grid[i][9].y + 10.0f);
    }

    for (int i = 0; i < 10; i++)
    {
        if (Grid[i][0].type == grids::empty)
        {
            Grid[i][0].type = grids::end_tile;
            break;
        }
    }
    
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
    if (ReleaseCOM(&bmpHeal) == DL_FAIL)LogError(L"Error releasing bmpHeal");
    if (ReleaseCOM(&bmpCastle) == DL_FAIL)LogError(L"Error releasing Castle");
    
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
void NewLevel()
{
    seconds = 120;
    ReleaseCOM(&Hero);
    ReleaseCOM(&Potion);
    ReleaseCOM(&Castle);

    if (!vBadArmy.empty())
        for (int i = 0; i < vBadArmy.size(); i++)ReleaseCOM(&vBadArmy[i]);
    vBadArmy.clear();

    InitGrid(1.0f, 51.0f, Grid);

    for (int j = 0; j < 10; j++)
    {
        bool found_first_col = false;
        int start_col = -1;
        int max_num_cols = 0;

        while (!found_first_col)
        {
            start_col++;
            if (start_col > 7)start_col = 0;
            if (rand() % 3 == 1)found_first_col = true;
        }

        max_num_cols = 9 - start_col;
        max_num_cols += start_col - rand() % 2;
        if (max_num_cols <= start_col)max_num_cols = start_col + 1;

        for (int i = start_col; i <= max_num_cols; i++)
        {
            switch (rand() % 10)
            {
            case 0:
                Grid[i][j].type = grids::rock;
                break;

            case 3:
                Grid[i][j].type = grids::tree;
                break;

            default:
                Grid[i][j].type = grids::empty;
            }
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (Grid[i][9].type == grids::empty)Hero = CreatureFactory(creatures::hero, Grid[i][9].x + 10.0f, Grid[i][9].y + 10.0f);
    }

    for (int i = 0; i < 10; i++)
    {
        if (Grid[i][0].type == grids::empty)
        {
            Grid[i][0].type = grids::end_tile;
            break;
        }
    }

    if (!Castle && rand() % 60 == 33)
    {
        int randx = rand() % 9;
        int randy = rand() % 9;

        Grid[randx][randy].type = grids::empty;
        Castle = new PROTON(Grid[randx][randy].x, Grid[randx][randy].y, 49.0f, 49.0f);
    }
}
BOOL CheckRecord()
{
    if (score < 1)return no_record;

    int result = 0;
    CheckFile(record_file, &result);

    if (result == FILE_NOT_EXIST)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; i++)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return first_record;
    }

    std::wifstream check(record_file);
    check >> result;
    check.close();

    if (result < score)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; i++)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return record;
    }

    return no_record;
}
void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);
    if (win_game)
    {
        score += 5000;
        if (sound)mciSendString(L"play .\\res\\snd\\tada.wav", NULL, NULL, NULL);
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
        if (bigText && TxtBrush)
            Draw->DrawText(L"ПРЕВЪРТЯ ИГРАТА !", 18, bigText, D2D1::RectF(50.0f, cl_height / 2 - 50, cl_width, cl_height), TxtBrush);
        Draw->EndDraw();
        Sleep(3500);
    }

    wchar_t end_text[50] = L"О, О, О ! ЗАГУБИ !";
    int size = 0;

    switch (CheckRecord())
    {
    case no_record:
        if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_ASYNC);
        size = 19;
        break;

    case first_record:
        wcscpy_s(end_text, L"ПЪРВИ РЕКОРД !");
        if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_ASYNC);
        size = 15;
        break;

    case record:
        wcscpy_s(end_text, L"СВЕТОВЕН РЕКОРД !");
        if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_ASYNC);
        size = 18;
        break;
    }

    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
    if (bigText && TxtBrush)
        Draw->DrawText(end_text, size, bigText, D2D1::RectF(50.0f, cl_height / 2 - 50, cl_width, cl_height), TxtBrush);
    Draw->EndDraw();
    Sleep(7000);

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void HallofFame()
{
    int result = 0;
    CheckFile(record_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)MessageBeep(MB_ICONASTERISK);
        MessageBox(bHwnd, L"Все още няма рекорд на играта !\n\nПостарай се повече !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    wchar_t hof[75] = L"НАЙ-ДОБЪР ИГРАЧ: ";
    wchar_t saved_pl[16] = L"\0";
    wchar_t add[5] = L"\0";

    std::wifstream rec(record_file);
    rec >> result;
    swprintf(add, 5, L"%d", result);
    for (int i = 0; i < 16; ++i)
    {
        int letter = 0;
        rec >> letter;
        saved_pl[i] = static_cast<wchar_t>(letter);
    }
    rec.close();

    wcscat_s(hof, saved_pl);
    wcscat_s(hof, L"\nСВЕТОВЕН РЕКОРД: ");
    wcscat_s(hof, add);

    result = 0;

    for (int i = 0; i < 75; i++)
    {
        if (hof[i] != '\0')result++;
        else break;
    }

    if (sound)mciSendString(L"play .\\res\\snd\\tada.wav", NULL, NULL, NULL);

    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateGray));
    if (bigText && TxtBrush)
        Draw->DrawTextW(hof, result, bigText, D2D1::RectF(10.0f, 200.0f, cl_width, cl_width), ButHgltBrush);
    Draw->EndDraw();
    Sleep(4000);

}
void SaveGame()
{
    int result = 0;
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        if (MessageBox(bHwnd, L"Ако продължиш, ще загубиш предишен запис !\n\nНаистина ли презаписваш ?",
            L"Презапис", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }

    std::wofstream save(save_file);

    save << score << std::endl;
    save << seconds << std::endl;
    for (int i = 0; i < 16; ++i)save << static_cast<int>(current_player[i]) << std::endl;
    save << name_set << std::endl;
    for (int rows = 0; rows < 10; rows++)
        for (int cols = 0; cols < 10; cols++)
            save << static_cast<int>(Grid[cols][rows].type) << std::endl;
    if (!Potion)save << 0 << std::endl;
    else
    {
        save << Potion->x << std::endl;
        save << Potion->y << std::endl;
    }
    if (!Castle)save << 0 << std::endl;
    else
    {
        save << Castle->x << std::endl;
        save << Castle->y << std::endl;
    }
    if (!Hero)save << 0 << std::endl;
    else
    {
        save << Hero->x << std::endl;
        save << Hero->y << std::endl;
        save << Hero->lifes << std::endl;
    }

    save << vBadArmy.size() << std::endl;
    if (!vBadArmy.empty())
    {
        for (int i = 0; i < vBadArmy.size(); ++i)
        {
            save << vBadArmy[i]->x << std::endl;
            save << vBadArmy[i]->y << std::endl;
            save << static_cast<int>(vBadArmy[i]->GetType()) << std::endl;
            save << static_cast<int>(vBadArmy[i]->dir) << std::endl;
            save << vBadArmy[i]->lifes << std::endl;
        }
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е запазена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
    int result = 0;
    float tempx = 0;
    float tempy = 0;

    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        if (MessageBox(bHwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли презаписваш ?",
            L"Презапис", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }
    else
    {
        if (sound)MessageBeep(MB_ICONASTERISK);
        MessageBox(bHwnd, L"Все още няма записана игра !\n\nПостарай се повече !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    ReleaseCOM(&Hero);
    ReleaseCOM(&Potion);
    ReleaseCOM(&Castle);

    if (!vBadArmy.empty())
        for (int i = 0; i < vBadArmy.size(); i++)ReleaseCOM(&vBadArmy[i]);
    vBadArmy.clear();

    std::wifstream save(save_file);

    save >> score;
    save >> seconds;
    wcscpy_s(current_player, L"\0");
    for (int i = 0; i < 16; ++i)
    {
        int letter = 0;
        save >> letter;
        current_player[i] = static_cast<wchar_t>(letter);
    }
    save >> name_set;
    
    for (int rows = 0; rows < 10; ++rows)
        for (int cols = 0; cols < 10; cols++)
        {
            int mtype = 0;
            save >> mtype;
            Grid[cols][rows].type = static_cast<grids>(mtype);
        }
    
    save >> tempx;
    if (tempx != 0)
    {
        save >> tempy;
        Potion = new PROTON(tempx, tempy, 20.0f, 17.0f);
    }

    save >> tempx;
    if (tempx != 0)
    {
        save >> tempy;
        Castle = new PROTON(tempx, tempy, 20.0f, 17.0f);
    }
    
    save >> tempx;
    if (tempx == 0)GameOver();
    else
    {
        save >> tempy;
        save >> result;
        Hero = CreatureFactory(creatures::hero, tempx, tempy);
        Hero->lifes = result;
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; ++i)
        {
            int mtype = 0;
            int mdir = 0;
            int mlifes = 0;

            save >> tempx;
            save >> tempy;
            save >> mtype;
            save >> mdir;
            save >> mlifes;

            vBadArmy.push_back(CreatureFactory(static_cast<creatures>(mtype), tempx, tempy));

            vBadArmy[i]->dir = static_cast<dirs>(mdir);
            vBadArmy[i]->lifes = mlifes;
        }
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е заредена !", L"Зареждане !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
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
        seconds--;
        minutes = seconds / 60;
        if (seconds == 0)GameOver();
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

        case mSave:
            pause = true;
            SaveGame();
            pause = false;
            break;

        case mLoad:
            pause = true;
            LoadGame();
            pause = false;
            break;

        case mHoF:
            pause = true;
            HallofFame();
            pause = false;
            break;
        }
        break;

    case WM_KEYDOWN:
        if (Hero)
        {
            if (Hero->killed)break;
        }
        switch (wParam)
        {
        case VK_UP:
            if (Hero)
            {
                int up_cell = Hero->GetCellNum() - 10;
                int up_cell_col = up_cell % 10;
                int up_cell_row = up_cell / 10;
                
                if (up_cell < 0)break;

                hero_prev_dir = Hero->dir;
                Hero->dir = dirs::up;
                if (Grid[up_cell_col][up_cell_row].type == grids::empty)
                {
                    Hero->x = Grid[up_cell_col][up_cell_row].x + 10.0f;
                    Hero->y = Grid[up_cell_col][up_cell_row].y + 10.0f;
                    Hero->SetEdges();
                }
                else
                {
                    if (Grid[up_cell_col][up_cell_row].type != grids::end_tile)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\cut.wav", NULL, NULL, NULL);
                        if (Hero->Attack() != 0)Grid[up_cell_col][up_cell_row].type = grids::empty;
                    }
                }

                if (Grid[up_cell_col][up_cell_row].type == grids::end_tile)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
                    NewLevel();
                }
            }
            break;

        case VK_DOWN:
            if (Hero)
            {
                int down_cell = Hero->GetCellNum() + 10;
                int down_cell_col = down_cell % 10;
                int down_cell_row = down_cell / 10;

                if (down_cell > 99)break;

                hero_prev_dir = Hero->dir;
                Hero->dir = dirs::down;
                if (Grid[down_cell_col][down_cell_row].type == grids::empty)
                {
                    Hero->x = Grid[down_cell_col][down_cell_row].x + 10.0f;
                    Hero->y = Grid[down_cell_col][down_cell_row].y + 10.0f;
                    Hero->SetEdges();
                }
                else
                {
                    if (Grid[down_cell_col][down_cell_row].type != grids::end_tile)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\cut.wav", NULL, NULL, NULL);
                        if (Hero->Attack() != 0)Grid[down_cell_col][down_cell_row].type = grids::empty;
                    }
                }
                if (Grid[down_cell_col][down_cell_row].type == grids::end_tile)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
                    NewLevel();
                }
            }
            break;

        case VK_LEFT:
            if (Hero)
            {
                int left_cell = Hero->GetCellNum() - 1;
                int left_cell_col = left_cell % 10;
                int left_cell_row = left_cell / 10;

                if (left_cell < 0)break;

                Hero->dir = dirs::left;
                if (Grid[left_cell_col][left_cell_row].type == grids::empty)
                {
                    Hero->x = Grid[left_cell_col][left_cell_row].x + 10.0f;
                    Hero->y = Grid[left_cell_col][left_cell_row].y + 10.0f;
                    Hero->SetEdges();
                }
                else
                {
                    if (Grid[left_cell_col][left_cell_row].type != grids::end_tile)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\cut.wav", NULL, NULL, NULL);
                        if (Hero->Attack() != 0)Grid[left_cell_col][left_cell_row].type = grids::empty;
                    }
                }
                if (Grid[left_cell_col][left_cell_row].type == grids::end_tile)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
                    NewLevel();
                }
            }
            break;

        case VK_RIGHT:
            if (Hero)
            {
                int right_cell = Hero->GetCellNum() + 1;
                int right_cell_col = right_cell % 10;
                int right_cell_row = right_cell / 10;

                if (right_cell > 99)break;

                Hero->dir = dirs::right;
                if (Grid[right_cell_col][right_cell_row].type == grids::empty)
                {
                    Hero->x = Grid[right_cell_col][right_cell_row].x + 10.0f;
                    Hero->y = Grid[right_cell_col][right_cell_row].y + 10.0f;
                    Hero->SetEdges();
                }
                else
                {
                    if (Grid[right_cell_col][right_cell_row].type != grids::end_tile)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\cut.wav", NULL, NULL, NULL);
                        if (Hero->Attack() != 0)Grid[right_cell_col][right_cell_row].type = grids::empty;
                    }
                }
                if (Grid[right_cell_col][right_cell_row].type == grids::end_tile)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
                    NewLevel();
                }
            }
            break;
        }
        break;

        case WM_LBUTTONDOWN:
            if (HIWORD(lParam) <= 50)
            {
                if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
                {
                    if (name_set)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;
                    }

                    if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                    if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &bDlgProc) == IDOK)name_set = true;
                    break;
                }

                if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                    if (sound)
                    {
                        sound = false;
                        PlaySound(NULL, NULL, NULL);
                        break;
                    }
                    else
                    {
                        sound = true;
                        PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                        break;
                    }
                    break;
                }

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
        (int)(scr_width), (int)(scr_height), NULL, NULL, bIns, NULL);
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
    gStop[0].color = D2D1::ColorF(D2D1::ColorF::LightGreen);
    gStop[1].position = 1.0f;
    gStop[1].color = D2D1::ColorF(D2D1::ColorF::AliceBlue);

    hr = Draw->CreateGradientStopCollection(gStop, 2, &gsCol);
    if (hr != S_OK)
    {
        LogError(L"Error creating GradientStopCollection ");
        ErrExit(eD2D);
    }

    if (gsCol)
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

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &FieldBrush);
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
        DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &nrmText);
    if (hr != S_OK)
    {
        LogError(L"Error creating nrmText ");
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_NORMAL, 44.0f, L"", &bigText);
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

    bmpHeal = Load(L".\\res\\img\\life.png", Draw);
    if (!bmpHeal)
    {
        LogError(L"Error loading bmpHeal ");
        ErrExit(eD2D);
    }

    bmpCastle = Load(L".\\res\\img\\castle.png", Draw);
    if (!bmpCastle)
    {
        LogError(L"Error loading bmpCastle ");
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
        if (TxtBrush && bigText)
            Draw->DrawText(show, i, bigText, D2D1::RectF(30.0f, cl_height / 2 - 50.0f, cl_width, cl_height), TxtBrush);
        Draw->EndDraw();
        Sleep(20);
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
                Draw->DrawText(L"ПАУЗА", 6, bigText, D2D1::RectF(200.0f, cl_height / 2 - 50.0f, cl_width, cl_height), TxtBrush);
            Draw->EndDraw();
            continue;
        }
        /////////////////////////////////////////////////////////////////

        //GAME ENGINE *****************************

        //BAD ARMY ********************************

        if (vBadArmy.size() < 5)
        {
            if (rand() % 50 == 6)
            {
                switch (rand() % 4)
                {
                case 0:
                    for (int i = 0; i < 10; i++)
                    {
                        if (Grid[i][0].type == grids::empty)
                        {
                            vBadArmy.push_back(CreatureFactory(creatures::fly, Grid[i][0].x, Grid[i][0].y));
                            vBadArmy.back()->dir = dirs::down;
                            break;
                        }
                    }
                    break;

                case 1:
                    for (int i = 0; i < 10; i++)
                    {
                        if (Grid[i][0].type == grids::empty)
                        {
                            vBadArmy.push_back(CreatureFactory(creatures::fly, Grid[i][9].x, Grid[i][9].y));
                            vBadArmy.back()->dir = dirs::up;
                            break;
                        }
                    }
                    break;

                case 2:
                    for (int i = 0; i < 10; i++)
                    {
                        if (Grid[0][i].type == grids::empty)
                        {
                            if (rand() % 3 == 1)
                                vBadArmy.push_back(CreatureFactory(creatures::walk, Grid[0][i].x, Grid[0][i].y));
                            else
                                vBadArmy.push_back(CreatureFactory(creatures::creep, Grid[0][i].x, Grid[0][i].y));
                            vBadArmy.back()->dir = dirs::right;
                            break;
                        }
                    }
                    break;

                case 3:
                    for (int i = 0; i < 10; i++)
                    {
                        if (Grid[9][i].type == grids::empty)
                        {
                            if (rand() % 3 == 1)
                                vBadArmy.push_back(CreatureFactory(creatures::walk, Grid[9][i].x, Grid[9][i].y));
                            else
                                vBadArmy.push_back(CreatureFactory(creatures::creep, Grid[9][i].x, Grid[9][i].y));
                            vBadArmy.back()->dir = dirs::left;
                            break;
                        }
                    }
                    break;



                }
            }
        }

        if (!vBadArmy.empty())
        {
            for (std::vector<cre_ptr>::iterator bad = vBadArmy.begin(); bad < vBadArmy.end(); ++bad)
            {
                if ((*bad)->killed)continue;
                switch ((*bad)->dir)
                {
                case dirs::down:
                    if ((*bad)->Move() == DL_FAIL)(*bad)->dir = dirs::up;
                    break;

                case dirs::up:
                    if ((*bad)->Move() == DL_FAIL)(*bad)->dir = dirs::down;
                    break;

                case dirs::left:
                    if ((*bad)->Move() == DL_FAIL)(*bad)->dir = dirs::right;
                    break;

                case dirs::right:
                    if ((*bad)->Move() == DL_FAIL)(*bad)->dir = dirs::left;
                    break;

                }
            }
        }

        if (!vBadArmy.empty())
        {
            for (std::vector<cre_ptr>::iterator bad = vBadArmy.begin(); bad < vBadArmy.end(); bad++)
            {
                int my_col = (*bad)->GetCellNum() % 10;
                int my_row = (*bad)->GetCellNum() / 10;

                switch ((*bad)->dir)
                {
                case dirs::down:
                    if (my_row < 9)
                    {
                        if (Grid[my_col][my_row + 1].type != grids::empty)(*bad)->dir = dirs::up;
                    }
                    break;

                case dirs::up:
                    if (my_row > 0)
                    {
                        if (Grid[my_col][my_row - 1].type != grids::empty)(*bad)->dir = dirs::down;
                    }
                    break;

                case dirs::right:
                    if (my_col < 9)
                    {
                        if (Grid[my_col + 1][my_row].type != grids::empty)(*bad)->dir = dirs::left;
                    }
                    break;

                case dirs::left:
                    if (my_col > 0)
                    {
                        if (Grid[my_col - 1][my_row].type != grids::empty)(*bad)->dir = dirs::right;
                    }
                    break;
                }
            }
        }

        if (!vBadArmy.empty())
        {
            for (std::vector<cre_ptr>::iterator bad = vBadArmy.begin(); bad < vBadArmy.end(); ++bad)
            {
                if ((*bad)->dir == dirs::stop)
                {
                    if ((*bad)->horizontal_move)
                    {
                        if (rand() % 2 == 1)(*bad)->dir = dirs::right;
                        else (*bad)->dir = dirs::left;
                    }
                    else
                    {
                        if (rand() % 2 == 1)(*bad)->dir = dirs::up;
                        else (*bad)->dir = dirs::down;
                    }
                }
            }
        }


        //ATACK *************************************

        if (Hero && !vBadArmy.empty())
        {
            for (std::vector<cre_ptr>::iterator bad = vBadArmy.begin(); bad < vBadArmy.end(); bad++)
            {
                if ((*bad)->killed || Hero->killed)continue;

                if (!(Hero->x >= (*bad)->ex || Hero->ex <= (*bad)->x || Hero->y >= (*bad)->ey || Hero->ey <= (*bad)->y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\sword.wav", NULL, NULL, NULL);
                    (*bad)->dir = dirs::stop;
                    (*bad)->lifes -= Hero->Attack();
                    if (rand() % 100 == 66)Hero->lifes -= 50;
                    else Hero->lifes -= (*bad)->Attack();
                    if ((*bad)->lifes <= 0)
                    {
                        score += 100;
                        (*bad)->killed = true;
                        if (sound)mciSendString(L"play .\\res\\snd\\killed.wav", NULL, NULL, NULL);
                    }
                    if (Hero->lifes <= 0)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\hurt.wav", NULL, NULL, NULL);
                        Hero->killed = true;
                        break;
                    }
                }
            }
        }

        if (!Potion && rand() % 400 == 33)
        {
            bool y_found = false;
            bool x_found = false;

            while (!y_found)
            {
                for (int row = 0; row < 10; row++)
                {
                    while (!x_found)
                    {
                        for (int col = 0; col < 10; col++)
                        {
                            if (Grid[col][row].type == grids::empty)
                            {
                                x_found = true;
                                y_found = true;
                                Potion = new PROTON(Grid[col][row].x + 15.0f, Grid[col][row].y + 15.0f, 20.0f, 17.0f);
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (Hero && Potion)
        {
            if (!(Hero->x >= Potion->ex || Hero->ex <= Potion->x || Hero->y >= Potion->ey || Hero->ey <= Potion->y))
            {
                if (sound)mciSendString(L"play .\\res\\snd\\lifes.wav", NULL, NULL, NULL);
                if (Hero->lifes + 30 >= 120)Hero->lifes += 30;
                else Hero->lifes = 120;
                ReleaseCOM(&Potion);
            }
        }

        if (Hero && Castle)
        {
            if (!(Hero->x >= Castle->ex || Hero->ex <= Castle->x || Hero->y >= Castle->ey || Hero->ey <= Castle->y))
            {
                if (sound)mciSendString(L"play .\\res\\snd\\win.wav", NULL, NULL, NULL);
                win_game = true;
                GameOver();
            }
        }


        // DRAW THINGS *******************************
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Azure));

        if (FieldBrush)
            Draw->FillRectangle(D2D1::RectF(0, 0, cl_width, 50.0f), ButBckgBrush);
        if (nrmText && TxtBrush && ButHgltBrush && ButInactBrush)
        {
            if (name_set)
                Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF(b1Rect.left + 5.0f, 0, b1Rect.right, 50.0f),
                    ButInactBrush);
            else
            {
                if (b1Hglt)
                    Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF(b1Rect.left + 5.0f, 0, b1Rect.right, 50.0f),
                        ButHgltBrush);
                else
                    Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF(b1Rect.left + 5.0f, 0, b1Rect.right, 50.0f),
                        TxtBrush);
            }

            if (b2Hglt)
                Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF(b2Rect.left + 5.0f, 0, b2Rect.right, 50.0f),
                    ButHgltBrush);
            else
                Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF(b2Rect.left + 5.0f, 0, b2Rect.right, 50.0f),
                    TxtBrush);

            if (b3Hglt)
                Draw->DrawText(L"ПОМОЩ", 6, nrmText, D2D1::RectF(b3Rect.left + 10.0f, 0, b3Rect.right, 50.0f),
                    ButHgltBrush);
            else
                Draw->DrawText(L"ПОМОЩ", 6, nrmText, D2D1::RectF(b3Rect.left + 10.0f, 0, b3Rect.right, 50.0f),
                    TxtBrush);
        }

        for (int rows = 0; rows < 10; rows++)
        {
            for (int cols = 0; cols < 10; cols++)
            {
                switch (Grid[cols][rows].type)
                {
                case grids::not_used:
                    if (FieldBrush)
                        Draw->FillRectangle(D2D1::RectF(Grid[cols][rows].x, Grid[cols][rows].y,
                            Grid[cols][rows].ex, Grid[cols][rows].ey), FieldBrush);
                    break;

                case grids::empty:
                    Draw->DrawBitmap(bmpTile, D2D1::RectF(Grid[cols][rows].x, Grid[cols][rows].y,
                        Grid[cols][rows].ex, Grid[cols][rows].ey));
                    break;

                case grids::rock:
                    Draw->DrawBitmap(bmpRockTile, D2D1::RectF(Grid[cols][rows].x, Grid[cols][rows].y,
                        Grid[cols][rows].ex, Grid[cols][rows].ey));
                    break;

                case grids::tree:
                    Draw->DrawBitmap(bmpTreeTile, D2D1::RectF(Grid[cols][rows].x, Grid[cols][rows].y,
                        Grid[cols][rows].ex, Grid[cols][rows].ey));
                    break;

                case grids::end_tile:
                    Draw->DrawBitmap(bmpEndTile, D2D1::RectF(Grid[cols][rows].x, Grid[cols][rows].y,
                        Grid[cols][rows].ex, Grid[cols][rows].ey));
                    break;
                }
            }
        }
        ///////////////////////////////////////////////////////////////////

        // HERO *****************************

        if (Hero)
        {
            if (!Hero->killed)
            {
                switch (Hero->dir)
                {
                case dirs::left:
                    Draw->DrawBitmap(bmpHeroL[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                    break;

                case dirs::stop:
                    Draw->DrawBitmap(bmpHeroR[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                    break;

                case dirs::right:
                    Draw->DrawBitmap(bmpHeroR[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                    break;

                case dirs::up:
                    if (hero_prev_dir == dirs::left)
                    {
                        Draw->DrawBitmap(bmpHeroL[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                        break;
                    }
                    else
                    {
                        Draw->DrawBitmap(bmpHeroR[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                        break;
                    }
                    break;

                case dirs::down:
                    if (hero_prev_dir == dirs::left)
                    {
                        Draw->DrawBitmap(bmpHeroL[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                        break;
                    }
                    else
                    {
                        Draw->DrawBitmap(bmpHeroR[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                        break;
                    }
                    break;
                }
            }
            else Draw->DrawBitmap(bmpKill, D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
                
            ID2D1SolidColorBrush* LifeBrush = nullptr;
            if (Hero->lifes > 80)
                Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &LifeBrush);
            else if (Hero->lifes <= 80 && Hero->lifes > 40)
                Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &LifeBrush);
            else
                Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &LifeBrush);

            Draw->DrawLine(D2D1::Point2F(Hero->x, Hero->y - 5.0f), D2D1::Point2F(Hero->x + Hero->lifes / 4, Hero->y - 5.0f), 
                LifeBrush, 5.0f);
            ReleaseCOM(&LifeBrush);
        }
        if (Potion)
            Draw->DrawBitmap(bmpHeal, D2D1::RectF(Potion->x, Potion->y, Potion->ex, Potion->ey));
        if (Castle)
            Draw->DrawBitmap(bmpCastle, D2D1::RectF(Castle->x, Castle->y, Castle->ex, Castle->ey));

        /////////////////////////////////////

        //BAD ARMY *************************

        if (!vBadArmy.empty())
        {
            for (std::vector<cre_ptr>::iterator bad = vBadArmy.begin(); bad < vBadArmy.end(); ++bad)
            {
                if (!(*bad)->killed)
                {
                    switch ((*bad)->GetType())
                    {
                    case creatures::creep:
                        if ((*bad)->dir == dirs::left)
                            Draw->DrawBitmap(bmpCreepL[(*bad)->GetFrame()], D2D1::RectF((*bad)->x, (*bad)->y, (*bad)->ex, (*bad)->ey));
                        else
                            Draw->DrawBitmap(bmpCreepR[(*bad)->GetFrame()], D2D1::RectF((*bad)->x, (*bad)->y, (*bad)->ex, (*bad)->ey));
                        break;

                    case creatures::walk:
                        if ((*bad)->dir == dirs::left)
                            Draw->DrawBitmap(bmpWalkL[(*bad)->GetFrame()], D2D1::RectF((*bad)->x, (*bad)->y, (*bad)->ex, (*bad)->ey));
                        else
                            Draw->DrawBitmap(bmpWalkR[(*bad)->GetFrame()], D2D1::RectF((*bad)->x, (*bad)->y, (*bad)->ex, (*bad)->ey));
                        break;

                    case creatures::fly:
                        Draw->DrawBitmap(bmpFly[(*bad)->GetFrame()], D2D1::RectF((*bad)->x, (*bad)->y, (*bad)->ex, (*bad)->ey));
                        break;
                    }
                }
                else
                {
                    Draw->DrawBitmap(bmpKill, D2D1::RectF((*bad)->x, (*bad)->y, (*bad)->ex, (*bad)->ey));
                    if ((*bad)->Killed() == DL_FAIL)
                    {
                        (*bad)->Release();
                        vBadArmy.erase(bad);
                        break;
                    }
                }
            }
        }
        ////////////////////////////////////

        // TEXT DRAW ***********************

        wchar_t info[100] = L"\0";
        wchar_t add[5] = L"\0";
        int size = 0;

        wcscpy_s(info, current_player);

        wcscat_s(info, L", резултат: ");
        swprintf(add, 5, L"%d", score);
        wcscat_s(info, add);

        for (int i = 0; i < 100; ++i)
        {
            if (info[i] != '\0')size++;
            else break;
        }

        if (bigText && TxtBrush)
            Draw->DrawText(info, size, bigText, D2D1::RectF(5.0f, cl_height - 120.0f, cl_width, cl_height), TxtBrush);

        size = 0;
        swprintf(add, 2, L"%d", minutes);
        wcscpy_s(info, add);
        wcscat_s(info, L" : ");
        swprintf(add, 3, L"%d", seconds - minutes * 60);
        if (seconds - minutes * 60 < 10)wcscat_s(info, L"0");
        wcscat_s(info, add);
        for (int i = 0; i < 100; ++i)
        {
            if (info[i] != '\0')size++;
            else break;
        }

        if (bigText && TxtBrush)
            Draw->DrawText(info, size, bigText, D2D1::RectF(cl_width / 2 - 50, cl_height - 50.0f, cl_width, cl_height), TxtBrush);

        ////////////////////////////////////////////////
        Draw->EndDraw();
        if (Hero)
        {
            if (Hero->killed)
                if (Hero->Killed() == DL_FAIL)GameOver();
        }
    }

    std::remove(tmp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}