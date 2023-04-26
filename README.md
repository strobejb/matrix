# Matrix Screensaver

This is the source code to the "Matrix" screensaver that was published to my website rather a long time ago - I'm guessing around the  1999/2000 timeframe. The code and screensaver haven't been altered in that time, and it must be well over 20 years old by now! Apparently there is still some interest in this project because I get asked questions from time to time. So here it is (found buried in a backup CD I made at the time). Please be mindful that this is _really_ old code, written purely in Win32 C++, and even that was considered dated at the time! 

# Building

This project was originally written for Visual Studio 6, and was designed to run on Windows 95 and Windows NT at the time. To build this project today, you will probably need to:

* Install a recent version of Visual Studio 
* Install Windows/Platform SDK of your choice
* Open the original project file (DSW/DSP) and let VS convert them to the new formats
* Retarget the project to use whatever Windows SDK has installed (right-click the project and edit the properties to select the SDK)

Tested on Visual Studio 2017 free/community edition, and amazingly it still compiles and appears to work.

# Releasing

To turn this into a 'proper' screen saver, I think all that needs to be done is to rename the `matrix.exe` executable to `matrix.scr`. Do these old-school screensavers even work in Windows anymore!? 



