#define SCALE 512

// Buffers
layout (binding = 0) uniform storageBuffer
{
    float highRange[BUFFSIZE];
    float lineData[BUFFSIZE];
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

float lineSegment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return smoothstep(0.0, 20.0, length(pa - ba*h));
}

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
    const float rad = 73.0f+(max_amp*8192.0f)+(bass_amp*1024.0f);
    const float den = distance_from_center-rad-sampData;

    // ------- Calculating background information -------
    const float maxAsymp = 2;
    const float xOffset = 0.0045;
    const float magnitude = 1330;
    const float colShiftMagnitude = maxAsymp/(1+exp(-magnitude*(sub_bass_amp-xOffset)));
    vec3 backgroundColor = vec3(0);
    backgroundColor.r = mix(0, den/1024.0f, colShiftMagnitude);

    vec3 outRingColor;
    // ------- Drawing Circle -------
    if (den >= -1)
    {
        outColor = vec3(1)/abs(den);
        // Render offset ring
        outRingColor = (mix(vec3(max_amp, 0, 0), vec3(max_amp/8, 0, sub_bass_amp), colShiftMagnitude)*512.0f);
        outColor+=outRingColor/abs(distance_from_center-outCircRad-sampData);
        
        // Render outer expanding (radially) rings
        for (int i=0; i<16; i++)
        {
            float rad = array[i];
            float historicalAmp = array[16+(i*1024)+index]*rad*32.0f;
            if (rad > 0 && distance_from_center<rad+historicalAmp+1)
            {
                outColor += vec3(0.4, 0, 2)/abs(distance_from_center-rad-historicalAmp);
            }
        }
        outColor += backgroundColor;
        // I'm a ghost 2:00
    }else
    {
        float offset = highRange[512]*6000.0f;
        vec2 p = vec2(((distance_xy.x+rad+offset)*1024.0f)/(2.0*(rad+offset)), (distance_xy.y)*1024.0f/(2.0*(rad+offset)));
        float fl = floor(p.x);
        float flAmp = lineData[int(fl)]*256.0f;
        float cl = ceil(p.x);
        float clAmp = lineData[int(cl)]*256.0f;
        // Draw lines
        outColor = mix(vec3(1.0), vec3(0.0), lineSegment(p, vec2(fl, flAmp), vec2(cl, clAmp)));
    }
}
