#define SCALE 512

// Buffers
layout (binding = 0) uniform storageBuffer
{
    float highRange[BUFSIZE];
};

layout (binding = 1) uniform ringBuffer
{
    float dist[128];
};

// Collecting uniforms
layout (Location = 0) uniform ivec2 screen_scale;
layout (Location = 1) uniform float max_amp;
layout (Location = 2) uniform float sub_bass_amp;
layout (Location = 4) uniform float bass_amp;
layout (Location = 3) uniform int ringCount;

// Output color
out vec4 outColor;

// Draw Circle function
void DrawCircle()
{
    // ------- Calculating Aspect Ratio -------
    float aspect_ratio = 1024.0f/screen_scale.x;
    // Get location of circle
    vec2 location = vec2(screen_scale.x/2, screen_scale.y/2);
    // ------- Distance Calculations -------
    // Get Distance as x,y components
    vec2 distance_xy = vec2(location.x-gl_FragCoord.x, location.y-gl_FragCoord.y)*aspect_ratio;
    // Get distance from center of circle
    float distance_from_center = sqrt((distance_xy.x*distance_xy.x) + (distance_xy.y*distance_xy.y));
    // ------- Radius Calculations -------
    // Calculate radius of the circle
    // Create index (based on conversion 2pi=1024, hence 2pi/1024)
    const int index = 1024-int((acos(distance_xy.y/distance_from_center))*(BUFSIZE/3.141592653589897f))-1;
    const float amp = ((highRange[index]*12000));
    const float rad = 100.0f+(amp)+(max_amp*128.0f*64.0f)+(sub_bass_amp*5000.0f)+(bass_amp*10000.0f);
    const float den = distance_from_center-rad;

    // Red Background Settings
    const float redInverseFac = sub_bass_amp*256;
    const float redExpMult = 1.0f;
    const float redYOffset = 0.6f;
    const float redExpVal = exp(redExpMult*(redInverseFac-redYOffset))-exp(redExpMult*(-redYOffset));

    // Colour outputting
    if (abs(distance_from_center) > den)
    {
        outColor = vec4(1)/abs(den);
        // Render outer rings
        // Ring 1
        outColor+=vec4((sub_bass_amp*128), 0, (max_amp*128), 1)/abs(den-16);
        // I'm a ghost 2:00
        // if (inverseFac >= 0.7f)
        // {
        // }
        // NOT WORKING - NEEDS FIXING:
    }
    outColor.r = mix(outColor.r, ((den/1024.0f)), redExpVal);
    for (int i=0; i<ringCount; i++)
    {
        if (dist[i] > den-16 && dist[i] > 0)
        {
            outColor+=vec4(0, 0, bass_amp*1024.0f, 1)/abs(distance_from_center-dist[i]-amp);
        }
    }
}

void main()
{
    // Draw Circle
    DrawCircle();
}