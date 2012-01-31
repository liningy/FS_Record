/*
 * KinectRecorder.cpp
 *
 *  Created on: 03/01/2011
 *      Author: arturo
 */

#include "ofxKinectRecorder.h"

ofxKinectRecorder::ofxKinectRecorder() {
	f=NULL;

}

ofxKinectRecorder::~ofxKinectRecorder() {
	// TODO Auto-generated destructor stub
}

void ofxKinectRecorder::init(const string & filename){
	f = fopen(ofToDataPath(filename).c_str(),"wb");
}

void ofxKinectRecorder::newFrame(unsigned char* rgb, unsigned char * raw_depth, int headx, int heady, int headz, int leftshoulderx, int leftshouldery, int rightshoulderx, int rightshouldery, int lefthandx, int lefthandy, int righthandx, int righthandy) {
	if(!f ||!raw_depth) return;
	
	if(rgb != NULL) {
		fwrite(rgb,640*480*4,1,f);
	}
	fwrite(raw_depth,640*480*4,1,f);

	fwrite(&headx,sizeof(int),1,f);
	fwrite(&heady,sizeof(int),1,f);
	fwrite(&headz,sizeof(int),1,f);
	fwrite(&leftshoulderx,sizeof(int),1,f);
	fwrite(&leftshouldery,sizeof(int),1,f);
	fwrite(&rightshoulderx,sizeof(int),1,f);
	fwrite(&rightshouldery,sizeof(int),1,f);
	fwrite(&lefthandx,sizeof(int),1,f);
	fwrite(&lefthandy,sizeof(int),1,f);
	fwrite(&righthandx,sizeof(int),1,f);
	fwrite(&righthandy,sizeof(int),1,f);
}

void ofxKinectRecorder::close(){
	if(!f) return;
	fclose(f);
	f=0;
}

bool ofxKinectRecorder::isOpened(){
	return f;
}
