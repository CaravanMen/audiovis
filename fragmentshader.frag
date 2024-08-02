#define SCALE 512

// Buffers
layout (binding = 0) buffer storageBuffer
{
    float highRange[BUFSIZE];
};

// UNIFORMS
layout (location = 0) uniform ivec2 screenSize;
float arat = sqrt((screenSize.x*screenSize.x)+(screenSize.y*screenSize.y));
layout (location = 1) uniform vec2 cursorPos;
layout (location = 2) uniform float amp;
uniform float err = 1;
uniform float visMag = 2;
uniform float colMag = 2;
// OUT
layout (location = 0, index = 0) out vec4 outColor;
// Freq Visualizer Calculations
float soundCoords = highRange[int(gl_FragCoord.x)*BUFSIZE/screenSize.x];
float visScaleFac = (screenSize.y/1024.0f)*visMag;
float errScaled = visScaleFac*err;
int hlf = screenSize.y/2;

float scaledSoundCoords = soundCoords*visScaleFac;

// VARIABLES
vec3 backgroundColor = vec3(0.01);

// Cursor distance calculations
vec2 cursorDist = vec2(abs(cursorPos.x-gl_FragCoord.x), abs(gl_FragCoord.y-(-cursorPos.y)-screenSize.y));
float cursorTan = sqrt((cursorDist.x*cursorDist.x)+(cursorDist.y*cursorDist.y));

vec3 freqColor = vec3(0);
// FUNCTIONS
void freqVis()
{
    vec3 color = vec3(mix(vec3(0, 1, 0), vec3(3, 0, 0), min(1, (sqrt(soundCoords/1024.0f)*colMag)))); // Optimize this (alongside a LOT of other things)
    vec3 colorOpp = vec3(1)-color;
    float visDist = abs(gl_FragCoord.y-hlf);
    if (visDist < scaledSoundCoords+1)
    {
        freqColor = color;
        float dist = abs(visDist-scaledSoundCoords);
        if (dist<errScaled)
        {
            freqColor = colorOpp;
        }
    }
}
// FIX THIS FUNCTION... grids don't work at different screen sizes.
vec3 gridCol = vec3(0);
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
        gridCol += color;
    }
    if (row <= 1)
    {
        gridCol += color;
    }
}

// Draw Cursor Function
// Make cool effect by dividing
vec3 cursorCol = vec3(0);
void drawCursor()
{
    float maxDist = (16+sqrt(amp/253)*52)*(arat/1024);
    if (cursorTan <= maxDist)
    {
        float fade = (-0.1*maxDist)/(cursorTan-maxDist);
        cursorCol = vec3(fade);
    }

}

void main()
{
    drawCursor();
    freqVis();
    vec3 finColor = freqColor+cursorCol;
    outColor = vec4(finColor, 1);
}
