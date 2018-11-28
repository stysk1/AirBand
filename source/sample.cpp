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
#include "bass.h"

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : onArm(false), isUnlocked(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
    {
    }
	int prevRoll = 0, prevPitch = 0, prevYaw = 0;
	int device = -1; // Default Sound device
	int freq = 44100; // Sample rate (Hz)
    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        roll_w = 0;
        pitch_w = 0;
        yaw_w = 0;
        onArm = false;
        isUnlocked = false;
    }

    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
    // as a unit quaternion.
	void onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro)
	{
		gyroX = gyro.x();
		gyroY = gyro.y();
		gyroZ = gyro.z();
	}
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

        // Convert the floating point angles in radians to a scale from 0 to 18.
        roll_w = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
        pitch_w = static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 18);
        yaw_w = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);
    }

    // onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
    // making a fist, or not making a fist anymore.
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        currentPose = pose;

        if (pose != myo::Pose::unknown && pose != myo::Pose::rest) {
            // Tell the Myo to stay unlocked until told otherwise. We do that here so you can hold the poses without the
            // Myo becoming locked.
            //myo->unlock(myo::Myo::unlockHold);

            // Notify the Myo that the pose has resulted in an action, in this case changing
            // the text on the screen. The Myo will vibrate.
            //myo->notifyUserAction();
        } else {
            // Tell the Myo to stay unlocked only for a short period. This allows the Myo to stay unlocked while poses
            // are being performed, but lock after inactivity.
            //myo->unlock(myo::Myo::unlockTimed);
        }
    }

    // onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection, float rotation,
                   myo::WarmupState warmupState)
    {
        onArm = true;
        whichArm = arm;
    }

    // onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmUnsync(myo::Myo* myo, uint64_t timestamp)
    {
        onArm = false;
    }

    // onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
    void onUnlock(myo::Myo* myo, uint64_t timestamp)
    {
        isUnlocked = true;
    }

    // onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
    void onLock(myo::Myo* myo, uint64_t timestamp)
    {
        isUnlocked = false;
    }

	std::string pickSound(std::string volume) {
		std::string tone = std::to_string(rand() % 4 + 1);
		std::string location = "sounds/";
		std::string extension = ".wav";
		std::string concat = location + volume + " " + tone + extension;
		return concat;
	}

	void playLeft(HSTREAM stream) {
		BASS_ChannelPlay(stream, FALSE);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 10));
	}
	void playRight(HSTREAM stream) {
		BASS_ChannelPlay(stream, FALSE);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 10));
	}

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
		// Clear the current line
		//std::cout << '\r';

		std::string temp = "";
		const char* sound;
	
		HSTREAM streamHandle; // Handle for open stream
		BASS_Init(device, freq, 0, 0, NULL); //Initialize output device
		
// 		if (accelX >= 1.0 && accelX < 1.3) {
// 			temp = pickSound("Snare Soft"); sound = temp.c_str();
// 			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
// 		}
//  		if (gyroZ >= 80 && pitch_w <= 8 && prevPitch > pitch_w + 1) {
//  			temp = pickSound("Snare Med"); sound = temp.c_str();
//  			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
//  		}
// 		if (accelX >= 1.8 && accelX < 2.3) {
// 			temp = pickSound("Snare Hard"); sound = temp.c_str();
// 			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
// 		}
// 		if (accelX >= 2.3) {
// 			temp = pickSound("Snare Hardest"); sound = temp.c_str();
// 			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
// 		}
//  		if (yaw_w < 9 && gyroZ >= 80 && pitch_w > 9 && prevPitch > pitch_w + 1) {
//  			temp = pickSound("Hi Hat/Closed Hi hat"); sound = temp.c_str();
//  			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
//  		}
// 		if (GetAsyncKeyState(VK_SPACE) & 0x80000000) {
// 			temp = pickSound("Kick"); sound = temp.c_str();
// 			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0); //Load randomized sound file
// 			BASS_ChannelPlay(streamHandle, FALSE);
// 		}
		
		if (gyroZ >= 80 && pitch_w < 6) { 
			temp = pickSound("Snare Med").c_str(); sound = temp.c_str(); 
			streamHandle = BASS_StreamCreateFile(FALSE, sound, 0, 0, 0);  
			BASS_ChannelPlay(streamHandle, FALSE); 
			std::cout << "JLSFHKSJDFHKJSDFHKJSDHFKJSDF" << '\n';
			Sleep(1000 / 10);
		}

		std::cout << '[' << gyroZ << ']'
			<< '[' << prevPitch << ']'
			<< '[' << pitch_w << ']' << '\n';
// 		if (whichArm == 0) {
// 			std::thread(&DataCollector::playRight, this, streamHandle).detach();
// 		}
		
// 		std::cout << '[' << accelX << ']'
// 			<< '[' << accelY << ']'
// 			<< '[' << accelZ << ']'
// 			<< '[' << accelMagnitude << ']';
			//std::cout << "Array Size: " << gyroValues.size() << ' ' << "Values: " << ' ';
			/*for (int i = 0; i < gyroValues.size(); ++i)
				std::cout << gyroValues[i] << ' ';*/
		//if (/*(prevPitch - pitch_w >= 2) && *//*(getStandardDev() >= 0.85) && */(accelX >= 1)) {


		/*  More accurate hit movement but still sometimes double hits.
		 *  TODO: Check out gyroscope/accelerometer/EMG values when using wrist movements
		 */
// 		if (gyroZ >= 80  && pitch_w <= 4 && prevPitch > pitch_w && whichArm == 1) {
// 			amtOfHits += 1;
// 			//std::cout << gyroValues.size() << '\n';
// 			BASS_ChannelPlay(streamHandle, FALSE);
// 			Sleep(1000 / 10);
// 		}
// 
// 		if(gyroZ >= 80  && pitch_w <= 4  && prevPitch > pitch_w && whichArm == 0){ 
// 			amtOfHits += 1;
// 			//std::cout << gyroValues.size() << '\n';
// 			BASS_ChannelPlay(streamHandle, FALSE);
// 			Sleep(1000 / 10);
// 		}
// 		else if (gyroZ >= 80 && yaw_w > 3 && prevPitch > pitch_w) {
// 			BASS_ChannelPlay(streamHandle, FALSE);
// 			Sleep(1000 / 10);
// 		}
		//std::cout << yaw_w << '\n';
		/*if (gyroValues.size() >= 60) { //Reset the array of gyroscope values once there are 60 values. TODO
			gyroValues = {};
		}*/
		prevPitch = pitch_w;
		std::cout << std::flush;
    }

    // These values are set by onArmSync() and onArmUnsync() above.
    bool onArm;
    myo::Arm whichArm;

    // This is set by onUnlocked() and onLocked() above.
    bool isUnlocked;

    // These values are set by onOrientationData() and onPose() above.
    int roll_w, pitch_w, yaw_w;
	float gyroX, gyroY, gyroZ;
    myo::Pose currentPose;
};

int main(int argc, char** argv)
{
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {

    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    myo::Hub hub("com.example.hello-myo");

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
		hub.run(1000 / 20);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
		collector.print();
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
