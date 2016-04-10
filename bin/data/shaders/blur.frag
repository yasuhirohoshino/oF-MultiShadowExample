// Blur shader based on: http://callumhay.blogspot.ca/2010/09/gaussian-blur-shader-glsl.html

#version 330

uniform float sigma;
uniform sampler2D blurSampler;
uniform vec2 resolution;
uniform int horizontal;

const float pi = 3.14159265;
const int numBlurPixelsPerSide = 3;

in vec2 texCoordVarying;

out vec4 fragColor;

void main() {
    
    vec2 blurMultiplyVec = vec2(0.0);
    if(horizontal == 1){
        blurMultiplyVec = vec2(1.0, 0.0) / resolution;
    }else{
        blurMultiplyVec = vec2(0.0, 1.0) / resolution;
    }
    
    vec3 incrementalGaussian;
    incrementalGaussian.x = 1.0 / (sqrt(2.0 * pi) * sigma);
    incrementalGaussian.y = exp(-0.5 / (sigma * sigma));
    incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;
    
    vec4 avgValue = vec4(0.0, 0.0, 0.0, 0.0);
    float coefficientSum = 0.0;
    
    avgValue += texture(blurSampler, texCoordVarying) * incrementalGaussian.x;
    coefficientSum += incrementalGaussian.x;
    incrementalGaussian.xy *= incrementalGaussian.yz;
    
    for (float i = 1.0; i <= numBlurPixelsPerSide; i++) {
        avgValue += texture(blurSampler, texCoordVarying - i * blurMultiplyVec) * incrementalGaussian.x;
        avgValue += texture(blurSampler, texCoordVarying + i * blurMultiplyVec) * incrementalGaussian.x;
        coefficientSum += 2.0 * incrementalGaussian.x;
        incrementalGaussian.xy *= incrementalGaussian.yz;
    }
    
    avgValue /= coefficientSum;
    
    fragColor = avgValue;
}