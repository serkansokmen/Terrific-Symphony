#include "TS_Tracker.h"

# define LANE_TRACKER_HEIGHT 120

//--------------------------------------------------------------
void TS_Tracker::setup(){
    
    // open an outgoing connection to HOST:PORT
	sender.setup(HOST, PORT);
    receiver.setup(PORT);
    
    vidWidth = 720;
    vidHeight = 576;
    
#ifdef _USE_LIVE_VIDEO
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(vidWidth,vidHeight);
#else
    vidPlayer.loadMovie("RefAnimation/Perspective_BW_1_v1.mov");
    vidPlayer.play();
#endif
    
    colorImg.allocate(vidWidth, vidHeight);
    grayImage.allocate(vidWidth, vidHeight);
    grayBg.allocate(vidWidth, vidHeight);
    grayDiff.allocate(vidWidth, vidHeight);
    
    bLearnBakground = true;
    threshold = 80;
    
    for (int i=0; i<slitCount; i++) {
        float x = i * vidWidth / slitCount;
        float y = vidHeight;
        float width = vidWidth / slitCount;
        float height = -LANE_TRACKER_HEIGHT;
        ofRectangle *rect = new ofRectangle(x, y, width, height);
        scanRects.push_back(rect);
    }
}

//--------------------------------------------------------------
void TS_Tracker::update()
{
    ofBackground(10, 10, 10);
    
    bool bNewFrame = false;
    
#ifdef _USE_LIVE_VIDEO
    vidGrabber.update();
    bNewFrame = vidGrabber.isFrameNew();
#else
    vidPlayer.update();
    bNewFrame = vidPlayer.isFrameNew();
#endif
    
    if (bNewFrame)
    {
#ifdef _USE_LIVE_VIDEO
        colorImg.setFromPixels(vidGrabber.getPixels(), vidWidth, vidHeight);
#else
        colorImg.setFromPixels(vidPlayer.getPixels(), vidWidth, vidHeight);
#endif
        grayImage = colorImg;
        if (bLearnBakground == true)
        {
            grayBg = grayImage;     // the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayDiff, 20, (340 * vidHeight) / 3, slitCount, false);    // find holes
    }
    
    
    // check for blobs
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        ofxCvBlob *blob = &contourFinder.blobs[i];
        
        for (int j=0; j<slitCount; j++)
        {
            int scanRectIndex = j;
            ofRectangle rect = *scanRects[scanRectIndex];
            ofxOscMessage m;
            
            m.setAddress("/line/index/state");
            m.addIntArg(scanRectIndex);
            
            if (rect.inside(blob->centroid)) {
                m.addStringArg("on");
            } else {
                m.addStringArg("off");
            }
            // m.addFloatArg(ofGetElapsedTimef());
            sender.sendMessage(m);
        }
        contourFinder.blobs[i].draw(0, 0);
    }
}

//--------------------------------------------------------------
void TS_Tracker::draw()
{
    // draw the incoming, the grayscale, the bg and the thresholded difference
    ofSetHexColor(0xffffff);
    colorImg.draw(0,0,vidWidth, vidHeight);
    //grayImage.draw(vidWidth/2,20,vidWidth/2, vidHeight/2);
    //grayBg.draw(20,vidHeight/2,vidWidth/2, vidHeight/2);
    //grayDiff.draw(vidWidth/2,vidHeight/2,vidWidth/2, vidHeight/2);
    
    // draw the contours:
    // ofFill();
    // ofSetHexColor(0x333333);
    // ofRect(vidWidth/2,vidHeight/2+100,vidWidth/2,vidHeight/2);
    // ofSetHexColor(0xffffff);
    
    // check for blobs
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        ofxCvBlob *blob = &contourFinder.blobs[i];
        
        for (int i=0; i<slitCount; i++)
        {
            ofRectangle rect = *scanRects[i];
            if (rect.inside(blob->centroid)) {
                ofFill();
                ofSetColor(255, 0, 0);
            } else {
                ofNoFill();
                ofSetColor(255, 255, 255);
            }
            ofRect(rect.x, rect.y, rect.width, rect.height);
        }
        contourFinder.blobs[i].draw(0, 0);
    }
    
    // report:
    ofSetHexColor(0xffffff);
    char reportStr[1024];
    sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f", threshold, contourFinder.nBlobs, ofGetFrameRate());
    //ofDrawBitmapString(reportStr, 20, 600);
    
    // check for waiting messages
	while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);
        string msg_string;
        
        msg_string = m.getAddress();
        msg_string += ": ";
        for(int i = 0; i < m.getNumArgs(); i++){
            // get the argument type
            msg_string += m.getArgTypeName(i);
            msg_string += ":";
            // display the argument - make sure we get the right type
            if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                msg_string += ofToString(m.getArgAsInt32(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                msg_string += ofToString(m.getArgAsFloat(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                msg_string += m.getArgAsString(i);
            }
            else{
                msg_string += "unknown";
            }
        }
        cout << msg_string << endl;
        
		// check for OSC message
		if (m.getAddress() == "/line/index/state")
        {
			// get scanRectIndex
            int scanRectIndex = m.getArgAsInt32(0);
            // cout << "rect index " << scanRectIndex << " is " << m.getArgAsString(1) << endl;
		}
        
	}
}

//--------------------------------------------------------------
void TS_Tracker::keyPressed(int key)
{
    
    switch (key){
        case ' ':
            bLearnBakground = true;
            break;
        case '+':
            threshold ++;
            if (threshold > 255) threshold = 255;
            break;
        case '-':
            threshold --;
            if (threshold < 0) threshold = 0;
            break;
    }
}

//--------------------------------------------------------------
void TS_Tracker::keyReleased(int key)
{
    
}

//--------------------------------------------------------------
void TS_Tracker::mouseMoved(int x, int y )
{
    
}

//--------------------------------------------------------------
void TS_Tracker::mouseDragged(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void TS_Tracker::mousePressed(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void TS_Tracker::mouseReleased(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void TS_Tracker::windowResized(int w, int h)
{
    
}

//--------------------------------------------------------------
void TS_Tracker::gotMessage(ofMessage msg)
{
    
}

//--------------------------------------------------------------
void TS_Tracker::dragEvent(ofDragInfo dragInfo)
{
    
}
