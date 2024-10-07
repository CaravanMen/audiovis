#version 460 core
// BUFFERS
layout (binding = 0) uniform SSBO { float array[1024]; };
// In
layout (location=1) uniform vec2 u_resolution;

// Uniforms
uniform float u_thickness = 10;

void main()
{
    gl_Position = vec4(gl_VertexID*2.0f/1024.0f-1.0f, array[gl_VertexID], 0, 1);
}