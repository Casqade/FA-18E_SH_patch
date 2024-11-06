#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>

//-----------------------------------------------------------------------------
VOID EraseCode(DWORD destAddress, BYTE nopCount);
VOID InjectCode(DWORD destAddress, VOID(*func)(VOID), BYTE nopCount);
VOID WriteBytesASM(DWORD destAddress, LPVOID patch, DWORD numBytes);

#if defined(PATCH_ENABLE_CONSOLE)
VOID CreateConsole();
#endif

//-----------------------------------------------------------------------------

DWORD currentTime;
DWORD previousTime;

//-----------------------------------------------------------------------------

BOOL  gameSpeedLockEnabled = TRUE;
CONST FLOAT gameSpeedRate = 30.0f;
CONST DWORD gameSpeedDelta = 1000.0f / gameSpeedRate;

//-----------------------------------------------------------------------------


DWORD returnAddr = 0;

DWORD gameProcId = 0;
DWORD windowInFocusProcId = 0;

DWORD injectAddr_MissionInit  = 0x413ACA;
DWORD injectAddr_SuspendCheck = 0x4045BE;

DWORD injectAddr_IffToggle    = 0x54E3D4;
DWORD injectAddr_EmconEnable  = 0x51E75B;
DWORD injectAddr_EmconDisable = 0x51E89F;

DWORD injectAddr_InhibitHUDRadarAltimeterP1 = 0x54277F;
DWORD injectAddr_InhibitHUDRadarAltimeterP2 = 0x542796;

DWORD injectAddr_InhibitEADRadarAltimeterP1 = 0x5307CD;
DWORD injectAddr_InhibitEADRadarAltimeterP2 = 0x530867;

DWORD injectAddr_InhibitFLIRRadarAltimeterP1 = 0x53F147;
DWORD injectAddr_InhibitFLIRRadarAltimeterP2 = 0x53F1C0;

DWORD injectAddr_CrankLEng     = 0x4DC735;
DWORD injectAddr_CrankREng     = 0x4DC7A5;

DWORD injectAddr_ShutdownApu      = 0x51DC68;
DWORD injectAddr_ShutdownApuLamps = 0x4CFB68;

DWORD injectAddr_ShutdownLEng     = 0x51DCF0;
DWORD injectAddr_ShutdownREng     = 0x51DD51;

DWORD injectAddr_PressRequestLandingBtn = 0x54E4A1;
DWORD injectAddr_DrawRequestLandingBtn = 0x54F581;


/// Ingame variables
DWORD addr_ShowFPS                = 0x603868;
DWORD addr_TimeSinceMissionStart  = 0x66412C;

DWORD addr_IffOn                  = 0x16D2768;
DWORD addr_EmconOn                = 0x633E04;
DWORD addr_BaroAltimeterOn        = 0x16DEAA4;

DWORD addr_ApuOn                  = 0x153BC78;
DWORD addr_ApuSwitchDown          = 0x153F900;
DWORD addr_ApuLampsTimeOut        = 0x153BC80;  // actually it's time point when lamps should go off
DWORD addr_ApuCooldownTimePoint   = 0x153BC7C;

DWORD addr_EngineLOn              = 0x153BBCC;
DWORD addr_EngineLRpm             = 0x153B4B0;

DWORD addr_EngineROn              = 0x153BC24;
DWORD addr_EngineRRpm             = 0x153B518;

DWORD addr_Channel1Frequency      = 0x16DEF30;
DWORD addr_LandingClearance       = 0x633674;

DWORD addr_LaunchBarOn            = 0x633E10;


/// Ingame functions
DWORD func_MissionInit  = 0x59F1B0;
DWORD func_MissionSubr  = 0x4A0450;
DWORD func_SwitchSetOff = 0x51E960;

DWORD func_ShutdownApu  = 0x4CFF25;
DWORD func_ShutdownLEng = 0x4DC6A0;
DWORD func_ShutdownREng = 0x4DC6C0;

DWORD func_RequestLanding = 0x515F20;
DWORD func_CancelLanding = 0x515E50;


/// DLL variables
DWORD iffOn   = 0;
DWORD iffPrev = 0;
DWORD emconOn = 0;
DWORD baroAltimeterOn = 0;

DWORD ufcdSelectedButton = 0;
DWORD pressedButtonIndex = 0;
DWORD channel1Frequency = 0;

DOUBLE engineLRpmIdle = 60.0;
DOUBLE engineRRpmIdle = 60.0;

DWORD requestLandingBtnState = 0;

CONST CHAR requestLandingBtnTitle[] = "REQUEST";
CONST CHAR requestLandingBtnValue[] = "Landing";
CONST CHAR cancelLandingBtnTitle[] = "CANCEL";


/// Injected functions
__declspec(naked) void IffToggle(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if ( *(DWORD*) addr_EmconOn != 0 )
    *(DWORD*) addr_IffOn = 1;

  iffOn = *(DWORD*) addr_IffOn;

  __asm
  {
    POPFD
    POPAD

    CMP iffOn, 0

    push returnAddr
    ret
  }
}


__declspec(naked) void EmconEnable(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  emconOn = 1;
  iffPrev = *(DWORD*) addr_IffOn;  // save IFF state to restore it after turning EMCON off

  __asm
  {
    POPFD
    POPAD

    MOV EAX, 0  //  disable IFF

    push returnAddr
    ret
  }
}


__declspec(naked) void EmconDisable(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  emconOn = 0;

  __asm
  {
    POPFD
    POPAD

    MOV EAX, iffPrev  // restore IFF state prior to enabling Emissions Control

    push returnAddr
    ret
  }
}


__declspec(naked) void InhibitRadarHUDAltimeter(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  baroAltimeterOn = *(DWORD*) addr_BaroAltimeterOn;

  if ( emconOn && !baroAltimeterOn )
    __asm
    {
      POPFD
      POPAD

      MOV ECX, 1  // inhibit radar altimeter if EMCON is active
    }
  else
    __asm
    {
      POPFD
      POPAD

      MOV EBX, baroAltimeterOn
    }

  __asm
  {
    push returnAddr
    ret
  }
}


__declspec(naked) void InhibitRadarAltimeterP1(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  baroAltimeterOn = *(DWORD*) addr_BaroAltimeterOn;

  if ( emconOn && !baroAltimeterOn )
    __asm
    {
      POPFD
      POPAD

      MOV EBX, 1  // inhibit radar altimeter if EMCON is active
    }
  else
    __asm
    {
      POPFD
      POPAD

      MOV EBX, baroAltimeterOn
    }

  __asm
  {
    push returnAddr
    ret
  }
}


__declspec(naked) void InhibitRadarAltimeterP2(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if ( emconOn && !baroAltimeterOn )
  {
    __asm
    {
      POPFD
      POPAD

      CMP emconOn, 1  // inhibit radar altimeter if EMCON is active
    }
  }
  else
  {
    __asm
    {
      POPFD
      POPAD

      CMP baroAltimeterOn, 0
    }
  }

  __asm
  {
    push returnAddr
    ret
  }
}


__declspec(naked) void ForbidEnginesToStartApu(void)
{
  __asm
  {
    pop returnAddr

    push returnAddr
    ret
  }
}


__declspec(naked) void ApuShutdown(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if ( *(DWORD*) addr_ApuSwitchDown == 0 )
  {
    *(DWORD*) addr_ApuOn = 0;           //  shutdown APU
    *(DWORD*) addr_ApuSwitchDown = 1;   //  throw its switch down
    *(DWORD*) addr_ApuLampsTimeOut = 0; //  reset cockpit lights illumintation timeout


    if (  *(DOUBLE*) addr_EngineLRpm < engineLRpmIdle &&
          *(DOUBLE*) addr_EngineRRpm < engineRRpmIdle )
    {
      *(DWORD*) addr_EngineLOn = 0;   //  shut down engines if
      *(DWORD*) addr_EngineROn = 0;   //  none of them reached idle RPM
    }
  }
  else
    *(DWORD*) addr_ApuSwitchDown = 0;

  __asm
  {
    POPFD
    POPAD

    MOV EAX, 261h

    push returnAddr
    ret
  }
}


__declspec(naked) void ApuShutdownLamps(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  __asm
  {
    POPFD
    POPAD

    MOV ESI, 1h     // force cockpit lights illuminated by APU startup
    TEST ESI, ESI   // to extinguish in case APU was shut down too fast

    push returnAddr
    ret
  }
}


__declspec(naked) void EngineLShutdown(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if (  *(DWORD*) addr_ApuOn == 0 &&
        *(DWORD*) addr_EngineROn > 0 &&
        *(DOUBLE*) addr_EngineRRpm < engineRRpmIdle )
  {
    __asm
    {
      CALL [func_ShutdownREng]  // shutdown right engine if it did not finish startup
    }
  }

  __asm
  {
    CALL [func_ShutdownLEng]

    POPFD
    POPAD

    CALL [func_SwitchSetOff]

    push returnAddr
    ret
  }
}


__declspec(naked) void EngineRShutdown(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if (  *(DWORD*) addr_ApuOn == 0 &&
        *(DWORD*) addr_EngineLOn > 0 &&
        *(DOUBLE*) addr_EngineLRpm < engineLRpmIdle )
  {
    __asm
    {
      CALL [func_ShutdownLEng]  // shutdown left engine if it did not finish startup
    }
  }

  __asm
  {
    CALL [func_ShutdownREng]

    POPFD
    POPAD

    CALL [func_SwitchSetOff]

    push returnAddr
    ret
  }
}


__declspec(naked) void PressRequestLandingBtn(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD

    MOV pressedButtonIndex, EAX
  }

  if ( pressedButtonIndex == 4 )
  {
    if ( *(DWORD*) addr_LandingClearance )
    {
      __asm
      {
        MOV ECX, 26
        CALL[func_CancelLanding]
        MOV requestLandingBtnState, EAX
      }
    }
    else
    {
      __asm
      {
        MOV ECX, 26
        CALL[func_RequestLanding]
        MOV requestLandingBtnState, EAX
      }
    }
  }

  channel1Frequency = *(DWORD*) addr_Channel1Frequency;   // required for moving to EDX later on

  __asm
  {
    POPFD
    POPAD

    MOV EDX, channel1Frequency
    CMP EAX, 3

    push returnAddr
    ret
  }
}


__declspec(naked) void DrawRequestLandingBtn(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  requestLandingBtnState = *(DWORD*) addr_LandingClearance;

  if ( requestLandingBtnState != 0 )
  {
    __asm
    {
      POPFD
      POPAD

      MOV ECX, requestLandingBtnState         // button state
      MOV EDX, offset cancelLandingBtnTitle   // title text
      MOV EBX, offset requestLandingBtnValue  // value text

      push returnAddr
      ret
    }
  }
  else
  {
    __asm
    {
      POPFD
      POPAD

      MOV ECX, requestLandingBtnState         // button state
      MOV EDX, offset requestLandingBtnTitle  // title text
      MOV EBX, offset requestLandingBtnValue  // value text

      push returnAddr
      ret
    }
  }
}


__declspec(naked) void MissionInit(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  emconOn = false;
  iffPrev = 0;
  *(DWORD*) addr_EmconOn = 0;      // fix EMCON not being initialized when mission starts
  *(DWORD*) addr_ApuOn = 0;       // fix APU not being initialized when mission starts on ground
  *(DWORD*) addr_LaunchBarOn = 0; // fix Launch bar not being initialized when mission starts
  previousTime = GetTickCount();

  __asm
  {
    POPFD
    POPAD

    CALL[func_MissionInit]

    push returnAddr
    ret
  }
}


__declspec(naked) void SuspendIfInactive(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  GetWindowThreadProcessId( GetForegroundWindow(), &windowInFocusProcId );

  while ( windowInFocusProcId != gameProcId )
  {
    Sleep(500);
    GetWindowThreadProcessId( GetForegroundWindow(), &windowInFocusProcId );
  }

  *(DWORD*) addr_ShowFPS = 1;

  if ( gameSpeedLockEnabled )
  {
    currentTime = GetTickCount();

    if ( ( currentTime < previousTime + gameSpeedDelta ) && ( currentTime >= previousTime ) )
    {
      do
      {
        Sleep(previousTime + gameSpeedDelta - currentTime);
        currentTime = GetTickCount();
      } while ( currentTime < previousTime + gameSpeedDelta );
    }

    previousTime += gameSpeedDelta;

    if ( previousTime < currentTime - gameSpeedDelta )
      previousTime = currentTime - gameSpeedDelta;
  }

  __asm
  {
    POPFD
    POPAD

    CALL[func_MissionSubr]

    push returnAddr
    ret
  }
}


// Initialize function called by the loader's inject function
extern "C" __declspec(dllexport) void Initialize()
{
  InjectCode(injectAddr_MissionInit,  MissionInit, 0);
  InjectCode(injectAddr_SuspendCheck, SuspendIfInactive, 0);

  InjectCode(injectAddr_IffToggle, IffToggle, 2);

  InjectCode(injectAddr_EmconEnable,  EmconEnable,  0);
  InjectCode(injectAddr_EmconDisable, EmconDisable, 0);

//  InjectCode(injectAddr_InhibitHUDRadarAltimeterP1, InhibitRadarHUDAltimeter, 1);
//  InjectCode(injectAddr_InhibitHUDRadarAltimeterP2, InhibitRadarAltimeterP2, 2);

  InjectCode(injectAddr_InhibitFLIRRadarAltimeterP1, InhibitRadarAltimeterP1, 1);
  InjectCode(injectAddr_InhibitFLIRRadarAltimeterP2, InhibitRadarAltimeterP2, 2);

  InjectCode(injectAddr_InhibitEADRadarAltimeterP1, InhibitRadarAltimeterP1, 1);
  InjectCode(injectAddr_InhibitEADRadarAltimeterP2, InhibitRadarAltimeterP2, 2);

  InjectCode(injectAddr_CrankLEng, ForbidEnginesToStartApu, 1);
  InjectCode(injectAddr_CrankREng, ForbidEnginesToStartApu, 1);

  InjectCode(injectAddr_ShutdownApu,      ApuShutdown,      17);
  InjectCode(injectAddr_ShutdownApuLamps, ApuShutdownLamps, 3);

  InjectCode(injectAddr_ShutdownLEng, EngineLShutdown, 0);
  InjectCode(injectAddr_ShutdownREng, EngineRShutdown, 0);

  InjectCode(injectAddr_PressRequestLandingBtn, PressRequestLandingBtn, 4);
  InjectCode(injectAddr_DrawRequestLandingBtn,  DrawRequestLandingBtn,  1);


  gameProcId = GetCurrentProcessId();

#if defined(PATCH_ENABLE_CONSOLE)
  CreateConsole();
#endif
}

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

#if defined(PATCH_ENABLE_CONSOLE)
VOID CreateConsole()
{
  int hConHandle = 0;
  HANDLE lStdHandle = 0;
  FILE *fp = 0;

  AllocConsole();

  freopen("CONIN$", "r", stdin);
  freopen("CONOUT$", "w", stderr);
  freopen("CONOUT$", "w", stdout);

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

//-----------------------------------------------------------------------------
