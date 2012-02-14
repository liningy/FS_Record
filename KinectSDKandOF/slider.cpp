#include "ofMain.h"
#include "slider.h"

slider::slider(int x, int y, int _sWidth)
{
	originX=x;
	originY=y;
	sWidth=_sWidth;
	sHeight = 15;
	sliderPosX=x;
	sliderPosY=y;
	img.loadImage("data/images/button.png");
}

//this function has to be used under void mousePressed() function
void slider::getSliderPosX(int _mouseX, int _mouseY){
	if((_mouseX>=originX && _mouseX<originX+sWidth) && (_mouseY>=originY-5 && _mouseY<originY+20))
		sliderPosX=_mouseX;
}

void slider::drawSlider(float startFrom, float endTo){
	//ofFill();
    //ofSetColor(0xCCCCCC);
	ofRect(originX+sWidth,originY,3,sHeight);
	//ofSetColor(0xFF9933);
	//ofTriangle(sliderPosX,sliderPosY+5,sliderPosX+16,sliderPosY+5,sliderPosX+8,sliderPosY-8);
	//img.draw(sliderPosX,sliderPosY);
	ofRect(originX,originY,sliderPosX,sHeight);

	value=ofMap(sliderPosX,originX,originX+sWidth,startFrom,endTo);
	//printf("value= %d \n", value);
}
//not tested yet
pair<bool,float> slider::sliderPressed(int x, int y){
		if((x>=originX && x<originX+sWidth) && (y>=originY && y<originY+sHeight)){//note: unlike button there is no trigger here
			return make_pair(true,(float)(x-originX)/(float)(sWidth));
		}else{
			return make_pair(false,0);
		}
}

void slider::setSliderPosX(int newSliderPosX){
	sliderPosX=newSliderPosX;
}
