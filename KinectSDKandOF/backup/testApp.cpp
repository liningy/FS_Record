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
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(bgColor.r,bgColor.g,bgColor.b);
	g_kinectGrabber.Kinect_Update();
    
	// load the RGBA values into a texture
	colorAlphaPixels = g_kinectGrabber.Kinect_getAlphaPixels();
	if(colorAlphaPixels != NULL) {
		texColorAlpha.loadData(colorAlphaPixels, VIDEO_WIDTH,VIDEO_HEIGHT, GL_RGBA);
	}

	// load grayimage (which includes depth information) into a texture
	grayPixels = (BYTE*)g_kinectGrabber.Kinect_getDepthPixels();	
	if (grayPixels != NULL) {
		texGray.loadData(grayPixels,DEPTH_WIDTH,DEPTH_HEIGHT, GL_RGBA);
	}
	
	// depthBuff stores unsigned short (2 bytes) pointers pointing to each pixels's depth in milimeter; in total it has 1 * width * height
	USHORT* depthBuff = g_kinectGrabber.Kinect_getDepthBuffer();

	
    // Gets the skeleton joints' positions
	g_kinectGrabber.getJointsPoints();
	headPositionX=g_kinectGrabber.headJoints_x;
	headPositionY=g_kinectGrabber.headJoints_y;
	headPositionZ=g_kinectGrabber.headJoints_z;
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
	ofCircle(headPositionX+640,headPositionY,10);
	ofCircle(g_kinectGrabber.shoulderLeft_x+640, g_kinectGrabber.shoulderLeft_y,10);
	ofCircle(g_kinectGrabber.shoulderRight_x+640,g_kinectGrabber.shoulderRight_y,10); 
	ofCircle(g_kinectGrabber.handLeft_x+640,g_kinectGrabber.handLeft_y,10); 
	ofCircle(g_kinectGrabber.handRight_x+640,g_kinectGrabber.handRight_y,10);
	ofLine(headPositionX+640,headPositionY,g_kinectGrabber.shoulderLeft_x+640, g_kinectGrabber.shoulderLeft_y);
	ofLine(g_kinectGrabber.shoulderLeft_x+640, g_kinectGrabber.shoulderLeft_y,g_kinectGrabber.handLeft_x+640,g_kinectGrabber.handLeft_y);
	ofLine(headPositionX+640,headPositionY,g_kinectGrabber.shoulderRight_x+640,g_kinectGrabber.shoulderRight_y);
	ofLine(g_kinectGrabber.shoulderRight_x+640,g_kinectGrabber.shoulderRight_y,g_kinectGrabber.handRight_x+640,g_kinectGrabber.handRight_y);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
//-------------------------------------------------------------
void testApp::exit(){
	//printf("cleaning up\n");
	g_kinectGrabber.Kinect_UnInit();
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){
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
