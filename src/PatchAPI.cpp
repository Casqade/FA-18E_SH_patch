#include "PatchAPI.hpp"

#if defined(PATCH_ENABLE_CONSOLE)
  #include <stdio.h>
#endif


// Initialize function called by the loader's inject function
extern "C" __declspec(dllexport) void Initialize()
{
  PatchInit();

#if defined(PATCH_ENABLE_CONSOLE)
  CreateConsole();
#endif
}


// Define the plugins main, use a define since the code is the same for all plugins
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
  // Get rid of compiler warnings since we do not use this parameter
  UNREFERENCED_PARAMETER(lpReserved);

  // If we are attaching to a process, we do not want the dll thread messages
  if (ulReason == DLL_PROCESS_ATTACH)
    DisableThreadLibraryCalls(hModule);

  // Always load/unload
  return TRUE;
}


// Writes bytes in the current process using an ASM method
VOID WriteBytesASM(DWORD destAddress, LPVOID patch, DWORD numBytes)
{
  if ( numBytes < 1 )
    return;


  // Store old protection of the memory page
  DWORD oldProtect = 0;

  // Store the source address
  DWORD srcAddress = PtrToUlong(patch);

  // Make sure page is writeable
  VirtualProtect((void*)(destAddress), numBytes, PAGE_EXECUTE_READWRITE, &oldProtect);

  // Do the patch (oldschool style to avoid memcpy)
  __asm
  {
    nop           // Filler
    nop           // Filler
    nop           // Filler

    mov esi, srcAddress   // Save the address
    mov edi, destAddress  // Save the destination address
    mov ecx, numBytes   // Save the size of the patch

    Start :
    cmp ecx, 0        // Are we done yet?
      jz Exit         // If so, go to end of function

      mov al, [esi]     // Move the byte at the patch into AL
      mov[edi], al      // Move AL into the destination byte
      dec ecx         // 1 less byte to patch
      inc esi         // Next source byte
      inc edi         // Next destination byte
      jmp Start       // Repeat the process

    Exit :
      nop           // Filler
      nop           // Filler
      nop           // Filler
  }

  // Restore old page protection
  VirtualProtect((void*)(destAddress), numBytes, oldProtect, &oldProtect);
}


VOID EraseCode(DWORD destAddress, BYTE nopCount)
{
  if ( nopCount < 1 )
    return;

  // Buffer of NOPs, static since we limit to 'UCHAR_MAX' NOPs
  BYTE nopPatch[0xFF] = { 0 };

  // Fill it with nops
  memset(nopPatch, 0x90, nopCount);

  // Make the patch now
  WriteBytesASM(destAddress, nopPatch, nopCount);
}


// Codecave function
VOID InjectCode(DWORD destAddress, VOID(*func)(VOID), BYTE nopCount)
{
  // Calculate the code cave
  DWORD offset = (PtrToUlong(func) - destAddress) - 5;

  // Construct the patch to the function call
  BYTE patch[5] = { 0xE8, 0x00, 0x00, 0x00, 0x00 };
  memcpy(patch + 1, &offset, sizeof(DWORD));
  WriteBytesASM(destAddress, patch, 5);

  EraseCode(destAddress + 5, nopCount);
}


#if defined(PATCH_ENABLE_CONSOLE)
VOID CreateConsole()
{
  AllocConsole();

  FILE* fDummy;
  freopen_s(&fDummy, "CONIN$", "r", stdin);
  freopen_s(&fDummy, "CONOUT$", "w", stderr);
  freopen_s(&fDummy, "CONOUT$", "w", stdout);

  // Note that there is no CONERR$ file
  HANDLE hStdout = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  HANDLE hStdin = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  SetStdHandle(STD_OUTPUT_HANDLE, hStdout);
  SetStdHandle(STD_ERROR_HANDLE, hStdout);
  SetStdHandle(STD_INPUT_HANDLE, hStdin);
}
#endif
