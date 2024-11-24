#define SCALE 512

// Buffers
layout (binding = 0) uniform storageBuffer
{
    float highRange[BUFFSIZE];
};

layout (binding = 1) buffer ringBuffer
{
    int maxRings;
    float array[];
};

// Collecting uniforms
layout (Location = 0) uniform ivec2 screen_scale;
layout (Location = 1) uniform float max_amp;
layout (Location = 2) uniform float sub_bass_amp;
layout (Location = 4) uniform float bass_amp;
layout (location = 3) uniform float outCircRad;

// Output color
out vec3 outColor;

void main()
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
    const int index = BUFFSIZE-int((acos(distance_xy.y/distance_from_center))*(BUFFSIZE/3.141592653589897f))-1;
    const float sampData = highRange[index]*6000.0f;
    const float rad = 75.0f+sampData+(max_amp*8192.0f)+(sub_bass_amp*1024.0f)+(bass_amp*1024.0f);
    const float den = distance_from_center-rad;

    // ------- Calculating background information -------
    const float inverseFac = sub_bass_amp;
    const float expMult = 128.0f;
    const float yOffset = 0.0f;
    const float colShiftMagnitude = expMult*exp((inverseFac-yOffset))-(expMult*exp((-yOffset)));
    vec3 backgroundColor = vec3(0);
    backgroundColor.r = mix(0, den/1024.0f, colShiftMagnitude);

    // ------- Drawing Circle -------
    if (den >= -1)
    {
        outColor = vec3(1)/abs(den);
        // Render offset ring
        outColor+=(mix(vec3(max_amp, 0, 0), vec3(max_amp/8, 0, sub_bass_amp), colShiftMagnitude)*512.0f)/abs(distance_from_center-outCircRad-sampData);
        
        // Render outer expanding (radially) rings
        for (int i=0; i<16; i++)
        {
            float rad = array[i];
            float historicalAmp = array[16+(i*1024)+index]*rad*64.0f;
            if (rad > 0 && distance_from_center<rad+historicalAmp+1)
            {
                outColor += (vec3(max_amp/8.0f, 0, sub_bass_amp)*1024.0f)/abs(distance_from_center-rad-historicalAmp);
            }
        }
        // I'm a ghost 2:00
    }else
    {
        // Draw lines
        outColor += vec3(0);
    }
    outColor += backgroundColor;
}
