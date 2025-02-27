#include <glad/glad.h>
#include "scene.h"
#include <GLFW/glfw3.h>
#include "glutils.h"
#include "../scenebasic_uniform.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#include <map>
#include <string>
#include <fstream>
#include <iostream>

class SceneRunner {
private:
    GLFWwindow * window;
    int fbw, fbh;
	bool debug;           // Set true to enable debug messages
    
    // Static pointer to the current scene for callbacks
    static Scene* currentScene;

public:
    SceneRunner(const std::string & windowTitle, int width = WIN_WIDTH, int height = WIN_HEIGHT, int samples = 0) : debug(true) {
        // Initialize GLFW
        if( !glfwInit() ) exit( EXIT_FAILURE );

#ifdef __APPLE__
        // Select OpenGL 4.1
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
#else
        // Select OpenGL 4.6
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
#endif
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        if(debug) 
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        if(samples > 0) {
            glfwWindowHint(GLFW_SAMPLES, samples);
        }

        // Open the window
        window = glfwCreateWindow( WIN_WIDTH, WIN_HEIGHT, windowTitle.c_str(), NULL, NULL );
        if( ! window ) {
			std::cerr << "Unable to create OpenGL context." << std::endl;
            glfwTerminate();
            exit( EXIT_FAILURE );
        }
        glfwMakeContextCurrent(window);

        // Get framebuffer size
        glfwGetFramebufferSize(window, &fbw, &fbh);

        // Load the OpenGL functions.
        if(!gladLoadGL()) { exit(-1); }

        GLUtils::dumpGLInfo();

        // Initialization
        glClearColor(0.5f,0.5f,0.5f,1.0f);
#ifndef __APPLE__
		if (debug) {
			glDebugMessageCallback(GLUtils::debugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
		}
#endif
    }

    int run(Scene & scene) {
        scene.setDimensions(fbw, fbh);
        scene.initScene();
        scene.resize(fbw, fbh);
        
        // Store the current scene for callbacks
        currentScene = &scene;
        
        // Set up keyboard callback
        glfwSetKeyCallback(window, keyCallback);
        
        // Set up mouse callback
        glfwSetCursorPosCallback(window, mouseCallback);
        
        // Capture the mouse cursor
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Enter the main loop
        mainLoop(window, scene);

#ifndef __APPLE__
		if( debug )
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "End debug");
#endif

		// Close window and terminate GLFW
		glfwTerminate();

        // Exit program
        return EXIT_SUCCESS;
    }

    static std::string parseCLArgs(int argc, char ** argv, std::map<std::string, std::string> & sceneData) {
        if( argc < 2 ) {
            printHelpInfo(argv[0], sceneData);
            exit(EXIT_FAILURE);
        }

        std::string recipeName = argv[1];
        auto it = sceneData.find(recipeName);
        if( it == sceneData.end() ) {
            printf("Unknown recipe: %s\n\n", recipeName.c_str());
            printHelpInfo(argv[0], sceneData);
            exit(EXIT_FAILURE);
        }

        return recipeName;
    }

private:
    static void printHelpInfo(const char * exeFile,  std::map<std::string, std::string> & sceneData) {
        printf("Usage: %s recipe-name\n\n", exeFile);
        printf("Recipe names: \n");
        for( auto it : sceneData ) {
            printf("  %11s : %s\n", it.first.c_str(), it.second.c_str());
        }
    }
    
    // Keyboard callback function
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        // Cast the scene to SceneBasic_Uniform to access processKeyInput
        SceneBasic_Uniform* scene = dynamic_cast<SceneBasic_Uniform*>(currentScene);
        if (scene) {
            // Convert GLFW key to ASCII
            unsigned char asciiKey = 0;
            if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
                asciiKey = 'A' + (key - GLFW_KEY_A);
            } else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
                asciiKey = '0' + (key - GLFW_KEY_0);
            } else if (key == GLFW_KEY_SPACE) {
                asciiKey = ' ';
            }
            
            if (asciiKey != 0) {
                bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
                scene->processKeyInput(asciiKey, pressed);
            }
        }
    }
    
    // Mouse callback function
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        // Cast the scene to SceneBasic_Uniform to access processMouseMovement
        SceneBasic_Uniform* scene = dynamic_cast<SceneBasic_Uniform*>(currentScene);
        if (scene) {
            scene->processMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
        }
    }

    void mainLoop(GLFWwindow * window, Scene & scene) {
        while( ! glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE) ) {
            GLUtils::checkForOpenGLError(__FILE__,__LINE__);
			
            scene.update(float(glfwGetTime()));
            scene.render();
            glfwSwapBuffers(window);

            glfwPollEvents();
			int state = glfwGetKey(window, GLFW_KEY_SPACE);
			if (state == GLFW_PRESS)
				scene.animate(!scene.animating());
        }
    }
};

// Initialize static member
Scene* SceneRunner::currentScene = nullptr;
