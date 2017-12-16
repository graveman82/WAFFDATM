#include <assert.h>
#include <stdlib.h>
#include <windows.h>
#include <algorithm>
//#include <stdio.h>
#include "ProjectInterface.h"
#include "WindowsApp.h"

long (*WAFFDATM_ConfigurateProject)();
long (*WAFFDATM_InitializeProject)();
long (*WAFFDATM_UpdateProject)(float deltaTime);
long (*WAFFDATM_RenderProject)();
long (*WAFFDATM_ShutdownProject)();
unsigned long (*WAFFDATM_GetProject_WindowWidth)();
unsigned long (*WAFFDATM_GetProject_WindowHeight)();
const char* (*WAFFDATM_GetProject_WindowTitle)();

namespace WAFFDATM
{
    HINSTANCE hInstance;        // Дескриптор приложения
    const int kWndClassName_BUFSIZE = 256;
    char wndClassName[kWndClassName_BUFSIZE];     // Имя класса окна
    HWND hWnd;                  // Дескриптор окна

    // Ограничитель числа кадров в цикле
    struct FrameLimiter {
        int lastTime_;      // время, зафиксированное при предыдущем сбросе счетчика кадров
        float frameTime_;   // время, отведенное на кадр
        int deltaTime_;     // время, фактически прошедшее с предыдущего сброса счетчика кадров
        int cFrames_;       // счетчик кадров
        bool fToUpdate_;    // устанавливается в true, если можно обновить состояние приложения в главном цикле
        float fps_;         // число кадров в секунду

        FrameLimiter() : lastTime_(0), frameTime_(33), cFrames_(0), fToUpdate_(false){}

        void UpdateTime() {
            const int curTime = ::timeGetTime();
            const int deltaTime = curTime - lastTime_;
            cFrames_++;
            fToUpdate_ = false;

            if ( deltaTime >= frameTime_ ){
                fps_ = float(cFrames_) * 1000.0f / float(deltaTime);
                //Log("fps=%f", fFPS);
                deltaTime_ = deltaTime;
                lastTime_ = curTime;
                cFrames_ = 0;
                fToUpdate_ = true;
            }
        }
    } frameLimiter;

    /* Состояние курсора.
    */
    struct {
        // центральная позиция курсора в клиентской области окна
        // (курсор нужно возвращать туда после движения мышью)
        POINT centralPosition_;
        // текущая позиция курсора в клиентской области окна
        POINT currentPosition_;
        float deltaX_, deltaY_;
        bool fShowed_;
        HCURSOR hCursor_;
        int cursorToogleVKey_;

        // Обновить центральную позицию для курсора в клиентской области окна
        void UpdateCentralPosition(HWND hWnd){
            RECT rc;
            ::GetClientRect(hWnd, &rc);
            centralPosition_.x = (rc.right - rc.left) / 2;
            centralPosition_.y = (rc.bottom - rc.top) / 2;
        }
        // Обновить текущую позицию для курсора в клиентской области окна
        void UpdateCurrentPosition(HWND hWnd){
            ::GetCursorPos(&currentPosition_);
            ::ScreenToClient(hWnd, &currentPosition_);
        }
        // Обновить смещения курсора относительно центра клиентской области окна
        void UpdateDeltas(HWND hWnd) {
            UpdateCentralPosition(hWnd);
            UpdateCurrentPosition(hWnd);
            deltaX_ = (float)(currentPosition_.x - centralPosition_.x);
            deltaY_ = (float)(currentPosition_.y - centralPosition_.y);
        }
        // Установить курсор в центральную позицию
        void SetCursorToCentralPosition(HWND hWnd) {
            UpdateCentralPosition(hWnd);
            POINT ptCenter = centralPosition_;
            ::ClientToScreen(hWnd, &ptCenter);
            ::SetCursorPos(ptCenter.x, ptCenter.y);
        }

        bool Hidden() { return !fShowed_; }
        void Show() { fShowed_ = true; }
        void Hide() { fShowed_ = false; }

    }cursorState;


    // Число клавиш клавиатуры
    const int kKEY_NUM = 160;
    // Состояния клавиш
    struct KeyState{
        bool fPressed_; // "нажатость"
        int pressTime_;
        int releaseTime_;

        KeyState() :
            fPressed_(false),
            pressTime_(0),
            releaseTime_(0){}

        void Press(int nTime){
            if (!fPressed_){
                pressTime_ = nTime;
                fPressed_ = true;
            }
        }

        void Release(int nTime){
            releaseTime_ = nTime;
            fPressed_ = false;
        }
    }keyStates[kKEY_NUM];



    // Коды виртуальных клавиш Windows
    //
    #define VK_A 0x41
    #define VK_B 0x42
    #define VK_C 0x43
    #define VK_D 0x44
    #define VK_E 0x45
    #define VK_F 0x46
    #define VK_G 0x47
    #define VK_H 0x48
    #define VK_I 0x49
    #define VK_J 0x4A
    #define VK_K 0x4B
    #define VK_L 0x4C
    #define VK_M 0x4D
    #define VK_N 0x4E
    #define VK_O 0x4F
    #define VK_P 0x50
    #define VK_Q 0x51
    #define VK_R 0x52
    #define VK_S 0x53
    #define VK_T 0x54
    #define VK_U 0x55
    #define VK_V 0x56
    #define VK_W 0x57
    #define VK_X 0x58
    #define VK_Y 0x59
    #define VK_Z 0x5A

    char vkeyToLibKeyMap[kKEY_NUM] = {
        0, 0, 0, 0, 0, 0, 0, 0, //0-7
        0, 0, 0, 0, 0, 0, 0, 0, //8-15
        0, 0, 0, 0, 0, 0, 0, 0, //16-23
        0, 0, 0, 0, 0, 0, 0, 0, //24-31
        0, 0, 0, 0, 0, 0, 0, 0, //32-39
        0, 0, 0, 0, 0, 0, 0, 0, //40-47
        0, 0, 0, 0, 0, 0, 0, 0, //48-55
        0, 0, 0, 0, 0, 0, 0, 0, //56-63
        0, kKEY_A, kKEY_B, kKEY_C, kKEY_D, kKEY_E, kKEY_F, kKEY_G, //64-71
        kKEY_H, kKEY_I, kKEY_J, kKEY_K, kKEY_L, kKEY_M, kKEY_N, kKEY_O, //72-79
        kKEY_P, kKEY_Q, kKEY_R, kKEY_S, kKEY_T, kKEY_U, kKEY_V, kKEY_W, //80-87
        kKEY_X, kKEY_Y, kKEY_Z, 0, 0, 0, 0, 0, // 88-95
        0, 0, 0, 0, 0, 0, 0, 0, //96-103
        0, 0, 0, 0, 0, 0, 0, 0, //104-111
        0, 0, 0, 0, 0, 0, 0, 0, //112-119
        0, 0, 0, 0, 0, 0, 0, 0, //120-127
        0, 0, 0, 0, 0, 0, 0, 0, //128-135
        0, 0, 0, 0, 0, 0, 0, 0, //136-143
        0, 0, 0, 0, 0, 0, 0, 0, //144-151
        0, 0, 0, 0, 0, 0, 0, 0, //152-159
    };

    int MapVKeyToLibKey (int vkey) { return vkeyToLibKeyMap[vkey]; }

    // Обработка ввода с устройств
    //

    // Во
    bool IsKeyPressed(int nKey);

    float GetMouseDeltaX() { return cursorState.deltaX_; }
    float GetMouseDeltaY() { return cursorState.deltaY_; }
    void ToggleCursor();
    bool IsCursorToogleKeyPressed(int vKey);
    void SetCursorToogleVKey(int vKey);
    bool IsCursorHidden();

    int GetDeltaTime();
    bool CheckForUpdate(float* deltaTime = 0);
    float GetFps();
    void SetFrameRate(float fps);
    void Update();

    // Инициализация начальных параметров переменных библиотеки - типа общего конструктора библиотеки
    void CommonInitialize();
    // Подготовить переменные перед запуском главного цикла
    void PrepareMainLoopVars();
    void Cleanup();

    HINSTANCE GetHinstanse() { return hInstance; }
    HWND GetWindowHandle() { return hWnd; }
    HWND CreateMainWindow(HINSTANCE _hInstance, int nCmdShow,
                          UINT width, UINT height, const char* title);
    LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
} // end of WAFFDATM

bool WAFFDATM::IsKeyPressed(int nKey) { return keyStates[nKey].fPressed_; }

void WAFFDATM::ToggleCursor(){
    if (cursorState.Hidden()){
        cursorState.Show();

    }
    else{
        cursorState.Hide();
    }
}

bool WAFFDATM::IsCursorToogleKeyPressed(int vKey) { return cursorState.cursorToogleVKey_ == vKey; }
void WAFFDATM::SetCursorToogleVKey(int vKey){
    cursorState.cursorToogleVKey_ = vKey;
}

bool WAFFDATM::IsCursorHidden(){
    return cursorState.Hidden();
}

int WAFFDATM::GetDeltaTime() { return frameLimiter.deltaTime_; }

bool WAFFDATM::CheckForUpdate(float* deltaTime){
    frameLimiter.UpdateTime();
    if (frameLimiter.fToUpdate_) {
        if (deltaTime != 0)
            *deltaTime = frameLimiter.deltaTime_;
        return true;
    }
    return false;
}

float WAFFDATM::GetFps() {return frameLimiter.fps_;}

void WAFFDATM::SetFrameRate(float fps){
    assert(fps > 0.0);
    frameLimiter.frameTime_ = 1000.0f / fps;
}

void WAFFDATM::Update()
{
    // обновить смещение курсора, если он скрыт
    if (cursorState.Hidden())
    {
        cursorState.UpdateDeltas(hWnd);
        cursorState.SetCursorToCentralPosition(hWnd);
    }

}

void WAFFDATM::CommonInitialize(){
    WAFFDATM_ConfigurateProject = 0;
    WAFFDATM_InitializeProject = 0;
    WAFFDATM_UpdateProject = 0;
    WAFFDATM_RenderProject = 0;
    WAFFDATM_ShutdownProject = 0;
    WAFFDATM_GetProject_WindowWidth = 0;
    WAFFDATM_GetProject_WindowHeight = 0;
    WAFFDATM_GetProject_WindowTitle = 0;

    cursorState.Show();
    SetCursorToogleVKey(VK_C);
    cursorState.deltaX_ = cursorState.deltaY_ = 0;
    wndClassName[0] = 0;
}

void WAFFDATM::PrepareMainLoopVars() {
    frameLimiter.lastTime_ = ::timeGetTime();
}

void WAFFDATM::Cleanup(){
     ::UnregisterClass( wndClassName, hInstance );
}

HWND WAFFDATM::CreateMainWindow(HINSTANCE _hInstance,
                           int nCmdShow,
                           UINT width, UINT height, const char* title){
    // Записываем и дополняем заголовок окна
    const char* titleSuffix = "_WAFFDATM";
    memset(&wndClassName[0], 0, kWndClassName_BUFSIZE * sizeof(char));
    int nCharsToCopy1 = std::min(strlen(title), (size_t)(kWndClassName_BUFSIZE - 1));
    strncpy_s(wndClassName, kWndClassName_BUFSIZE, title, nCharsToCopy1);
    int nCharsToCopy2 = std::min(strlen(titleSuffix), (size_t)(kWndClassName_BUFSIZE - 1 - nCharsToCopy1));
    strncpy_s(&wndClassName[nCharsToCopy1], kWndClassName_BUFSIZE - nCharsToCopy1, titleSuffix, nCharsToCopy2);
    wndClassName[std::min(nCharsToCopy1 + nCharsToCopy2, kWndClassName_BUFSIZE - 1)] = 0;
    //printf ("wndClassName=%s\n", wndClassName); // проверочный код

    hInstance = _hInstance;

    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW + 1;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = MainWndProc;
    wc.lpszClassName = &wndClassName[0];
    wc.lpszMenuName = 0;
    wc.style = CS_HREDRAW |CS_VREDRAW;

    if (::RegisterClassEx(&wc) == 0) {
        wndClassName[0] = 0;
        return 0;
    }
    hWnd = ::CreateWindowEx(0, wc.lpszClassName, title, WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                            0, 0, hInstance, 0);
    if (hWnd == 0) {
        wndClassName[0] = 0;
        return 0;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd );

    cursorState.hCursor_ = (HCURSOR)::GetClassLong(hWnd, GCL_HCURSOR);
    cursorState.SetCursorToCentralPosition(hWnd);
    return hWnd;
}

LRESULT CALLBACK WAFFDATM::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int msgTime = timeGetTime();
    ULONG_PTR hc;

    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

         case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) DestroyWindow(GetWindowHandle());
            else if (IsCursorToogleKeyPressed(wParam)) ToggleCursor();
            else if (wParam < kKEY_NUM)
                keyStates[MapVKeyToLibKey(wParam)].Press(msgTime);
            return 0;

        case WM_KEYUP:
            if (wParam < kKEY_NUM)
                keyStates[MapVKeyToLibKey(wParam)].Release(msgTime);
            return 0;

        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) { // наша клиентная область
                if (cursorState.Hidden()){
                    ::SetCursor(0);  // никакого курсора!
                }
                else {
                    cursorState.hCursor_ = (HCURSOR)::GetClassLong(hWnd, GCL_HCURSOR);
                    ::SetCursor(cursorState.hCursor_);
                }

            }
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

//-----------------------------------------------------------------------------

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    WAFFDATM::CommonInitialize();
    SetWAFFDATM_Functions();
    assert(WAFFDATM_ConfigurateProject != 0);
    assert(WAFFDATM_InitializeProject != 0);
    assert(WAFFDATM_UpdateProject != 0);
    assert(WAFFDATM_RenderProject != 0);
    assert(WAFFDATM_ShutdownProject != 0);
    assert(WAFFDATM_GetProject_WindowWidth != 0);
    assert(WAFFDATM_GetProject_WindowHeight != 0);
    assert(WAFFDATM_GetProject_WindowTitle != 0);
    if (WAFFDATM_ConfigurateProject == 0) return 0;
    if (WAFFDATM_InitializeProject == 0) return 0;
    if (WAFFDATM_UpdateProject == 0) return 0;
    if (WAFFDATM_RenderProject == 0) return 0;
    if (WAFFDATM_ShutdownProject == 0) return 0;
    if (WAFFDATM_GetProject_WindowWidth == 0) return 0;
    if (WAFFDATM_GetProject_WindowHeight == 0) return 0;
    if (WAFFDATM_GetProject_WindowTitle == 0) return 0;

    if (WAFFDATM_ConfigurateProject() != 0) {
        return 0;
    }
    assert (WAFFDATM_GetProject_WindowWidth() >= 32); // Слишком маленькие окна не поддерживаем
    assert (WAFFDATM_GetProject_WindowHeight() >= 32);
    assert (WAFFDATM_GetProject_WindowTitle() != 0);
    if (WAFFDATM::CreateMainWindow(hThisInstance, nCmdShow,
                                    WAFFDATM_GetProject_WindowWidth(),
                                    WAFFDATM_GetProject_WindowHeight(),
                                    WAFFDATM_GetProject_WindowTitle()) == 0)
        return 0;

    bool fQuitMainLoop = false; // триггер выхода из главного цикла
    if (WAFFDATM_InitializeProject() != 0) {
        fQuitMainLoop = true;
    }
    else {
        WAFFDATM::PrepareMainLoopVars();
    }

    // Enter the message loop

    MSG msg;

    while(!fQuitMainLoop) {
        if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT)
                fQuitMainLoop = true;

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else {
            if (WAFFDATM::CheckForUpdate()) {
                WAFFDATM::Update();
                WAFFDATM_UpdateProject(WAFFDATM::GetDeltaTime());

            }
            WAFFDATM_RenderProject();
        }
    }

    WAFFDATM_ShutdownProject();
    WAFFDATM::Cleanup();

    return 0;
}
