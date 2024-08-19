#include "framework.h"
#include "LUNATIC BIRDS.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "D2BMPLOADER.h"
#include "ErrH.h" 
#include "FCheck.h"
#include "BirdEngine.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <vector>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "birdengine.lib")

#define bWinClassName L"CrazyShooters"

#define sound_file L".\\res\\snd\\sound.wav"
#define temp_file ".\\res\\data\\temp.dat"
#define Ltemp_file L".\\res\\data\\temp.dat"
#define help_file L".\\res\\data\\help.dat"
#define record_file L".\\res\\data\\record.dat"
#define save_file L".\\res\\data\save.dat"

#define mNew 1001
#define mExit 1002
#define mLoad 1003
#define mSave 1004
#define mHoF 1005

#define no_record 2001
#define first_record 2002
#define record 2003

WNDCLASS bWin = { 0 };
HINSTANCE bIns = nullptr;
MSG bMsg = { 0 };
BOOL bRet = 0;
UINT bTimer = -1;

HICON mainIcon = nullptr;
HCURSOR mainCursor = nullptr;
HCURSOR outCursor = nullptr;
POINT cur_pos = { 0,0 };

HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;

HWND bHwnd = nullptr;
HDC PaintDC = nullptr;
PAINTSTRUCT bPaint = { 0 };

////////////////////////////////////////////////////

bool pause = false;
bool show_help = false;
bool sound = true;
bool in_client = true;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool name_set = false;

wchar_t current_player[16] = L"ONE CRAZY BIRD";

D2D1_RECT_F b1Rect = { 0, 0, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect = { scr_width / 3, 0, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect = { scr_width * 2 / 3, 0, scr_width, 50.0f };

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* ButBckgBrush = nullptr;
ID2D1SolidColorBrush* TxtBrush = nullptr;
ID2D1SolidColorBrush* InactTxt = nullptr;
ID2D1SolidColorBrush* HgltTxt = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmText = nullptr;
IDWriteTextFormat* midText = nullptr;
IDWriteTextFormat* bigText = nullptr;

ID2D1Bitmap* bmpBackground[10] = { nullptr };
ID2D1Bitmap* bmpField = nullptr;
ID2D1Bitmap* bmpHBoard = nullptr;
ID2D1Bitmap* bmpVBoard = nullptr;
ID2D1Bitmap* bmpPremHBoard = nullptr;
ID2D1Bitmap* bmpPremVBoard = nullptr;

ID2D1Bitmap* bmpBomb[2] = { nullptr };
ID2D1Bitmap* bmpGray[12] = { nullptr };
ID2D1Bitmap* bmpRed[7] = { nullptr };
ID2D1Bitmap* bmpYellow[9] = { nullptr };
ID2D1Bitmap* bmpSling[6] = { nullptr };

ID2D1Bitmap* bmpPig[12] = { nullptr };
ID2D1Bitmap* bmpBigPig[15] = { nullptr };

///////////////////////////////////////////////////

int score = 0;
int mins = 0;
int secs = 0;

////////////////////////////////////////////////

template<typename T> concept Releaseable = requires(T parameter)
{
    parameter.Release();
};
template<Releaseable Arg> bool ClearObject(Arg** what)
{
    if (*what)
    {
        (*what)->Release();
        (*what) = nullptr;
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream log(L".\\res\\data\\error.log", std::ios::app);
    log << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl;
    log.close();
}
void ReleaseResources()
{
    ClearObject(&iFactory);
    ClearObject(&Draw);
    ClearObject(&ButBckgBrush);
    ClearObject(&TxtBrush);
    ClearObject(&InactTxt);
    ClearObject(&HgltTxt);
    ClearObject(&iWriteFactory);
    ClearObject(&nrmText);
    ClearObject(&midText);
    ClearObject(&bigText);

    ClearObject(&bmpField);
    ClearObject(&bmpHBoard);
    ClearObject(&bmpVBoard);
    ClearObject(&bmpPremHBoard);
    ClearObject(&bmpPremVBoard);

    for (int i = 0; i < 10; i++)ClearObject(&bmpBackground[i]);
    for (int i = 0; i < 2; i++)ClearObject(&bmpBomb[i]);
    for (int i = 0; i < 12; i++)ClearObject(&bmpGray[i]);
    for (int i = 0; i < 7; i++)ClearObject(&bmpRed[i]);
    for (int i = 0; i < 9; i++)ClearObject(&bmpYellow[i]);
    for (int i = 0; i < 6; i++)ClearObject(&bmpSling[i]);
    for (int i = 0; i < 12; i++)ClearObject(&bmpPig[i]);
    for (int i = 0; i < 15; i++)ClearObject(&bmpBigPig[i]);
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);
    ReleaseResources();
    std::remove(temp_file);
    exit(1);
}










int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;






    std::remove(temp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}