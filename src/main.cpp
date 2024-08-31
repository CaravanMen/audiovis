
#include <malloc.h>
#include <memory.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <string>
#include <chrono>
#include <math.h>
#include <thread>
#include <pulse/simple.h>
#include <pulse/error.h>

// Own files
#include <main.h>
#include <fileHandler.h>
#include <filterHandler.h>
/**
 * NOTE
 * * Possibly make audiobuffer only store raw buffer data.
**/
fftwType audioBuffer[BUFSIZE];
// Global Variables
int fwidth, fheight;
pa_simple* paConn = NULL;

bool initialize_pulse_audio()
{
    // Device initialization (This is PulseAudio; Linux (Ubuntu) sound server)
    // Consider using a more widely available API, such as PortAudio...
    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_S24_32LE;    // 16-bit signed little-endian format
    sampleSpec.rate = SAMPLE_RATE;            // Sample rate (Hz)
    sampleSpec.channels = CHANNELS;            // Number of channels (stereo)

    pa_buffer_attr bufferAttr;
    bufferAttr.maxlength = (int32_t) BUFSIZE*2;
    bufferAttr.tlength = (int32_t) BUFSIZE;
    bufferAttr.minreq = (int32_t) -1;
    bufferAttr.prebuf = (int32_t) -1;
    bufferAttr.fragsize = (int32_t) BUFSIZE;

    int err = NULL;
    paConn = pa_simple_new(NULL, "read-audio", PA_STREAM_RECORD, PA_DEV_NAME, "read-audio", &sampleSpec, NULL, &bufferAttr, &err);

    
    if (err)
    {
        fprintf(stderr, "[ERR] pa_simple_new() failed: %s\n", pa_strerror(err));
        return 0;  
    }
 
    return 1;
}

// OPENGL STUFF
// Callbacks
void error_callback(int err, const char* desc)
{
    printf("[ERR] GLFW ERR CODE: %i\n DESC: %s\n", err, desc);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    fwidth = width;
    fheight = height;
    glViewport(0, 0, width, height);
    glUniform2i(0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    glUniform2f(1, (float)xpos, (float)ypos);
}

// Startup Functions
unsigned int load_shader(GLuint shaderType, const char* fileName)
{
    const char* typeChar;
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
            typeChar = "Vertex Shader";
        break;
        case GL_FRAGMENT_SHADER:
            typeChar = "Fragment Shader";
        break;
    }
    // Collecting shader source code from files
    int size;
    char* src = (char*)malloc(sizeof(char)*10000);
    strcpy(src, (char*)"#version 460 core\n#define BUFSIZE ");
    strcat(src, std::to_string(SAMPDTL).c_str());
    strcat(src, "\n");
    strcat(src, read_file(fileName, size));
    printf("%s", src);
    
    // Create shader
    unsigned int shader = glCreateShader(shaderType);
    // Attach shader source
    glShaderSource(shader, 1, &src, 0);

    // Compile shaders
    glCompileShader(shader);

    delete[] src;

    // Collect shader compile status
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // Shader compilation failed
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("[ERR] %s Compilation failed: %s\n", typeChar, infoLog);
        return 0;
    }
    printf("[LOG] Successfully compiled %s!\n", typeChar);
    // Return shader ID
    return shader;
}

// Possibly modify to include shader sources?
unsigned int load_program()
{
    // Load shaders
    unsigned int vertShader = load_shader(GL_VERTEX_SHADER, "vertexshader.vert");
    if (!vertShader)
    {
        return 0;
    }
    unsigned int fragShader = load_shader(GL_FRAGMENT_SHADER, "fragmentshader.frag");
    if (!fragShader)
    {
        return 0;
    }
    // Create shader program
    unsigned int shaderProgram = glCreateProgram();

    // Link shaders
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    // Get shaderProgram link (compile) status
    int success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("[ERR] Failed to link shaderProgram: %s\n", infoLog);
        return -1;
    }

    printf("[LOG] Successfully linked program!\n");
    // Delete remaining shader data
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return shaderProgram;
}

int main()
{
    // Initialize pulseaudio   
    if (!initialize_pulse_audio())
    {
        printf("[ERR] Failed to initialize Pulseaudio\n");
        glfwTerminate();
        return -1;
    }
    printf("[LOG] Successfully initialized Pulseaudio\n");

    // Initialize OpenGL
    // set error callback
    glfwSetErrorCallback(error_callback);
    // Initialize GLFW
    if (!glfwInit())
    {
        // GLFW initialization failed
        printf("[ERR] Failed to initialize GLFW\n");
        return -1;
    }
    printf("[LOG] Successfully initialized GLFW\n");
    // GLFW window hints
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_4_6);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_4_6);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    // Create GLFW window

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GLFW window", 0, 0);

    if (!window)
    {
        // Failed to create GLFW window
        printf("[ERR] Failed to create window\n");
        glfwTerminate();
        return -1;
    }
    printf("[LOG] Successfully created window\n");
    // Select GLFW window as current context (for gl drawing)
    glfwMakeContextCurrent(window);
    // Set window callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    // load opengl
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("[ERR] Failed to load openGL\n");
        return -1;
    }
    printf("[LOG] Successfully loaded openGL\n");

    // Create shader program
    unsigned int shaderProgram = load_program();
    if (!shaderProgram)
    {
        return -1;
    }

    // // generate a buffer object
    unsigned int ssboID;
    glCreateBuffers(1, &ssboID);
    glNamedBufferStorage(ssboID, sizeof(fftwType)*SAMPDTL, 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ssboID);

    // Final Initialization of GLFW
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Initialize fftw
    filter_init(BUFSIZE, SAMPLE_RATE);
    int32_t buffer[SAMPDTL];
    int error;
    // Use the shaderProgram
    glUseProgram(shaderProgram);

    glEnable(GL_CLEAR);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glUniform2i(0, WIDTH, HEIGHT);
    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        // PULSEAUDIO STUFF
        int err;
        if (pa_simple_read(paConn, buffer, sizeof(buffer), &error) < 0) {
            fprintf(stderr, "[ERR] pa_simple_read() failed: %s\n", pa_strerror(error));
            break;
        }
        for (size_t i=0; i<SAMPDTL; i++)
        {
            audioBuffer[i] = static_cast<fftwType>(buffer[i])/8388607.0f;
        }
        fftwType freqData[SAMPDTL];
        fftwType amp = 0;
        fftw_filter(audioBuffer, freqData, MINFREQ, MAXFREQ, amp);
        // RENDERING STUFF
        glClear(GL_COLOR_BUFFER_BIT);

        // Update the SSBO with the latest audio data (if needed) (Push new data into storage buffer)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(fftwType)*SAMPDTL, freqData);
        glUniform1f(2, amp);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Error checking
        int glErr = glGetError();
        while (glErr != GL_NO_ERROR)
        {
            char msg[1024];
            switch (glErr)
            {
            case GL_INVALID_ENUM:
                memcpy(&msg, "GL_INVALID_ENUM", sizeof("GL_INVALID_ENUM"));
                break;
            case GL_INVALID_VALUE:
                memcpy(&msg, "GL_INVALID_VALUE", sizeof("GL_INVALID_VALUE"));
                break;
            case GL_INVALID_OPERATION:
                memcpy(&msg, "GL_INVALID_VALUE", sizeof("GL_INVALID_VALUE"));
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                memcpy(&msg, "GL_INVALID_FRAMEBUFFER_OPERATION", sizeof("GL_INVALID_FRAMEBUFFER_OPERATION"));
                break;
            case GL_OUT_OF_MEMORY:
                memcpy(&msg, "GL_OUT_OF_MEMORY", sizeof("GL_OUT_OF_MEMORY"));
                break;
            case GL_STACK_UNDERFLOW:
                memcpy(&msg, "GL_STACK_UNDERFLOW", sizeof("GL_STACK_UNDERFLOW"));
                break;
            case GL_STACK_OVERFLOW:
                memcpy(&msg, "GL_STACK_OVERFLOW", sizeof("GL_STACK_OVERFLOW"));
                break;
            default:
                memcpy(&msg, "glGetError had an unexpected error...", sizeof("glGetError had an unexpected error..."));
                break;
            }
            printf("OpenGL Error: [0x%x], '%s';\n", glErr, msg);
            glErr = glGetError();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &ssboID);
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &ssboID);
    glDeleteProgram(shaderProgram);

    filter_end();
    glfwTerminate();
    return 0;
}