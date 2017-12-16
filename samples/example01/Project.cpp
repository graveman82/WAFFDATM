#include <windows.h>
#include <iostream>
#include "ProjectInterface.h"
#include "WindowsApp.h"

/* Пример найипростейшей реализации проекта для работы с библиотекой.
*/
unsigned long GetProject_WindowWidth() { return 200; }
unsigned long GetProject_WindowHeight() { return 320; }
const char* GetProject_WindowTitle() { return "WAFFDATM sample"; }

long ConfigurateProject(){
    WAFFDATM::SetFrameRate(60);
    //WAFFDATM::SetCursorToogleVKey(0x4A);//VK_J
    return 0;
}

long InitializeProject()
{
    return 0;
}

long UpdateProject(float deltaTime){

    printf("deltaTime=%.1f ms, fps(%.2f)\n", deltaTime, WAFFDATM::GetFps());
    return 0;
}

long RenderProject(){

    return 0;
}

long ShutdownProject()
{
    return 0;
}

void SetWAFFDATM_Functions(){
    WAFFDATM_ConfigurateProject = ConfigurateProject;
    WAFFDATM_InitializeProject = InitializeProject;
    WAFFDATM_UpdateProject = UpdateProject;
    WAFFDATM_RenderProject = RenderProject;
    WAFFDATM_ShutdownProject = ShutdownProject;
    WAFFDATM_GetProject_WindowWidth = GetProject_WindowWidth;
    WAFFDATM_GetProject_WindowHeight = GetProject_WindowHeight;
    WAFFDATM_GetProject_WindowTitle = GetProject_WindowTitle;
}




