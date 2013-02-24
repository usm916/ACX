#pragma once

#include "ofMain.h"
#include "ofxGameCamera.h"
#include "particleFlow.h"
#include "sphere.h"
#include "ofxOsc.h"
//#include "ofxSyphon.h"


#define PORT 12000
#define NUM_MSG_STRINGS 20

class testApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
        void exit();
    
        void drawGrid();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void birth();
    
        void buildMesh();
        void setScreenMeshVertex();
        bool checkAreaRect(float _x, float _y, ofPoint _centerPoint, ofVec2f _size);
        ofVboMesh scrVboMesh;
        ofVboMesh fluidScrMesh;
        ofTexture* scr;
        bool isMouseDragged;
        int msX, msY;
        ofPoint center;
        int cornerCnt;

        TextMove textPart, textPart2;
        ofxGameCamera camera;
        ofImage milkyWay;
        ofPoint screenSize;
    
        int recTimeCnt;
        int* curTimeCnt;
        int moveTimeCnt;
    
        bool bRecord, bGrid, bSphere, bSetVertex, bScreenParam, bUseMouse, bBirth;
        bool bDrawConnection = false;
        ofVec3f *pos, *vec, *rot;
    
        ofFbo fboScr;
        ofImage savingImage;
    
        Sphere univers;
        /////////// to use OSC ////////////////
        ofxOscReceiver receiver;
        int current_msg_string;
        string msg_strings[NUM_MSG_STRINGS];
        float timers[NUM_MSG_STRINGS];
    
        int mouseX, mouseY;
        string mouseButtonState;
    
        //to Output image
//        ofxSyphonServer mainOutputSyphonServer;

    
};
