#define SCALE 512

// Buffers
layout (binding = 0) uniform storageBuffer
{
    float highRange[BUFSIZE];
};

// Collecting uniforms
layout (Location = 0) uniform ivec2 screen_scale;
layout (Location = 1) uniform float max_amp;

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
    int index = 1024-int((acos(distance_xy.y/distance_from_center))*(BUFSIZE/3.141592653589897f));
    float amp = ((highRange[index]*32.0f)*(48000/1200));
    float rad = 128+(amp*16);
    float den = distance_from_center-(max_amp*1024*64)-(rad);
    outColor = vec4((0.24f+(amp+max_amp)*aspect_ratio))/abs(den);
    outColor += vec4(((max_amp*1024))*aspect_ratio, 0, 0, 1)/abs(den-16);
}

void main()
{
    // Draw Circle
    DrawCircle();
}
