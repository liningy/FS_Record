#include "ofMain.h"
#include "button.h"



button::button(string _buttonName, int _originX, int _originY, int _bWidth, int _bHeight, bool _trigger,string _imgName_a, string _imgName_b, int _alternateOriginX, int _alternateOriginY)
{
	buttonName=_buttonName;	
	fontSize=16;
	font.loadFont("DIN.otf",12);

	originX=_originX;
	originY=_originY;
	alternateOriginX=_alternateOriginX;
	alternateOriginY=_alternateOriginY;
	bWidth=_bWidth;
	bHeight=_bHeight;
	trigger=_trigger;

	img_a.loadImage(_imgName_a);
	img_b.loadImage(_imgName_b);

	
}

void button::drawFont(bool _buttonPressed, bool alternatePic){
	//ofPushMatrix();
	if (_buttonPressed && !alternatePic){
		//ofSetColor(0xFF9933);
		img_b.draw(originX,originY);
	} else {//from openframeworks example
		//ofSetColor(0,0,0,76);    // black, 30% transparent
		//ofSetColor(0xCCCCCC);
		if ((alternateOriginX != NULL) && (alternateOriginY != NULL)){
			img_a.draw(alternateOriginX,alternateOriginY);
		}
		else{
			img_a.draw(originX,originY);
		}
	}
	//font.drawString(buttonName, originX,originY-10);
	
	//ofNoFill();
	////ofRect(originX, originY-bHeight,bWidth,bHeight);
	//ofRect(originX,originY,bWidth,bHeight);
	//ofPopMatrix();
}

bool button::buttonPressed(int x, int y){
		if((x>=originX && x<originX+bWidth) && (y>=originY && y<originY+bHeight) && trigger){
			return true;
		}else{
			return false;
		}
}

void button::typeWindow(int xPosition, int yPosition){
	ofSetColor(0,0,0,255);
	ofRect(xPosition,yPosition,60,20);
	font.drawString(typeContents,xPosition, yPosition);
}

void button::updateTrigger(bool newTrigger){
	trigger = newTrigger;
}
