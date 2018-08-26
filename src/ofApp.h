#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"

#define N_CAMERAS 6

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		vector<ofVideoPlayer>      movies;
		ofVideoPlayer              audioMovie;
		int                        timeTuner;//for tuning how often a sync check is executed.
        int                        timerUpdate;

        //Corner warping
        ofxCvColorImage         giWarped[N_CAMERAS];
        ofxCvColorImage         camImage[N_CAMERAS];
        ofxCvColorImage         dumpImage[N_CAMERAS];
        ofFbo                   fboWarp[N_CAMERAS];

        bool                    warp;


        int						vidWidth, vidHeight;
        int                     origWidth, origHeight;
        int						activePointIn, activeQuadIn;
        int						xVidPos, yVidPos;
        int                     offsetAmount;
        int                     scaleAmount;
        int                     xAdjust[N_CAMERAS];
        bool                    xAdjustState[N_CAMERAS];

        ofPoint *				srcPositions[N_CAMERAS];
        ofPoint *				dstPositions[N_CAMERAS];

        ofColor                 circleColor[N_CAMERAS];

        ofxXmlSettings          settings;
        ofxXmlSettings          dstPointPositions;
        ofxXmlSettings          xAdjustPositions;

};
