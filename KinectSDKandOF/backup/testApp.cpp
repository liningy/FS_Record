#include "testApp.h"
#include <algorithm>

//--------------------------------------------------------------
void testApp::setup(){	

	bgColor.r=0;
	bgColor.g=0;
	bgColor.b=0;
	ofBackground(bgColor.r,bgColor.g,bgColor.b);	            // Set the background color (right now, white)

	// initialize kinect and application
	g_kinectGrabber.Kinect_Zero();
	g_kinectGrabber.Kinect_Init();

	// allocate memory for textures
	texColorAlpha.allocate(VIDEO_WIDTH,VIDEO_HEIGHT,GL_RGBA);
	texGray.allocate(DEPTH_WIDTH, DEPTH_HEIGHT,GL_RGBA); // gray depth texture; the lighter the gray color is, the closer the skeleton is to the Kinect
	//ka - also, I added the code relating to these variables
	bRecord = false;
	bPlayback = false;
	//ka
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(bgColor.r,bgColor.g,bgColor.b);
	if (!bPlayback){
		g_kinectGrabber.Kinect_Update();
	}
	else{
		
	}
	size_t successfullyread = 0;
    if (bPlayback){
		rgbdepthpair = kinectPlayer.updatea();
		colorAlphaPixels= rgbdepthpair.first;
		grayPixels =      rgbdepthpair.second.first;
		headPositionX =   rgbdepthpair.second.second.first;
		headPositionY =   rgbdepthpair.second.second.second.first;
		headPositionZ =   rgbdepthpair.second.second.second.second.first;
		leftShoulderX =   rgbdepthpair.second.second.second.second.second.first;
		leftShoulderY =   rgbdepthpair.second.second.second.second.second.second.first;
		rightShoulderX =  rgbdepthpair.second.second.second.second.second.second.second.first;
		rightShoulderY =  rgbdepthpair.second.second.second.second.second.second.second.second.first;
		leftHandPX =      rgbdepthpair.second.second.second.second.second.second.second.second.second.first;
		leftHandPY =      rgbdepthpair.second.second.second.second.second.second.second.second.second.second.first;
		rightHandPX =     rgbdepthpair.second.second.second.second.second.second.second.second.second.second.second.first;
		rightHandPY =     rgbdepthpair.second.second.second.second.second.second.second.second.second.second.second.second;
	}
	if (!bPlayback){
	// load the RGBA values into a texture
		colorAlphaPixels = g_kinectGrabber.Kinect_getAlphaPixels();
	}
	if(colorAlphaPixels != NULL) {
		texColorAlpha.loadData(colorAlphaPixels, VIDEO_WIDTH,VIDEO_HEIGHT, GL_RGBA);
	//ka I'm unsure if old stored RGBA values can work with (VIDEO_WIDTH, VIDEO_HEIGHT), since it may change in between recordings
		//I just store the texture, whereas ofx has options on whether to use teh loaded rgb and depth values (convert to tex) or not)
	}
	//}

	if(!bPlayback){
		// load grayimage (which includes depth information) into a texture
		grayPixels = (BYTE*)g_kinectGrabber.Kinect_getDepthPixels();	
	}
	if (grayPixels != NULL) {
		texGray.loadData(grayPixels,DEPTH_WIDTH,DEPTH_HEIGHT, GL_RGBA);
	}

	// depthBuff stores unsigned short (2 bytes) pointers pointing to each pixels's depth in milimeter; in total it has 1 * width * height
	USHORT* depthBuff = g_kinectGrabber.Kinect_getDepthBuffer();

	
    // Gets the skeleton joints' positions
	if (!bPlayback){
		g_kinectGrabber.getJointsPoints();
		headPositionX=g_kinectGrabber.headJoints_x;
		headPositionY=g_kinectGrabber.headJoints_y;
		headPositionZ=g_kinectGrabber.headJoints_z;
		//ka added
		leftShoulderX = g_kinectGrabber.shoulderLeft_x;
		leftShoulderY = g_kinectGrabber.shoulderLeft_y;
		rightShoulderX = g_kinectGrabber.shoulderRight_x;
		rightShoulderY = g_kinectGrabber.shoulderRight_y;
		leftHandPX = g_kinectGrabber.handLeft_x;
		leftHandPY = g_kinectGrabber.handLeft_y;
		rightHandPX = g_kinectGrabber.handRight_x;
		rightHandPY = g_kinectGrabber.handRight_y;
		//ka added
	}
	if(bRecord){// && kinectRecorder.isOpened()) {
		kinectRecorder.newFrame(colorAlphaPixels, grayPixels,headPositionX, headPositionY, headPositionZ, leftShoulderX, leftShoulderY, rightShoulderX, rightShoulderY,leftHandPX, leftHandPY, rightHandPX, rightHandPY);
	}

}

//--------------------------------------------------------------
void testApp::draw(){
	ofEnableSmoothing();
	ofEnableAlphaBlending();
	
	////////////////////////////////////////////////////////// RGB video and Skeleton Tracking Visualization
	
	//video image
	texColorAlpha.draw(640+20,0+25,VIDEO_WIDTH, VIDEO_HEIGHT);
	texGray.draw(20,0+25,VIDEO_WIDTH, VIDEO_HEIGHT);

	//circle drawn on head of tracked individual
	//ka modified to use variables instead of function calls
	ofCircle(headPositionX+640,headPositionY,10);
	ofCircle(leftShoulderX+640, leftShoulderY,10);
	ofCircle(rightShoulderX+640,rightShoulderY,10); 
	ofCircle(leftHandPX+640,leftHandPY,10); 
	ofCircle(rightHandPX+640,rightHandPY,10);
	ofLine(headPositionX+640,headPositionY,leftShoulderX+640, leftShoulderY);
	ofLine(leftShoulderX+640, leftShoulderY,leftHandPX+640,leftHandPY);
	ofLine(headPositionX+640,headPositionY,rightShoulderX+640,rightShoulderY);
	ofLine(rightShoulderX+640,rightShoulderY,rightHandPX+640,rightHandPY);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
//-------------------------------------------------------------
void testApp::exit(){
	//printf("cleaning up\n");
	g_kinectGrabber.Kinect_UnInit();
	kinectPlayer.close();
	kinectRecorder.close();
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(key == 'r') {
		if (!bRecord){
			startRecording();
		}
		else{
			stopRecording();
		}
	}
	else if (key == 'p'){
		if (!bPlayback){
			startPlayback();
		}
		else{
			stopPlayback();
		}
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//------------- -------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}


//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}
//ka record stuff start
void testApp::startRecording() {

	// stop playback if recording
	stopPlayback();
	kinectRecorder.init(ofToDataPath("recording.dat"));
	bRecord = true;
}

//--------------------------------------------------------------
void testApp::stopRecording() {

	kinectRecorder.close();
	bRecord = false;
}
//ka record stuff end
//ka playback stuff start
void testApp::startPlayback() {

	
	stopRecording();

	// set record file and source
	kinectPlayer.setup(ofToDataPath("recording.dat"), true);
	kinectPlayer.loop();
	bPlayback = true;
}

//--------------------------------------------------------------
void testApp::stopPlayback() {
	kinectPlayer.close();
	bPlayback = false;
}
//playback stuff end