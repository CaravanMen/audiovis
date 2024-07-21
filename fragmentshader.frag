#define SCALE 512

// Buffers
layout (binding = 0) buffer storageBuffer
{
    float highRange[BUFSIZE];
};

// UNIFORMS
layout (location = 0) uniform ivec2 screenSize;
layout (location = 1) uniform vec2 cursorPos;
layout (location = 2) uniform float amp;
uniform float err = 1;
uniform float visMag = 2;
uniform float colMag = 2;
// OUT
layout (location = 0, index = 0) out vec4 outColor;
// VARIABLES
vec3 backgroundColor = vec3(0.1);
vec3 higCol = backgroundColor;
float visScaleFac = (screenSize.y/1024.0f)*visMag;
float errScaled = visScaleFac*err;
int hlf = screenSize.y/2;

// Freq Visualizer Calculations
float soundCoords = highRange[int(gl_FragCoord.x)*BUFSIZE/screenSize.x];
float scaledSoundCoords = soundCoords*visScaleFac;

// Cursor distance calculations
vec2 cursorDist = vec2(abs(cursorPos.x-gl_FragCoord.x), abs(gl_FragCoord.y-(-cursorPos.y)-screenSize.y));
float cursorDistNorm = sqrt(cursorDist.x*cursorDist.x+cursorDist.y*cursorDist.y);

// FUNCTIONS
void freqVis()
{
    vec3 color = vec3(mix(vec3(0, 1, 0), vec3(3, 0, 0), min(1, (sqrt(soundCoords/1024.0f)*colMag)))); // Optimize this (alongside a LOT of other things)
    vec3 colorOpp = vec3(1)-color;

    if (gl_FragCoord.y > hlf && gl_FragCoord.y < hlf+scaledSoundCoords)
    {
        higCol = color;
        float dist = abs(gl_FragCoord.y-scaledSoundCoords-hlf);
        if (dist<errScaled)
        {
            higCol = colorOpp;
        }
    }else if (gl_FragCoord.y < hlf && gl_FragCoord.y > hlf-scaledSoundCoords)
    {
        higCol = color;
        float dist = abs(hlf-gl_FragCoord.y-scaledSoundCoords);
        if (dist<errScaled)
        {
            higCol = colorOpp;
        }
    }
}
// FIX THIS FUNCTION... grids don't work at different screen sizes.
void drawGrid()
{
    int colDiv = 32;
    int rowDiv = 32;
    float colDist = 1024/colDiv;
    float rowDist = 1024/rowDiv;
    float column = mod(int(gl_FragCoord.x*1024/screenSize.x), colDist);
    float row = mod(int(gl_FragCoord.y*1024/screenSize.y), rowDist);
    vec3 color = vec3(0.25);

    if (column <= 1)
    {
        higCol += color;
    }
    if (row <= 1)
    {
        higCol += color;
    }
}

float bgCol()
{
    if (gl_FragCoord.x>10)
    {
        backgroundColor = vec3(amp);
        return 0;
    }
    backgroundColor = vec3(0);
}

// Draw Cursor Function
// Make cool effect by dividing
void drawCursor(float maxDist)
{
    float fade = (maxDist+scaledSoundCoords-cursorDistNorm)/(maxDist+scaledSoundCoords);
    higCol = vec3(1/mix(0, 16, fade));
    if (higCol.x < 0) higCol.x = 0;
    if (higCol.y < 0) higCol.y = 0;
    if (higCol.z < 0) higCol.z = 0;
    if (higCol.x > 1) higCol.x = 1;
    if (higCol.y > 1) higCol.y = 1;
    if (higCol.z > 1) higCol.z = 1;
}

void main()
{
    drawCursor(16+(amp/4));
    freqVis();
    outColor = vec4(higCol, 1);
}
