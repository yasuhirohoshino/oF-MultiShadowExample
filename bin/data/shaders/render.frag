#version 330

#define MAX_LIGHTS 8

uniform mat4 viewMatrix;
uniform sampler2D shadowMap;

struct Light{
    bool isEnabled;
    vec3 position;
    vec4 color;
    vec3 direction;
    int type;
    int shadowType;
    float intensity;
    float exponent;
    float cutoff;
    float radius;
    float softShadowExponent;
    float bias;
};

uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform vec2 depthMapAtrasRes;
uniform vec2 depthTexMag;
uniform float roughness;

in vec4 colorVarying;
in vec2 texCoordVarying;
in vec3 normalVarying;
in vec4 positionVarying;

in vec3 v_normalVarying;
in vec4 v_positionVarying;

in vec3 v_lightPos;
in vec3 v_lightDir;
in vec4 v_shadowCoord[MAX_LIGHTS];

out vec4 fragColor;

float BechmannDistribution(float d, float m) {
    float d2 = d * d;
    float m2 = m * m;
    return exp((d2 - 1.0) / (d2 * m2)) / (m2 * d2 * d2);
}

float CookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness){
    vec3 l = normalize(lightDirection);
    vec3 n = normalize(surfaceNormal);
    
    vec3 v = normalize(viewDirection);
    vec3 h = normalize(l + v);
    
    float hn = dot(h, n);
    float ln = dot(l, n);
    float lh = dot(l, h);
    float vn = dot(v, n);
    
    float f = 0.02 + pow(1.0 - dot(v, h), 5.0) * (1.0 - 0.02);
    float d = BechmannDistribution(hn, roughness);
    float t = 2.0 * hn / dot(v, h);
    float g = min(1.0, min(t * vn, t * ln));
    float m = 3.14159265 * vn * ln;
    float spec = max(f * d * g / m, 0.0);
    return spec;
}

float Falloff(float dist, float lightRadius) {
    float att = clamp(1.0 - dist * dist / (lightRadius * lightRadius), 0.0, 1.0);
    att *= att;
    return att;
}

vec3 CalcPointLight(vec4 v_positionVarying, vec3 v_normalVarying, vec3 color, Light light) {
    vec3 s = normalize(light.position - v_positionVarying.xyz);
    float lambert = max(dot(s, v_normalVarying), 0.0);
    float falloff = Falloff(length(light.position - v_positionVarying.xyz), light.radius);
    float specular = CookTorranceSpecular(s, -normalize(v_positionVarying.xyz), v_normalVarying, roughness);
    return light.intensity * (color.rgb * light.color.rgb * lambert * falloff + color.rgb * light.color.rgb * specular * falloff);
}

vec3 CalcSpotLight(vec4 v_positionVarying, vec3 v_normalVarying, vec3 color, Light light) {
    vec3 s = normalize(light.position - v_positionVarying.xyz);
    float angle = acos(dot(-s, light.direction));
    float cutoff = radians(clamp(light.cutoff, 0.0, 90.0));
    if(angle < cutoff){
        float spotFactor = pow(dot(-s, light.direction), light.exponent);
        float lambert = max(dot(s, v_normalVarying), 0.0);
        float falloff = Falloff(length(light.position - v_positionVarying.xyz), light.radius);
        float specular = CookTorranceSpecular(s, -normalize(v_positionVarying.xyz), v_normalVarying, roughness);
        return light.intensity * (color.rgb * light.color.rgb * lambert * falloff * spotFactor + color.rgb * light.color.rgb * specular * falloff * spotFactor);
    }else{
        return vec3(0.0, 0.0, 0.0);
    }
}

vec3 CalcDirectionalLight(vec4 v_positionVarying, vec3 v_normalVarying, vec3 color, Light light) {
    float lambert = max(dot(-light.direction, v_normalVarying), 0.0);
    float specular = CookTorranceSpecular(-light.direction, -normalize(v_positionVarying.xyz), v_normalVarying, roughness);
    return light.intensity * (color.rgb * light.color.rgb * lambert + color.rgb * light.color.rgb * specular);
}

float CalcSoftShadow(int index, float expo){
    float visiblity = 1.0;
    vec3 projCoords = v_shadowCoord[index].xyz / v_shadowCoord[index].w;
    float currentDepth = projCoords.z;
    vec2 offset = vec2(depthTexMag.x * (index % 4), depthTexMag.y * floor(index / 4));
    float depth = texture(shadowMap, offset + projCoords.xy * depthTexMag).r;
    visiblity = exp(expo * depth) * exp(-expo * currentDepth);
    if(projCoords.x >= 1.0 || projCoords.x <= 0.0 ||
       projCoords.y >= 1.0 || projCoords.y <= 0.0 ||
       projCoords.z >= 1.0 || projCoords.z <= 0.0){
        visiblity = 1.0;
    }
    return visiblity;
}

float CalcHardShadow(int index){
    float visiblity = 1.0;
    vec3 projCoords = v_shadowCoord[index].xyz / v_shadowCoord[index].w;
    float currentDepth = projCoords.z;
    vec2 offset = vec2(depthTexMag.x * (index % 4), depthTexMag.y * floor(index / 4));
    if(currentDepth - lights[index].bias > texture(shadowMap, offset + projCoords.xy * depthTexMag).r){
        visiblity -= 1.0;
    }
    if(projCoords.x >= 1.0 || projCoords.x <= 0.0 ||
       projCoords.y >= 1.0 || projCoords.y <= 0.0 ||
       projCoords.z >= 1.0 || projCoords.z <= 0.0){
        visiblity = 1.0;
    }
    return visiblity;
}

vec3 CalcLightAndShadow(vec4 v_positionVarying, vec3 v_normalVarying, vec3 color, int index)
{
    vec3 light = vec3(0.0);
    if(lights[index].type == 0){
        light = CalcDirectionalLight(v_positionVarying, v_normalVarying, color, lights[index]);
    }else if(lights[index].type == 1){
        light = CalcSpotLight(v_positionVarying, v_normalVarying, color, lights[index]);
    }else if(lights[index].type == 2){
        light = CalcPointLight(v_positionVarying, v_normalVarying, color, lights[index]);
    }
    
    float shadow = 1.0;
    if(lights[index].shadowType == 0){
        shadow = 1.0;
    }else if(lights[index].shadowType == 1){
        shadow = CalcHardShadow(index);
    }else if(lights[index].shadowType == 2){
        shadow = CalcSoftShadow(index, lights[index].softShadowExponent);
    }

    return vec3(light * clamp(shadow, 0.0, 1.0));
}

void main (void) {
    vec3 color = vec3(0.05);
    vec3 light = vec3(0.0);
    float shadow = 0.0;
    for(int i=0; i<numLights; i++){
        if(lights[i].isEnabled == true){
            color += CalcLightAndShadow(v_positionVarying, v_normalVarying, colorVarying.xyz, i);
        }
    }
    fragColor = vec4(color, 1.0);
}