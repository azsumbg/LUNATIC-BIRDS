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
int gold = 30;
int level = 1;


////////////////////////////////////////////////

dll::FieldItem Background = nullptr;
dll::FieldItem Ground = nullptr;
dll::FieldItem Sling = nullptr;
bool now_shooting = false;

dll::Bird Terminator = nullptr;
float terminator_dest_x = 0;
float terminator_dest_y = 0;

birds BirdPool[5];

dll::Pig Evil = nullptr;
std::vector<dll::FieldItem> vBoards;

///////////////////////////////////////////////

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
void InitLevel()
{
    if (level > 1)
    {
        mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
        if (Draw && bigText && HgltTxt)
        {
            for (int i = 0; i < 27; i++)
            {
                Draw->BeginDraw();
                Draw->Clear(D2D1::ColorF(D2D1::ColorF::Beige));
                if (i % 2 == 0)
                    Draw->DrawTextW(L"СЛЕДВАЩО НИВО !", 16, bigText, D2D1::RectF(50.0f, 200.0f, scr_width, scr_height), HgltTxt);
                Draw->EndDraw();
                Sleep(100);
            }
        }
        if (!vBoards.empty())
            for (int i = 0; i < vBoards.size(); i++)ClearObject(&vBoards[i]);
        vBoards.clear();
    }

    float next_board_x = 250.0f;
    float next_board_y = 120.0f;
    gold = 50;

    for (int i = 0; i < 4; i++)
    {
        switch (rand() % 2)
        {
        case 0:
            vBoards.push_back(dll::CreateFieldItem(fields::h_board, 450.0f, next_board_y));
            next_board_y += 45.0f;
            break;

        case 1:
            vBoards.push_back(dll::CreateFieldItem(fields::prem_h_board, 450.0f, next_board_y));
            next_board_y += 96.0f;
            break;
        }
        
    }
    for (int i = 0; i < 2; i++)
    {
        switch (rand() % 2)
        {
        case 0:
            vBoards.push_back(dll::CreateFieldItem(fields::v_board, next_board_x, scr_height - 115.0f));
            next_board_x += 101.0f;
            break;

        case 1:
            vBoards.push_back(dll::CreateFieldItem(fields::prem_v_board, next_board_x, scr_height - 155.0f));
            next_board_x += 101;
            break;
        }
    }
    
    ClearObject(&Evil);
    switch (rand() % 2)
    {
    case 0:
        Evil = dll::CreatePig(460.0f, scr_height - 150.0f, pigs::pig);
        break;

    case 1:
        Evil = dll::CreatePig(460.0f, scr_height - 125.0f, pigs::big_pig);
        break;
    }
}
void InitGame()
{
    wcscpy_s(current_player, L"ONE CRAZY BIRD");
    name_set = false;
    score = 0;
    mins = 0;
    secs = 0;
    gold = 50;
    level = 1;

    ClearObject(&Background);
    Background = dll::CreateFieldItem(fields::background, 0, 50.0f);

    ClearObject(&Ground);
    Ground = dll::CreateFieldItem(fields::field, 0, scr_height - 40.0f);

    ClearObject(&Sling);
    Sling = dll::CreateFieldItem(fields::sling, 2.0f, scr_height - 170.0f);

    ClearObject(&Terminator);
    for (int i = 0; i < 5; i++)BirdPool[i] = static_cast<birds>(rand() % 4);

    ClearObject(&Evil);

    if (!vBoards.empty())
        for (int i = 0; i < vBoards.size(); i++)ClearObject(&vBoards[i]);
    vBoards.clear();
    
    
    InitLevel();
}

void GameOver()
{
    KillTimer(bHwnd, bTimer);
    PlaySound(NULL, NULL, NULL);


    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}

INT_PTR CALLBACK bDlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(mainIcon));
        return (INT_PTR)(TRUE);
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
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                wcscpy_s(current_player, L"ONE CRAZY BIRD");
                name_set = false;
                
                if (sound)mciSendString(L"play .\\res\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
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
        SetTimer(hwnd, bTimer, 1000, NULL);
        srand((unsigned int)(time(0)));

        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();

        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Меню за данни");

        AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
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
        if (sound)mciSendString(L"play .\\res\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Изход ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(20, 20, 20)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_TIMER:
        if (pause)break;
        secs++;
        mins = secs / 60;
        if (secs % 2 == 0)gold += 10;
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
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }

                SetCursor(outCursor);
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

            SetCursor(mainCursor);
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
            if (sound)mciSendString(L"play .\\res\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
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

    case WM_LBUTTONDOWN:
        if (now_shooting || Terminator)break;
        now_shooting = true;
        terminator_dest_x = LOWORD(lParam);
        terminator_dest_y = HIWORD(lParam);
        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltemp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream start_file(Ltemp_file);
        start_file << L"Game started at: " << std::chrono::system_clock::now();
        start_file.close();
    }
    
    int start_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2);

    if (GetSystemMetrics(SM_CXSCREEN) < start_x + scr_width || GetSystemMetrics(SM_CYSCREEN) < scr_height + 50)ErrExit(eScreen);

    mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 253, LR_LOADFROMFILE));
    if (!mainIcon)ErrExit(eIcon);
    mainCursor = LoadCursorFromFile(L".\\res\\main.ani");
    outCursor = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCursor || !outCursor)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &bWinProc;
    bWin.hCursor = mainCursor;
    bWin.hIcon = mainIcon;
    bWin.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
    bWin.style = CS_DROPSHADOW;
    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindowW(bWinClassName, L"БЕСНИ ЖИВОТНИ", WS_CAPTION | WS_SYSMENU, start_x, 50, (int)(scr_width), 
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);


        HRESULT hr = S_OK;

        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 Factory !");
            ErrExit(eD2D);
        }

        if (iFactory)
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 HWND Render Target !");
            ErrExit(eD2D);
        }

        if (Draw)
        {
            ID2D1GradientStopCollection* gsCol = nullptr;
            D2D1_GRADIENT_STOP gStops[2] = { 0 };

            gStops[0].position = 0;
            gStops[0].color = D2D1::ColorF(D2D1::ColorF::MediumSlateBlue);
            gStops[1].position = 1.0f;
            gStops[1].color = D2D1::ColorF(D2D1::ColorF::Indigo);

            hr = Draw->CreateGradientStopCollection(gStops, 2, &gsCol);
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 GradientStopCollection !");
                ErrExit(eD2D);
            }
            if (gsCol)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
                    D2D1::Point2F(0, 0), scr_width / 2, 25.0f), gsCol, &ButBckgBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating D2D1 Background Brush !");
                    ErrExit(eD2D);
                }
                ClearObject(&gsCol);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGreen), &TxtBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &HgltTxt);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &InactTxt);

            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 Solid Color Brushes !");
                ErrExit(eD2D);
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 Write Factory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"Bookman Old Style", NULL, DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 16, L"",  & nrmText);
            hr = iWriteFactory->CreateTextFormat(L"Bookman Old Style", NULL, DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 32, L"", &midText);
            hr = iWriteFactory->CreateTextFormat(L"Bookman Old Style", NULL, DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 50, L"", &bigText);
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 Wrte text Formats !");
                ErrExit(eD2D);
            }
        }

        bmpField = Load(L".\\res\\img\\field\\field.png", Draw);
        if (!bmpField)
        {
            LogError(L"Error loading bmpField !");
            ErrExit(eD2D);
        }
        bmpHBoard = Load(L".\\res\\img\\field\\hboard.png", Draw);
        if (!bmpHBoard)
        {
            LogError(L"Error loading bmpHBoard !");
            ErrExit(eD2D);
        }
        bmpVBoard = Load(L".\\res\\img\\field\\vboard.png", Draw);
        if (!bmpVBoard)
        {
            LogError(L"Error loading bmpVBoard !");
            ErrExit(eD2D);
        }
        bmpPremHBoard = Load(L".\\res\\img\\field\\premium_hboard.png", Draw);
        if (!bmpPremHBoard)
        {
            LogError(L"Error loading bmpPremHBoard !");
            ErrExit(eD2D);
        }
        bmpPremVBoard = Load(L".\\res\\img\\field\\premium_vboard.png", Draw);
        if (!bmpPremVBoard)
        {
            LogError(L"Error loading bmpPremVBoard !");
            ErrExit(eD2D);
        }
        for (int i = 0; i < 10; i++)
        {
            wchar_t name[150] = L".\\res\\img\\field\\background\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpBackground[i] = Load(name, Draw);
            if (!bmpBackground[i])
            {
                LogError(L"Error loading bmpBackground !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 2; i++)
        {
            wchar_t name[150] = L".\\res\\img\\birds\\bomb\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpBomb[i] = Load(name, Draw);
            if (!bmpBomb[i])
            {
                LogError(L"Error loading bmpBomb !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 12; i++)
        {
            wchar_t name[150] = L".\\res\\img\\birds\\gray\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpGray[i] = Load(name, Draw);
            if (!bmpGray[i])
            {
                LogError(L"Error loading bmpGray !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 7; i++)
        {
            wchar_t name[150] = L".\\res\\img\\birds\\red\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpRed[i] = Load(name, Draw);
            if (!bmpRed[i])
            {
                LogError(L"Error loading bmpRed !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 6; i++)
        {
            wchar_t name[150] = L".\\res\\img\\birds\\sling\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpSling[i] = Load(name, Draw);
            if (!bmpSling[i])
            {
                LogError(L"Error loading bmpSling !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 9; i++)
        {
            wchar_t name[150] = L".\\res\\img\\birds\\yellow\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpYellow[i] = Load(name, Draw);
            if (!bmpYellow[i])
            {
                LogError(L"Error loading bmpYellow !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 15; i++)
        {
            wchar_t name[150] = L".\\res\\img\\pigs\\big_pig\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpBigPig[i] = Load(name, Draw);
            if (!bmpBigPig[i])
            {
                LogError(L"Error loading bmpBigPig !");
                ErrExit(eD2D);
            }
        }
        for (int i = 0; i < 12; i++)
        {
            wchar_t name[150] = L".\\res\\img\\pigs\\pig\\";
            wchar_t add[3] = L"\0";
            wsprintf(add, L"%d", i);
            wcscat_s(name, add);
            wcscat_s(name, L".png");
            bmpPig[i] = Load(name, Draw);
            if (!bmpPig[i])
            {
                LogError(L"Error loading bmpPig !");
                ErrExit(eD2D);
            }
        }
    }

    D2D1_RECT_F IntroRect = { 10.0f,150.0f,scr_width,scr_height };

    wchar_t intro_text[30] = L"БЕСНИ ДОБИТЪЦИ !\n\ndev. Daniel";
    wchar_t show_text[30] = L"\0";

    if (Draw && bigText && TxtBrush)
    {
        mciSendString(L"play .\\res\\snd\\intro.wav", NULL, NULL, NULL);
        for (int i = 0; i < 30; i++)
        {
            show_text[i] = intro_text[i];
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
            Draw->DrawTextW(show_text, i, bigText, IntroRect, TxtBrush);
            Draw->EndDraw();
            Sleep(100);
        }
        Sleep(2500);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
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
            if (Draw && bigText && TxtBrush)
            {
                Draw->BeginDraw();
                Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
                Draw->DrawTextW(L"ПАУЗА", 6, bigText, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f,
                    scr_width, scr_height), TxtBrush);
                Draw->EndDraw();
                continue;
            }
        }
        /////////////////////////////////////////////////////////

        // TERMINATOR ******************************************

        if (Terminator)
        {
            if (!Terminator->Shoot(Terminator->x, Terminator->y, terminator_dest_x, terminator_dest_y))
            {
                ClearObject(&Terminator);
            }
        }
        if (Terminator && !vBoards.empty())
        {
            for (std::vector<dll::FieldItem>::iterator board = vBoards.begin(); board < vBoards.end(); ++board)
            {
                if (!(Terminator->x >= (*board)->ex || Terminator->ex <= (*board)->x
                    || Terminator->y >= (*board)->ey || Terminator->ey <= (*board)->y))
                {
                    (*board)->lifes -= Terminator->GetDamage();
                    ClearObject(&Terminator);
                    if ((*board)->lifes <= 0)
                    {
                        score += 20;
                        (*board)->Release();
                        vBoards.erase(board);
                    }
                    break;
                }
            }
        }
        if (Terminator && Evil)
        {
            if (!(Terminator->x > Evil->ex || Terminator->ex<Evil->x
                || Terminator->y>Evil->ey || Terminator->ey < Evil->y))
            {
                Evil->lifes -= Terminator->GetDamage();
                ClearObject(&Terminator);
                if (Evil->lifes <= 0)
                {
                    level++;
                    score += 200 - secs;
                    if (score <= 0)score = 0;
                    InitLevel();
                }
            }
        }

        ///////////////////////////////////////////////////////






        //DRAW THINGS ********************************************

        if (Draw && nrmText && ButBckgBrush && TxtBrush && HgltTxt && InactTxt)
        {
            Draw->BeginDraw();
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), ButBckgBrush);
            if (name_set)
                Draw->DrawText(L"СТРЕЛЕЦ", 8, nrmText, b1Rect, InactTxt);
            else
            {
                if (b1Hglt)Draw->DrawText(L"СТРЕЛЕЦ", 8, nrmText, b1Rect, HgltTxt);
                else Draw->DrawText(L"СТРЕЛЕЦ", 8, nrmText, b1Rect, TxtBrush);
            }
            if (b2Hglt)Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, HgltTxt);
            else Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, TxtBrush);
            if (b3Hglt)Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, HgltTxt);
            else Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, TxtBrush);
        }
        Draw->DrawBitmap(bmpBackground[Background->GetFrame()], D2D1::RectF(Background->x, Background->y,
            Background->ex, Background->ey));
        Draw->DrawBitmap(bmpField, D2D1::RectF(Ground->x, Ground->y, Ground->ex, Ground->ey));
        if (Sling)
        {
            if (now_shooting)
            {
                int frame = Sling->GetFrame();
                if (frame < 5)Draw->DrawBitmap(bmpSling[frame], D2D1::RectF(Sling->x, Sling->y, Sling->ex, Sling->ey));
                   
                else
                {
                    Draw->DrawBitmap(bmpSling[5], D2D1::RectF(Sling->x, Sling->y, Sling->ex, Sling->ey));
                    Sling->InitFrame();
                    now_shooting = false;
                    
                    birds TempPool[5];
                    for (int i = 0; i < 4; i++)TempPool[i] = BirdPool[i + 1];
                    TempPool[4] = static_cast<birds>(rand() % 4);
                    bool bird_shooted = false;

                    switch (BirdPool[0])
                    {
                    case birds::bomb:
                        if (gold >= 50)
                        {
                            gold -= 50;
                            Terminator = dll::CreateBird(Sling->ex - 25.0f, Sling->y, birds::bomb);
                            bird_shooted = true;
                        }
                        else mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;

                    case birds::red:
                        if (gold >= 40)
                        {
                            gold -= 40;
                            Terminator = dll::CreateBird(Sling->ex - 25.0f, Sling->y, birds::red);
                            bird_shooted = true;
                        }
                        else mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;

                    case birds::gray:
                        if (gold >= 30)
                        {
                            gold -= 30;
                            Terminator = dll::CreateBird(Sling->ex - 25.0f, Sling->y, birds::gray);
                            bird_shooted = true;
                        }
                        else mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;

                    case birds::yellow:
                        if (gold >= 20)
                        {
                            gold -= 20;
                            Terminator = dll::CreateBird(Sling->ex - 25.0f, Sling->y, birds::yellow);
                            bird_shooted = true;
                        }
                        else mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                        break;
                    }
                    
                    if (bird_shooted)
                        for (int i = 0; i < 5; i++)BirdPool[i] = TempPool[i];
                }
            }
            else
                Draw->DrawBitmap(bmpSling[0], D2D1::RectF(Sling->x, Sling->y, Sling->ex, Sling->ey));
        }
        if (Terminator)
        {
            switch (Terminator->GetType())
            {
            case birds::red:
                Draw->DrawBitmap(bmpRed[Terminator->GetFrame()], D2D1::RectF(Terminator->x, Terminator->y,
                    Terminator->ex, Terminator->ey));
                break;

            case birds::gray:
                Draw->DrawBitmap(bmpGray[Terminator->GetFrame()], D2D1::RectF(Terminator->x, Terminator->y,
                    Terminator->ex, Terminator->ey));
                break;

            case birds::bomb:
                Draw->DrawBitmap(bmpBomb[Terminator->GetFrame()], D2D1::RectF(Terminator->x, Terminator->y,
                    Terminator->ex, Terminator->ey));
                break;

            case birds::yellow:
                Draw->DrawBitmap(bmpYellow[Terminator->GetFrame()], D2D1::RectF(Terminator->x, Terminator->y,
                    Terminator->ex, Terminator->ey));
                break;
            }
        }
        for(int i = 0; i < 5; i++)
            switch (BirdPool[i])
            {
            case birds::red:
                Draw->DrawBitmap(bmpRed[0], D2D1::RectF(300.0f + i * 50.0f, 50.0f, 300.0f + i * 50.0f + 50.0f, 100.0f));
                break;

            case birds::gray:
                Draw->DrawBitmap(bmpGray[0], D2D1::RectF(300.0f + i * 50.0f, 50.0f, 300.0f + i * 50.0f + 50.0f, 100.0f));
                break;

            case birds::bomb:
                Draw->DrawBitmap(bmpBomb[0], D2D1::RectF(300.0f + i * 50.0f, 50.0f, 300.0f + i * 50.0f + 50.0f, 100.0f));
                break;

            case birds::yellow:
                Draw->DrawBitmap(bmpYellow[0], D2D1::RectF(300.0f + i * 50.0f, 50.0f, 300.0f + i * 50.0f + 50.0f, 100.0f));
                break;
            }
        if (!vBoards.empty())
        {
            for (std::vector<dll::FieldItem>::iterator it = vBoards.begin(); it < vBoards.end(); it++)
            {
                switch ((*it)->GetType())
                {
                case fields::h_board:
                    Draw->DrawBitmap(bmpHBoard, D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;

                case fields::v_board:
                    Draw->DrawBitmap(bmpVBoard, D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;

                case fields::prem_h_board:
                    Draw->DrawBitmap(bmpPremHBoard, D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;

                case fields::prem_v_board:
                    Draw->DrawBitmap(bmpPremVBoard, D2D1::RectF((*it)->x, (*it)->y, (*it)->ex, (*it)->ey));
                    break;
                }
            }
        }
        if (Evil)
        {
            switch (Evil->GetType())
            {
            case pigs::pig:
                Draw->DrawBitmap(bmpPig[Evil->GetFrame()], D2D1::RectF(Evil->x, Evil->y, Evil->ex, Evil->ey));
                break;

            case pigs::big_pig:
                Draw->DrawBitmap(bmpBigPig[Evil->GetFrame()], D2D1::RectF(Evil->x, Evil->y, Evil->ex, Evil->ey));
                break;
            }
        }


        //STATUS ****************
        wchar_t stat_line[200] = L"СТРЕЛЕЦ: ";
        wchar_t add[5] = L"\0";
        int size = 0;

        wcscat_s(stat_line, current_player);
        
        wcscat_s(stat_line, L", ЗЛАТО: ");
        wsprintf(add, L"%d", gold);
        wcscat_s(stat_line, add);
        
        wcscat_s(stat_line, L", РЕЗУЛТАТ: ");
        wsprintf(add, L"%d", score);
        wcscat_s(stat_line, add);
        
        if (mins <= 9)wcscat_s(stat_line, L" 0");
        wsprintf(add, L"%d", mins);
        wcscat_s(stat_line, add);
        wcscat_s(stat_line, L" : ");

        if (secs - mins * 60 <= 9)wcscat_s(stat_line, L" 0");
        wsprintf(add, L"%d", secs - mins * 60);
        wcscat_s(stat_line, add);

        for (int i = 0; i < 200; i++)
        {
            if (stat_line[i] != '\0')size++;
            else break;

        }

        if (nrmText && HgltTxt)
            Draw->DrawTextW(stat_line, size, nrmText, D2D1::RectF(10.0f, scr_height - 35.0f, scr_width, scr_height), HgltTxt);

        //////////////////////
        Draw->EndDraw();

    }

    std::remove(temp_file);
    ReleaseResources();
    return (int) bMsg.wParam;
}