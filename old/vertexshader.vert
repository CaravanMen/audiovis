#version 460 core
int size = 2048;
layout (binding = 0) buffer storageBuffer
{
    double audioBuffer[];
};
void main()
{
    int maxValue = size/2;
    float x = (gl_VertexID-maxValue);
    double y = audioBuffer[gl_VertexID]/32767;
    gl_Position = vec4(x, y*maxValue, 0, maxValue);
}