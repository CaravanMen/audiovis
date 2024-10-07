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
fftwType audioBuffer[SAMPDTL];
// Global Variables
int fwidth, fheight;
pa_simple* paConn = NULL;

bool initialize_pulse_audio()
{
    // Device initialization (This is PulseAudio; Linux (Ubuntu) sound server)
    // Consider using a more widely available API, such as PortAudio...
    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_S32LE;    // 16-bit signed little-endian format
    sampleSpec.rate = SAMPLE_RATE;            // Sample rate (Hz)
    sampleSpec.channels = CHANNELS;            // Number of channels (stereo)

    pa_buffer_attr bufferAttr;
    bufferAttr.maxlength = (uint32_t) -1;
    bufferAttr.tlength = (uint32_t) SAMPDTL;
    bufferAttr.minreq = (uint32_t) -1;
    bufferAttr.prebuf = (uint32_t) SAMPDTL;
    bufferAttr.fragsize = (uint32_t) pa_usec_to_bytes(1666, &sampleSpec);

    int err = 0;
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
    glUniform2f(1, (float)width, (float)height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    return;
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
    char* src = (char*)read_file(fileName, size);
    printf("%s\n", src);
    
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


    // Initialize fftw
    filter_init(SAMPDTL, SAMPLE_RATE);
    int32_t buffer[SAMPDTL];
    int error;

    // // ---------Vertex Arrays, Buffers and Data (OLD)---------
    // // Creating vertex array object (These objects store the FORMAT of the vertex buffer objects, 
    // // so that the objects can be referenced from clearly and easily)
    // unsigned int VAO;
    // glCreateVertexArrays(1, &VAO);
    // glEnableVertexArrayAttrib(VAO, 0);
    // glVertexArrayAttribFormat(VAO, 0, 1, GL_FLOAT, false, 0);
    // // Creating Vertex Buffer Object 
    // // (This object actually stores the data - Data from within it is 
    // // referenced by using format information from the vertex array object that is bound prior)
    // unsigned int VBO;
    // glCreateBuffers(1, &VBO);
    // glNamedBufferStorage(VBO, sizeof(float)*SAMPDTL, 0, GL_DYNAMIC_STORAGE_BIT);
    // // What input will the attribute defined in the vertex array object lead to in the vertex buffer object?
    // // Binding the Vertex Buffer Object to the Vertex Array Object,\n
    // // so that the vertex array object knows which buffer object to utilize for the formatting)
    // glBindVertexArray(VAO);
    // glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(float));
    // //----------VERTEX ARRAYS, BUFFERS AND DATA END-----------

    //----------SHADER STORAGE BUFFER OBJECTS AND DATA----------
    unsigned int SSBO;
    // Creating a SSBO with id stored in SSBO
    glCreateBuffers(1, &SSBO);
    // Allocating storage (permanent allocation)
    glNamedBufferStorage(SSBO, sizeof(float)*SAMPDTL, 0, GL_DYNAMIC_STORAGE_BIT);
    // Binding buffer to 0 binding point
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, SSBO);
    //----------SHADER STORAGE BUFFER OBJECT AND DATA END----------

    // Final Initialization of GLFW
    glClearColor(0.0, 0.0, 0.0, 1.0);
    // Use the shaderProgram
    glUseProgram(shaderProgram);
    glUniform2f(1, (float)WIDTH, (float)HEIGHT);
    glDepthRange(0, 0.1);
    glEnable(GL_CLEAR);
    // glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1.0f);
    // Main while loop
    while (!glfwWindowShouldClose(window))
    {
        //---------PULSEAUDIO DATA RETRIEVAL AND PROCESSING----------
        int err = 0;
        // Retrieving information from pulseaudio sound server
        if (pa_simple_read(paConn, &buffer, sizeof(buffer), &err) < 0)
        {
            printf("[ERR] PulseAudio error: %s\n", pa_strerror(err));
        }
        // Normalizing all data points to be between 0 and 1
        GLfloat retBuffer[SAMPDTL];
        for (int i = 0; i<SAMPDTL; i++)
        {
            retBuffer[i] = ((float)buffer[i])/2147483647.0f;
        }
        GLfloat out[SAMPDTL];
        fftw_filter(retBuffer, out, MINFREQ, MAXFREQ);
        // Setting openGL vertex buffer to the retBuffer data
        glNamedBufferSubData(SSBO, 0, sizeof(out), out);
        //---------PULSEAUDIO DATA RETRIEVAL AND PROCESSING END----------
        // RENDERING THE VISUALIZER
        glClear(GL_COLOR_BUFFER_BIT);
        glLineWidth(2);
        glDrawArrays(GL_LINE_STRIP, 0, SAMPDTL);
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
            printf("[ERR] OpenGL Error: [0x%x], '%s';\n", glErr, msg);
            glErr = glGetError();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Deleting arrays, buffers and programs
    glDeleteProgram(shaderProgram);

    filter_end();
    glfwTerminate();
    return 0;
}