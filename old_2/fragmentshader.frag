#version 460 core

// Buffers
layout (binding = 0) buffer storageBuffer
{
    float bassRange[2048];
    float lowRange[2048];
    float midRange[2048];
    float highRange[2048];
    float bassAmp;
    float lowAmp;
    float midAmp;
    float highAmp;
};

// UNIFORMS
layout (location = 0) uniform ivec2 screenSize;
layout (location = 1) uniform vec2 cursorPos;
// IN

// OUT
out vec4 outColor;

// VARIABLES
vec3 basCol;
vec3 lowCol;
vec3 midCol;
vec3 higCol;
float test = 7;
float scale = screenSize.y/4.75;
float err = 2;

// FUNCTIONS

void bassRangeVis()
{
    // Bass end amp
    float coords = gl_FragCoord.y-scale;
    float soundCoords = bassRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/3/64;
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 distNorm = dist/scale*test;
    vec2 fade = 1/(distNorm*distNorm);

    vec3 color = vec3(0, 1, 0);
    vec3 offColor = vec3(bassAmp*fade.y)+0.2;
    color = mix(color, vec3(1)-color, (bassAmp));
    vec3 finColor = vec3(offColor);
    if (dist.y > err)
    {
        finColor = color*fade.y*bassAmp;
    }
    basCol = finColor;
}

void lowRangeVis()
{
    float coords = gl_FragCoord.y-scale*2;
    float soundCoords = lowRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/3/64;
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 distNorm = dist/scale*test;
    vec2 fade = 1/(distNorm*distNorm);

    vec3 color = vec3(1, 0, 1);
    vec3 offColor = vec3(lowAmp*fade.y)+0.2;
    color = mix(color, vec3(1)-color, lowAmp);
    vec3 finColor = vec3(offColor);
    if (dist.y > err)
    {
        finColor = color*fade.y*lowAmp;
    }
    lowCol = finColor;
}

void midRangeVis()
{
    // Bass end amp
    float coords = gl_FragCoord.y-scale*3;
    float soundCoords = midRange[int(floor((gl_FragCoord.x*2048)/screenSize.x))]/3/64;
    vec2 dist = vec2(coords.x, abs(coords-soundCoords));
    vec2 distNorm = dist/scale*test;
    vec2 fade = 1/(distNorm*distNorm);

    vec3 color = vec3(0, 0, 1);
    vec3 offColor = vec3(midAmp*fade.y)+0.2;
    color = mix(color, vec3(1)-color, midAmp);
    vec3 finColor = vec3(offColor);
    if (dist.y > err)
    {
        finColor = color*fade.y*midAmp;
    }
    midCol = finColor;
}
void highRangeVis()
{
    // Bass end amp
    float coords = gl_FragCoord.y-scale*4;
    float soundCoords = highRange[int((gl_FragCoord.x*2048)/screenSize.x)]/(3*64);
    vec2 dist = vec2(abs(int((gl_FragCoord.x))-gl_FragCoord.x), abs(soundCoords-coords));

    float distNorm = sqrt(dist.x*dist.x+dist.y*dist.y)/scale*test;
    float fade = 1/(distNorm*distNorm);

    vec3 color = vec3(1, 0, 0);
    vec3 offColor = vec3(highAmp*fade)+0.2;
    color = mix(color, vec3(1)-color, highAmp);
    vec3 finColor = vec3(offColor);
    if (dist.y > err)
    {
        finColor = color*fade*highAmp;
    }
    higCol = finColor;
}
void main()
{
    scale = screenSize.y/4.75;
    outColor = vec4(vec3(0), 1);
    bassRangeVis();
    lowRangeVis();
    midRangeVis();
    highRangeVis();
    outColor = vec4((basCol+lowCol+midCol+higCol), 1);
}
