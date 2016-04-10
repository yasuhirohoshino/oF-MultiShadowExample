#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    model.loadModel("dragon.obj");
    modelScale = model.getModelMatrix().getScale();
    
    renderingShader.load("shaders/render");
    blurShader.load("shaders/blur");
    
    isGuiShown = true;
    guiWidth = int(ofGetWidth() / 9);
    gui.setDefaultWidth(guiWidth);
    gui.setup("gui");
    gui.setPosition(0, 0);
    gui.add(roughness.set("roughness", 0.05, 0.0, 1.0));
    gui.add(baseColor.set("baseColor", ofColor(255, 255, 255, 255), ofColor(0, 0, 0, 0), ofColor(255, 255, 255, 255)));
    gui.add(numLights.set("numLights", 3, 0, MAX_LIGHTS));
    numLights.addListener(this, &ofApp::resizeShadowMap);
    gui.add(shadowOffset.set("shadowOffset", ofFloatColor(0.05, 0.05, 0.05, 1.0), ofFloatColor(0.0, 0.0, 0.0, 1.0), ofFloatColor(1.0, 1.0, 1.0, 1.0)));
    
    depthMapRes = 1024;
    setShadowMap(numLights);
    
    for(int i=0; i<MAX_LIGHTS; i++){
        pbrLight[i].setDepthMapRes(depthMapRes);
        pbrLight[i].setSoftShadowExponent(75.0);
        pbrLight[i].setShadowBias(0.001);
        pbrLight[i].setScale(2.0);
        
        ofFloatColor c = ofFloatColor(ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), ofRandom(0.0, 1.0), 1.0);
        lightGui[i].setup("light " + ofToString(i));
        lightGui[i].setPosition(guiWidth + i * guiWidth, 0);
        lightGui[i].add(isEnabled[i].set("enable", true));
        lightGui[i].add(lightType[i].set("lightType", LightType_Spot, LightType_Directional, LightType_Point));
        lightGui[i].add(lightTypeName[i].set(lightTypeNames[1]));
        lightGui[i].add(shadowType[i].set("shadowType", ShadowType_Soft, ShadowType_None, ShadowType_Soft));
        lightGui[i].add(shadowTypeName[i].set(shadowTypeNames[2]));
        lightGui[i].add(color[i].set("color", c, ofFloatColor(0.0, 0.0, 0.0, 1.0), ofFloatColor(1.0, 1.0, 1.0, 1.0)));
        lightGui[i].add(cutoff[i].set("cutoff", 60, 10, 90));
        lightGui[i].add(exponent[i].set("exponent", 32, 0, 128));
        lightGui[i].add(intensity[i].set("intensity", 1.0, 0.0, 5.0));
        lightGui[i].add(radius[i].set("radius", 3000.0, 100.0, 6000.0));
        lightGui[i].add(nearClip[i].set("nearClip", 500.0, 10.0, 6000.0));
        lightGui[i].add(farClip[i].set("farClip", 4000, 10.0, 6000.0));
    }
}

//--------------------------------------------------------------
void ofApp::resizeShadowMap(int & numLights){
    setShadowMap(numLights);
}

//--------------------------------------------------------------
void ofApp::setShadowMap(int numLights){
    depthMapAtrasWidth = depthMapRes * min(numLights, 4);
    depthMapAtrasHeight = depthMapRes * (floor(numLights / 5) + 1);
    
    depthTexMag.x = float(depthMapRes) / float(depthMapAtrasWidth);
    depthTexMag.y = float(depthMapRes) / float(depthMapAtrasHeight);
    
    settings.width  = depthMapRes;
    settings.height = depthMapRes;
    settings.textureTarget = GL_TEXTURE_2D;
    settings.internalformat = GL_R8;
    settings.useDepth = true;
    settings.depthStencilAsTexture = true;
    settings.useStencil = true;
    settings.minFilter = GL_LINEAR;
    settings.maxFilter = GL_LINEAR;
    settings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
    settings.wrapModeVertical = GL_CLAMP_TO_EDGE;
    
    depthMap.allocate(settings);
    
    settings.internalformat = GL_R32F;
    blurHFbo.allocate(settings);
    blurVFbo.allocate(settings);
    
    settings.width  = depthMapAtrasWidth;
    settings.height = depthMapAtrasHeight;
    
    depthSumFbo.allocate(settings);
    depthSumFbo.begin();
    ofClear(255, 0, 0);
    depthSumFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    for(int i=0; i<numLights; i++){
        pbrLight[i].enable(isEnabled[i]);
        pbrLight[i].setLightType(static_cast<LightType>(lightType[i].get()));
        pbrLight[i].setShadowType(static_cast<ShadowType>(shadowType[i].get()));
        pbrLight[i].setColor(color[i]);
        pbrLight[i].setCutoff(cutoff[i]);
        pbrLight[i].setExponent(exponent[i]);
        pbrLight[i].setIntensity(intensity[i]);
        pbrLight[i].setRadius(radius[i]);
        pbrLight[i].setNearClip(nearClip[i]);
        pbrLight[i].setFarClip(farClip[i]);
        
        float rad = ofGetElapsedTimef() + TWO_PI * i / fmaxf(numLights, 1);
        pbrLight[i].setPosition(ofVec3f(1500.0 * cos(rad), 1500.0, 1500.0 * sin(rad)));
        pbrLight[i].lookAt(ofVec3f(0, 0, 0));
        
        lightTypeName[i].set(lightTypeNames[lightType[i].get()]);
        shadowTypeName[i].set(shadowTypeNames[shadowType[i].get()]);
    }
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    for(int i=0; i<numLights; i++){
        shadowMatrix[i] = pbrLight[i].getShadowMatrix(cam.getModelViewMatrix());
        
        if(pbrLight[i].getShadowType() != ShadowType_None && pbrLight[i].getIsEnabled()){
            depthMap.begin();
            ofClear(255, 0, 0, 255);
            pbrLight[i].beginDepthCamera();
            renderScene();
            pbrLight[i].endDepthCamera();
            depthMap.end();
            
            if(pbrLight[i].getShadowType() == ShadowType_Soft){
                blurVFbo.begin();
                ofClear(255, 0, 0, 255);
                blurShader.begin();
                blurShader.setUniform2f("resolution", blurHFbo.getWidth(), blurHFbo.getHeight());
                blurShader.setUniform1f("sigma", 2.0);
                blurShader.setUniformTexture("blurSampler", depthMap.getDepthTexture(), 1);
                blurShader.setUniform1i("horizontal", 0);
                depthMap.getDepthTexture().draw(0, 0);
                blurShader.end();
                blurVFbo.end();
                
                blurHFbo.begin();
                ofClear(255, 0, 0, 255);
                blurShader.begin();
                blurShader.setUniform2f("resolution", blurHFbo.getWidth(), blurHFbo.getHeight());
                blurShader.setUniform1f("sigma", 2.0);
                blurShader.setUniformTexture("blurSampler", blurVFbo.getTexture(), 1);
                blurShader.setUniform1i("horizontal", 1);
                blurVFbo.draw(0, 0);
                blurShader.end();
                blurHFbo.end();
            }
            
            depthSumFbo.begin();
            if(pbrLight[i].getShadowType() == ShadowType_Hard){
                depthMap.getDepthTexture().draw((i % 4) * depthMapRes, floor(i / 4) * depthMapRes);
            }else if(pbrLight[i].getShadowType() == ShadowType_Soft){
                blurHFbo.draw((i % 4) * depthMapRes, floor(i / 4) * depthMapRes);
            }
            depthSumFbo.end();
        }
    }
    
    ofEnableAlphaBlending();
    cam.begin();
    renderingShader.begin();
    renderingShader.setUniform1i("numLights", numLights);
    renderingShader.setUniformMatrix4f("viewMatrix", ofGetCurrentViewMatrix());
    renderingShader.setUniformTexture("shadowMap", depthSumFbo.getTexture(), 1);
    renderingShader.setUniform2f("depthMapAtrasRes", depthMapAtrasWidth, depthMapAtrasHeight);
    renderingShader.setUniform2f("depthTexMag", depthTexMag);
    renderingShader.setUniform1f("roughness", roughness);
    renderingShader.setUniform4f("shadowOffset", shadowOffset);
    for(int i=0; i<numLights; i++){
        string index = ofToString(i);
        renderingShader.setUniform1i("lights["+ index +"].isEnabled", pbrLight[i].getIsEnabled());
        renderingShader.setUniform3f("lights["+ index +"].position", pbrLight[i].getViewSpacePosition(ofGetCurrentViewMatrix()));
        renderingShader.setUniform4f("lights["+ index +"].color", pbrLight[i].getColor());
        renderingShader.setUniform3f("lights["+ index +"].direction", pbrLight[i].getViewSpaceDirection(ofGetCurrentViewMatrix()));
        renderingShader.setUniform1i("lights["+ index +"].type", pbrLight[i].getLightType());
        renderingShader.setUniform1i("lights["+ index +"].shadowType", pbrLight[i].getShadowType());
        renderingShader.setUniform1f("lights["+ index +"].intensity", pbrLight[i].getIntensity());
        renderingShader.setUniform1f("lights["+ index +"].exponent", pbrLight[i].getExponent());
        renderingShader.setUniform1f("lights["+ index +"].cutoff", pbrLight[i].getCutoff());
        renderingShader.setUniform1f("lights["+ index +"].radius", pbrLight[i].getRadius());
        renderingShader.setUniform1f("lights["+ index +"].softShadowExponent", pbrLight[i].getSoftShadowExponent());
        renderingShader.setUniform1f("lights["+ index +"].bias", pbrLight[i].getShadowBias());
    }
    glUniformMatrix4fv(renderingShader.getUniformLocation("shadowMatrix"), numLights, false, shadowMatrix[0].getPtr());
    renderScene();
    renderingShader.end();
    cam.end();
    
    ofDisableDepthTest();
    
    if(isGuiShown){
        gui.draw();
        for(int i=0;i<numLights;i++){
            lightGui[i].draw();
        }
    }
}

//--------------------------------------------------------------
void ofApp::renderScene(){
    ofEnableDepthTest();
    ofPushStyle();
    ofSetColor(baseColor);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    for(int i=0; i<model.getNumMeshes(); i++){
        ofPushMatrix();
        ofMultMatrix(model.getMeshHelper(i).matrix);
        ofScale(modelScale.x, modelScale.y, modelScale.z);
        model.getCurrentAnimatedMesh(i).draw();
        ofPopMatrix();
    }
    
    ofDrawBox(0, -5, 0, 10000, 10, 10000);
    glDisable(GL_CULL_FACE);
    ofPopStyle();
    ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key == 'g'){
        isGuiShown = !isGuiShown;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
