#version 460 core
// Variables
int size = 2048;
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
// IN

// OUT

void main()
{
    vec2 boxBoundry = vec2(0, 0);

    //  -1/1            1/1
    //  -1/-1           1/-1
    switch (gl_VertexID)
    {
        case 0:
            boxBoundry = vec2(-1, 1);
        break;
        case 1:
            boxBoundry = vec2(1, 1);
        break;
        case 2:
            boxBoundry = vec2(1, -1);
        break;
        case 3:
            boxBoundry = vec2(1, -1);
        break;
        case 4:
            boxBoundry = vec2(-1, -1);
        break;
        case 5:
            boxBoundry = vec2(-1, 1);
        break;
    };
    
    gl_Position = vec4(boxBoundry, 0.0, 1);
}
