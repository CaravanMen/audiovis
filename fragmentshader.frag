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
layout (Location = 2) uniform float bass_amp;

layout (Location = 3) uniform int ringCount;

// Output color
out vec4 outColor;

// Draw Circle function
void DrawCircle()
{
    // ------- Calculating Aspect Ratio -------
    float aspect_ratio = 1024/sqrt((screen_scale.x*screen_scale.x)+(screen_scale.y*screen_scale.y));
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
    const float rad = 100+((amp+(bass_amp*12000))/2);
    const float den = distance_from_center-(rad);
    const float main_dist = distance_from_center-(rad)-(max_amp*512*128);

    // Red Background Settings
    const float redInverseFac = bass_amp*256;
    const float redExpMult = 5.0f;
    const float redYOffset = 1.0f;
    const float redExpVal = exp(redExpMult*(redInverseFac-redYOffset))-exp(redExpMult*(-redYOffset));

    // Colour outputting
    if (abs(distance_from_center) > den)
    {
        outColor = vec4(1)/abs(main_dist);
        // Render outer rings
        // Ring 1
        outColor+=vec4((bass_amp*512), 0, (max_amp*512), 1)/abs(main_dist-16);
        // I'm a ghost 2:00
        // if (inverseFac >= 0.7f)
        // {
        // }
        // NOT WORKING - NEEDS FIXING:
    }
    outColor.r = mix(outColor.r, ((den/512))-outColor.r, redExpVal);
    for (int i=0; i<ringCount; i++)
    {
        if (dist[i] > main_dist-16 && dist[i] > 0)
        {
            outColor+=vec4(0, 0, 1, 1)/abs(den-dist[i]);
        }
    }
}

void main()
{
    // Draw Circle
    DrawCircle();
}