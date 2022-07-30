# AMTest

## Introduction

It is a simple browser of a map that is read from a file. It is possible to move and zoom. It works on Android and Windows.

Long years ago, I crafted it as homework while applying to a company that specializes in car navigation products. First, I created a prototype, and it was enough to get the job. Then, out of curiosity, I pushed the app somewhat farther. Those days, the company specialized in applications working on devices with WinMobile/WinCE, then also added products for Android and iOS.

## Sources

I worked on this project in my free time at the turn of 2012-2013. Recently, before pushing to GitHub, I refreshed it a little, improved the code slightly (mainly C++), and ported it to the newest Android Studio (Chipmunk 2021.2.1) and Visual Studio 2022 (Community Edition).

### The directory structure

* BackEnd: platform independent backend implemented in C++ - loading the map, structures, and algorithms responsible for storing and processing it
	* various limitations and assumptions are set in [detail/beConsts.h](BackEnd/detail/beConsts.h)
	* [detail/beConfig.h](BackEnd/detail/beConfig.h) contains a few macros to drive the diagnostic code, which by default is enabled in the Debug configuration only.
* FrontEndAndroid: Android application (implemented with Java)
	* [assets/mapa.dat](FrontEndAndroid/app/src/main/assets/mapa.dat) - provided sample map
	* [app/src/main/java](FrontEndAndroid/app/src/main/java) - front-end code
	* [cpp/backendBridge.cpp](FrontEndAndroid/app/src/main/cpp/backendBridge.cpp) - JNI connector between front-end (Java) and back-end (C++)
* FrontEndWinAPI: Windows application (implemented with WinAPI/C++)

### Format of the map

The sample map is located under [mapa.dat](FrontEndAndroid/app/src/main/assets/mapa.dat).
It is a binary file, and its format is as follows:
* int32: number of segments
* then segments in the following format (each segment consists of):
	- int32: class of the road
	- int32: number of points
	- then points in the following format (each point is):
		- int32: x
		- int32: y

## How to build

### Android

* open [FrontEndAndroid project](FrontEndAndroid) in Android Studio (tested on version Chipmunk 2021.2.1)
* select build variant (debug or release)
* build, and run

[Click here to open a sample screenshot of the application running on the emulator in the Android Studio.](https://raw.githubusercontent.com/marinesovitch/media/trunk/AMTest/android_studio_emulator.png)

### Windows

* open [AMtestWin.sln](AMtestWin.sln) in Visual Studio (tested on VS2022 Community Edition)
* choose the desired configuration (Debug or Release) and platform (Win32 or x64)
* build, depending on the chosen setup, the application will be stored in `[Win32|x64]/[Debug|Release]/FrontEndWinAPI.exe`
* copy [mapa.dat](FrontEndAndroid/app/src/main/assets/mapa.dat) to:
	- [FrontEndWinAPI](FrontEndWinAPI) project directory in case the application is executed from VS
	- or to the directory where the `FrontEndWinAPI.exe` binary is located, e.g. in case it is started directly
* run

[Click here to open a sample screenshot of the application running under Windows.](https://raw.githubusercontent.com/marinesovitch/media/trunk/AMTest/winapi.png)

## How to use

### User actions

| Action                     | Android gesture                                  | Windows mouse / keyboard                 |
|           :---:            |                   :---:                          |                 :---:                    |
| center map to a given point | single-tap with one finger                      | left-mouse-button-click                  |
| zoom in                    | double-tap with one finger                       | CTRL-button + left-mouse-button-click    |
| zoom out                   | single-tap with two fingers                      | CTRL-button + right-mouse-button-click   |
| pinch-to-zoom              | spread / pinch two fingers                       | *unavailable*                            |
| move / scroll              | pan with one finger or arrows on DPAD (if available) | arrow keyboard buttons               |
| context menu               | tap on the three-dots symbol in the top-right corner | right-mouse-button-click             |

### Context menu

In the context menu, there are available two options:
* "Reset view" - bring back the initial view (position and zoom factor). Useful when a user moves too far and loses the map from the view.
* "Show params" - show current view parameters in the top-left corner. Useful during debugging. There are:
	- the edges position (l-left, t-top, r-right, b-bottom)
	- the zoom-factor

## Some implementation details

I use the range tree and the interval tree to build a structure that is used to search the map. The former is for detecting segments that have at least one end inside the currently displayed map area, while the latter is meant for detecting segments that have both ends outside of the visible area but cross its edges.

I use the Bresenham algorithm to draw lines. To determine the part of the segment that is visible (and therefore has to be drawn), I use windowing by halving.

Only integer arithmetic and the simplest operations like addition / subtraction and shifts (for division / multiplication) are used in critical pieces of code. Floating-point operations occur in pieces of code that are not critical for overall performance.

While working on the project, I used the following literature:
* "Computational Geometry. Algorithms and Applications", M. de Berg, M. van Kreveld, M. Overmars, O. Schwarzkopf
* "Elementy grafiki komputerowej" (Eng. "Elements of computer graphics"), Michał Jankowski
