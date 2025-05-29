#include "PatchAPI.hpp"

// hidden by WIN32_LEAN_AND_MEAN
typedef double DOUBLE;


DWORD returnAddr = 0;


/// Patch addresses
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

DWORD injectAddr_ApuAutoShutDown1 = 0x4CFBB6;
DWORD injectAddr_ApuAutoShutDown2 = 0x4CFF25;

DWORD injectAddr_ApuAutoSwitchDown = 0x4DA9DA;

DWORD injectAddr_CrankLEng = 0x4DC72B;
DWORD injectAddr_CrankREng = 0x4DC79B;

DWORD injectAddr_LEngSwitchUp = 0x4DC70D;
DWORD injectAddr_REngSwitchUp = 0x4DC77D;

DWORD injectAddr_LEngAutoSwitchDown = 0x4DAA43;
DWORD injectAddr_REngAutoSwitchDown = 0x4DAAAC;

DWORD injectAddr_ToggleApu = 0x51DC68;
DWORD injectAddr_ShutdownApuLamps = 0x4CFB68;

DWORD injectAddr_ShutdownLEng = 0x51DCF0;
DWORD injectAddr_ShutdownREng = 0x51DD51;

DWORD injectAddr_PressRequestLandingBtn = 0x54E4A1;
DWORD injectAddr_DrawRequestLandingBtn  = 0x54F581;


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
DWORD gameProcId = 0;
DWORD windowInFocusProcId = 0;

DWORD currentTime;
DWORD previousTime;

BOOL gameSpeedLockEnabled = TRUE;
CONST FLOAT gameSpeedRate = 30.0f;
CONST DWORD gameSpeedDelta = 1000.0f / gameSpeedRate;

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


__declspec(naked) void ApuToggle(void)
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
    *(DWORD*) addr_ApuCooldownTimePoint = 0; // reset apu cooldown


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


__declspec(naked) void EngineLSwitchUp(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if ( *(DWORD*) addr_ApuOn > 0 ||
       *(DWORD*) addr_EngineROn > 0 &&
       *(DWORD*) addr_EngineRRpm >= engineRRpmIdle )
    *(DWORD*) addr_EngineLOn = 1;

  __asm
  {
    POPFD
    POPAD

    MOV EAX, 29h

    push returnAddr
    ret
  }
}


__declspec(naked) void EngineRSwitchUp(void)
{
  __asm
  {
    pop returnAddr

    PUSHAD
    PUSHFD
  }

  if ( *(DWORD*) addr_ApuOn > 0 ||
       *(DWORD*) addr_EngineLOn > 0 &&
       *(DWORD*) addr_EngineLRpm >= engineLRpmIdle )
    *(DWORD*) addr_EngineROn = 1;

  __asm
  {
    POPFD
    POPAD

    MOV EAX, 29h

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

    CALL [func_ShutdownLEng]
  }

  if (  *(DWORD*) addr_EngineLOn < 1 &&
        *(DWORD*) addr_ApuOn == 0 &&
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

    CALL [func_ShutdownREng]
  }

  if (  *(DWORD*) addr_EngineROn < 1 &&
        *(DWORD*) addr_ApuOn == 0 &&
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

//  yield CPU time when not in focus
  while ( windowInFocusProcId != gameProcId )
  {
    Sleep(500);
    GetWindowThreadProcessId( GetForegroundWindow(), &windowInFocusProcId );
  }

  *(DWORD*) addr_ShowFPS = 1;

//  We lock game speed to 30 FPS to avoid anomalies like
//  Course & Heading (CRS & HDG) Select switches
//  not being able to rotate counter-clockwise.
//
//  The following snippet is identical to
//  DxWnd's LimitFrameCount implementation
//  (https://github.com/DxWnd/DxWnd.reloaded/blob/7235efb7388b25a5dd1192b74df3c4860435ff65/dll/dxwcore.cpp#L678),
//  meaning VSync & any FPS limits in your
//  DxWnd config must be disabled
//
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


void PatchInit()
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


//  Prevent automatic APU shutdown
  EraseCode(injectAddr_ApuAutoShutDown1, 7);
  EraseCode(injectAddr_ApuAutoShutDown2, 28);

//  Prevent APU switch to go down automatically
  EraseCode(injectAddr_ApuAutoSwitchDown, 21);


//  Prevent both engines from starting up APU
  EraseCode(injectAddr_CrankLEng, 21);
  EraseCode(injectAddr_CrankREng, 21);

//  Prevent engines from starting without active APU
  InjectCode(injectAddr_LEngSwitchUp, EngineLSwitchUp, 11);
  InjectCode(injectAddr_REngSwitchUp, EngineRSwitchUp, 11);

//  Prevent both engine switches to go down automatically
  EraseCode(injectAddr_LEngAutoSwitchDown, 21);
  EraseCode(injectAddr_REngAutoSwitchDown, 20);


  InjectCode(injectAddr_ToggleApu, ApuToggle, 17);
  InjectCode(injectAddr_ShutdownApuLamps, ApuShutdownLamps, 3);

  InjectCode(injectAddr_ShutdownLEng, EngineLShutdown, 0);
  InjectCode(injectAddr_ShutdownREng, EngineRShutdown, 0);

  InjectCode(injectAddr_PressRequestLandingBtn, PressRequestLandingBtn, 4);
  InjectCode(injectAddr_DrawRequestLandingBtn,  DrawRequestLandingBtn,  1);


  gameProcId = GetCurrentProcessId();
}
