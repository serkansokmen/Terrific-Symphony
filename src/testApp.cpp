#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
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
        float height = -20;
        ofRectangle *rect = new ofRectangle(x, y, width, height);
        scanRects.push_back(rect);
    }
}

//--------------------------------------------------------------
void testApp::update()
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
}

//--------------------------------------------------------------
void testApp::draw()
{
    // draw the incoming, the grayscale, the bg and the thresholded difference
    ofSetHexColor(0xffffff);
    colorImg.draw(0,0,vidWidth, vidHeight);
    //grayImage.draw(vidWidth/2,20,vidWidth/2, vidHeight/2);
    //grayBg.draw(20,vidHeight/2,vidWidth/2, vidHeight/2);
    //grayDiff.draw(vidWidth/2,vidHeight/2,vidWidth/2, vidHeight/2);
    
    //ofPushMatrix();
    for (int i=0; i<slitCount; i++)
    {
        ofRectangle rect = *scanRects[i];
//        ofTranslate(rect.x, rect.y);
        ofSetColor(255, 0, 0);
        ofRect(rect.x, rect.y, rect.width, rect.height);
    }
    //ofPopMatrix();
    
    // then draw the contours:
    
    //    ofFill();
    //    ofSetHexColor(0x333333);
    //    ofRect(vidWidth/2,vidHeight/2+100,vidWidth/2,vidHeight/2);
    //    ofSetHexColor(0xffffff);
    
    // we could draw the whole contour finder
    //contourFinder.draw(360,540);
    
    // or, instead we can draw each blob individually,
    // this is how to get access to them:
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        contourFinder.blobs[i].draw(0, 0);
    }
    
    // finally, a report:
    
    ofSetHexColor(0xffffff);
    char reportStr[1024];
    sprintf(reportStr, "bg subtraction and blob detection\npress ' ' to capture bg\nthreshold %i (press: +/-)\nnum blobs found %i, fps: %f", threshold, contourFinder.nBlobs, ofGetFrameRate());
    ofDrawBitmapString(reportStr, 20, 600);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
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
void testApp::keyReleased(int key)
{
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{
    
}
