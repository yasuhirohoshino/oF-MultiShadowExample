#include "customLight.h"

customLight::customLight(){
    setupPerspective();
    setForceAspectRatio(1.0);
    setFov(90);
}

customLight::~customLight(){}

void customLight::enable(bool isEnabled){
    this->isEnabled = isEnabled;
}

void customLight::disable(){
    this->isEnabled = false;
}

bool customLight::getIsEnabled(){
    return isEnabled;
}

void customLight::setPosition(ofVec3f pos){
    setGlobalPosition(pos);
}

ofVec3f customLight::getPosition(){
    return getGlobalPosition();
}

// depth camera
void customLight::setNearClip(float near){
    ofCamera::setNearClip(near);
}

float customLight::getNearClip(){
    ofCamera::getNearClip();
}

void customLight::setFarClip(float far){
    ofCamera::setFarClip(far);
}

float customLight::getFarClip(){
    return ofCamera::getFarClip();
}

void customLight::lookAt(ofVec3f target){
    return ofCamera::lookAt(target);
}

ofVec3f customLight::getLookAtDir(){
    return ofCamera::getLookAtDir();
}

void customLight::setScale(float scale){
    ofCamera::setScale(scale);
}

void customLight::setDepthMapRes(float resolution){
    depthMapRes = resolution;
}

void customLight::beginDepthCamera(){
    begin();
}

void customLight::endDepthCamera(){
    end();
}

// for rendering shader
ofVec3f customLight::getViewSpacePosition(ofMatrix4x4 viewMatrix){
    if(lightType == LightType_Directional){
        ofVec4f pos = ofVec4f(getGlobalPosition().x, getGlobalPosition().y, getGlobalPosition().z, 0.0);
        return pos * viewMatrix;
    }else{
        ofVec4f pos = ofVec4f(getGlobalPosition().x, getGlobalPosition().y, getGlobalPosition().z, 1.0);
        return pos * viewMatrix;
    }
}

ofMatrix4x4 customLight::getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix){
    ofMatrix4x4 viewMatrix, projectionMatrix;
    viewMatrix = getModelViewMatrix();
    if(lightType == LightType_Directional){
        projectionMatrix.makeOrthoMatrix(-depthMapRes * 0.5, depthMapRes * 0.5, -depthMapRes * 0.5, depthMapRes * 0.5, ofCamera::getNearClip(), ofCamera::getFarClip());
    }else{
        projectionMatrix = getProjectionMatrix();
    }
    return cameraModelViewMatrix.getInverse() * viewMatrix * projectionMatrix * biasMatrix;
}

ofVec3f customLight::getViewSpaceDirection(ofMatrix4x4 viewMatrix){
    ofVec4f dir = ofVec4f(getLookAtDir().x, getLookAtDir().y, getLookAtDir().z, 0.0);
    return ofVec3f(dir * viewMatrix).getNormalized();
}

// color
void customLight::setColor(ofFloatColor color){
    this->color = color;
}

void customLight::setColor(ofColor color){
    this->color.r = color.r / 255.0;
    this->color.g = color.g / 255.0;
    this->color.b = color.b / 255.0;
    this->color.a = color.a / 255.0;
}

ofFloatColor customLight::getColor(){
    return color;
}

// light
void customLight::setLightType(LightType lightType){
    this->lightType = lightType;
    if(lightType == LightType_Directional){
        enableOrtho();
    }else{
        disableOrtho();
    }
}

LightType customLight::getLightType(){
    return lightType;
}

void customLight::setIntensity(float intensity){
    this->intensity = intensity;
}

float customLight::getIntensity(){
    return intensity;
}

// spotlight & pointlight
void customLight::setRadius(float radius){
    this->radius = radius;
}

float customLight::getRadius(){
    return radius;
}

// spotLight
void customLight::setCutoff(float cutoff){
    this->cutoff = cutoff;
}

float customLight::getCutoff(){
    return cutoff;
}

void customLight::setExponent(float exponent){
    this->exponent = exponent;
}

float customLight::getExponent(){
    return exponent;
}

// shadow
void customLight::setShadowType(ShadowType shadowType){
    this->shadowType = shadowType;
}

ShadowType customLight::getShadowType(){
    return shadowType;
}

void customLight::setShadowBias(float shadowBias){
    this->shadowBias = shadowBias;
}

float customLight::getShadowBias(){
    return shadowBias;
}

void customLight::setSoftShadowExponent(float softShadowExponent){
    this->softShadowExponent = softShadowExponent;
}

float customLight::getSoftShadowExponent(){
    return softShadowExponent;
}