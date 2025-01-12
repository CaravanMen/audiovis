#include <main.h>
// Own files
#include <fileHandler.h>
#include <filterHandler.h>
#include <audioHandler.h>
// Libraries
#include <memory.h>
#include <string>

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
    strcpy(src, (char*)"#version 460 core\n#define BUFFSIZE ");
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
        exit(-1);
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
        exit(-1);
    }

    printf("[LOG] Successfully linked program!\n");
    // Delete remaining shader data
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return shaderProgram;
}


// Main function
int main() {

    // Initialize OpenGL
    glfwSetErrorCallback(error_callback); /* set error callback */
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
    glNamedBufferStorage(ssboID, sizeof(fftwType)*SAMPDTL*2, 0, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ssboID);

    // Final Initialization of GLFW
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Use the shaderProgram
    glUseProgram(shaderProgram);
    glEnable(GL_CLEAR);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    int glErr;
    // Main while loop
    glUniform2i(0, WIDTH, HEIGHT);

    // Rings Buffer Setup
    RadialCircleDataStruct<fftwType> bassRings;
    unsigned int bassRingsBuffer;
    glCreateBuffers(1, &bassRingsBuffer);
    glNamedBufferStorage(bassRingsBuffer, sizeof(bassRings.array)+sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferSubData(bassRingsBuffer, 0, sizeof(int), &bassRings.maxRings);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bassRingsBuffer);
    
    // All bassThreshold Stuff
    float minBassAmp = 0.006f;
    float bassThreshold = minBassAmp;
    float highestBass = minBassAmp;
    double bassThresholdTimePassed = 0.0;
    double bassRingTimePassed = 0;
    float outCircRad = 16;

    fftwType amp = 0;
    fftwType bassAmp = 0;
    fftwType subBassAmp = 0;
    fftwType bgSmoothing = 0.0f;
    // Initialise audio handler
    InitialiseAudioHandler(SAMPDTL, RINGBUFFERSIZE);
    // Initialize fftw
    filter_init(BUFFSIZE, SAMPLERATE);
    int32_t buffer[SAMPDTL];
    int error;
    // Open Stream and start adding to Rolling Buffer
    OpenStream(Pa_GetDefaultInputDevice(), 1, 0, SAMPLERATE);
    StartStream();
    // System clock stuff
    initClock();
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        lastTime = currTime;

        // Reading audio
        int framesRead = ReadRingBuffer(audioBuffer, SAMPDTL);
        if (framesRead)
        {
            // Filtering Audio
            fftwType freqData[SAMPDTL];
            fftw_filter(audioBuffer, freqData, MINFREQ, MAXFREQ, nullptr);
            fftw_filter(audioBuffer, nullptr, 30, MAXFREQ, &amp);
            fftw_filter(audioBuffer, nullptr, 60, 300, &bassAmp);
            fftw_filter(audioBuffer, nullptr, 30, 60, &subBassAmp);
            bgSmoothing = (bgSmoothing+subBassAmp)/2.0f;

            // Update the SSBO with the latest audio data (if needed) (Push new data into storage buffer)
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboID);
            glNamedBufferSubData(ssboID, 0, sizeof(fftwType)*SAMPDTL, freqData);
            glNamedBufferSubData(ssboID, sizeof(fftwType)*SAMPDTL, sizeof(fftwType)*SAMPDTL, audioBuffer);
            glUniform1f(1, amp);
            glUniform1f(2, bgSmoothing);
            glUniform1f(4, bassAmp);
            // Updating Bass Threshold
            if (bassThresholdTimePassed > 0.03)
            {
                float deltaBass = bassThreshold-highestBass;
                if (deltaBass < 0.0008f)
                {
                    // THIS is scuffed truth table, should the bassThreshold be updated b4 or after rings are added, and how can BassThreshold be updated asyncronously???
                    // The issue is that bassThreshold is updated in a way that the peak volume isn't considered (meaning that when the peak volume occurs, a circle has likely already spawned,
                    // or the peak volume is being ignored as a result of a circle being spawned in)
                    // bass threshhold stuff
                    bassThreshold = (highestBass+bassThreshold)/2.0f;
                    if (bassRingTimePassed > 0.17)
                    {
                        printf("bassRing: %i, bassThreshold: %f, timePassed: %f, highestBassAmp: %f\n", bassRings.nextAvailable, bassThreshold, bassThresholdTimePassed, highestBass);
                        bassRings.array[bassRings.nextAvailable] = outCircRad;
                        memcpy(&bassRings.array[(bassRings.nextAvailable*SAMPDTL)+16], &freqData[0], SAMPDTL*sizeof(freqData[0]));

                        bassRings.nextAvailable = (bassRings.nextAvailable < bassRings.maxRings-1)?bassRings.nextAvailable+1:0;
                        bassRingTimePassed = 0;
                    }
                }else if (deltaBass > 0.0008f)
                {
                    bassThreshold -= 0.003f*bassThresholdTimePassed;
                    if (bassThreshold < minBassAmp)
                    {
                        bassThreshold = minBassAmp;
                    }
                }
                bassThresholdTimePassed = 0;
                highestBass = 0;
            }
            if (highestBass < bassAmp)
            {
                highestBass = bassAmp;
            }
        }

        float newPos = 75.0f+(amp/2)+(amp*128.0f*64.0f)+(bassAmp*1024.0f)+16.0f;
        if (newPos > outCircRad)
        {
            outCircRad = newPos;
        }else
        {
            outCircRad-=128.0f*deltaTime;
        }
        glUniform1f(3, outCircRad);

        // Bass Ring stuff
        for (int i=0; i<bassRings.maxRings; i++)
        {
            if (bassRings.array[i] > 0.0f)
            {
                if (bassRings.array[i] < newPos)
                {
                    bassRings.array[i] = outCircRad;
                }else{
                    
                    bassRings.array[i] += 512.0f*deltaTime;
                }
            }
            if (bassRings.array[i] > 1024.0f) {
                bassRings.array[i] = 0;
            }
        }

        glNamedBufferSubData(bassRingsBuffer, sizeof(int), sizeof(float)*(16+(SAMPDTL*16)), &bassRings.array[0]);
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

        bassRingTimePassed += deltaTime;
        bassThresholdTimePassed += deltaTime;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteBuffers(1, &ssboID);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgram);
    // Terminating all subsystems
    TerminateAudioHandler();
    filter_end();
    glfwTerminate();
    return 0;
}