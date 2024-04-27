#include "framework.h"
#include "Jungle run.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "FCheck.h"
#include "ErrH.h"
#include "D2BMPLOADER.h"
#include "gorapi.h"
#include <vector>
#include <ctime>
#include <chrono>
#include <fstream>

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"d2d1.lib")
#pragma comment (lib,"dwrite.lib")
#pragma comment (lib,"fcheck.lib")
#pragma comment (lib,"errh.lib")
#pragma comment (lib,"d2bmploader.lib")
#pragma comment (lib,"gorapi.lib")

#define bWinClassName L"Runner"

#define tmp_file ".\\res\\data\\temp.dat"
#define Ltmp_file L".\\res\\data\\temp.dat"
#define hlp_file L".\\res\\data\\help.dat"
#define snd_file L".\\res\\snd\\main.wav"
#define record_file L".\\res\\data\\record.dat"
#define save_file L".\\res\\data\\save.dat"

#define mNew 1001
#define mSpeed 1002
#define mExit 1003
#define mSave 1004
#define mLoad 1005
#define mHoF 1006

WNDCLASS bWin = { 0 };
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HICON mainIcon = nullptr;
HCURSOR mainCursor = nullptr;
HCURSOR outCursor = nullptr;
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
HDC PaintDC = nullptr;
PAINTSTRUCT bPaint = { 0 };
POINT cur_pos = { 0 };
UINT bTimer = -1;

MSG bMsg = { 0 };
BOOL bRet = 0;

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;
ID2D1RadialGradientBrush* ButBckg = nullptr;
ID2D1SolidColorBrush* Txt = nullptr;
ID2D1SolidColorBrush* InactTxt = nullptr;
ID2D1SolidColorBrush* HgltTxt = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmTxt = nullptr;
IDWriteTextFormat* bigTxt = nullptr;

ID2D1Bitmap* bmpField1 = nullptr;
ID2D1Bitmap* bmpField2 = nullptr;
ID2D1Bitmap* bmpBanana = nullptr;
ID2D1Bitmap* bmpExit = nullptr;
ID2D1Bitmap* bmpPlatform = nullptr;
ID2D1Bitmap* bmpSun = nullptr;
ID2D1Bitmap* bmpCloud1 = nullptr;
ID2D1Bitmap* bmpCloud2 = nullptr;
ID2D1Bitmap* bmpDizzy = nullptr;
ID2D1Bitmap* bmpRock = nullptr;

ID2D1Bitmap* bmpHero[6] = { nullptr };
ID2D1Bitmap* bmpGorilla1[35] = { nullptr };
ID2D1Bitmap* bmpGorilla2[4] = { nullptr };
ID2D1Bitmap* bmpGorilla3[24] = { nullptr };

//////////////////////////////////////////////////

bool pause = false;
bool show_help = false;
bool sound = true;
bool in_client = true;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool set_name = false;

int dizzy_cooldown = 0;

wchar_t current_player[16] = L"JUNGLE RUNNER";

D2D1_RECT_F b1Rect = { 0, 0, 200.0f, 50.0f };
D2D1_RECT_F b2Rect = { 250.0f, 0, 450.0f, 50.0f };
D2D1_RECT_F b3Rect = { 500.0f, 0, scr_width, 50.0f };

D2D1_RECT_F b1TxtRect = { 20, 0, 200.0f, 50.0f };
D2D1_RECT_F b2TxtRect = { 270.0f, 0, 450.0f, 50.0f };
D2D1_RECT_F b3TxtRect = { 520.0f, 0, scr_width, 50.0f };

D2D1_RECT_F Field1Rect = { 0,50.0f,scr_width,scr_height };
D2D1_RECT_F Field2Rect = { scr_width,50.0f,scr_width + 700.0f,scr_height };

dll::ATOM Sun(400.0f, 60.0f, 100.0f, 100.0f);
dll::ATOM Cloud1(0, 70.0f, 100.0f, 53.0f);
dll::ATOM Cloud2(scr_width, 80.0f, 100.0f, 52.0f);

int score = 0;
int speed = 1;
int minutes = 0;
int seconds = 300;

// CREATURES *****************************************

dll::creat_ptr Hero = nullptr;
std::vector<dll::creat_ptr> vGorillas;

std::vector<dll::ATOM>vRocks;


//////////////////////////////////////////////////////

template <typename Obj> bool ClearIt(Obj** what)
{
    if ((*what))
    {
        (*what)->Release();
        (*what) = nullptr;
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream err_file(L".\\res\\data\\error.log", std::ios::app);
    err_file << what << L" ! Time stamp: " << std::chrono::system_clock::now();
    err_file.close();
}
void ReleaseCOM()
{
    ClearIt(&iFactory);
    ClearIt(&Draw);
    ClearIt(&ButBckg);
    ClearIt(&Txt);
    ClearIt(&InactTxt);
    ClearIt(&HgltTxt);
    ClearIt(&iWriteFactory);
    ClearIt(&nrmTxt);
    ClearIt(&bigTxt);
    ClearIt(&bmpDizzy);
    ClearIt(&bmpRock);

    ClearIt(&bmpBanana);
    ClearIt(&bmpExit);
    ClearIt(&bmpField1);
    ClearIt(&bmpField2);
    ClearIt(&bmpPlatform);
    ClearIt(&bmpSun);
    ClearIt(&bmpCloud1);
    ClearIt(&bmpCloud2);
    for (int i = 0; i < 6; ++i)ClearIt(&bmpHero[i]);
    for (int i = 0; i < 35; ++i)ClearIt(&bmpGorilla1[i]);
    for (int i = 0; i < 4; ++i)ClearIt(&bmpGorilla2[i]);
    for (int i = 0; i < 24; ++i)ClearIt(&bmpGorilla3[i]);
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_ICONERROR | MB_APPLMODAL | MB_OK);

    std::remove(tmp_file);
    ReleaseCOM();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    score = 0;
    speed = 1;
    minutes = 0;
    seconds = 300;
    wcscpy_s(current_player, L"JUNGLE RUNNER");
    set_name = false;
    dizzy_cooldown = 0;
    vRocks.clear();

    ClearIt(&Hero);
    if (!vGorillas.empty())
    {
        for (int i = 0; i < vGorillas.size(); ++i)ClearIt(&vGorillas[i]);
    }
    vGorillas.clear();

    Hero = dll::iFactory(dll::types::hero, scr_height - 180.0f);
}

INT_PTR CALLBACK bDlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)mainIcon);
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
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 15) < 1)
            {
                wcscpy_s(current_player, L"JUNGLE RUNNER");
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
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
        SetTimer(hwnd, bTimer, 1000, NULL);
        srand((unsigned int)(time(0)));

        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();

        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

        AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
        AppendMenu(bMain, MF_STRING, mSpeed, L"Скорост на играта");
        AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bMain, MF_STRING, mExit, L"Изход");

        AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
        AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
        AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");
        SetMenu(hwnd, bBar);
        InitGame();
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nИзлизаш ли ?",
            L"Изход ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(50, 50, 50)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_TIMER:
        if (pause)break;
        seconds--;
        if (dizzy_cooldown > 0)dizzy_cooldown--;
        minutes = (int)(floor(seconds / 60));
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                pause = false;
                in_client = true;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        b1Hglt = true;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                    if (b2Hglt || b3Hglt)
                    {
                        b2Hglt = false;
                        b3Hglt = false;
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
                    if (b1Hglt || b3Hglt)
                    {
                        b1Hglt = false;
                        b3Hglt = false;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        b3Hglt = true;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                    if (b1Hglt || b2Hglt)
                    {
                        b1Hglt = false;
                        b2Hglt = false;
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    }
                }

                SetCursor(outCursor);
                return true;
            }
            else
            {
                if (b1Hglt || b2Hglt || b3Hglt)
                {
                    b1Hglt = false;
                    b2Hglt = false;
                    b3Hglt = false;
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                }
                SetCursor(mainCursor);
                return true;
            }

        }
        else
        {
            if (in_client)
            {
                pause = true;
                in_client = false;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
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
            if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nРестартираш ли ?",
                L"Рестарт ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
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

    case WM_KEYDOWN:
        if (dizzy_cooldown > 0)break;
        if (Hero)
        {
            if(Hero->dir != dll::dirs::up && Hero->dir!=dll::dirs::down)
                switch (LOWORD(wParam))
                {
                case VK_LEFT:
                    Hero->dir = dll::dirs::left;
                    break;

                case VK_RIGHT:
                    Hero->dir = dll::dirs::right;
                    break;

                case VK_UP:
                    Hero->Jump();
                    break;

                case VK_DOWN:
                    Hero->dir = dll::dirs::stop;
                    break;
                }
        }
        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return(LRESULT)(FALSE);
}
////////////////////////////////////////////////////////

void CreateResources()
{
    int result = 0;
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream start(Ltmp_file);
        start << L"Igrata distvuva";
        start.close();
    }

    int window_x = static_cast<int>((int)(GetSystemMetrics(SM_CXSCREEN) / 2) - (int)(scr_width / 2));
    int window_y = 100;

    if (GetSystemMetrics(SM_CXSCREEN) < window_x + scr_width || GetSystemMetrics(SM_CYSCREEN) < window_y + scr_height)
        ErrExit(eScreen);
    
    mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 48, 48, LR_LOADFROMFILE));
    if (!mainIcon)ErrExit(eIcon);

    mainCursor = LoadCursorFromFileW(L".\\res\\main.ani");
    outCursor = LoadCursorFromFileW(L".\\res\\out.ani");
    if (!mainCursor || !outCursor)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &bWinProc;
    bWin.hbrBackground = CreateSolidBrush(RGB(50, 50, 50));
    bWin.hIcon = mainIcon;
    bWin.hCursor = mainCursor;
    bWin.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindowW(bWinClassName, L"Прибиране в къщи !", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, window_x, window_y,
        (int)(scr_width), (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else ShowWindow(bHwnd, SW_SHOWDEFAULT);

    HRESULT hr = S_OK;
    D2D1_GRADIENT_STOP gStop[2] = { 0 };
    ID2D1GradientStopCollection* StopCol = nullptr;
    
    
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 Factory");
        ErrExit(eD2D);
    }

    if (iFactory)
        hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(bHwnd, D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 HwndRenderTarget");
        ErrExit(eD2D);
    }

    gStop[0].position = 0;
    gStop[0].color = D2D1::ColorF(D2D1::ColorF::GreenYellow);
    gStop[1].position = 1.0f;
    gStop[1].color = D2D1::ColorF(D2D1::ColorF::DarkGreen);

    hr = Draw->CreateGradientStopCollection(gStop, 2, &StopCol);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 GradientStop Collection");
        ErrExit(eD2D);
    }
    if (StopCol)
        hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
            D2D1::Point2F(0, 0), scr_width / 2, 25.0f), StopCol, &ButBckg);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 RadialGradientBrush");
        ErrExit(eD2D);
    }
    ClearIt(&StopCol);

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), &Txt);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 Txt Brush");
        ErrExit(eD2D);
    }
    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &HgltTxt);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 HgltTxt Brush");
        ErrExit(eD2D);
    }
    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &InactTxt);
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 InactTxt Brush");
        ErrExit(eD2D);
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
    if (hr != S_OK)
    {
        LogError(L"Error creating D2D1 Write Factory");
        ErrExit(eD2D);
    }
    if (iWriteFactory)
    {
        hr = iWriteFactory->CreateTextFormat(L"Gabriola", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
            DWRITE_FONT_STRETCH_NORMAL, 18, L"", &nrmTxt);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 nrmTxt");
            ErrExit(eD2D);
        }
        hr = iWriteFactory->CreateTextFormat(L"Gabriola", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
            DWRITE_FONT_STRETCH_NORMAL, 64, L"", &bigTxt);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 bigTxt");
            ErrExit(eD2D);
        }
    }
    /////////////////////////////////////////////////////////////////////////////

    bmpField1 = Load(L".\\res\\field1.png", Draw);
    if (!bmpField1)
    {
        LogError(L"Error loading Field1");
        ErrExit(eD2D);
    }
    bmpField2 = Load(L".\\res\\field2.png", Draw);
    if (!bmpField2)
    {
        LogError(L"Error loading Field2");
        ErrExit(eD2D);
    }
    bmpBanana = Load(L".\\res\\img\\banana.png", Draw);
    if (!bmpBanana)
    {
        LogError(L"Error loading Banana");
        ErrExit(eD2D);
    }
    bmpExit = Load(L".\\res\\img\\exit.png", Draw);
    if (!bmpExit)
    {
        LogError(L"Error loading exit");
        ErrExit(eD2D);
    }
    bmpPlatform = Load(L".\\res\\img\\platform.png", Draw);
    if (!bmpPlatform)
    {
        LogError(L"Error loading Platform");
        ErrExit(eD2D);
    }
    bmpSun = Load(L".\\res\\img\\sun.png", Draw);
    if (!bmpSun)
    {
        LogError(L"Error loading Sun");
        ErrExit(eD2D);
    }
    bmpCloud1 = Load(L".\\res\\img\\cloud1.png", Draw);
    if (!bmpCloud1)
    {
        LogError(L"Error loading Cloud1");
        ErrExit(eD2D);
    }
    bmpCloud2 = Load(L".\\res\\img\\cloud2.png", Draw);
    if (!bmpCloud2)
    {
        LogError(L"Error loading Cloud2");
        ErrExit(eD2D);
    }
    bmpDizzy = Load(L".\\res\\img\\dizzy.png", Draw);
    if (!bmpDizzy)
    {
        LogError(L"Error loading Dizzy");
        ErrExit(eD2D);
    }
    bmpRock = Load(L".\\res\\img\\rock.png", Draw);
    if (!bmpRock)
    {
        LogError(L"Error loading Rock");
        ErrExit(eD2D);
    }

    for (int i = 0; i < 6; i++)
    {
        wchar_t path[100] = L".\\res\\img\\hero\\";
        wchar_t name[3] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpHero[i] = Load(path, Draw);

        if (!bmpHero[i])
        {
            wchar_t err_text[75] = L"Error loading bmpHero - frame number: ";
            wcscat_s(err_text, name);
            LogError(err_text);
            ErrExit(eD2D);
        }
    }
    for (int i = 0; i < 35; i++)
    {
        wchar_t path[100] = L".\\res\\img\\gorilla1\\";
        wchar_t name[3] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpGorilla1[i] = Load(path, Draw);

        if (!bmpGorilla1[i])
        {
            wchar_t err_text[75] = L"Error loading bmpGorilla1 - frame number: ";
            wcscat_s(err_text, name);
            LogError(err_text);
            ErrExit(eD2D);
        }
    }
    for (int i = 0; i < 4; i++)
    {
        wchar_t path[100] = L".\\res\\img\\gorilla2\\";
        wchar_t name[3] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpGorilla2[i] = Load(path, Draw);

        if (!bmpGorilla2[i])
        {
            wchar_t err_text[75] = L"Error loading bmpGorilla2 - frame number: ";
            wcscat_s(err_text, name);
            LogError(err_text);
            ErrExit(eD2D);
        }
    }
    for (int i = 0; i < 24; i++)
    {
        wchar_t path[100] = L".\\res\\img\\gorilla3\\";
        wchar_t name[3] = L"\0";
        wsprintf(name, L"%d", i);
        wcscat_s(path, name);
        wcscat_s(path, L".png");

        bmpGorilla3[i] = Load(path, Draw);

        if (!bmpGorilla3[i])
        {
            wchar_t err_text[75] = L"Error loading bmpGorilla3 - frame number: ";
            wcscat_s(err_text, name);
            LogError(err_text);
            ErrExit(eD2D);
        }
    }
    ////////////////////////////////////////////////////////

    D2D1_RECT_F Rup_text = { 75.0f,-50.0f,scr_width,50.0f };
    D2D1_RECT_F Rdown_text = { 75.0f,scr_height,scr_width,scr_height + 50.0f };

    bool up_in_place = false;
    bool down_in_place = false;

    while (!up_in_place || !down_in_place)
    {
        if (!up_in_place)
        {
            Rup_text.top += 0.8f;
            Rup_text.bottom += 0.8f;
            if (Rup_text.bottom >= scr_height / 2)up_in_place = true;
        }
        if (!down_in_place)
        {
            Rdown_text.top -= 0.8f;
            Rdown_text.bottom -= 0.8f;
            if (Rdown_text.top <= scr_height / 2 + 50.0f)down_in_place = true;
        }
        if (Txt && bigTxt)
        {
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
            Draw->DrawTextW(L"БЕСНИ ГОРИЛИ !", 15, bigTxt, Rup_text, Txt);
            Draw->DrawTextW(L"dev. Daniel !", 14, bigTxt, Rdown_text, Txt);
            Draw->EndDraw();
        }
    }
    Sleep(2000);
}



//////////////////////////////////////////////////


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"hInstance is nullptr");
        ErrExit(eClass);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessageW(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkKhaki));
            if (Txt && bigTxt)
                Draw->DrawTextW(L"ПАУЗА", 6, bigTxt, D2D1::RectF(scr_width / 2 - 50.0f, scr_height / 2 - 50.0f,
                    scr_width, scr_height), Txt);
            Draw->EndDraw();
            continue;
        }

        ///////////////////////////////////////////////////////////

        if (vRocks.size() < 2 && rand() % 100 == 6)
        {
            vRocks.push_back(dll::ATOM(scr_width, scr_height - 138, 50.0f, 38.0f));
        }

        // HERO ******************************

        if (Hero)
        {
            if (Hero->dir == dll::dirs::up || Hero->dir == dll::dirs::down)Hero->Jump();
            else Hero->Move((float)(speed));
        }



        /////////////////////////////////////









        ////////////////////////////////////////////////////////

        // DRAW THINGS ****************************************

        Draw->BeginDraw();
        if (ButBckg && Txt && HgltTxt && InactTxt && nrmTxt)
        {
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), ButBckg);
            if (set_name) Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, InactTxt);
            else
            {
                if(!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, Txt);
                else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, HgltTxt);
            }
            if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTxt, b2TxtRect, Txt);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTxt, b2TxtRect, HgltTxt);
            if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTxt, b3TxtRect, Txt);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTxt, b3TxtRect, HgltTxt);
        }

        Draw->DrawBitmap(bmpField1, Field1Rect);
        Draw->DrawBitmap(bmpField2, Field2Rect);
        if (Hero)
        {
            if (Hero->dir != dll::dirs::stop)
            {
                Field1Rect.left -= speed;
                Field1Rect.right -= speed;
                if (Field1Rect.right <= 0)
                {
                    Field1Rect.left = Field2Rect.right;
                    Field1Rect.right = Field1Rect.left + 700.0f;
                }
                Field2Rect.left -= speed;
                Field2Rect.right -= speed;
                if (Field2Rect.right <= 0)
                {
                    Field2Rect.left = Field1Rect.right;
                    Field2Rect.right = Field2Rect.left + 700.0f;
                }

                if (vRocks.size() > 0)
                {
                    for (int i = 0; i < vRocks.size(); i++)
                    {
                        vRocks[i].x -= speed;
                        vRocks[i].SetEdges();
                        if (!(Hero->x >= vRocks[i].ex || Hero->ex <= vRocks[i].x || Hero->y >= vRocks[i].ey || Hero->ey <= vRocks[i].y))
                        {
                            vRocks.erase(vRocks.begin() + i);
                            dizzy_cooldown = 3;
                            if (Hero->dir != dll::dirs::up && Hero->dir != dll::dirs::down)Hero->dir = dll::dirs::stop;
                            break;
                        }
                        if (vRocks[i].ex <= 0)
                        {
                            vRocks.erase(vRocks.begin() + i);
                            break;
                        }
                    }
                }
            }
            
        }
        Cloud1.x += 0.5f;
        Cloud1.SetEdges();
        Cloud2.x -= 0.3f;
        Cloud2.SetEdges();
        if (Cloud1.x >= scr_width)
        {
            Cloud1.x = -100.0f;
            Cloud1.SetEdges();
        }
        if (Cloud2.ex <= 0)
        {
            Cloud2.x = scr_width;
            Cloud2.SetEdges();
        }
        Draw->DrawBitmap(bmpSun, D2D1::RectF(Sun.x, Sun.y, Sun.ex, Sun.ey));
        Draw->DrawBitmap(bmpCloud1, D2D1::RectF(Cloud1.x, Cloud1.y, Cloud1.ex, Cloud1.ey));
        Draw->DrawBitmap(bmpCloud2, D2D1::RectF(Cloud2.x, Cloud2.y, Cloud2.ex, Cloud2.ey));
        ////////////////////////////////////////////////////////////

        if (Hero)
        {
            if(Hero->dir==dll::dirs::stop)
                Draw->DrawBitmap(bmpHero[3], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
            else Draw->DrawBitmap(bmpHero[Hero->GetFrame()], D2D1::RectF(Hero->x, Hero->y, Hero->ex, Hero->ey));
            if (dizzy_cooldown > 0)Draw->DrawBitmap(bmpDizzy, D2D1::RectF(Hero->x, Hero->y - 25.0f, Hero->x + 50.0f, 
                Hero->y + 23.0f));
        }
        if (vRocks.size() > 0)
        {
            for (int i = 0; i < vRocks.size(); i++)
                Draw->DrawBitmap(bmpRock, D2D1::RectF(vRocks[i].x, vRocks[i].y, vRocks[i].ex, vRocks[i].ey));
        
        }

        ////////////////////////////////////////////////////////

        Draw->EndDraw();

    }

    std::remove(tmp_file);
    ReleaseCOM();
    return (int) bMsg.wParam;
}