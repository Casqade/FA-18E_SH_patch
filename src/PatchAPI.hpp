#pragma once

#include <windows.h>

#if defined(PATCH_ENABLE_CONSOLE)
  VOID CreateConsole();
#endif


VOID EraseCode(DWORD destAddress, BYTE nopCount);
VOID InjectCode(DWORD destAddress, VOID(*func)(VOID), BYTE nopCount);
VOID WriteBytesASM(DWORD destAddress, LPVOID patch, DWORD numBytes);

VOID PatchInit();
