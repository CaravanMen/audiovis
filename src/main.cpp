#include <main.h>
// Own files
#include <fileHandler.h>
#include <filterHandler.h>
// Libraries
#include <memory.h>
#include <string>
// Pulseaudio stuff
#include <pulse/simple.h>
#include <pulse/error.h>

pa_simple* paConn = NULL;

bool initialize_pulse_audio()
{
    // Device initialization (This is PulseAudio; Linux (Ubuntu) sound server)
    // Consider using a more widely available API, such as PortAudio...
    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_S32LE;    // 16-bit signed little-endian format
    sampleSpec.rate = SAMPLERATE;            // Sample rate (Hz)
    sampleSpec.channels = CHANNELS;            // Number of channels (stereo)

    pa_buffer_attr bufferAttr;
    bufferAttr.maxlength = (uint32_t) -1;
    bufferAttr.tlength = (uint32_t) SAMPDTL;
    bufferAttr.minreq = (uint32_t) -1;
    bufferAttr.prebuf = (uint32_t) SAMPDTL;
    bufferAttr.fragsize = (uint32_t) pa_usec_to_bytes(1667, &sampleSpec);

    int err=0;
    paConn = pa_simple_new(NULL, "read-audio", PA_STREAM_RECORD, PA_DEV_NAME, "read-audio", &sampleSpec, NULL, &bufferAttr, &err);

    
    if (err)    
    {
        fprintf(stderr, "[ERR] pa_simple_new() failed: %s\n", pa_strerror(err));
        return 0;  
    }
 
    return 1;
}

// Time stuff
#include <chrono>
using namespace std::chrono;
#define timeNow system_clock::now()
time_point<system_clock> lastTime, currTime;
duration<float> _duration;
float deltaTime;

static inline void initClock()
{
    currTime=timeNow;
}
// Global Variables
fftwType audioBuffer[SAMPDTL];
int fwidth, fheight;

// OpenGL Libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// OpenGL Functions
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
        printf("[ERR] Failed to load vertex shader.\n");
        return 0;
    }
    unsigned int fragShader = load_shader(GL_FRAGMENT_SHADER, "fragmentshader.frag");
    if (!fragShader)
    {
        printf("[ERR] Failed to load fragment shader.\n");
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

// Main function
int main()
{
    // Initialize pulseaudio   
    if (!initialize_pulse_audio())
    {
        printf("[ERR] Failed to initialize Pulseaudio\n");
        glfwTerminate();
        return 1;
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
        return 1;
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
        return 1;
    }
    printf("[LOG] Successfully created window\n");
    // Select GLFW window as current context (for gl drawing)
    glfwMakeContextCurrent(window);
    // Set window callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // load opengl
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("[ERR] Failed to load openGL\n");
        return 1;
    }
    printf("[LOG] Successfully loaded openGL\n");

    // Create shader program
    unsigned int shaderProgram = load_program();
    // if (!shaderProgram)
    // {
    //     return 1;
    // }

    // // generate a buffer object
    unsigned int ssboID;
    glCreateBuffers(1, &ssboID);
    glNamedBufferStorage(ssboID, sizeof(fftwType)*SAMPDTL, 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ssboID);

    // Final Initialization of GLFW
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Initialize fftw
    filter_init(BUFSIZE, SAMPLERATE);
    int32_t buffer[SAMPDTL];
    int error;
    // Use the shaderProgram
    glUseProgram(shaderProgram);
    glEnable(GL_CLEAR);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    int glErr;
    // Main while loop
    glUniform2i(0, WIDTH, HEIGHT);
    // Rings stuff
    int ringCount = 16;
    float dist[ringCount];
    unsigned int rads;
    glCreateBuffers(1, &rads);
    glNamedBufferStorage(rads, sizeof(float)*ringCount, 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, rads);

    // System clock stuff
    initClock();
    float timePassed = 0;

    while (!glfwWindowShouldClose(window))
    {
        lastTime = currTime;
        timePassed += deltaTime;
        // PULSEAUDIO STUFF
        int err=0;
        if (pa_simple_read(paConn, buffer, sizeof(buffer), &error) < 0) {
            fprintf(stderr, "[ERR] pa_simple_read() failed: %s\n", pa_strerror(error));
            break;
        }
        for (size_t i=0; i<=SAMPDTL; i++)
        {
            audioBuffer[i] = static_cast<fftwType>(buffer[i])/2147483647.0f;
        }
        fftwType freqData[SAMPDTL];
        fftwType amp = 0;
        fftwType subBassAmp = 0;
        fftwType bassAmp = 0;
        fftw_filter(audioBuffer, freqData, MINFREQ, MAXFREQ, nullptr);
        fftw_filter(audioBuffer, nullptr, 60, MAXFREQ, &amp);
        fftw_filter(audioBuffer, nullptr, 20, 60, &subBassAmp);
        fftw_filter(audioBuffer, nullptr, 90, 320, &bassAmp);
        // RENDERING STUFF
        if (bassAmp != 0.0f)
        {
            printf("%f\n", bassAmp);
        }
        // Update the SSBO with the latest audio data (if needed) (Push new data into storage buffer)
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(fftwType)*SAMPDTL, freqData);
        glUniform1f(1, amp);
        glUniform1f(2, subBassAmp);
        glUniform1f(4, bassAmp);

        // Calculate rings
        glUniform1i(3, ringCount);
        glNamedBufferSubData(rads, 0, sizeof(float)*ringCount, dist);
        for (int i=0; i<ringCount; i++)
        {
            if (dist[i] > 0)
            {
                dist[i]+=(1024*deltaTime);
            }
        }
        if (timePassed >= 0.0624f && bassAmp >= 0.005f)
        {
            float newPos = 100.0f+((amp*128.0f*64.0f)+(subBassAmp*5000.0f))+(bassAmp*10000.0f)+16.0f;
            for (int i=ringCount-1; i>0; i--)
            {
                if (dist[i] < dist[0] && dist[i] > 0)
                {
                    dist[i] = newPos;
                }else{
                    dist[i] = dist[i-1];
                }
            }
            dist[0] = newPos;
            
            timePassed = 0;
        }
        // End calculation
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);


        // Error checking
        glErr = glGetError();
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

        // Deltatime stuff
        currTime=timeNow;
        _duration = currTime-lastTime;
        deltaTime = _duration.count();

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