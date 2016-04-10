#version 330

#define MAX_LIGHTS 8

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

in vec4  position;
in vec3  normal;
in vec2  texcoord;

out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;

out vec4 colorVarying;
out vec3 v_normalVarying;
out vec4 v_positionVarying;

uniform mat4 viewMatrix;
uniform int numLights;
uniform mat4 shadowMatrix[MAX_LIGHTS];
out vec4 v_shadowCoord[MAX_LIGHTS];

void main() {
    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));
    
    normalVarying = normal;
    positionVarying = position;
    texCoordVarying = texcoord;
    colorVarying = globalColor;
    
    v_normalVarying = normalize(vec3(normalMatrix * normal));
    v_positionVarying = modelViewMatrix * position;
    
    for(int i=0; i<numLights; i++){
        v_shadowCoord[i] = shadowMatrix[i] * v_positionVarying;
    }
    
    gl_Position = modelViewProjectionMatrix * position;
    
}