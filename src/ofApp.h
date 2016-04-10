#pragma once

#define MAX_LIGHTS 8

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "customLight.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void renderScene();
        void resizeShadowMap(int & numLights);
        void setShadowMap(int numLights);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
    ofxAssimpModelLoader model;
    ofVec3f modelScale;
    ofEasyCam cam;
    
    customLight pbrLight[MAX_LIGHTS];

    ofShader renderingShader, blurShader;
    ofMatrix4x4 inverseCameraMatrix;
    int depthMapRes;
    int depthMapAtrasWidth, depthMapAtrasHeight;
    ofVec2f depthTexMag;
    ofFbo depthMap;
    ofFbo depthSumFbo, blurVFbo, blurHFbo;
    ofFbo::Settings settings;
    ofMatrix4x4 shadowMatrix[MAX_LIGHTS];
    const ofMatrix4x4 biasMatrix = ofMatrix4x4(
                                               0.5, 0.0, 0.0, 0.0,
                                               0.0, 0.5, 0.0, 0.0,
                                               0.0, 0.0, 0.5, 0.0,
                                               0.5, 0.5, 0.5, 1.0
                                               );
    // gui
    ofxPanel gui;
    ofParameter<float> roughness;
    ofParameter<ofColor> baseColor;
    ofParameter<int> numLights;
    
    ofParameter<bool> isEnabled[MAX_LIGHTS];
    ofParameter<int> lightType[MAX_LIGHTS], shadowType[MAX_LIGHTS];
    ofParameter<ofFloatColor> color[MAX_LIGHTS];
    ofParameter<float> cutoff[MAX_LIGHTS];
    ofParameter<float> exponent[MAX_LIGHTS];
    ofParameter<float> intensity[MAX_LIGHTS];
    ofParameter<float> radius[MAX_LIGHTS];
    ofParameter<float> nearClip[MAX_LIGHTS];
    ofParameter<float> farClip[MAX_LIGHTS];
    ofxPanel lightGui[MAX_LIGHTS];
    ofParameter<string> lightTypeName[MAX_LIGHTS], shadowTypeName[MAX_LIGHTS];
    
    int guiWidth;
    bool isGuiShown;
    
    const string lightTypeNames[3] = {"Directional Light", "Spot Light", "Point Light"};
    const string shadowTypeNames[3] = {"No Shadow", "hard Shadow", "Soft Shadow"};
};
