// ShooterGame.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ShooterGame.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SHOOTERGAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SHOOTERGAME));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SHOOTERGAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SHOOTERGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

#include <thread>
#include <mutex>
#include <vector>

using namespace std;


LONG WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);

#define RADIUS 25
#define BULLET_SPEED 10

mutex frameMutex;
int dT = 40;


int height = 400;
int width = 800;

class Plate;

class Projectile {
    int dx, dy;
public:
    int x, y;
    int size;
    int radius;
    int color;
    bool shouldBeDeleted;
    Projectile(int _x, int _y, int _dx, int _dy, int _radius, int _size, int _color) {
        x = _x;
        y = _y;
        dx = _dx;
        dy = _dy;
        radius = _radius;
        size = _size;
        color = _color;
        shouldBeDeleted = false;
    }
    void doStep() {
        x += dx;
        y += dy;
        shouldBeDeleted = shouldBeDeleted | isOutside();
    }
    bool isOutside() {
        if ((x < 60) || (x > width - 25)) return true;
        if ((y < 25) || (y > height - 50)) return true;
        return false;
    }
    virtual int getType() = 0;
    virtual int getSize() { return size; }
    virtual int getColor() { return color; }
    virtual int getPoints(Projectile* projectile) = 0;
    bool isContacted(Projectile* projectile) {
        if (getType() != 0) return false;
        if (projectile->getType() == 0) return false;
        if (projectile->shouldBeDeleted) return false;
        int len = (x - projectile->x) * (x - projectile->x) + (y - projectile->y) * (y - projectile->y);
        if (len < projectile->radius * projectile->radius) return true;
    }
};

class Plate : public Projectile {
public:
    Plate(int _x, int _y, int _dx, int _dy, int radius, int size, int color) : Projectile(_x, _y, _dx, _dy, radius, size, color) {};
    virtual int getType() override { return 1; }


    virtual int getPoints(Projectile* projectile) override { return 0; }
};

class Bullet : public Projectile {
public:
    Bullet(int _x, int _y) :Projectile(_x, _y, BULLET_SPEED, 0, 10, -1, -1) {}
    virtual int getType() override { return 0; }
    virtual int getPoints(Projectile* projectile) override {
        if (shouldBeDeleted) return 0;
        if (!projectile->getType()) return 0;
        if ((projectile->getSize() == 0 && projectile->getColor() == 0) ||
            (projectile->getSize() == 1 && projectile->getColor() == 1)) {
            projectile->shouldBeDeleted = true;
            shouldBeDeleted = true;
            return 100;
        }
        else {
            projectile->shouldBeDeleted = true;
            shouldBeDeleted = true;
            return -100;
        }
    }
};

vector<Projectile*> projectiles;
mutex projectilesMutex;

int score = 1000;
int maxScore = 1000;

class Shooter {
public:
    int x, y;
    int gunX;
    int gunY;
    int radius;
    int dy = 10;

    Shooter(int _radius) {
        radius = _radius;
        x = radius;
        y = radius;
        gunY = (y + radius*3/4);
        gunX = (x + radius * 2);
    }
    void moveTop() {
        if (y - dy > radius) {
            y -= dy;
            gunY -= dy;
        }
    }
    void moveBottom() {
        if (y + dy < (height - radius*5/2)) {
            y += dy;
            gunY += dy;
        }
    }
    void shoot() {
        projectilesMutex.lock();
        Bullet* bul = new Bullet(gunX, gunY);
        projectiles.push_back(bul);
        projectilesMutex.unlock();
    }
};



Shooter shooter(RADIUS);


bool loose = false;

int checkHits() {
    int points = 0;
    projectilesMutex.lock();
    for (Projectile* projA : projectiles) {
        for (Projectile* projB : projectiles) {
            if (projA->isContacted(projB)) points += projA->getPoints(projB);
        }
    }
    projectilesMutex.unlock();
    return points;
}

void doSteps() {
    projectilesMutex.lock();
    for (Projectile* proj : projectiles)
        proj->doStep();
    projectilesMutex.unlock();
}

void clearField() {
    vector<Projectile*> newProjectiles;
    projectilesMutex.lock();
    for (Projectile* proj : projectiles)
        if (!proj->shouldBeDeleted)
            newProjectiles.push_back(proj);
    projectiles = newProjectiles;
    projectilesMutex.unlock();
}

void spawnPlate() {
    projectilesMutex.lock();
    int color = rand() % 2;
    int size = rand() % 2;

    Plate* proj = new Plate(rand() % width, rand() % height, (rand() % 10) - 5, (rand() % 10) - 5, 20 + 20 *size, size, color);
    projectiles.push_back(proj);

    projectilesMutex.unlock();
}

void recalcMaxScore() {
    if (maxScore < score) maxScore = score;
}

void THPlateSpawner() {
    while (1) {
        if (rand() % 100 < 25) {
            spawnPlate();
        }
        Sleep(dT);
    }
}

void THfunction(HINSTANCE hInstance, WNDCLASS w, int nCmdShow)
{
    HWND hwnd;
    hwnd = CreateWindow(L"My Class", L"Shooter", WS_OVERLAPPEDWINDOW, 400, 400, width, height, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    while (1) {
        score = 1000;
        maxScore = 1000;
        while (score >= 0) {
            score--;
            

            if (GetAsyncKeyState(VK_UP) < 0) {
                shooter.moveTop();
            }
            if (GetAsyncKeyState(VK_DOWN) < 0) {
                shooter.moveBottom();
            }
            if (GetAsyncKeyState(VK_SPACE) < 0) {
                shooter.shoot();
            }

            frameMutex.lock();
            
            doSteps();
            clearField();
            score += checkHits();
            recalcMaxScore();
            frameMutex.unlock();

            InvalidateRect(hwnd, 0, 1); //подготовка к перерисовке
            UpdateWindow(hwnd);			//перерисовка содержания окна
            Sleep(dT);  //временной интервал до следующей перерисовки
        }
        loose = true;

        InvalidateRect(hwnd, 0, 1); //подготовка к перерисовке
        UpdateWindow(hwnd);			//перерисовка содержания окна

        Sleep(5000);
        loose = false;
    }
}

void DrawShooter(HDC hdc, RECT rectClient)
{
    HBRUSH hBrush; // кисть для рисования


    frameMutex.lock();

    hBrush = CreateSolidBrush(RGB(50, 50, 50));  // gray
    SelectObject(hdc, hBrush);
    Rectangle(hdc, shooter.x, shooter.gunY+5, shooter.gunX, shooter.gunY-5);
    DeleteObject(hBrush);
    hBrush = CreateSolidBrush(RGB(150, 150, 150));  // gray
    SelectObject(hdc, hBrush);
    Ellipse(hdc, shooter.x- shooter.radius, shooter.y-shooter.radius, shooter.x + shooter.radius, shooter.y + shooter.radius);
    DeleteObject(hBrush);

    frameMutex.unlock();
}

void DrawProjectile(HDC hdc, RECT rectClient, Projectile* projectile) {
    HBRUSH hBrush;
    if (projectile->color == -1) {
        hBrush = CreateSolidBrush(RGB(0, 0, 0));  // black
    }
    else if (projectile->color == 0) {
        hBrush = CreateSolidBrush(RGB(255, 0, 0));  // red
    }
    else {
        hBrush = CreateSolidBrush(RGB(0, 255, 0));  // green
    }
    SelectObject(hdc, hBrush);
    Ellipse(hdc, 
        projectile->x - projectile->radius, 
        projectile->y - projectile->radius, 
        projectile->x + projectile->radius, 
        projectile->y + projectile->radius);
    DeleteObject(hBrush);
}

void DrawProjectiles(HDC hdc, RECT rectClient) {
    for (Projectile* proj : projectiles)
        DrawProjectile(hdc, rectClient, proj);

}

void DrawScore(HDC hdc, RECT rectClient) {
    TCHAR text[256];
    swprintf_s(text, 256, L"Score: %d", score);
    TextOut(hdc, width - 150, 20, text, wcslen(text));
    swprintf_s(text, 256, L"Max score: %d", maxScore);
    TextOut(hdc, width - 150, 5, text, wcslen(text));
}

void DrawLoose(HDC hdc, RECT rectClient) {
    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    SelectObject(hdc, hBrush);
    Rectangle(hdc, 0, 0, width, height);
    DeleteObject(hBrush);
    TCHAR text[256];
    swprintf_s(text, 256, L"Max score: %d", maxScore);
    TextOut(hdc, width/2 - 50, height/2 - 50, text, wcslen(text));
}


int WINAPI WinMain(HINSTANCE hInstance,  //главный процесс
    HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS w; //структура данных для описания класса окон
    //регистрация класса окон для вывода результатов:
    memset(&w, 0, sizeof(WNDCLASS));
    w.style = CS_HREDRAW | CS_VREDRAW;
    w.lpfnWndProc = WndProc; // имя оконной функции
    w.hInstance = hInstance; // дескриптор (номер) процесса в Windows - главной функции
    w.hbrBackground = CreateSolidBrush(0x00FFFF7F); // цвет фона окна SpringGreen
    w.lpszClassName = L"My Class";
    RegisterClass(&w); // регистрация класса в системе
    // создание потоков:
    srand(time(0));
    std::thread TH0(THfunction, hInstance, w, nCmdShow);
    std::thread TH1(THPlateSpawner);
    TH1.detach();
    TH0.join();
}


LONG WINAPI WndProc(HWND hwnd, UINT Message,
    WPARAM wparam, LPARAM lparam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    HPEN hpen;
    switch (Message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        if (!loose) {
            DrawShooter(hdc, ps.rcPaint);
            DrawProjectiles(hdc, ps.rcPaint);
            DrawScore(hdc, ps.rcPaint);
        }
        else {
            DrawLoose(hdc, ps.rcPaint);
        }

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, Message, wparam, lparam);
    }

    return 0;
}