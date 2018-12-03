// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <conio.h>
#include <windows.h>
#include <cwchar>
#include "bass.h"

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp
#include <myo/myo.hpp>

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : roll_w(0), pitch_w(0), yaw_w(0)
    {
    }
	int prevRoll = 0, prevPitch = 0, prevYaw = 0;
	bool readyLeft = true, readyRight = true;
	float centerRight, centerLeft;
	std::string temp = "";
	const char* sound = "";
	bool modify = true;
	int device = -1; // Default Sound device
	int freq = 44100; // Sample rate (Hz)
	std::vector<myo::Myo*> knownMyos; //vector of Myos for hand identification

	//onPair() override will output which hand the Myo is in
	void onPair(myo::Myo* myo, uint64_t timestamp, myo::FirmwareVersion firmwareVersion) {
		knownMyos.push_back(myo);
		if (knownMyos.size() == 1) std::cout << "Right hand connected." << std::endl;
		else if (knownMyos.size() == 2) std::cout << "Left hand connected." << std::endl;
	}

	//identifyMyo() returns which hand the Myo is in: Right=1, Left=2
	size_t identifyMyo(myo::Myo* myo) {
		for (size_t i = 0; i < knownMyos.size(); ++i) {
			if (knownMyos[i] == myo) return i + 1;
		}
		return 0;
	}

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        roll_w = 0;
        pitch_w = 0;
        yaw_w = 0;

    }

	//pickSound() chooses a random variation of the 4 hits on the selected sound
	std::string pickSound(std::string volume) {
		std::string tone = std::to_string(rand() % 4 + 1);
		std::string location = "../sounds/";
		std::string extension = ".wav";
		std::string concat = location + volume + " " + tone + extension;
		return concat;
	}

	//playRight() plays the sound that was hit on the drum set and waits a realistic fraction of time to prevent flams from a single right hand hit
	void playRight(HSTREAM stream) {
		readyRight = false;
		BASS_ChannelPlay(stream, FALSE);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 9));
		readyRight = true;
	}

	//playLeft() plays the sound that was hit on the drum set and waits a realistic fraction of time to prevent flams from a single left hand hit
	void playLeft(HSTREAM stream) {
		readyLeft = false;
		BASS_ChannelPlay(stream, FALSE);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 9));
		readyLeft = true;
	}

	// onGyroscopeData() is called whenever the Myo device provides its current change in angular velocity, which is represented
	// as degrees per second
	void onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro)
	{
		gyroX = gyro.x();
		gyroY = gyro.y();
		gyroZ = gyro.z();

		HSTREAM streamHandle; // Handle for open stream
		BASS_Init(device, freq, 0, 0, NULL); //Initialize output device

		if ((GetAsyncKeyState(VK_SPACE) & 0x80000000)) { //The space bar is used to center the devices at their current location
			modify = true; //Set modify boolean to true to allow the center values to be changed
		}


		/* RIGHT HAND ALGORITHMS
		 * Snare Drum - Wrist is straight and can be angled slightly to the left or the right
		 * Hi Hat - On the left side of the snare
		 * Crash Cymbal- On the right side of the snare
		 * Ride Cymbal - Farther right past the crash cymbal
		 * Hard Snare Hit - Turn wrist so knuckles are facing up. Hits in any position
		 */
		if (identifyMyo(myo) == 1 && roll_w >= 25 && ((abs(centerRight - yaw_w) <= 3) || (yaw_w - centerRight > 47 && yaw_w - centerRight <= 50))) {
			temp = pickSound("Snare/Snare Med");
			sound = temp.c_str(); 
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 1 && roll_w >= 25 && ((yaw_w - centerRight >= 5 && yaw_w - centerRight < 15) || (centerRight - yaw_w > 36 && centerRight - yaw_w <= 46))) {
			temp = pickSound("Hi Hat/Closed Hi hat"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 1 && roll_w >= 25 && ((centerRight-yaw_w >= 5 && centerRight-yaw_w < 10) || (yaw_w - centerRight > 41 && yaw_w - centerRight <= 46))) {
			temp = pickSound("Crash/Crash alt"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 1 && roll_w >= 25 && ((centerRight - yaw_w >= 10 && centerRight - yaw_w <= 20) || (yaw_w - centerRight >= 31 && yaw_w - centerRight <= 41))) {
			temp = pickSound("Ride/Ride"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 1 && gyroZ >= 500 && readyRight) {
			temp = pickSound("Snare/Snare Hardest"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
			std::thread(&DataCollector::playRight, this, streamHandle).detach();
			std::cout << "[RIGHT: " << temp << "]" << std::endl;
		}


		/* LEFT HAND ALGORITHMS
		 * Snare Drum - Wrist is straight and can be angled slightly to the left or the right
		 * Hi Hat - On the left side of the snare
		 * Crash Cymbal- On the right side of the snare
		 * Ride Cymbal - Farther right past the crash cymbal
		 * Bass Drum - Turn wrist so knuckles are facing up. Hits in any position
		 *
		 * Snare uses the difference of the absolute value to account for both left and right positions around the center
		 * The yaw values are represented as a circle so everything after the ORs account for reaching around the other side of the circle
		 * EX: If center is 1, the values to the left such as 50 and downwards need to be accounted for
		 */
		if (identifyMyo(myo) == 2 && roll_w >= 25 && ((yaw_w - centerLeft >= 5 && yaw_w - centerLeft < 10) || (centerLeft - yaw_w > 41 && centerLeft - yaw_w <= 46))) {
			temp = pickSound("Hi Hat/Closed Hi hat"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 2 && roll_w >= 25 && ((abs(centerLeft - yaw_w) <= 3) || (yaw_w - centerLeft > 47 && yaw_w - centerLeft <= 50))) {
			temp = pickSound("Snare/Snare Med"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 2 && roll_w >= 25 && ((centerLeft - yaw_w >= 5 && centerLeft - yaw_w < 10) || (yaw_w - centerLeft > 41 && yaw_w - centerLeft <= 46))) {
			temp = pickSound("Crash/Crash alt"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 2 && roll_w >= 25 && ((centerLeft - yaw_w >= 10 && centerLeft - yaw_w <= 15) || (yaw_w - centerLeft > 36 && yaw_w - centerLeft <= 41))) {
			temp = pickSound("Ride/Ride"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
		}
		if (identifyMyo(myo) == 2 && gyroZ <= -500 && readyLeft) {
			temp = pickSound("Kick/Kick Hard"); sound = temp.c_str();
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
			std::thread(&DataCollector::playLeft, this, streamHandle).detach();
			std::cout << "[LEFT: " << temp << "]" << std::endl;
		}


		/* HIT DETECTION ALGORITHM
		 * If the velocity is fast enough (negative when doing a downward hitting motion) and the busy wait is finished (to prevent flams), call the thread to play a sound
		 * Each arm has its own thread that it runs to play concurrent sounds
		 */
		if (identifyMyo(myo) == 1 && readyRight && gyroX <= -500) {
			std::thread(&DataCollector::playRight, this, streamHandle).detach();
			std::cout << "[RIGHT: " << temp << "]" << std::endl;
		}
		if (identifyMyo(myo) == 2 && readyLeft && gyroX <= -500) {
			std::thread(&DataCollector::playLeft, this, streamHandle).detach();
			std::cout << "[LEFT: " << temp << "]" << std::endl;
		}
	}

	// onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
	// as a unit quaternion
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        using std::atan2;
        using std::asin;
        using std::sqrt;
        using std::max;
        using std::min;
		
        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                           1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y())); //twisting arm
        float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x())))); //up and down
        float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                        1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z())); //left and right
		
        // Convert the floating point angles in radians to a scale from 1 to 50.
        roll_w = static_cast<int>(((roll + (float)M_PI)/(M_PI * 2.0f) * 50) + 1);
        pitch_w = static_cast<int>(((pitch + (float)M_PI/2.0f)/M_PI * 50) + 1);
		//Yaw is used to judge the left and right motion of the wrist when hitting the drum
		//Utilized for areas around the drum
        yaw_w = static_cast<int>(((yaw + (float)M_PI)/(M_PI * 2.0f) * 50) + 1);

		if (modify && identifyMyo(myo) == 1) { centerRight = yaw_w; modify = false; } //Center right Myo
		else if (modify && identifyMyo(myo) == 2) { centerLeft = yaw_w; modify = false; } //Center left Myo

    }

    // These values are set by onOrientationData() and onGyroscope() above.
    int roll_w, pitch_w, yaw_w;
	float gyroX, gyroY, gyroZ;
};

int main(int argc, char** argv)
{
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {

    //Windows console editor to make the output easier to see
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;                   // Width of each character in the font
	cfi.dwFontSize.Y = 30;                  // Height
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	std::wcscpy(cfi.FaceName, L"Consolas"); // Choose your font
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	// First, we create a Hub with our application identifier. The Hub provides access to one or more Myos.
    myo::Hub hub("com.tcnj.airband");

    std::cout << "Attempting to find a Myo..." << std::endl;

    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
    // if that fails, the function will return a null pointer.
    myo::Myo* myo = hub.waitForMyo(10000);

    // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!myo) {
        throw std::runtime_error("Unable to find a Myo!");
    }

    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

    // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
    DataCollector collector;

    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub.addListener(&collector);

    // Finally we enter our main loop.
    while (1) {
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 60 times a second, so we run for 1000/60 milliseconds.
		hub.run(1000 / 100);
    }

	/* As very last, close Bass */
	BASS_Free();

    // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}