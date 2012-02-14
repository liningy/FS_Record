/*
 * KinectRecorder.h
 *
 *  Created on: 03/01/2011
 *      Author: arturo
 */

#pragma once

#include "ofMain.h"

class ofxKinectRecorder {
public:
	ofxKinectRecorder();
	virtual ~ofxKinectRecorder();

	void init(const string & filename);
	void newFrame(unsigned char* rgb = NULL, unsigned char * raw_depth = NULL, time_t = NULL, int headx = NULL, int heady = NULL, int headz = NULL, int leftshoulderx = NULL, int leftshouldery = NULL, int rightshoulderx = NULL, int rightshouldery = NULL, int lefthandx = NULL, int lefthandy = NULL, int righthandx = NULL, int righthandy = NULL, bool righthandraised = false);
	void close();

	bool isOpened();

private:
	FILE * f;
};
