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
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0f, 1.0f );
    return smoothstep(3.3f, 0.0f, length(pa - ba*h));
}

void main()
{
    // ------- Calculating Aspect Ratio -------
    float aspect_ratio = 1024.0f/screen_scale.x;
    // Set the location of circle
    vec2 location = vec2(screen_scale.x/2, screen_scale.y/2);

    // ------- Distance Calculations -------
    // Get distance from the location of the circle as x,y components
    vec2 distance_xy = vec2(location.x-gl_FragCoord.x, location.y-gl_FragCoord.y)*aspect_ratio;
    // Get distance from center of circle
    float distance_from_center = sqrt((distance_xy.x*distance_xy.x) + (distance_xy.y*distance_xy.y));

    // ------- Radius Calculations -------
    // Calculate radius of the circle
    // Create index (based on conversion 2pi=1024, hence 2pi/1024)
    const int index = BUFFSIZE-int((acos(distance_xy.y/distance_from_center))*floor(BUFFSIZE/3.14f))-1;
    const float rad = 73.0f+(max_amp*0.2f)+(bass_amp*0.025f);
    const float sampData = highRange[index]/1024.0f;
    const float den = distance_from_center-rad-(rad*sampData); // This is a weird way of setting the denominator tbh.

    // ------- Calculating background information -------
    const float xOffset = 0.0045f;
    const float magnitude = 960.0f;
    const float colShiftMag = 1.0f/(1.0f+exp(-magnitude*((sub_bass_amp/40960.0f)-xOffset)));
    vec3 backgroundColor = vec3(0);
    backgroundColor.r = mix(0.0f, den/512.0f, colShiftMag);

    // ------- Drawing Circle -------
    if (den >= -1)
    {
        outColor = vec3(1)/abs(den);
        // Render offset ring
        vec3 outRingColor = mix(vec3(max_amp*0.0125f, 0.0f, 0.0f), vec3(-max_amp*0.009375f, 0.0f, max_amp*0.025f), colShiftMag);
        
        outColor += outRingColor/abs(distance_from_center-outCircRad-(outCircRad*sampData));
        
        // Render outer expanding (radially) rings
        for (int i=0; i<16; i++)
        {
            float rad = array[i];
            float historicalAmp = rad+array[16+(i*1024)+index]*rad/1024.0f;
            if (rad > 0 && distance_from_center<rad+historicalAmp+1)
            {
                outColor += vec3(0.5f, 0.0f, 4.0f)/abs(distance_from_center-historicalAmp);
            }
        }
        outColor += backgroundColor;
    }else
    {
        vec3 lineColor = vec3(1.0f);
        outColor = vec3(lineColor)/abs(den);

        float offset = rad+(rad*highRange[512]/1024.0f);
        const vec2 p = vec2(((distance_xy.x+offset)*1024.0f)/(2.0*(offset)), (distance_xy.y)*1024.0f/(2.0*(offset)));
        // Draw linesrad+offset
        for (int i=-3; i<=3; i++)
        {
            int fl = int(floor(p.x))+i;
            float flAmp = lineData[fl]*256.0f;
            int cl = fl+1;
            float clAmp = lineData[cl]*256.0f;
            outColor += mix(vec3(0), mix(vec3(0.0f), lineColor, abs(rad-distance_from_center)/64.0f), lineSegment(p, vec2(fl, flAmp), vec2(cl, clAmp)));
        }
    }
}
