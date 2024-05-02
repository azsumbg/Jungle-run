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

#define record 2001
#define first_record 2002
#define no_record 2003

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
IDWriteTextFormat* statusTxt = nullptr;

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
ID2D1Bitmap* bmpRoger = nullptr;

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
bool on_platform = true;
bool hero_killed = false;
bool win_game = false;

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
int bananas = 3;
int killed_delay = 7;

float roger_x = 0;
float roger_y = 0;

// CREATURES *****************************************

dll::creat_ptr Hero = nullptr;
std::vector<dll::creat_ptr> vGorillas;

std::vector<dll::ATOM>vRocks;
std::vector<dll::ATOM>vPlatforms;
std::vector<dll::ATOM>vBananas;
std::vector<dll::ATOM>vShots;
dll::ATOM* Exit = nullptr;

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
    ClearIt(&statusTxt);
    ClearIt(&bmpDizzy);
    ClearIt(&bmpRock);
    ClearIt(&bmpRoger);

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
    else
    {
        std::wifstream rec(record_file);
        rec >> result;
        rec.close();
    }

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
    score += bananas * 10 * speed;

    wchar_t final_txt[35] = L"О, О, О ! ЗАГУБИ !";
    int txt_size = 19;

    switch (CheckRecord())
    {
    case no_record:
        if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_ASYNC);
        break;

    case first_record:
        if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_ASYNC);
        wcscpy_s(final_txt, L"ПЪРВИ РЕКОРД НА ИГРАТА !");
        txt_size = 25;
        break;

    case record:
        if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_ASYNC);
        wcscpy_s(final_txt, L"НОВ СВЕТОВЕН РЕКОРД НА ИГРАТА !");
        txt_size = 32;
        break;
    }

    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkGreen));
    if (bigTxt && InactTxt)
        Draw->DrawTextW(final_txt, txt_size, bigTxt, D2D1::RectF(20.0f, scr_height / 2 - 100.0f, scr_width, scr_height), InactTxt);
    Draw->EndDraw();
    Sleep(6500);

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    score = 0;
    speed = 1;
    minutes = 0;
    seconds = 300;
    bananas = 3;

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
    vPlatforms.clear();
    vBananas.clear();
    vShots.clear();

    Hero = dll::iFactory(dll::types::hero, scr_height - 180.0f);
    
    if (Exit)
    {
        delete Exit;
        Exit = nullptr;
    }
    win_game = false;

}
void SaveGame()
{
    int result = 0;
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)MessageBeep(MB_ICONASTERISK);
        if (MessageBox(bHwnd, L"Съществува записана игра, която ще загубиш !\n\nДа я презапиша ли ?",
            L"Презапис ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }

    std::wofstream save(save_file);

    save << score << std::endl;
    save << bananas << std::endl;
    save << seconds << std::endl;
    for (int i = 0; i < 16; i++)save << static_cast<int>(current_player[i]) << std::endl;
    save << set_name << std::endl;
    save << dizzy_cooldown << std::endl;
    
    if (!Hero)save << 0 << std::endl;
    else save << Hero->x << std::endl;

    save << vRocks.size() << std::endl;
    if (!vRocks.empty())
        for (int i = 0; i < vRocks.size(); i++)save << vRocks[i].x << std::endl;

    save << vPlatforms.size() << std::endl;
    if (!vPlatforms.empty())
        for (int i = 0; i < vPlatforms.size(); i++)save << vRocks[i].x << std::endl;

    save << vBananas.size() << std::endl;
    if (!vBananas.empty())
        for (int i = 0; i < vBananas.size(); i++)
        {
            save << vBananas[i].x << std::endl;
            save << vBananas[i].y << std::endl;
        }

    save << win_game << std::endl;

    if (!Exit)save << 0 << std::endl;
    else save << Exit->x << std::endl;

    save << hero_killed << std::endl;
    if (hero_killed)
    {
        save << roger_x << std::endl;
        save << roger_y << std::endl;
        save << killed_delay << std::endl;
    }

    save << vGorillas.size() << std::endl;
    if (!vGorillas.empty())
    {
        for (int i = 0; i < vGorillas.size(); i++)
        {
            save << static_cast<int>(vGorillas[i]->GetType()) << std::endl;
            save << vGorillas[i]->x << std::endl;
        }
    }

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е запазена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
    int result = 0;
    CheckFile(save_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        MessageBox(bHwnd, L"Все още няма записана игра !\n\nПостарай се повече !",
            L"Липсва файл !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        if (sound)MessageBeep(MB_ICONASTERISK);
        if (MessageBox(bHwnd, L"Настоящата игра ще бъде загубена !\n\nДа я презапиша ли ?",
            L"Презапис ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }
    ////////////////////////////////////////
    vRocks.clear();

    ClearIt(&Hero);

    if (!vGorillas.empty())
    {
        for (int i = 0; i < vGorillas.size(); ++i)ClearIt(&vGorillas[i]);
    }
    vGorillas.clear();
    vPlatforms.clear();
    vBananas.clear();
    vShots.clear();

    if (Exit)
    {
        delete Exit;
        Exit = nullptr;
    }
    ///////////////////////////////////////

    std::wifstream save(save_file);
    float tempx = 0;
    float tempy = 0;
    result = 0;

    save >> score;
    save >> bananas;
    save >> seconds;
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;
        save >> letter;
        current_player[i] = static_cast<wchar_t>(letter);
    }
    save >> set_name;
    save >> dizzy_cooldown;

    save >> tempx;
    if (tempx == 0)
    {
        hero_killed = 0;
        killed_delay = 7;
    }
    else
    {
        Hero = dll::iFactory(dll::types::hero, scr_height - 180.0f);
        Hero->x = tempx;
        Hero->SetEdges();
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            save >> tempx;
            vRocks.push_back(dll::ATOM(tempx, scr_height - 138, 50.0f, 38.0f));
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            save >> tempx;
            vPlatforms.push_back(dll::ATOM(tempx, scr_height - 250.0f, 100.0f, 17.0f));
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            save >> tempx;
            save >> tempy;
            vBananas.push_back(dll::ATOM(tempx, tempy, 20.0f, 13.0f));
        }
    }

    save >> win_game;

    save >> tempx;
    if (tempx > 0)Exit = new dll::ATOM(tempx, scr_height - 170.0f, 70.0f, 70.0f);

    save >> hero_killed;
    if (hero_killed)
    {
        save >> roger_x;
        save >> roger_y;
        save >> killed_delay;
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; i++)
        {
            int atype = -1;
            save >> atype;
            save >> tempx;

            dll::types gorilla_type = static_cast<dll::types>(atype);

            switch (gorilla_type)
            {
            case dll::types::gorilla1:
                vGorillas.push_back(dll::iFactory(dll::types::gorilla1, scr_height - 170.0f));
                break;

            case dll::types::gorilla2:
                vGorillas.push_back(dll::iFactory(dll::types::gorilla2, scr_height - 200.0f));
                break;

            case dll::types::gorilla3:
                vGorillas.push_back(dll::iFactory(dll::types::gorilla3, scr_height - 190.0f));
                break;
            }

            vGorillas.back()->x = tempx;
            vGorillas.back()->SetEdges();
        }
    }

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е заредена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void HallOfFame()
{
    wchar_t status[200] = L"НАЙ-ДОБЪР БЕГАЧ: ";
    wchar_t add[5] = L"\0";
    wchar_t saved_player[16] = L"\0";
    int result = 0;

    CheckFile(record_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)MessageBeep(MB_ICONEXCLAMATION);
        MessageBox(bHwnd, L"Все още няма рекорд на играта !\n\nПостарай се повече !",
            L"Липсва файл !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    std::wifstream rec(record_file);
    rec >> result;
    wsprintf(add, L"%d", result);
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;
        rec >> letter;
        saved_player[i] = static_cast<wchar_t>(letter);
    }
    wcscat_s(status, saved_player);
    wcscat_s(status, L"\n\nСВЕТОВЕН РЕКОРД: ");
    wcscat_s(status, add);
    result = 0;
    for (int i = 0; i < 200; i++)
    {
        if (status[i] != '\0')result++;
        else break;
    }
    if (sound)mciSendString(L"play .\\res\\snd\\tada.wav", NULL, NULL, NULL);

    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkGreen));
    if (bigTxt && InactTxt)
        Draw->DrawTextW(status, result, statusTxt, D2D1::RectF(100.0f, scr_height / 2 - 100.0f, scr_width, scr_height), InactTxt);
    Draw->EndDraw();
    Sleep(3000);
}
void ShowHelp()
{
    int result = 0;
    CheckFile(hlp_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)MessageBeep(MB_ICONERROR);
        MessageBox(bHwnd, L"Грешка при зареждане на информацията !\n\nСвържете се с разработчика !",
            L"Липсва файл !", MB_OK | MB_APPLMODAL | MB_ICONERROR);
        return;
    }

    wchar_t showtxt[1000] = L"\0";
    int txt_size = 0;
    std::wifstream hlp(hlp_file);
    hlp >> txt_size;
    for (int i = 0; i < txt_size; i++)
    {
        int letter = 0;
        hlp >> letter;
        showtxt[i] = static_cast<wchar_t>(letter);
    }

    if (sound)mciSendString(L"play .\\res\\snsd\\tada.wav", NULL, NULL, NULL);
    
    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkOliveGreen));
    if (ButBckg && Txt && HgltTxt && InactTxt && nrmTxt)
    {
        Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), ButBckg);
        if (set_name) Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, InactTxt);
        else
        {
            if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, Txt);
            else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, HgltTxt);
        }
        if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTxt, b2TxtRect, Txt);
        else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTxt, b2TxtRect, HgltTxt);
        if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTxt, b3TxtRect, Txt);
        else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTxt, b3TxtRect, HgltTxt);
    }
    if (InactTxt && statusTxt)
        Draw->DrawTextW(showtxt, txt_size, statusTxt, D2D1::RectF(100.0f, 100.0f, scr_width, scr_height), InactTxt);
    Draw->EndDraw();
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
        if (hero_killed)killed_delay--;
        if (killed_delay <= 0)GameOver();
        if (win_game)break;
        if (seconds > 0)seconds--;
        else
        {
            win_game = true;
            Exit = new dll::ATOM(scr_width, scr_height - 170.0f, 70.0f, 70.0f);
        }
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

        case mSpeed:
            pause = true;
            if (sound)MessageBeep(MB_ICONEXCLAMATION);
            if (MessageBox(hwnd, L"Сигурен ли си, че увеличаваш скоростта ?",
                L"Турбо !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            else speed++;
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
            HallOfFame();
            pause = false;
            break;
        }
        break;

    case WM_KEYDOWN:
        if (dizzy_cooldown > 0)break;
        if (Hero)
        {
            if(Hero->dir != dll::dirs::up && Hero->dir!=dll::dirs::down && Hero->dir != dll::dirs::fall)
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

                case VK_SHIFT:
                    if (bananas < 1 || dizzy_cooldown > 0)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;
                    }
                    bananas--;
                    if (Hero)
                        vShots.push_back(dll::ATOM(Hero->ex, Hero->y + 30.0f, 20.0f, 13.0f));
                    if (sound)mciSendString(L"play .\\res\\snd\\splash.wav", NULL, NULL, NULL);
                    break;

                }
        }
        break;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) < 50)
        {
            if (LOWORD(lParam) >= b1TxtRect.left && LOWORD(lParam) <= b1TxtRect.right)
            {
                if (set_name)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                    break;
                }
                
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);

                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &bDlgProc) == IDOK)set_name = true;
                break;
            }
            if (LOWORD(lParam) >= b2TxtRect.left && LOWORD(lParam) <= b2TxtRect.right)
            {
                mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(snd_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
            if (LOWORD(lParam) >= b3TxtRect.left && LOWORD(lParam) <= b3TxtRect.right)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
            
                if (!show_help)
                {
                    show_help = true;
                    pause = true;
                    ShowHelp();
                    break;
                }
                else
                {
                    show_help = false;
                    pause = false;
                    break;
                }
            }
        }
        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return(LRESULT)(FALSE);
}

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
        hr = iWriteFactory->CreateTextFormat(L"Gabriola", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL, 28, L"", &statusTxt);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 statusTxt");
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
    bmpRoger = Load(L".\\res\\img\\hollyroger.png", Draw);
    if (!bmpRoger)
    {
        LogError(L"Error loading HollyRoger");
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

    D2D1_RECT_F Rup_text = { 100.0f,-20.0f,scr_width,50.0f };
    D2D1_RECT_F Rdown_text = { 150.0f,scr_height,scr_width,scr_height + 50.0f };

    bool up_in_place = false;
    bool down_in_place = false;

    mciSendString(L"play .\\res\\snd\\intro.wav", NULL, NULL, NULL);
    while (!up_in_place || !down_in_place)
    {
        if (!up_in_place)
        {
            Rup_text.top += 1.2f;
            Rup_text.bottom += 1.2f;
            if (Rup_text.bottom >= scr_height / 2)up_in_place = true;
        }
        if (!down_in_place)
        {
            Rdown_text.top -= 1.2f;
            Rdown_text.bottom -= 1.2f;
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
    Sleep(1000);
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

        if (hero_killed)
        {
            Draw->BeginDraw();
            if (ButBckg && Txt && HgltTxt && InactTxt && nrmTxt)
            {
                Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), ButBckg);
                if (set_name) Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, InactTxt);
                else
                {
                    if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, Txt);
                    else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmTxt, b1TxtRect, HgltTxt);
                }
                if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTxt, b2TxtRect, Txt);
                else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmTxt, b2TxtRect, HgltTxt);
                if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTxt, b3TxtRect, Txt);
                else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmTxt, b3TxtRect, HgltTxt);
            }

            Draw->DrawBitmap(bmpField1, Field1Rect);
            Draw->DrawBitmap(bmpField2, Field2Rect);

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

            Draw->DrawBitmap(bmpRoger, D2D1::RectF(roger_x, roger_y, roger_x + 50.0f, roger_y + 50.0f));

            if (vRocks.size() > 0)
            {
                for (int i = 0; i < vRocks.size(); i++)
                    Draw->DrawBitmap(bmpRock, D2D1::RectF(vRocks[i].x, vRocks[i].y, vRocks[i].ex, vRocks[i].ey));

            }
            if (vPlatforms.size() > 0)
            {
                for (int i = 0; i < vPlatforms.size(); ++i)
                    Draw->DrawBitmap(bmpPlatform, D2D1::RectF(vPlatforms[i].x, vPlatforms[i].y,
                        vPlatforms[i].ex, vPlatforms[i].ey));
            }
            if (!vBananas.empty())
            {
                for (std::vector<dll::ATOM>::iterator it = vBananas.begin(); it < vBananas.end(); it++)
                    Draw->DrawBitmap(bmpBanana, D2D1::RectF(it->x, it->y, it->ex, it->ey));

            }
            if (!vGorillas.empty())
            {
                for (std::vector<dll::creat_ptr>::iterator it = vGorillas.begin(); it < vGorillas.end(); ++it)
                {
                    switch ((*it)->GetType())
                    {
                    case dll::types::gorilla1:
                        Draw->DrawBitmap(bmpGorilla1[(*it)->GetFrame()], D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                        break;

                    case dll::types::gorilla2:
                        Draw->DrawBitmap(bmpGorilla2[(*it)->GetFrame()], D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                        break;

                    case dll::types::gorilla3:
                        Draw->DrawBitmap(bmpGorilla3[(*it)->GetFrame()], D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                        break;
                    }
                }
            }

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
            if (Hero->dir == dll::dirs::left || Hero->dir == dll::dirs::right)
            {
                on_platform = false;
                if (Hero->y >= scr_height - 180.0f)
                {
                    Hero->y = scr_height - 180.0f;
                    Hero->SetEdges();
                    on_platform = true;
                    
                }
                if (!vPlatforms.empty())
                {
                    for (int i = 0; i < vPlatforms.size(); ++i)
                    {
                        if (!(Hero->x > vPlatforms[i].ex || Hero->ex < vPlatforms[i].x
                            || Hero->y > vPlatforms[i].ey || Hero->ey < vPlatforms[i].y))
                        {
                            Hero->SetEdges();
                            on_platform = true;
                                          
                            break;
                        }
                    }
                }
            }
            else
            {
                if ((Hero->dir == dll::dirs::stop || Hero->dir == dll::dirs::down || Hero->dir == dll::dirs::fall)
                    && Hero->y < scr_height - 180.0f)
                {
                    on_platform = false;
                    if (!vPlatforms.empty())
                    {
                        for (int i = 0; i < vPlatforms.size(); ++i)
                        {
                            if (!(Hero->x > vPlatforms[i].ex || Hero->ex < vPlatforms[i].x
                                || Hero->y > vPlatforms[i].ey || Hero->ey < vPlatforms[i].y))
                            {
                                Hero->SetEdges();
                                on_platform = true;
                                Hero->dir = dll::dirs::stop;
                                break;
                            }
                        }
                    }
                }
            }
            if (!on_platform) Hero->dir = dll::dirs::fall;
            
        }
        

        if (Hero)
        {
            if (Hero->dir == dll::dirs::up || Hero->dir == dll::dirs::down) Hero->Jump();
            else if (Hero->dir == dll::dirs::left || Hero->dir == dll::dirs::right || Hero->dir == dll::dirs::stop)
                Hero->Move((float)(speed));
            else if (Hero->dir == dll::dirs::fall)
            {
                Hero->Fall((float)(speed));
                if (Hero->y >= scr_height - 180.0f)
                {
                    Hero->y = scr_height - 180.0f;
                    Hero->SetEdges();
                    Hero->dir = dll::dirs::stop;
                    on_platform = true;
                    
                }
            }
        }

        if (Hero && !vGorillas.empty())
        {
            for (std::vector<dll::creat_ptr>::iterator gor = vGorillas.begin(); gor < vGorillas.end(); ++gor)
            {
                if (!(Hero->x > (*gor)->ex || Hero->ex < (*gor)->x || Hero->y >(*gor)->ey || Hero->ey < (*gor)->y))
                {
                    roger_x = Hero->x + 20.0f;
                    roger_y = Hero->y + 20.0f;
                    hero_killed = true;
                    if (sound)mciSendString(L"play .\\res\\snd\\killed.wav", NULL, NULL, NULL);
                    ClearIt(&Hero);
                    break;
                }
            }
        }

        //PLATFROMS ************************

        if (vPlatforms.size() < 2 && rand() % 200 == 66)
            vPlatforms.push_back(dll::ATOM(scr_width, scr_height - 250.0f, 100.0f, 17.0f));

        if (!vPlatforms.empty())
        {
            if (Hero)
            {
                if (Hero->dir != dll::dirs::stop)
                {
                    for (std::vector<dll::ATOM>::iterator it = vPlatforms.begin(); it < vPlatforms.end(); it++)
                    {

                        it->x -= speed;
                        it->SetEdges();
                        if (it->ex < 0)
                        {
                            vPlatforms.erase(it);
                            break;
                        }
                    }
                }
            }
        }

        if (Hero)
        {
            if (Hero->y < scr_height - 180 && on_platform && !vPlatforms.empty())
            {
                for (std::vector<dll::ATOM>::iterator it = vPlatforms.begin(); it < vPlatforms.end(); it++)
                {
                    if (!(Hero->x >= it->ex || Hero->ex <= it->x || Hero->y >= it->ey || Hero->ey <= it->y))
                    {
                        if (Hero->ey > it->y + 5)
                        {
                            Hero->ey = it->y;
                            Hero->y = Hero->ey - Hero->GetHeight();
                            break;
                        }
                    }
                }
            }
        }

        if (Exit)
        {
            Exit->x -= speed;
            Exit->SetEdges();
            if(Hero)
                if (Exit->ex <= Hero->x)
                {
                    score += 1000;
                    GameOver();
                }
        }
        /////////////////////////////////////

        //BANANAS ***************************

        if (vBananas.size() < 5 && rand() % 150 == 13)
            vBananas.push_back(dll::ATOM(scr_width + (float)(rand() % 30), (float)(rand() % 300 + 100), 20.0f, 13.0f));

        if (!vBananas.empty())
        {
            for (std::vector<dll::ATOM>::iterator it = vBananas.begin(); it < vBananas.end(); it++)
            {
                if (Hero)
                    if (Hero->dir == dll::dirs::stop)break;
                it->x -= speed;
                it->SetEdges();
                if (it->ex < 0)
                {
                    vBananas.erase(it);
                    break;
                }
            }
        }
        if (Hero && !vBananas.empty())
        {
            for (std::vector<dll::ATOM>::iterator ban = vBananas.begin(); ban < vBananas.end(); ++ban)
            {
                if (!(Hero->x > ban->ex || Hero->ex<ban->x || Hero->y>ban->ey || Hero->ey < ban->y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\get.wav", NULL, NULL, NULL);
                    score += 5 * speed;
                    bananas++;
                    vBananas.erase(ban);
                    break;
                }
            }
        }

        if (!vShots.empty())
        {
            for (std::vector<dll::ATOM>::iterator it = vShots.begin(); it < vShots.end(); it++)
            {
                it->x += 2.5f;
                it->SetEdges();
                if (it->ex > scr_width)
                {
                    vShots.erase(it);
                    break;
                }
            }
        }

        /////////////////////////////////////

        //GORILLAS *************************

        if (vGorillas.size() < 2 && rand() % 300 == 66)
        {
            int type = rand() % 3;
            switch (type)
            {
            case 0:
                vGorillas.push_back(dll::iFactory(dll::types::gorilla1, scr_height - 170.0f));
                break;

            case 1:
                vGorillas.push_back(dll::iFactory(dll::types::gorilla2, scr_height - 200.0f));
                break;

            case 2:
                vGorillas.push_back(dll::iFactory(dll::types::gorilla3, scr_height - 190.0f));
                break;
            }
        }

        if (!vGorillas.empty())
        {
            for (std::vector<dll::creat_ptr>::iterator it = vGorillas.begin(); it < vGorillas.end(); it++)
            {
                if ((*it)->Move((float)(speed)) == FAIL)
                {
                    (*it)->Release();
                    vGorillas.erase(it);
                    break;
                }
            }
        }

        if (!vGorillas.empty() && !vShots.empty())
        {
            for (std::vector<dll::ATOM>::iterator ban = vShots.begin(); ban < vShots.end(); ban++)
            {
                bool shot = false;
                for (std::vector<dll::creat_ptr>::iterator gor = vGorillas.begin(); gor < vGorillas.end(); gor++)
                {
                    if (!((*gor)->x > ban->ex || (*gor)->ex<ban->x || (*gor)->y>ban->ey || (*gor)->ey < ban->y))
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\gorillakilled.wav", NULL, NULL, NULL);
                        score += 20 * speed;
                        (*gor)->Release();
                        vGorillas.erase(gor);
                        vShots.erase(ban);
                        shot = true;
                        break;
                    }
                }
                if (shot)break;
            }
        }

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
                        if (!(Hero->x > vRocks[i].ex || Hero->ex < vRocks[i].x || Hero->y > vRocks[i].ey || Hero->ey < vRocks[i].y))
                        {
                            vRocks.erase(vRocks.begin() + i);
                            dizzy_cooldown = 3;
                            if (sound)mciSendString(L"play .\\res\\snd\\dizzy.wav", NULL, NULL, NULL);
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
        
        wchar_t stat_txt[200] = L"\0";
        wchar_t add[5] = L"\0";
        int txt_size = 0;

        wcscpy_s(stat_txt, current_player);

        wsprintf(add, L"%d", bananas);
        wcscat_s(stat_txt, L", банани: ");
        wcscat_s(stat_txt, add);

        wsprintf(add, L"%d", score);
        wcscat_s(stat_txt, L", резултат: ");
        wcscat_s(stat_txt, add);
        
        wsprintf(add, L"%d", speed);
        wcscat_s(stat_txt, L", скорост: ");
        wcscat_s(stat_txt, add);

        wsprintf(add, L"%d", minutes);
        wcscat_s(stat_txt, L", ");
        wcscat_s(stat_txt, add);

        wsprintf(add, L"%d", seconds - minutes * 60);
        wcscat_s(stat_txt, L" : ");
        if (seconds - minutes * 60 < 10)wcscat_s(stat_txt, L"0");
        wcscat_s(stat_txt, add);

        for (int i = 0; i < 200; i++)
        {
            if (stat_txt[i] != '\0')txt_size++;
            else break;
        }

        Draw->DrawTextW(stat_txt, txt_size, statusTxt, D2D1::RectF(20.0f, scr_height - 50.0f, scr_width, scr_height), InactTxt);

        if (Exit)
            Draw->DrawBitmap(bmpExit, D2D1::RectF(Exit->x, Exit->y, Exit->ex, Exit->ey));
        
        ////////////////////////////////////////////////////////////

        if (Hero)
        {
            if(Hero->dir==dll::dirs::stop || Hero->dir==dll::dirs::up || Hero->dir == dll::dirs::down 
                || Hero->dir == dll::dirs::fall)
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
        if (vPlatforms.size() > 0)
        {
            for (int i = 0; i < vPlatforms.size(); ++i)
                Draw->DrawBitmap(bmpPlatform, D2D1::RectF(vPlatforms[i].x, vPlatforms[i].y,
                    vPlatforms[i].ex, vPlatforms[i].ey));
        }
        if (!vBananas.empty())
        {
            for (std::vector<dll::ATOM>::iterator it = vBananas.begin(); it < vBananas.end(); it++)
                Draw->DrawBitmap(bmpBanana, D2D1::RectF(it->x, it->y, it->ex, it->ey));

        }
        if (!vShots.empty())
        {
            for (std::vector<dll::ATOM>::iterator it = vShots.begin(); it < vShots.end(); it++)
                Draw->DrawBitmap(bmpBanana, D2D1::RectF(it->x, it->y, it->ex, it->ey));
        }
        if (!vGorillas.empty())
        {
            for(std::vector<dll::creat_ptr>::iterator it = vGorillas.begin(); it < vGorillas.end(); ++it)
            {
                switch ((*it)->GetType())
                {
                case dll::types::gorilla1:
                    Draw->DrawBitmap(bmpGorilla1[(*it)->GetFrame()], D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;
                
                case dll::types::gorilla2:
                    Draw->DrawBitmap(bmpGorilla2[(*it)->GetFrame()], D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;
                
                case dll::types::gorilla3:
                    Draw->DrawBitmap(bmpGorilla3[(*it)->GetFrame()], D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;
                }
            }
        }

        ////////////////////////////////////////////////////////

        Draw->EndDraw();

    }

    std::remove(tmp_file);
    ReleaseCOM();
    return (int) bMsg.wParam;
}