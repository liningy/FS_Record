#include "testApp.h"
#include <algorithm>

//--------------------------------------------------------------
//comments are in test app.h
void testApp::setup(){	
  //TODO: store joints in list of ints and gestures in list of gestures so you don't have to modify playback and record everytime you get a new joint
  //or gesture
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
	rightHandUp = false;
	rightHandUpCount = 0;
	updateDelay = true;
	paused = false;
	///gui
	stopButtonWidth = 109; //in pixels
	stopButtonHeight = 82;
	replayButtonWidth = 104;
	replayButtonHeight = 81;
	recordButtonWidth = 104;
	recordButtonHeight = 83;
	//pauseButtonWidth = 98;
	//pauseButtonHeight = 76;
	goodIdeaButtonWidth = 93;
	goodIdeaButtonHeight = 63;
	stopButtonActive = false;
	pauseButtonOn = false;
	replayButtonActive = true;
	recordButtonActive = true;
	goodIdeaButtonActive = false;
	smallButtonActive = false;
	int hButtonOffset = 30;
	stopButton = new button("Stop", VIDEO_WIDTH - stopButtonWidth - hButtonOffset , VIDEO_HEIGHT + 50 , stopButtonWidth, stopButtonHeight, stopButtonActive, "images\\r_stop.png", "images\\r_stop.png");
	replayButton = new button("Replay", VIDEO_WIDTH - hButtonOffset, VIDEO_HEIGHT + 50, replayButtonWidth, replayButtonHeight, replayButtonActive, "images\\r_pause.png", "images\\r_replay.png",   VIDEO_WIDTH - 4 - hButtonOffset, VIDEO_HEIGHT + 55);
	recordButton = new button("Record", VIDEO_WIDTH + replayButtonWidth - hButtonOffset, VIDEO_HEIGHT + 50 , recordButtonWidth, recordButtonHeight, recordButtonActive, "images\\r_record.png", "images\\r_record.png");
	goodIdeaButton = new button("Good Idea", 0.25*VIDEO_WIDTH , 1.25*VIDEO_HEIGHT + 50 , goodIdeaButtonWidth, goodIdeaButtonHeight, goodIdeaButtonActive, "images\\r_good.png", "images\\r_good.png");
	rhrIndex = 0;

	timerWidth = VIDEO_WIDTH;
	timerStartX = VIDEO_WIDTH/2;
	timerStartY = 1.5*VIDEO_HEIGHT;
	timer = new slider(timerStartX,timerStartY,timerWidth);
	nFrames = 0;

	lastPlaybackTime = 0;
	firstPlaybackTime = 0;
	rawTime = 0;
	prevTimeMillis = 0;
	timerProgression = 0;
	(*timer).setSliderPosX(timerProgression*timerWidth);
	playbackRangeSearchOn = false;
	currFrame = 0;
	nFrames = 0;
	nFramesValid = false;
	skipToMode = false;
	pOSlider = 0.0;
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(bgColor.r,bgColor.g,bgColor.b);
	if (!bPlayback){
		g_kinectGrabber.Kinect_Update();
	}
	else{
		
	}
    if (bPlayback){//if bPlayback
		if(!paused){//while paused we can't search for playback range
			rgbdepthpair = kinectPlayer.updatea(updateDelay);
			rawTime =      rgbdepthpair.second.second.first;
			rightHandUp =  rgbdepthpair.second.second.second.second.second.second.second.second.second.second.second.second.second.second;
			if (!playbackRangeSearchOn && !skipToMode){//we can show pictures while skipping /searching for playback range if we don't have these
				colorAlphaPixels = rgbdepthpair.first;
				grayPixels =       rgbdepthpair.second.first;
				headPositionX =    rgbdepthpair.second.second.second.first;
				headPositionY =    rgbdepthpair.second.second.second.second.first;
				headPositionZ =    rgbdepthpair.second.second.second.second.second.first;
				leftShoulderX =    rgbdepthpair.second.second.second.second.second.second.first;
				leftShoulderY =    rgbdepthpair.second.second.second.second.second.second.second.first;
				rightShoulderX =   rgbdepthpair.second.second.second.second.second.second.second.second.first;
				rightShoulderY =   rgbdepthpair.second.second.second.second.second.second.second.second.second.first;
				leftHandPX =       rgbdepthpair.second.second.second.second.second.second.second.second.second.second.first;
				leftHandPY =       rgbdepthpair.second.second.second.second.second.second.second.second.second.second.second.first;
				rightHandPX =      rgbdepthpair.second.second.second.second.second.second.second.second.second.second.second.second.first;
				rightHandPY =	   rgbdepthpair.second.second.second.second.second.second.second.second.second.second.second.second.second.first;
			}
			else if (skipToMode){
				skipTo(pOSlider);
			}
			else{//I put this last because I assumed people would skip more than start videos in sessions
				getPlaybackRange();
			}
			if (nFramesValid){//if you don't want slider updated during skipToMode, modify this (add && !skipToMode)
				currFrame = (currFrame+1) % nFrames;
				if(!skipToMode){
					timerProgression = ((float)currFrame/(float)nFrames);
				}
			}
			else{
				timerProgression = 0;
			}
			(*timer).setSliderPosX(timerProgression*timerWidth);
		}
     }
	else{//if !bPlayback
		time(&rawTime );
		colorAlphaPixels = g_kinectGrabber.Kinect_getAlphaPixels();
		grayPixels = (BYTE*)g_kinectGrabber.Kinect_getDepthPixels();	
		g_kinectGrabber.getJointsPoints(); // Gets the skeleton joints' positions (up to rightHandPY)
		headPositionX=g_kinectGrabber.headJoints_x;
		headPositionY=g_kinectGrabber.headJoints_y;
		headPositionZ=g_kinectGrabber.headJoints_z;
		leftShoulderX = g_kinectGrabber.shoulderLeft_x;
		leftShoulderY = g_kinectGrabber.shoulderLeft_y;
		rightShoulderX = g_kinectGrabber.shoulderRight_x;
		rightShoulderY = g_kinectGrabber.shoulderRight_y;
		leftHandPX = g_kinectGrabber.handLeft_x;
		leftHandPY = g_kinectGrabber.handLeft_y;
		rightHandPX = g_kinectGrabber.handRight_x;
		rightHandPY = g_kinectGrabber.handRight_y;
		rightHandUp = rightHandRaised();
	}
	//regardless of bPlayback's value, do below
	timeinfo = localtime (&rawTime );

	if(colorAlphaPixels != NULL) {
		texColorAlpha.loadData(colorAlphaPixels, VIDEO_WIDTH,VIDEO_HEIGHT, GL_RGBA); // load the RGBA values into a texture
		//ka I'm unsure if old stored RGBA values can work with (VIDEO_WIDTH, VIDEO_HEIGHT), since it may change in between recordings. If it matters, store video width and height as well
	}

	if (grayPixels != NULL) {
		texGray.loadData(grayPixels,DEPTH_WIDTH,DEPTH_HEIGHT, GL_RGBA); //load grayimage (which includes depth information) into a texture
	}

	// depthBuff stores unsigned short (2 bytes) pointers pointing to each pixels's depth in milimeter; in total it has 1 * width * height
	USHORT* depthBuff = g_kinectGrabber.Kinect_getDepthBuffer();

	if(bRecord){// && kinectRecorder.isOpened()) {
	  kinectRecorder.newFrame(colorAlphaPixels, grayPixels, rawTime, headPositionX, headPositionY, headPositionZ, leftShoulderX, leftShoulderY, rightShoulderX, rightShoulderY,leftHandPX, leftHandPY, rightHandPX, rightHandPY, rightHandUp);
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
		ofDrawBitmapString (asctime (timeinfo), 1.5*VIDEO_WIDTH + 50, 1.5*VIDEO_HEIGHT + 50);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		(*stopButton).drawFont(stopButtonActive);
		(*replayButton).drawFont(replayButtonActive, pauseButtonOn);
		(*recordButton).drawFont(recordButtonActive);
		(*goodIdeaButton).drawFont(goodIdeaButtonActive);
		(*timer).drawSlider(timerStartX, timerStartX + timerWidth);//unsure what the variables do!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if ((playbackRangeSearchOn) || (skipToMode)){
			ofDrawBitmapString ("Loading...", timerStartX, timerStartY + 50);
		}
		if (rightHandUp){
			ofCircle(60,VIDEO_HEIGHT - 20,20);
		}
		if (smallButtonActive && nFramesValid){	// nFramesValid should imply playback
			for (int i = 0;i<rhrIndex;i++){
				if (smallRHRLocations[i] !=NULL){//TODO: should be modified to be generalizable
					//tempSS << "smallGood" << smallRHRLocations[i];
					//tempS = tempSS.str();
					tempButtonX = (int) ((float) timerStartX + ((float) smallRHRLocations[i]/(float) nFrames)*timerWidth);
					(button("smallGood", tempButtonX, timerStartY + 15, 22, 25, smallButtonActive, "images\\r_small.png", "images\\r_small.png")).drawFont(true);
				}
			}
		}
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
		if (!bPlayback){
			standardRecord();
		}
	}
	else if (key == 'p'){
		if (!bRecord){
			standardReplay();
		}
	}
	else if (key =='s'){
		if (bRecord || bPlayback){
			standardStop();
		}
	}
	else if (key == 'b'){
		if (bPlayback){
			if (paused){
				play();
			}
			else{
				pause();
			}
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
	if ((*stopButton).buttonPressed(x,y)){
		standardStop();
	}
	if ((*replayButton).buttonPressed(x,y)){
		standardReplay();
	}
	if ((*recordButton).buttonPressed(x,y)){
		standardRecord();
	}
	if ((*goodIdeaButton).buttonPressed(x,y)){
		smallButtonActive = true;
	}
	if ((*timer).sliderPressed(x,y).first){
		if (bPlayback && !paused){//reason for not paused is in skipTo function
			interruptFastPlayback();//to be able to stop other skipTo's and make a new point to skipT0.
								//(but it also stops getPlaybackRange if that's happening. This may/may not be a problem)
			skipTo((*timer).sliderPressed(x,y).second);
		}
	}
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}


//======================================================================
//BACKEND REPLAY/RECORD SECTION
/// make sure you don't record and playback simultaneously 
/// using the same file!!!

void testApp::startRecording() {
	stopPlayback(); // stop playback if recording
	kinectRecorder.init(ofToDataPath("recording.dat"));
	bRecord = true;
}

void testApp::stopRecording() {
	kinectRecorder.close();
	bRecord = false;
	safeResetLiveMode();
}

void testApp::startPlayback() {
	stopRecording(); //stop recording if starting playback
	kinectPlayer.setup(ofToDataPath("recording.dat"), true); // set record file and source
	kinectPlayer.loop();
	bPlayback = true;
	rightHandUpCount = 0; //reset right hand up count
	getPlaybackRange();
	if (bPlayback && paused){//if already playing, unpause
		standardReplay();
	}
	play();
}

void testApp::stopPlayback() {
	kinectPlayer.close();
	bPlayback = false;
	rightHandUpCount = 0; //reset right hand up count
	nFramesValid = false;
	safeResetLiveMode();
}

//PLAYBACK HELPER FUNCTIONS
///// functions for resetting playback and live modes respectively. Reset some backend and gui variables
void testApp::safeResetReplayMode(){//intended to work if you're already in playback mode
	if (playbackRangeSearchOn){
		currFrame = 0;
		nFrames = 0;
		firstPlaybackTime = 0;
		lastPlaybackTime = 0;
		timerProgression = 0;
		(*timer).setSliderPosX(timerProgression*timerWidth);
	}
	if (bPlayback){
		play();
		interruptFastPlayback();
		stopButtonActive = true;
		replayButtonActive = true;
		recordButtonActive = false;
		goodIdeaButtonActive = true;
		smallButtonActive = false;
		pauseButtonOn = true;
		updateButtonTriggers();
	}
}

void testApp::safeResetLiveMode(){//intended to wrok if you're already in live mode
	play();
	currFrame = 0;
	nFrames = 0;
	nFramesValid = false;
	timerProgression = 0;
	(*timer).setSliderPosX(timerProgression*timerWidth);
	interruptFastPlayback();
	stopButtonActive = false;
	replayButtonActive = true;
	recordButtonActive = true;
	goodIdeaButtonActive = false;
	smallButtonActive = false;
	pauseButtonOn = false;
	updateButtonTriggers();
}

/////function to allow interruption of initial loading period
void testApp::interruptFastPlayback(){
	updateDelay = true;
	if (playbackRangeSearchOn){
		playbackRangeSearchOn = false;
		nFramesValid = false;
		nFrames = 0;
	}
	if (skipToMode){
		skipToMode = false;
		pOSlider = 0.0;
	}
}
/////function to get the number of frames (and where each type of gesture is)
void testApp::getPlaybackRange(){ 
	if (nFrames == 1){
		if (rightHandUp){//before nFrames++ because this is the value for the prevFrame (nFrames is ahead by 1)
			smallRHRLocations[rhrIndex] = nFrames;
			rhrIndex++;
		}
		nFrames++;
		firstPlaybackTime = rawTime;//can interrupt with play, but we don't guaratee properties of the playback bar if you do
	}
	if (!playbackRangeSearchOn){//starts here
		nFramesValid = false;
		playbackRangeSearchOn = true;
		nFrames = 1;//here, we counted the current (unplaybacked) frame as one frame, so we subtract it below
		updateDelay = false;
		prevTimeMillis = 0; //resetting this variable
	}
	else if (prevTimeMillis <= rawTime){
		updateDelay = false;
		prevTimeMillis = rawTime;
		if (rightHandUp){//before nFrames++ because this is the value for the prevFrame (nFrames is ahead by 1)
			smallRHRLocations[rhrIndex%20] = nFrames;
			rhrIndex++;
		}
		nFrames++; //TODO: maybe show how many seconds have been loaded in the future?
	}
	else{
		currFrame = 0;//initializing current frame
		nFrames --; //subtracting the frame we had added earlier
		nFramesValid = true;
		lastPlaybackTime = prevTimeMillis;
		playbackRangeSearchOn = false;
		interruptFastPlayback();
	}
}

void testApp::skipTo(float percentOfSlider){//if user pauses or stops, skipTo should be reset to true and regular playback should resume
											//also, it only works while you are playing (not pausing) (standardReplay erases FastPlayback)
	if ((skipToMode == true) && (currFrame == (int) floor(percentOfSlider*nFrames))){
		standardReplay(); //(TODO: should we pause? to pause it after the frame is found (so people can see things on specific frames?))
		pOSlider = 0.0;
		skipToMode = false;
	}
	else if ((percentOfSlider <= 1.0) && (percentOfSlider >= 0.0) && (skipToMode == false) && nFramesValid){//starts here
		skipToMode = true;
		updateDelay = true;
		pOSlider = percentOfSlider;
	}
	else{
		printf("error in skip function");//maybe remove this. May help with debugging
	}
}

/////GESTURE CHECKS (return booleans)
bool testApp::rightHandRaised(){
	//don't use this unless setting right hand up since there is a counter on it and the answer may differ in between calls
	//dimensions work oppositely top left is 0,0. Then y goes down i think? at least for comparisons
	if ((rightHandPY > headPositionY) || rightHandUpCount >= 100){
		rightHandUpCount = 0;
	}
	if ((rightHandPY < headPositionY)){
		rightHandUpCount ++;
		if (rightHandUpCount == 20){
			return true;
		}
	}
	return false;
}


/////BUTTON CONTROL SECTION
void testApp::standardStop(){
		if (bRecord){
			stopRecording();
			stopButtonActive = false;
			replayButtonActive = true;
			recordButtonActive = true;
			goodIdeaButtonActive = false;
			smallButtonActive = false;
			pauseButtonOn = false;
		}
		if (bPlayback){
			stopPlayback();
			stopButtonActive = false;
			replayButtonActive = true;
			recordButtonActive = true;
			goodIdeaButtonActive = false;
			smallButtonActive = false;
			pauseButtonOn = false;
		}
		updateButtonTriggers();
}
void testApp::standardReplay(){
		if (!bPlayback){
			startPlayback();
			stopButtonActive = true;
			replayButtonActive = true;
			recordButtonActive = false;
			goodIdeaButtonActive = true;
			smallButtonActive = false;
			pauseButtonOn = true; //note, this indicates whether replay picture changes or not
		}
		else{
			if (!paused){
				pause();
				pauseButtonOn = false;
				interruptFastPlayback();
			}
			else if (paused){
				play();
				pauseButtonOn = true;
				interruptFastPlayback();
			}
		}
		updateButtonTriggers();
}
void testApp::standardRecord(){
		startRecording();
		stopButtonActive = true;
		replayButtonActive = true;
		recordButtonActive = false;
		goodIdeaButtonActive = false;
		smallButtonActive = false;
		updateButtonTriggers();
}
void testApp::updateButtonTriggers(){
	(*stopButton).trigger = stopButtonActive;
	(*replayButton).trigger = replayButtonActive;
	(*recordButton).trigger = recordButtonActive;
	(*goodIdeaButton).trigger = goodIdeaButtonActive;
	//note, small buttons don't need to be updated because they are made the moment before drawing them
}