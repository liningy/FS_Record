//#pragma once

#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "grabVideo.h"
#include <ctime>
/////
#include "ofxKinectRecorder.h"
#include "ofxKinectPlayer.h"
//////

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

		//ka most is from ofx
		/// start/stop recording and playback,
		/// make sure you don't record and playback simultaneously 
		/// using the same file!!!
		void startRecording();
		void stopRecording();
		void startPlayback();
		void stopPlayback();
		//ka end
	private:
		KinectGrabber   g_kinectGrabber;
	
		ofTexture		texColorAlpha;
		ofTexture		texGray;
		
		unsigned char 	* colorAlphaPixels;
		unsigned char	* grayPixels;

		int headPositionX, headPositionY, headPositionZ;
		int neckPositionX, neckPositionY, leftShoulderX, leftShoulderY, rightShoulderX, rightShoulderY,leftHandPX, leftHandPY, rightHandPX, rightHandPY;
		
		ofColor         bgColor;

		bool bRecord;//ka
		bool bPlayback;//ka
		ofxKinectRecorder 	kinectRecorder;
		ofxKinectPlayer 	kinectPlayer;
		pair<unsigned char *,pair<unsigned char *,pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,pair<int,int>>>>>>>>>>>> rgbdepthpair;
};

#endif
	
