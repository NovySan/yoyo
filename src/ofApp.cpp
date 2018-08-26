#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(24);
    ofSetVerticalSync(true);
    ofBackground(0, 0, 0);

    movies.assign(N_CAMERAS, ofVideoPlayer());

    for (int i = 0; i < N_CAMERAS; i++){
        movies[i].setPixelFormat(OF_PIXELS_RGB);
        movies[i].loadMovie("movies/yoyoma-1-" + ofToString((i+1),1) +"-prepped.mov");
        movies[i].setLoopState(OF_LOOP_NORMAL);
        movies[i].play();
   }
    audioMovie.setPixelFormat(OF_PIXELS_RGB);
    audioMovie.loadMovie("movies/yoyo_audio.mov");
    audioMovie.setLoopState(OF_LOOP_NORMAL);
    audioMovie.play();

    timeTuner = 3;//for tuning how often a sync check is executed.
    timerUpdate = 3;


    //load warp settings
    settings.loadFile("positions.xml");



    scaleAmount     = 4;
    vidWidth 		= movies[0].getWidth() / scaleAmount;
	vidHeight 		= movies[0].getHeight() / scaleAmount;
	origWidth       = vidWidth * scaleAmount;
    origHeight      = vidHeight * scaleAmount;
	xVidPos			= 0;
	yVidPos			= 0;
    offsetAmount    = vidWidth * scaleAmount + 0;



	// allocate memory
    for (int i = 0; i < N_CAMERAS; i++){
        giWarped[i].allocate(vidWidth, vidHeight);
        camImage[i].allocate(vidWidth, vidHeight);
        fboWarp[i].allocate(vidWidth, vidHeight,GL_RGB);
        dumpImage[i].allocate(origWidth, origHeight);
        circleColor[i].r = ofRandom(255);
        circleColor[i].g = ofRandom(255);
        circleColor[i].b = ofRandom(255);
        xAdjust[i] = 0;
        xAdjustState[i] = false;

    }



    for( int i = 0;i < N_CAMERAS;i++){

        srcPositions[i]  = new ofPoint[4];
        srcPositions[i][0].set(xVidPos, yVidPos, 0);
        srcPositions[i][1].set(origWidth + xVidPos, yVidPos, 0);
        srcPositions[i][2].set(origWidth + xVidPos, origHeight + yVidPos, 0);
        srcPositions[i][3].set(xVidPos, origHeight + yVidPos, 0);

        dstPositions[i]  = new ofPoint[4];
        dstPositions[i][0].set(xVidPos, yVidPos, 0);
        dstPositions[i][1].set(origWidth + xVidPos, yVidPos, 0);
        dstPositions[i][2].set(origWidth + xVidPos, origHeight + yVidPos, 0);
        dstPositions[i][3].set(xVidPos, origHeight + yVidPos, 0);

    }

    warp = true;


}

//--------------------------------------------------------------
void ofApp::update(){

    ofSetBackgroundAuto(true);
    audioMovie.update();
    int currentFrame = audioMovie.getCurrentFrame();
    for (int i = 0; i < N_CAMERAS; i++){
        movies[i].update();

        }


    if (timerUpdate > timeTuner){
        for (int i = 0; i < N_CAMERAS; i++){
            int checkSync = movies[i].getCurrentFrame();
            if (checkSync != currentFrame){
            movies[i].setFrame(currentFrame);
            }
        }
        timerUpdate = 0;
    }


   timerUpdate++;

        //resize the movies
        for (int i = 0; i < N_CAMERAS; i++){
            unsigned char * pixels = movies[i].getPixels();
            dumpImage[i].resize(origWidth, origHeight);
            dumpImage[i].setFromPixels(pixels, origWidth, origHeight);
            dumpImage[i].resize(vidWidth, vidHeight);

        }

        //copy the fbos into the unwarped image containers then warp them.
        for (int i = 0; i < N_CAMERAS; i++){
            fboWarp[i].readToPixels(camImage[i].getPixelsRef());
            giWarped[i].warpIntoMe(camImage[i],srcPositions[i],dstPositions[i] );

        }

        for (int i = 0; i < N_CAMERAS; i++){

            xAdjust[i] =  xAdjust[i];
        }



}

//--------------------------------------------------------------
void ofApp::draw(){



    // draw the warped images
	ofSetColor(255, 255, 255, 255);
    int offset = 0;
    for (int i = 0; i < N_CAMERAS; i++){
        giWarped[i].draw(xVidPos + offset + xAdjust[i], yVidPos, origWidth, origHeight);
        offset += offsetAmount;
    }


    // build the fbos
    for (int i = 0; i < N_CAMERAS; i++){
        fboWarp[i].begin();
        dumpImage[i].draw(0,0);
        if (warp){
            //ofCircle(xVidPos + (sin(ofGetFrameNum())*10)+30, yVidPos, 25);
            ofSetColor(circleColor[i]);//for registration purposes
            ofCircle(xVidPos+200, yVidPos+100, 25);
        }
        fboWarp[i].end();
        ofSetColor(255, 255, 255, 255);

    }

if (warp){
	// Draw lines between points
    int offset = 0;
    for (int i = 0; i < N_CAMERAS; i++){
        ofSetColor(255,0,0);
        ofNoFill();
        ofBeginShape();
        for (int j = 0; j < 4; j++){
            ofVertex(dstPositions[i][j].x + offset, dstPositions[i][j].y);
        }
        ofEndShape(true);
        offset += offsetAmount;
    }


	// Draw points
	ofFill();
	//ofSetColor(0,255,0);
    offset = 0;
    for (int i = 0; i < N_CAMERAS; i++){
        ofSetColor(circleColor[i]);
        for (int j = 0; j < 4; j++){
            ofCircle(dstPositions[i][j].x + offset, dstPositions[i][j].y, 50);
        }
        offset += offsetAmount;
        }
    }



}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key){

        case 'w':
            warp = !warp;
        break;
        case 's':
            if (warp){
            dstPointPositions.saveFile("positions.xml");


             //format xAdjust for XML export.
            xAdjustPositions.clear();
            xAdjustPositions.addTag("x_positions");
            xAdjustPositions.pushTag("x_positions");
                for (int i = 0; i < N_CAMERAS; i++){
                    xAdjustPositions.addTag("quad" + ofToString(i));
                    xAdjustPositions.pushTag("quad" + ofToString(i));
                    xAdjustPositions.addTag("x_offset" + ofToString(i));
                    xAdjustPositions.pushTag("x_offset" + ofToString(i));

                    //so set the xAdjust values in the file
                    xAdjustPositions.addValue("X", xAdjust[i]);
                    xAdjustPositions.popTag();//pop position
                    xAdjustPositions.popTag();//pop position
                }
            xAdjustPositions.popTag();//pop position
            xAdjustPositions.saveFile("xAdjust.xml");
        }
        break;

        case 'r':
            if (warp){
            if(settings.loadFile("positions.xml")){
                settings.pushTag("positions");
                for (int i = 0; i < N_CAMERAS; i++){
                    settings.pushTag("quad" + ofToString(i));
                    for (int j = 0; j < 4; j++){
                        settings.pushTag("point" + ofToString(j));
                        dstPositions[i][j].x = settings.getValue("X", 0);
                        dstPositions[i][j].y = settings.getValue("Y", 0);
                        dstPositions[i][j].z = settings.getValue("Z", 0);
                        settings.popTag();
                    }

                settings.popTag(); //pop position
                }
                settings.popTag(); //pop position
            }else{
                ofLogError("WHAT WHAT?? Position file did not load!");
                }

            if (xAdjustPositions.loadFile("xAdjust.xml")){
                xAdjustPositions.pushTag("x_positions");
                for (int i = 0; i < N_CAMERAS; i++){
                    xAdjustPositions.pushTag("quad" + ofToString(i));
                    xAdjustPositions.pushTag("x_offset" + ofToString(i));
                    xAdjust[i] = xAdjustPositions.getValue("X",0);
                    xAdjustPositions.popTag();//gets out of 'x_offset' tag
                    xAdjustPositions.popTag();//gets out of 'quad' tag??
                }
                xAdjustPositions.popTag();//gets out of 'x_positions' tag??
            }else{
                ofLogError("xAdjust file did not load!");
                }

        }
        break;
        case '0':
            xAdjustState[0] = true;
            xAdjustState[1] = false;
            xAdjustState[2] = false;
            xAdjustState[3] = false;
            xAdjustState[4] = false;
            xAdjustState[5] = false;
        break;
        case '1':
            xAdjustState[0] = false;
            xAdjustState[1] = true;
            xAdjustState[2] = false;
            xAdjustState[3] = false;
            xAdjustState[4] = false;
            xAdjustState[5] = false;
        break;
        case '2':
            xAdjustState[0] = false;
            xAdjustState[1] = false;
            xAdjustState[2] = true;
            xAdjustState[3] = false;
            xAdjustState[4] = false;
            xAdjustState[5] = false;
        break;
        case '3':
            xAdjustState[0] = false;
            xAdjustState[1] = false;
            xAdjustState[2] = false;
            xAdjustState[3] = true;
            xAdjustState[4] = false;
            xAdjustState[5] = false;
        break;
        case '4':
            xAdjustState[0] = false;
            xAdjustState[1] = false;
            xAdjustState[2] = false;
            xAdjustState[3] = false;
            xAdjustState[4] = true;
            xAdjustState[5] = false;
        break;
        case '5':
            xAdjustState[0] = false;
            xAdjustState[1] = false;
            xAdjustState[2] = false;
            xAdjustState[3] = false;
            xAdjustState[4] = false;
            xAdjustState[5] = true;
        break;
        case '6':
            xAdjustState[0] = false;
            xAdjustState[1] = false;
            xAdjustState[2] = false;
            xAdjustState[3] = false;
            xAdjustState[4] = false;
            xAdjustState[5] = false;
        break;

        case '+':
            if (warp){

                if (xAdjustState[0]== true){
                    xAdjust[0]++;
                }else if (xAdjustState[1]== true){
                    xAdjust[1]++;
                }else if (xAdjustState[2]== true){
                    xAdjust[2]++;
                }else if (xAdjustState[3]== true){
                    xAdjust[3]++;
                }else if (xAdjustState[4]== true){
                    xAdjust[4]++;
                }else {
                    xAdjust[5]++;
                }
            }
        break;

        case '-':
             if (warp){
                if (xAdjustState[0]== true){
                    xAdjust[0]--;
                }else if (xAdjustState[1]== true){
                    xAdjust[1]--;
                }else if (xAdjustState[2]== true){
                    xAdjust[2]--;
                }else if (xAdjustState[3]== true){
                    xAdjust[3]--;
                }else if (xAdjustState[4]== true){
                    xAdjust[4]--;
                }else {
                    xAdjust[5]--;
                }
             }
        break;

    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

    if (warp){
        if (activePointIn > -1){

            dstPositions[activeQuadIn][activePointIn].x = x - (offsetAmount * activeQuadIn);
            dstPositions[activeQuadIn][activePointIn].y = y;


        }
    }

    //format dstPositions for XML export.
    dstPointPositions.clear();
    dstPointPositions.addTag("positions");
    dstPointPositions.pushTag("positions");
    for (int i = 0; i < N_CAMERAS; i++){
        dstPointPositions.addTag("quad" + ofToString(i));
        dstPointPositions.pushTag("quad" + ofToString(i));
        for (int j = 0; j < 4; j++){

            //each position tag represents one point
            dstPointPositions.addTag("point" + ofToString(j));
            dstPointPositions.pushTag("point" + ofToString(j));

            //so set the three values in the file
            dstPointPositions.addValue("X", dstPositions[i][j].x);
            dstPointPositions.addValue("Y", dstPositions[i][j].y);
            dstPointPositions.addValue("Z", dstPositions[i][j].z);
            dstPointPositions.popTag();//pop position
        }
        dstPointPositions.popTag();//pop position
    }
    dstPointPositions.popTag();//pop position

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

    if (warp){
    //this is not the best way
        activeQuadIn = -1;
        activePointIn = -1;

        float smallestDist  = 999999;
        float clickRadius = 50;
        int offset = 0;
        for (int i = 0; i < N_CAMERAS; i++){
             for (int j = 0; j < 4; j++){
                 ofPoint inputPt;
                 inputPt.x = dstPositions[i][j].x + offset;
                 inputPt.y = dstPositions[i][j].y;
                 inputPt.z = 0;
                 float len = sqrt( ((inputPt.x) - x) * ((inputPt.x) - x) +
                         (inputPt.y - y) * (inputPt.y - y));
                 if (len < clickRadius && len < smallestDist){
                     activePointIn  = j;
                     activeQuadIn = i;
                     //cout << activeQuadIn << "+" << activePointIn << endl;
                     smallestDist = len;
                 }

             }
        offset += offsetAmount;
        }
    }

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
