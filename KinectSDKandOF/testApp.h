//#pragma once

#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "grabVideo.h"
#include <ctime>
#include "ofxKinectRecorder.h"
#include "ofxKinectPlayer.h"
#include "time.h"
#include "button.h"
#include "slider.h"
#include <sstream> //suggestion from from not so frequently asked questions faq && stackoverflow
#include "ofSoundStream.h"

class testApp : public ofBaseApp{
	
	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		//======================================================================
		//BACKEND REPLAY/RECORD SECTION
		/// make sure you don't record and playback simultaneously 
		/// using the same file!!!
		void startRecording(); //deals with starting recording
		void stopRecording(); //deals with stopping recording, also affects gui (through safe reset live)
		void startPlayback(); //deals with starting playback
		void stopPlayback(); //deals with stopping playback, also affects gui (through safe reset live)

		//playback helper functions
		void safeResetReplayMode(); //resets variables (gui and backend) that may have changed (resets playback mode)
		void safeResetLiveMode(); //resets variables (gui and backend) that may have changed (resets live video mode)
		void interruptFastPlayback(); //resets variables changed by playing back without delays between frames
		void getPlaybackRange(); //gets the playback range for the purposes of displaying a slider
		void skipTo(float percentOfSlider);//takes in a percent value of the slider progress, converts it to a percent value of the number of frames, then forwards until it gets there and pauses.
		void pause() {paused = true;} //allows live and recorded video to be paused
		void play()  {paused = false;} //sets paused = false (undos the pause effect stated above)
		/////Gesture Checks
		bool rightHandRaised(); //checks whether the right hand of the skeleton we are tracking is currently raised (returns true if it is)

		//sound recording
		ofSoundPlayer  soundfile;
		int startRecordTime;
		int stopRecordTime;
		int videoFrameDelay;
		int numRecordedFrames;
		float currSoundPos;

		//======================================================================
		//BUTTON CONTROL SECTION
		//these three do the actions of the buttons safely (updating what needs to be updated, etc) 
		void standardStop(); //stops playback/recording and changes variables that inform buttons whether they should be active or not
		void standardReplay(); //replays saved files and changes variables that inform buttons whether they should be active or not
		void standardRecord(); //starts recording and changes variables that inform buttons whether they should be active or not
		void updateButtonTriggers(); //informs buttons whether they should be active or not
		//
	private:
		KinectGrabber   g_kinectGrabber;
	
		ofTexture		texColorAlpha; //rgb pixels
		ofTexture		texGray; //depth pixels
		
		unsigned char 	* colorAlphaPixels;
		unsigned char	* grayPixels;

		int headPositionX, headPositionY, headPositionZ; //skeleton joints
		int neckPositionX, neckPositionY, leftShoulderX, leftShoulderY, rightShoulderX, rightShoulderY,leftHandPX, leftHandPY, rightHandPX, rightHandPY; //skeleton joints
		
		ofColor         bgColor;

		bool bRecord;  //true if you are recording
		bool bPlayback;//true if you are in playback mode (even if paused)
		ofxKinectRecorder 	kinectRecorder; //accepts variables and stores them externally
		ofxKinectPlayer 	kinectPlayer; //reads variables off of a file and returns the variables
		pair<unsigned char *,pair<unsigned char *,pair<time_t,pair<int,pair<int,pair<int,pair<int,
			pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,bool>>>>>>>>>>>>>> rgbdepthpair;//variable that accepts the variables that kinectPlayer sends

		pair<int, pair<int, pair<int, pair<int, pair<int, pair<int, pair<int, pair<int, pair<int,
			pair<int, int>>>>>>>>>> frameInfo;//variable that accepts the variables that kinectPlayerInfo sends (nFrames and gesture audio indices)

		/////Gesture
		bool rightHandUp; // is true if the skeleton's right hand is raised for at least 20 frames
		int rightHandUpCount; //counts the number of frames that right hand has been up. resets every 100 frames
		/////Time
		time_t rawTime; //current time
		struct tm * timeinfo; //intermediate step in printing rawTime as a calendar date
		/////Playback
		bool updateDelay; //a variable passed into kinectPlayer.update - it states whether there should be a delay in between outputting frames 
							//(false for regular playback, true for searching)
		bool paused; //stores whether playback is currently paused or not
		//======================================================================
		//GUI PRIVATE SECTION
		/////Buttons
		button* stopButton;
		button* pauseButton;
		button* replayButton;
		button* recordButton;
		button* goodIdeaButton;
		/////Button picture dimensions
		int stopButtonWidth; //in pixels
		int stopButtonHeight;
		int replayButtonWidth;
		int replayButtonHeight;
		int recordButtonWidth;
		int recordButtonHeight;
		int goodIdeaButtonWidth;
		int goodIdeaButtonHeight;
		///// Whether Buttons are Clickable or not
		bool stopButtonActive;
		bool replayButtonActive;
		bool recordButtonActive;
		bool pauseButtonOn;//checks whetehr replay button should change or not (true = it should be pause)
		bool goodIdeaButtonActive;
		bool smallButtonActive;
		///// Small Button variables
		ofImage smallGoodImg;
		int smallRHRLocations[20];//the frame numbers of up to 20 locations on the slider at which small right hand raised buttons should be (stores the last 20)
		int rhrIndex; //the index of the index after the last currently  filled slot in the small RHRLocations array
		int maxNoIndices; //the max of each kind of gesture of audio allowable
		std::stringstream tempSS;// intermediate variable to convert an int to a string (since each button should have a different name)
		std::string tempS;// variable to hold small button name
		int tempButtonX;
		/////Sliders
		slider* timer;
		/////Slider parameters
		int timerStartX;//replay slider start x
		int timerStartY;//replay slider start y
		int timerWidth; //replay timer width
		int nFrames; //number of frames in the recorded video you are playing
		int currFrame; //the frame number of the frame you are currently replaying (always <= nFrames)
		float timerProgression; //the fraction (currFrame/nFrames)
		bool nFramesValid; //is whether nFrames is valid or not
		/////Slider helper variables
		int lastPlaybackTime; //timestamp of the last frame in the recording
		time_t firstPlaybackTime; //timestamp of the first frame in the recording
		bool playbackRangeSearchOn; //true if you are still doing playback startup things (like fining nFrames and the locations of gestures)
		time_t prevTimeMillis; //timestap of the previous frame that you saw
		bool skipToMode; //whether we're currently skipping to a point in the video or not
		float pOSlider; //global variable to store what percent of the slider we want to be at
		int rhrFrame1, rhrFrame2, rhrFrame3, rhrFrame4, rhrFrame5, rhrFrame6, rhrFrame7, rhrFrame8, rhrFrame9, rhrFrame10;
};

#endif
	
