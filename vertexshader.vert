// Buffers
layout (binding = 0) buffer storageBuffer
{
    float highRange[BUFSIZE];
};

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
    
    gl_Position = vec4(boxBoundry, 1.0, 1);
}
