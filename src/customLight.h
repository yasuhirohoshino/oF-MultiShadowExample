#pragma once

#include "ofMain.h"

enum LightType{
    LightType_Directional = 0,
    LightType_Spot = 1,
    LightType_Point = 2,
    NumLightTypes = 3
};

enum ShadowType{
    ShadowType_None = 0,
    ShadowType_Hard = 1,
    ShadowType_Soft = 2,
    NumShadowTypes = 3
};

class customLight: protected ofCamera{
public:
    customLight();
    ~customLight();
    
    void enable(bool isEnabled = true);
    void disable();
    bool getIsEnabled();
    void setPosition(ofVec3f pos);
    ofVec3f getPosition();
    
    // depth camera
    void setNearClip(float near);
    float getNearClip();
    void setFarClip(float far);
    float getFarClip();
    void lookAt(ofVec3f target);
    ofVec3f getLookAtDir();
    void setScale(float scale);
    void setDepthMapRes(float resolution);
    void beginDepthCamera();
    void endDepthCamera();
    
    // for rendering shader
    ofVec3f getViewSpacePosition(ofMatrix4x4 viewMatrix);
    ofMatrix4x4 getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix);
    ofVec3f getViewSpaceDirection(ofMatrix4x4 viewMatrix);
    
    // color
    void setColor(ofFloatColor color);
    void setColor(ofColor color);
    ofFloatColor getColor();
    
    // light
    void setLightType(LightType lightType);
    LightType getLightType();
    void setIntensity(float intensity);
    float getIntensity();
    
    // spotlight & pointlight
    void setRadius(float radius);
    float getRadius();
    
    // spotLight
    void setCutoff(float cutoff);
    float getCutoff();
    void setExponent(float exponent);
    float getExponent();
    
    // shadow
    void setShadowType(ShadowType shadowType);
    ShadowType getShadowType();
    void setShadowBias(float shadowBias);
    float getShadowBias();
    void setSoftShadowExponent(float softShadowExponent);
    float getSoftShadowExponent();
    
private:
    ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
    LightType lightType = LightType_Directional;
    ShadowType shadowType = ShadowType_Soft;
    float exponent = 1;
    float cutoff = 45;
    float radius = 1000;
    float depthMapRes = 1024;
    float shadowBias = 0.001;
    float softShadowExponent = 75.0;
    float intensity = 1.0;
    bool isEnabled = true;
    ofMatrix4x4 shadowTransMatrix;
    
    const ofMatrix4x4 biasMatrix = ofMatrix4x4(
                                               0.5, 0.0, 0.0, 0.0,
                                               0.0, 0.5, 0.0, 0.0,
                                               0.0, 0.0, 0.5, 0.0,
                                               0.5, 0.5, 0.5, 1.0
                                               );
};