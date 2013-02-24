#include "testApp.h"
#include "ofxPublishScreen.h"

ofxPublishScreen::FboPublisher pub;
/*
 These functions are for adding quads and triangles to an ofMesh -- either
 vertices, or texture coordinates.
 */
//--------------------------------------------------------------
void addFace(ofVboMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c) {
	mesh.addVertex(a);
	mesh.addVertex(b);
	mesh.addVertex(c);
}

//--------------------------------------------------------------
void addFace(ofVboMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d) {
	addFace(mesh, a, b, c);
	addFace(mesh, a, c, d);
}

//--------------------------------------------------------------
void addTexCoords(ofVboMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c) {
	mesh.addTexCoord(a);
	mesh.addTexCoord(b);
	mesh.addTexCoord(c);
}

//--------------------------------------------------------------
void addTexCoords(ofVboMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d) {
	addTexCoords(mesh, a, b, c);
	addTexCoords(mesh, a, c, d);
}

//--------------------------------------------------------------
void testApp::buildMesh(){
    // OF_PRIMITIVE_TRIANGLES means every three vertices create a triangle
    float width = 5, height = 5;//Number of Grid
	scrVboMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    float gridSizeX, gridSizeY;

    gridSizeX = fboScr.getWidth()/width;
    gridSizeY = fboScr.getHeight()/height;
	int skip = 1;	// this controls the resolution of the mesh
	ofVec3f zero(0, 0, 0);
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			/*
			 To construct a mesh, we have to build a collection of quads made up of
			 the current pixel, the one to the right, to the bottom right, and
			 beneath. These are called nw, ne, se and sw. To get the texture coords
			 we need to use the actual image indices.
			 */
			ofVec3f nw = ofVec3f( x*gridSizeX, y*gridSizeY, 0);
			ofVec3f ne = ofVec3f( x*gridSizeX+gridSizeX, y*gridSizeY, 0);
			ofVec3f se = ofVec3f( x*gridSizeX+gridSizeX, y*gridSizeY+gridSizeY, 0);
			ofVec3f sw = ofVec3f( x*gridSizeX, y*gridSizeY+gridSizeY, 0);
			ofVec2f nwi((float)x/width, (float)y/height);
			ofVec2f nei(float((x+1.0)/width), (float)y/height);
			ofVec2f swi((float)x/width, (float)(y+1)/height);
			ofVec2f sei(float((x+1.0)/width), (float)(y+1)/height);
            //            cout << nwi <<" | "<< nei <<" | "<< swi <<" | "<< sei << endl;
			
// ignore any zero-data (where there is no depth info)
//			if(nw != zero && ne != zero && sw != zero && se != zero) {
            addFace(scrVboMesh, nw, ne, se, sw);
            addTexCoords(scrVboMesh, nwi, nei, sei, swi);
//			}
		}
	}
    // FLUID MESH
    ofVec3f nw = ofVec3f( -ofGetWidth()/2, -ofGetHeight()/2, 0);
    ofVec3f ne = ofVec3f( ofGetWidth()/2, -ofGetHeight()/2, 0);
    ofVec3f se = ofVec3f( ofGetWidth()/2, ofGetHeight()/2, 0);
    ofVec3f sw = ofVec3f( -ofGetWidth()/2, ofGetHeight()/2, 0);
    ofVec2f nwi( 0, 0);
    ofVec2f nei( 1, 0);
    ofVec2f swi( 1, 1);
    ofVec2f sei( 0, 1);
    addFace(fluidScrMesh, nw, ne, se, sw);
    addTexCoords(fluidScrMesh, nwi, nei, swi, sei);

    bSetVertex = false;
    scr = &fboScr.getTextureReference();
    scr->setTextureWrap(GL_REPEAT, GL_REPEAT);
}

//--------------------------------------------------------------
void testApp::setup(){
    
    receiver.setup(PORT);
    pub.setup(20000, 1280, 720);
    
    ofSetWindowTitle("Particles with Syphon");
//	mainOutputSyphonServer.setName("Screen Output");

    ofSetFrameRate(30);
    ofEnableNormalizedTexCoords(); //This lets you do 0-1 range instead of 0-640 (in pixels)
    ofSetVerticalSync(true);
    
    screenSize = ofPoint(1920,1350);
    textPart = TextMove(0,&screenSize);
    textPart2 = TextMove(1,&screenSize);
//    textPart = TextMove(&screenSize, 1024);
    textPart.setup();
    textPart2.setup();
    
    pos = &textPart.pos;
    vec = &textPart.vec;
    rot = &textPart.rot;
    
    bRecord = false;
    bGrid = false;
    bSphere = false;
    bScreenParam = false;
    bBirth = false;
    
//    textPart.iniRandom(2000);
    
    camera.setup();
//	camera.autosavePosition = true;
	camera.loadCameraPosition();
    
    recTimeCnt = 0;
    moveTimeCnt = 0;
    curTimeCnt = &moveTimeCnt;
    
    int fboWidth = 1920;
    int fboHeight = 1350;
    
    fboScr.allocate(fboWidth, fboHeight, GL_RGB);//A, 4);
    fboScr.begin();
    ofClear(0,0,0,0);
    fboScr.end();
    center = ofPoint((ofGetWidth()-fboScr.getWidth())/2, 0);
    
    buildMesh();
    cornerCnt=0;
//    ofDisableArbTex();
//    univers.sphereSetup("images/milkyWay.jpg");
    
}

//--------------------------------------------------------------
void testApp::update(){
    
    // hide old messages
    for(int i = 0; i < NUM_MSG_STRINGS; i++){
        if(timers[i] < ofGetElapsedTimef()){
            msg_strings[i] = "";
        }
    }
    if(bSetVertex)setScreenMeshVertex();
    vector<int> updtid;
    textPart.resetAttr();
    textPart2.resetAttr();
    updtid.clear();
    float coodScale = 255.0;
    float yscl = 1.68;
    float yOffSet =217;
    float zh = 100;
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        
        // check for mouse moved message
        if(m.getAddress() == "/TSPS/personEntered/"){
            int pid = m.getArgAsInt32(0);
            float x = m.getArgAsFloat(3);
            float y = m.getArgAsFloat(4);
            textPart.setAttr(pid,ofVec3f(coodScale*x,zh, yOffSet-coodScale*y*yscl));
            textPart2.setAttr(pid,ofVec3f(coodScale*x,zh, yOffSet-coodScale*y*yscl));
//            cout << "p enter++++++ " << m.getArgAsInt32(0)<< endl;
        }else
        if(m.getAddress() == "/TSPS/personUpdated/"){
            int pid = m.getArgAsInt32(0);
            float x = m.getArgAsFloat(3);
            float y = m.getArgAsFloat(4);
            if(updtid.empty()){
                updtid.push_back(pid);
                textPart.setAttr(pid,ofVec3f(coodScale*x,zh, yOffSet-coodScale*y*yscl));
                textPart2.setAttr(pid,ofVec3f(coodScale*x,zh, yOffSet-coodScale*y*yscl));
            }else{
                for(int i=0;i<updtid.size();i++){
                    for(int j=i+1;j<updtid.size();j++){
                        if(i!=j && pid!=updtid[j]){
                            textPart.setAttr(pid,ofVec3f(coodScale*x, zh, yOffSet-coodScale*y*yscl));
                            textPart2.setAttr(pid,ofVec3f(coodScale*x, zh, yOffSet-coodScale*y*yscl));
                            cout << "recog some" << endl;
                        }
                    }
                }
            }
//            textPart2.updateAttr(pid,ofVec3f(200*x,200*y,70));
//            textPart2.updateAttr(m.getArgAsInt32(0),ofVec3f(200*m.getArgAsFloat(3),200*m.getArgAsFloat(4),70));
//            cout << "mssg pid" << m.getArgAsInt32(0) << endl;
//            cout << "p updat " <<  m.getArgAsInt32(0) << " " << m.getArgAsFloat(3) << "' " << m.getArgAsFloat(4)  << endl;
        }else
        if(m.getAddress() == "/TSPS/personWillLeave/"){
//            textPart.deleteAttr(m.getArgAsInt32(0));
//            textPart2.deleteAttr(m.getArgAsInt32(0));
//            cout << "p leave------ " << m.getArgAsInt32(0) << endl;
        }
        else{
            // unrecognized message: display on the bottom of the screen
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
            // add to the list of strings to display
            msg_strings[current_msg_string] = msg_string;
            timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
            current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
            // clear the next line
            msg_strings[current_msg_string] = "";
        }
        
    }
    if(bBirth){
        textPart.birth();
        textPart2.birth();
    }
    textPart.update();
    textPart2.update();
}


//--------------------------------------------------------------
void testApp::draw(){
    ofBackground(0);
    
    ofColor strt = ofColor(pow(double(ofNoise(ofGetElapsedTimef()/32)),3.0)*140, pow(double(ofNoise(ofGetElapsedTimef()/26)),3.0)*140, pow( double(ofNoise(ofGetElapsedTimef()/42)),3.0)*140);
    ofColor end = ofColor(0,0,0);
    
    pub.begin();

//    fboScr.begin();
    ofClear(0);
    ofEnableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
//    ofBackgroundGradient(strt, end);
    camera.begin();
    ofPushMatrix();
    ofVec3f currentPos = *pos + ( *vec * (float)*curTimeCnt);
    ofTranslate(currentPos);
    ofRotateX(rot->x* (float)*curTimeCnt);
    ofRotateY(rot->y* (float)*curTimeCnt);
    ofRotateZ(rot->z* (float)*curTimeCnt);
    ofPushStyle();
    if(bDrawConnection)textPart.drawConnection(1,ofColor(ofMap(strt.r,0,140,64,180),ofMap(strt.g,0,140,64,180),ofMap(strt.r,0,140,64,180)) );

    textPart.draw(ofColor(ofMap(strt.r,0,140,180,256),ofMap(strt.g,0,140,180,256),ofMap(strt.r,0,140,180,256)) );
    textPart2.draw(ofColor(ofMap(strt.r,0,140,180,256),ofMap(strt.g,0,140,180,256),ofMap(strt.r,0,140,180,256)) );
//    univers.drawSphere(0, 0, 2000);
    ofPopStyle();
    if(bGrid)drawGrid();
    ofPopMatrix();
    
    camera.end();
//    fboScr.end();
    
    pub.end();
    
    if(bRecord && recTimeCnt<1600){
        char filename[512];
        sprintf(filename, "%s_%05d.png", textPart.objName.c_str(), recTimeCnt);
        fboScr.getTextureReference().readToPixels(savingImage.getPixelsRef());
        savingImage.mirror(true, false);
        savingImage.saveImage(filename);
        recTimeCnt++;
    }
    
    /*
    /////////////////////FBO
    ofSetColor(255);
    ofPushMatrix();
    ofTranslate(center);
    scr->setTextureWrap(GL_REPEAT, GL_REPEAT);
    scr->bind();
//    // draw fbo
//    //    brcosaShader.begin();
//    //    brcosaShader.setUniformTexture("tex0", *scr, 0);
//    //    brcosaShader.setUniform1f("contrast", 1.0);
//    //    brcosaShader.setUniform1f("brightness", 1.0);
//    //    brcosaShader.setUniform1f("saturation", 5*sin(ofGetElapsedTimef()));
//    //    brcosaShader.setUniform1f("alpha", 1.0);
//    
//    chromaShader.begin();
//    chromaShader.setUniformTexture("tex0", *scr, 0);
//    chromaShader.setUniform2f("windowSize", ofGetHeight(), ofGetHeight());
//    float line = ofRandom(0,ofGetWidth());
//    chromaShader.setUniform2f("roiSize", ofRandom(-2,0)+line, ofRandom(0,2)+line );
//    
//    chromeNoise = ofNoise( (float)ofGetElapsedTimeMillis()/100.0);
//    changeBG();
//    if(chromeNoise>0.65){
//        //        chromeMove = ofRandom(-30,30);
//        chromaShader.setUniform1f("offsetALL", chromeMove);//10*sin(ofGetElapsedTimef()));
//    }else{
//        chromaShader.setUniform1f("offsetALL", 0);
//    }
//    if(!bInvertBG){
//        //    if(!bBlack){
//        chromaShader.setUniform1i("bInvert", 0);
//    }else{
//        chromaShader.setUniform1i("bInvert", 1);
//    }
    scrVboMesh.draw();
    //    brcosaShader.end();
//    chromaShader.end();
    scr->unbind();
    ofPopMatrix();
    
    if(bSetVertex)setScreenMeshVertex();
    */
//    fboScr.getTextureReference().draw(ofRectangle(0,800,fboScr.getWidth(),-fboScr.getHeight()));
    pub.draw();
    
    if(bScreenParam){
        ofPushMatrix();
        ofSetColor(255);
        ofDrawBitmapString("FrameNumber:: " + ofToString(*curTimeCnt) + " FPS::" + ofToString(ofGetFrameRate()), 20,20);
        if(textPart.bMove){
            ofDrawBitmapString("'m' moving: Move", 20,40);
        }else{
            ofDrawBitmapString("'m' moving: Stop", 20,40);
        }
        ofDrawBitmapString("'space' saveframe: Disable", 20,55);
        if(bGrid){
            ofDrawBitmapString("'g' grid: ON", 20,70);
        }else{
            ofDrawBitmapString("'g' grid: OFF", 20,70);
        }
        if(bSphere){
            ofDrawBitmapString("'p' attr Sphere: ON", 20,85);
        }else{
            ofDrawBitmapString("'p' attr Sphere: OFF", 20,85);
        }
        if(bDrawConnection){
            ofDrawBitmapString("'l' Draw Connection: ON", 20,100);
        }else{
            ofDrawBitmapString("'l' Draw Connection: OFF", 20,100);
        }
        if(textPart.bModeAttr){
            ofDrawBitmapString("'/' Mode Attr: ON", 20,115);
        }else{
            ofDrawBitmapString("'/' Mode Attr: OFF", 20,115);
        }
        ofDrawBitmapString("'q/r' Roll Camera\n'w/s' go/bacl\n'a/d' slide L/R\n'e/c' slide UP/DN\n's + shift' save camera position\n'r + shift' reset Camera position", 20,125);
        ofPopMatrix();
        if(textPart.bMove)moveTimeCnt++;
        
        ofPushMatrix();
        ofTranslate(0, 200);
        for(int i = 0; i < NUM_MSG_STRINGS; i++){
            ofDrawBitmapString(msg_strings[i], 10, 40 + 15 * i);
        }
        ofPopMatrix();
    }
//    mainOutputSyphonServer.publishScreen();

}

//--------------------------------------------------------------
void testApp::drawGrid(){
    ofSetColor(128,64,0,199);
    int set = 10;
    for(int k=-set;k<set;k++){
        for(int j=-set;j<set;j++){
            for(int i=-set;i<set;i++){
                ofPushMatrix();
                ofTranslate(100*i,100*j, 100*k);
                ofLine(-2,0,0,10,0,0);
                ofLine(0,-2,0,0,10,0);
                ofLine(0,0,-2,0,0,10);
                ofPopMatrix();
            }
        }
    }
}

//--------------------------------------------------------------
void testApp::birth(){
    textPart.birth();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key==' '){
        bRecord = !bRecord;
        textPart.bMove = !textPart.bMove;
    }
    if(key=='l') bDrawConnection = !bDrawConnection;
    if(key=='m') textPart.bMove = !textPart.bMove;
    if(key=='v') textPart.bVideo = !textPart.bVideo;
    if(key=='/'){
        textPart.bModeAttr = !textPart.bModeAttr;
        textPart2.bModeAttr = !textPart2.bModeAttr;
    }
    if(key=='R') camera.reset();
    if(key=='o') textPart.setAttrPos(ofVec3f(0,0,0));
    if(key=='g') bGrid = !bGrid;
    if(key=='i') bScreenParam = !bScreenParam;
    if(key=='-'){
        bSetVertex = !bSetVertex;
        cornerCnt = 4;
    }
    if(key=='p'){
        bSphere = !bSphere;
        textPart.bSphere = !textPart.bSphere;
    }
    if(key=='S') {
        cout << "save pram" << endl;
        camera.saveCameraPosition();
        textPart.saveAllParam();
    }
    if(key=='C'){ textPart.clear(); textPart2.clear(); }
    if(key=='b')bBirth = !bBirth;
    if(key=='|')ofToggleFullscreen();
    if(key=='0')textPart.iniRandom(1600);
    if(bRecord){curTimeCnt = &recTimeCnt;}else{curTimeCnt = &moveTimeCnt;}
    
    if(key=='!'){ textPart.setTargetPos(0); textPart2.setTargetPos(0); }
    if(key=='@'){ textPart.setTargetPos(1); textPart2.setTargetPos(1); }
    if(key=='#'){ textPart.setTargetPos(2); textPart2.setTargetPos(2); }
    if(key=='$'){ textPart.setTargetPos(3); textPart2.setTargetPos(3); }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
    textPart.setK(pow((double)x/(double)ofGetWidth(),10.0));
    textPart2.setK(pow((double)x/(double)ofGetWidth(),10.0));
    msX = x;
    msY = y;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    isMouseDragged = true;
    msX = x;
    msY = y;

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    isMouseDragged = false;

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

//------------------------------------------------------------------
bool testApp::checkAreaRect(float _x, float _y, ofPoint _centerPoint, ofVec2f _size){
    float xmin, xmax, ymin, ymax;
    xmin = _centerPoint.x - _size.x/2;
    xmax = _centerPoint.x + _size.x/2;
    ymin = _centerPoint.y - _size.y/2;
    ymax = _centerPoint.y + _size.y/2;
    if( xmin < _x && xmax > _x && ymin < _y && ymax > _y ){
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofRect(_centerPoint, _size.x, _size.y);
        ofSetRectMode(OF_RECTMODE_CORNER);
        return true;
    }else{
        return false;
    }
}

void testApp::setScreenMeshVertex(){
    ofNoFill();
    ofSetColor(255);
//    cout << "msX" << msX << endl;
//    cout << "msY" << msY << endl;
    ofPushMatrix();
    ofTranslate(center);
    for(int i=0;i<scrVboMesh.getNumVertices();i++){
        ofPoint vi = scrVboMesh.getVertex(i);
        ofCircle(vi, 10);
        if(isMouseDragged && checkAreaRect(msX-center.x, msY-center.y, vi, ofVec2f(50,50) ) ){
//            cout << "moved edge" << endl;
            scrVboMesh.setVertex(i, ofPoint(msX-center.x, msY-center.y, 0.0));
        }
    }
    scrVboMesh.disableTextures();
    scrVboMesh.drawWireframe();
    scrVboMesh.enableTextures();
    ofPopMatrix();
}

//--
void testApp::exit(){
    textPart.exit();
}