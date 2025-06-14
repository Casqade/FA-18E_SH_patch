# FA-18E_SH_patch

[![Windows (MSVC)](https://github.com/casqade/FA-18E_SH_patch/actions/workflows/windows-build-msvc.yml/badge.svg)](https://github.com/casqade/FA-18E_SH_patch/actions/workflows/windows-build-msvc.yml)
[![GitHub Releases](https://img.shields.io/github/release/casqade/FA-18E_SH_patch.svg)](https://github.com/casqade/FA-18E_SH_patch/releases/latest)

---

A fanmade patch for F/A-18E Super Hornet (2000, Digital Integration). 
It uses a technique called DLL injection, 
the way it works is described in
[Beginners Guide to Codecaves](https://web.archive.org/web/20240910225721/codeproject.com/Articles/20240/The-Beginners-Guide-to-Codecaves),
and the injection routine itself is taken from there as well. 
I only added CMake support, executable hash verification 
and the code for the patch itself. 
The current codebase version is based on my 
[dll injection template](https://github.com/Casqade/dll-injection-template).

This project was my first experience in reverse engineering back in 2020. 
Annoyed by several inconsistencies between the game and its manual, I
booted up Cheat Engine, dived into assembly underworld and somehow arrived here. 


## Requirements:

This patch expects the following MD5 hash for `F18.exe`: `3E9301D75C8D3B212AA18C9A88CD06BF`. 
It's valid for both [Steam version](https://store.steampowered.com/app/776050/FA18E_Super_Hornet) 
and retail version with Albanian Campaign DLC installed and NoCD applied 
(Fun fact: Steam version is distributed with NoCD, I bet the same goes for GOG version). 
Unfortunately, I don't own GOG version to check the hash, 
so if you happen to have it, please report whether this patch works for you. 


## Usage: 

1. Download & unpack the [latest release](https://github.com/casqade/FA-18E_SH_patch/releases/latest) of `FA-18E_SH_patch-win32.zip`
2. Copy both `F18loader.exe` and `F18patch.dll` into 
game's root directory right next to original `F18.exe`. 
3. Run `F18loader.exe`.
4. If no errors occurred, the loader should automatically launch the game. 


## Fixes:
- High framerate causes issues with the game,
e.g. Course & Heading Select switches stop working properly.
The game was clearly designed to run at 30 FPS, so this patch
locks framerate to this value (Note: if you use DxWnd,
make sure to disable VSync and other timing features). 
- In original game, it's impossible to power off
APU & engines using GUI. APU is automatically disabled after a
certain time, and engines can only be turned off via hotkeys.
This patch disables automatic APU shutdown and allows full
control over both APU and engines with corresponding switches.
- Landing request can only be initiated using a hotkey. The patch
adds a dedicated button into UFCD's communication channel selection screen.
- Some minor ones like IFF and radar altimeter not being inhibited by
EMCON mode and some game variables not being reset when mission is restarted. 


## Demonstration

### CRS & HDG Select switches, APU & Engines switches

**Original**:

https://github.com/user-attachments/assets/e093a70e-b89c-42f7-9d17-9e3efb6580a4

**Patched**:

https://github.com/user-attachments/assets/9bef5ee2-c305-4f18-aafc-a2f9fd50b6c5


### Request landing permission button

**Original**:

https://github.com/user-attachments/assets/3ce1467b-e2e4-47e4-be4b-7b72da1c26fc

**Patched**:

https://github.com/user-attachments/assets/50e97278-dbac-4b94-b824-9e9dba8ca337


## References:
- [The Beginners Guide to Codecaves](https://www.codeproject.com/Articles/20240/The-Beginners-Guide-to-Codecaves)
- [A More Complete DLL Injection Solution Using CreateRemoteThread](https://www.codeproject.com/Articles/20084/A-More-Complete-DLL-Injection-Solution-Using-Creat)

