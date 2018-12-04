# AirBand
*CSC355: Human Computer Interaction* project using two Myo Armbands to simulate air drumming. The program interprets gestures that the user makes and simulates drumming by importing sounds. The idea is to simulate air drumming by sitting down, putting on two Myo Armbands, and moving your arms and wrists like actual drumming to get a real-time drum session recording.

##### Authors: Pranav Nair, Mark Meddleton, Kevin Stys, Matthew Bottone
##### Release Date: December 3rd, 2018

### Files & Folders:
* include: The include file consists of two files -
	* bass.h - The bass.h is a c++ header file that loads the sound files. 
	* myo -  myo folder contains the files for the myo framework and SDKs
* sounds: Folder containing the various drum sounds that can be played by the myo armband
* source: Folder containing the main.cpp program that runs the myo project
* lib: The lib folder contains the myo.framework and static libraries
* bin: The bin folder contains the dynamic libraries of bass and myo and the executable for Windows

### How Each File Works Together:
All the files are placed in a single folder. The main.cpp file contains the myo.hpp which is included to use the Myo C++ SDK. Similarly, the main.cpp file includes the bass.h which is used to load the drum sounds and play them through different channels.

### To Compile and Run:
**Windows**:

Find the MyoDrum.exe within the files of the projects. Open the .exe file to run the program. Once the file is opened, click on build to run the project. Disconnect each armband first. Then follow the procedure for the first armband(used as the right hand):- connect, disconnect, and re-connect. A “Right hand connected” string will output in the terminal. After that, follow the same procedure for the second armband(used in the left hand). Once this is satisfied, you can start moving the myo armband and start playing the virtual drums.
* Drums Sounds
	* Snare Drum - Middle
	* Hi-Hats - Left
	* Crash Cymbal - Right
	* Ride Cymbal - Slightly right of the Crash
	* Base Drum - Anywhere (face left hand down, knuckles up)
	* Hard Snare Hit - Anywhere (face right hand down, knuckles up)

**Mac**:

When user loads Xcode program, open the AirBand.xcodeproj file which contains all the files for the myo project. Once the file is opened, click on build to run the project. Disconnect each armband first. Then follow the procedure for the first armband(used as the right hand):- connect, disconnect, and re-connect. A “Right hand connected” string will output in the terminal. After that, follow the same procedure for the second armband(used in the left hand). Once this is satisfied, you can start moving the myo armband and start playing the virtual drums.
* Drums Sounds
	* Snare Drum - Middle
	* Hi-Hats - Left
	* Crash Cymbal - Right
	* Ride Cymbal - Slightly right of the Crash


### Milestones:

**Milestone #1**:

 - [x] Connect both Myo Armbands simultaneously
 - [x] Create new 'drum' gesture (using acceleration)
 - [x] Devise a way to make sounds (BASS library with .wav samples)
 - [x] Play overlapping drum sounds

**Milestone #2**:

 - [x] Add different pallettes of sounds
 - [x] Look into using moving arms around in a space for playing different drums
 - [x] Remove flamming from detecting multiple points at acceleration value
 
**Final Code**:

 - [x] Rework hit algorithm using angular velocity
 - [x] Narrow down the 3D space for hitting different drums
 - [x] Fully implement threads for each arm
 - [x] Ability to play a realistic sounding drum beat
 - [x] Clean up the code

**Nice to Have's**:

- [ ] Add ability to create and save recordings

### Useful Links:

- [Myo Connect Download - Windows](https://drive.google.com/file/d/1Z41JeAPAQwa57O41nb4_a2Ps3fxQU3MA/view)
- [Myo Connect Download - Mac](https://drive.google.com/open?id=1mbA6V0xOycXhOzLOsqA97mJdOj_aZ5zk)
- [GitHub Markdown Tutorial](https://guides.github.com/features/mastering-markdown/)
- [Myo Support Page](https://support.getmyo.com/hc/en-us/categories/200376195-Myo-101)
- [C++ Resources](https://www.tutorialspoint.com/cplusplus/)
- [Visual Studio](https://visualstudio.microsoft.com/vs/features/cplusplus/)
- [Similar Project](https://developerblog.myo.com/exploring-live-sound-spatialisation-using-gestural-control/)
