#version 460 core

// Buffers
layout (binding = 0) buffer storageBuffer
{
    // float bassRange[2048];
    // float lowRange[2048];
    // float midRange[2048];
    float highRange[2048];
    float bassAmp;
    float lowAmp;
    float midAmp;
    float highAmp;
};

// UNIFORMS
layout (location = 0) uniform vec2 screenSize;
// IN

// OUT
out vec4 outColor;

// VARIABLES
vec4 basCol;
vec4 lowCol;
vec4 midCol;
vec4 higCol;

// Color stuff
vec3 low = vec3(0, 1, 0);
vec3 hig = vec3(1, 0, 0);

float scale = screenSize.y/4.75;
float err = 2;

// FUNCTIONS

void background()
{
    outColor = vec4(0, 0, 0, 1);
}

void bassRangeVis()
{
    // Bass end amp
    float coords = gl_FragCoord.y-scale+(scale/4);
    float soundCoords = bassRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/3/64;
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 distNorm = dist/scale*8;
    vec2 luminance = 1/(distNorm);

    vec3 color = vec3(0, 1, 0);
    vec3 offColor = vec3((bassAmp*luminance.y)/5)+0.2;
    color = mix(color, vec3(1)-color, (bassAmp*bassAmp)*7);
    vec4 finColor = vec4(offColor, 1.0f);
    if (dist.y > err)
    {
        finColor = vec4((color*bassAmp)*luminance.y, luminance);
    }
    basCol = finColor;
}

void lowRangeVis()
{
    float coords = gl_FragCoord.y-scale*2+(scale/4);
    float soundCoords = lowRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/3/64;
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 scaledDist = dist/scale*8;
    vec2 fade = 1/(scaledDist*scaledDist);

    vec3 color = vec3(1, 0, 1);
    vec3 offColor = vec3((lowAmp*fade.y)/5)+0.2;
    color = mix(color, vec3(1)-color, (lowAmp*lowAmp)*5);
    vec4 finColor = vec4(offColor, 1.0f);
    if (dist.y > err)
    {
        finColor = vec4((color*lowAmp)*fade.y, fade);
    }
    lowCol = finColor;
}

void midRangeVis()
{
    // Bass end amp
    float coords = gl_FragCoord.y-scale*3+(scale/4);
    float soundCoords = midRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/3/64;
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 scaledDist = dist/scale*8;
    vec2 fade = 1/(scaledDist*scaledDist);

    vec3 color = vec3(0, 0, 1);
    vec3 offColor = vec3((midAmp*fade.y)/5)+0.2;
    color = mix(color, vec3(1)-color, midAmp*midAmp*midAmp*5);
    vec4 finColor = vec4(offColor, 1.0f);
    if (dist.y > err)
    {
        finColor = vec4((color*midAmp)*fade.y, fade);
    }
    midCol = finColor;
}
void highRangeVis()
{
    // Bass end amp
    float coords = gl_FragCoord.y-scale*4+(scale/4);
    float soundCoords = highRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/(3*64);
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 scaledDist = dist/scale*8;
    vec2 fade = 1/(scaledDist*scaledDist);

    vec3 color = vec3(1, 0, 0);
    vec3 offColor = vec3((highAmp*fade.y)/5)+0.2;
    color = mix(color, vec3(1)-color, highAmp*highAmp*highAmp*8);
    vec4 finColor = vec4(offColor, 1.0f);
    if (dist.y > err)
    {
        finColor = vec4((color*highAmp)*fade.y, fade);
    }
    higCol = finColor;
}
void main()
{
    background();
    bassRangeVis();
    lowRangeVis();
    midRangeVis();
    highRangeVis();
    vec4 finalColor = basCol+lowCol+midCol+higCol;
    outColor = finalColor;
}
