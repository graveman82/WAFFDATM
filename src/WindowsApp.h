#ifndef WAFFDATM_WINDOWSAPP_H
#define WAFFDATM_WINDOWSAPP_H

#include <windows.h>

/** @file */
/** WinAppUsage Использование модуля приложения windows.
Вы должны реализовать функции согласно прототипам:
long WAFFDATM_ConfigurateProject();
long WAFFDATM_InitializeProject();
long WAFFDATM_UpdateProject(float deltaTime);
long WAFFDATM_RenderProject();
long WAFFDATM_ShutdownProject();

unsigned long WAFFDATM_GetProject_WindowWidth();
unsigned long WAFFDATM_GetProject_WindowHeight();
const char* WAFFDATM_GetProject_WindowTitle();
Далее реализуйте функцию SetWAFFDATM_Functions() в своем коде, в которой проинициализируйте соответствующие
указатели на функции, объявления которых даны в файле ProjectInterface.h. Смотрите пример к библиотеке.
( @see SetWAFFDATM_Functions() )

Включите файлы WindowsApp.h, ProjectInterface.h и WindowsApp.cpp в свой проект (в среде разработки).
Функции, вызов которых разрешен в функции, реализованной согласно прототипу WAFFDATM_ConfigurateProject():
 - SetCursorToogleVKey()
 - SetFrameRate

Функции, вызов которых разрешен в функции, реализованной согласно прототипу WAFFDATM_InitializeProject():
 - IsCursorHidden()
 - GetHinstanse()
 - GetWindowHandle()

Функции, вызов которых разрешен в UpdateProject():
 - IsCursorHidden()
 - GetHinstanse()
 - GetWindowHandle()
 - IsKeyPressed()
 - GetMouseDeltaX()
 - GetMouseDeltaY()
 - GetDeltaTime()
 - GetFps()
*/
namespace WAFFDATM
{
// Коды клавиш движка
const unsigned long kKEY_A = 10;
const unsigned long kKEY_B = 11;
const unsigned long kKEY_C = 12;
const unsigned long kKEY_D = 13;
const unsigned long kKEY_E = 14;
const unsigned long kKEY_F = 15;
const unsigned long kKEY_G = 16;
const unsigned long kKEY_H = 17;
const unsigned long kKEY_I = 18;
const unsigned long kKEY_J = 19;
const unsigned long kKEY_K = 20;
const unsigned long kKEY_L = 21;
const unsigned long kKEY_M = 22;
const unsigned long kKEY_N = 23;
const unsigned long kKEY_O = 24;
const unsigned long kKEY_P = 25;
const unsigned long kKEY_Q = 26;
const unsigned long kKEY_R = 27;
const unsigned long kKEY_S = 28;
const unsigned long kKEY_T = 29;
const unsigned long kKEY_U = 30;
const unsigned long kKEY_V = 31;
const unsigned long kKEY_W = 32;
const unsigned long kKEY_X = 33;
const unsigned long kKEY_Y = 34;
const unsigned long kKEY_Z = 35;

/** @brief Возвращает true, если клавиша с заданным кодом находится в нажатом состоянии.
*/
bool IsKeyPressed(int nKey);
/** @brief Возвращает смещение указателя мыши по X в пикселях. */
float GetMouseDeltaX();
/** @brief Возвращает смещение указателя мыши по Y в пикселях. */
float GetMouseDeltaY();
/** @brief Возвращает true, если курсор скрыт.*/
bool IsCursorHidden();
/** @brief Установить виртуальную клавишу переключения видимости курсора. */
void SetCursorToogleVKey(int vKey);
/** @brief Возвращает время, прошедшее с момента предыдущего обновления проекта.
Обновление проекта происходит при вызове UpdateProject().
*/
int GetDeltaTime();
/** @brief Получить число кадров в секунду. */
float GetFps();
/** @brief Установка частоты кадро обновления состояния проекта. */
void SetFrameRate(float fps);

/// Получить дескриптор приложения.
HINSTANCE GetHinstanse();
/// Получить дескриптор окна.
HWND GetWindowHandle();

} // end of WAFFDATM

#endif // WAFFDATM_WINDOWSAPP_H

