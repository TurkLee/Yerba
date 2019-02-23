# Yerba ![alt text](https://i.imgur.com/RHlmKYL.png) ![alt text](https://i.imgur.com/1OKJ96Z.png) ![alt text](https://i.imgur.com/RPz95ve.png) ![alt text](https://i.imgur.com/ZXcfFYJ.png)
Open-source cheat for Counter-Strike: Global Offensive game.

## Features:
* D3D9 menu with automatically adjusted positions of the selectables.
* Netvar manager.
* Easy to understand VMT hooking class.
* Debug console output.
* Basic hooking and unhooking concept.

## Information:
* You should install the fonts that are in the Antario folder.
* Based on Antario (https://github.com/Wando1423/Antario)

## Menu:
![alt text](https://i.imgur.com/dPCQvFG.png)
![alt text](https://i.imgur.com/3lVaWR5.png)
![alt text](https://i.imgur.com/rIgGA5r.png)

## Getting started

### Prerequisites
Microsoft Visual Studio 2017 15.9.4 and Windows SDK 10.0.17763.0 (10.0.17134.0 or lower should work too but requires change in project settings) are required in order to compile yerba. If you don't have ones, you can download VS [here](https://visualstudio.microsoft.com/) (Windows SDK is installed during Visual Studio Setup).

### Cloning
The very first step in order to compile yerba is to clone this repo from GitHub to your local computer:
```
git clone https://github.com/Lumm1t/Yerba.git
```

### Compiling from source

When you have equiped a copy of source code, next step is opening **yerba.sln** in Microsoft Visual Studio 2017.

Then change build configuration to `Release | x86` and simply press **Build solution**.

If everything went right you should receive `yerba.dll`  binary file.

### Loading / Injecting into game process

Open [DLL injector](https://en.wikipedia.org/wiki/DLL_injection) and inject `yerba.dll` into `csgo.exe` process.

Press `INSERT` key while focused on CS:GO window.
