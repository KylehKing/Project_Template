#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>  // Add this for std::ifstream
#include <direct.h> // Add this for _getcwd on Windows
#define NOMINMAX  // Add this before windows.h to prevent macro conflicts
#include <windows.h>  // Add this for Windows API functions
#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/ext/matrix_clip_space.hpp>
#include "helper/texture.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;
SceneBasic_Uniform::SceneBasic_Uniform() :
	tPrev(0), angle(0.0f), rotSpeed(glm::pi<float>()/8.0f),
	cameraPos(vec3(-1.0f, 1.5f, 0.5f)),
	cameraFront(vec3(1.0f, 0.0f, 0.0f)),
	cameraUp(vec3(0.0f, 1.0f, 0.0f)),
	cameraSpeed(5.0f),
	yaw(0.0f), pitch(0.0f),
	lastX(400.0f), lastY(300.0f),
	firstMouse(true),
	mouseSensitivity(0.1f),
    fogMinDist(10.0f),
    fogMaxDist(30.0f),
    fogColor(vec3(0.5f, 0.5f, 0.5f)) {
	lizard = ObjMesh::load("media/lizard_creature_28_retopology.obj", false, false);
	corridor = ObjMesh::load("media/Corridor 8 straight.obj", false, false);
	
	// Initialize key states
	for (int i = 0; i < 256; i++) {
		keys[i] = false;
	}
}

void SceneBasic_Uniform::initScene()
{
    compile();

	// Setup framebuffer
    setupFBO();

    // Load textures for lizard
    GLuint diffTex = Texture::loadTexture("media/Lizard_Base_Colour.png");
    GLuint normalTex = Texture::loadTexture("media/Lizard_NormalMap.png");
    
    // Load textures for corridor
    GLuint aluminumTex = Texture::loadTexture("media/63_aluminium scratch metal texture-seamless.jpg");
    GLuint corridorNormalTex = Texture::loadTexture("media/Main Corridor Straight Normal Map.png");
    
    // Store texture IDs for later use
    textures["lizardDiff"] = diffTex;
    textures["lizardNorm"] = normalTex;
    textures["corridorDiff"] = aluminumTex;
    textures["corridorNorm"] = corridorNormalTex;

 	glEnable(GL_DEPTH_TEST);
    model = mat4(1.0f);
    projection = mat4(1.0f);
    angle = 0.0f;

    prog.setUniform("Light.L", vec3(0.9f));
    prog.setUniform("Light.La", vec3(0.3f));
    
    prog.setUniform("Material.Kd", vec3(0.9f, 0.9f, 0.9f));
    prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
    prog.setUniform("Material.Ka", vec3(0.1f, 0.1f, 0.1f));
    prog.setUniform("Material.Shininess", 100.0f);
    
    // Initialize fog uniforms
    prog.setUniform("Fog.MaxDist", fogMaxDist);
    prog.setUniform("Fog.MinDist", fogMinDist);
    prog.setUniform("Fog.Color", fogColor);
    
    // Set edge detection threshold
    prog.setUniform("EdgeThreshold", 0.05f);

    // Set up the full-screen quad for the second pass
    GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };

    GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    };

	unsigned int handle[2];
	glGenBuffers(2, handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

	glGenVertexArrays(1, &fsQuad);
	glBindVertexArray(fsQuad);

	glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	prog.setUniform("EdgeThreshold", 0.05f);
}

void SceneBasic_Uniform::setupFBO() {
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    
    // Create the texture we'll use for rendering
    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
    
    // Create a depth buffer for the FBO
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    
    // Set up the draw buffers
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    
    // Check if the framebuffer is complete
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer is complete" << endl;
    }
	else {
		std::cout << "Framebuffer error" << result << endl;
	}
    
    // Unbind the framebuffer to return to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::pass1() {
	prog.setUniform("Pass", 1);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

	// Use the updated camera position
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	prog.setUniform("Light.Position", view*glm::vec4(10.0f*cos(angle), 1.0f, 10.0f*sin(angle), 1.0f));
	
	// Render lizard
	// Bind lizard textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures["lizardDiff"]);
	prog.setUniform("DiffTex", 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures["lizardNorm"]);
	prog.setUniform("NormalTex", 1);
	
	prog.setUniform("Material.Kd", vec3(1.0f, 1.0f, 1.0f));
	prog.setUniform("Material.Ka", vec3(0.3f, 0.3f, 0.3f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Shininess", 30.0f);
	
	model = mat4(1.0f);
	model = glm::translate(model, vec3(3.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, vec3(1.8f));
	setMatrices();
	lizard->render();
	
	// Render multiple corridors
	// Bind corridor textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures["corridorDiff"]);
	prog.setUniform("DiffTex", 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures["corridorNorm"]);
	prog.setUniform("NormalTex", 1);
	
	prog.setUniform("Material.Kd", vec3(0.8f, 0.8f, 0.8f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("Material.Shininess", 120.0f);
	
	int numCorridors = 15;
	
	for (int i = 0; i < numCorridors; i++) {
		model = mat4(1.0f);
		// Position each corridor with an x-offset of 5 units
		model = glm::translate(model, vec3(-45.0f + (i * 5.0f), 2.0f, -0.0f));
		model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, vec3(0.5f));
		setMatrices();
		corridor->render();
	}
}

void SceneBasic_Uniform::pass2() {
	// Set the second pass
	prog.setUniform("Pass", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderTex);
    
    // Bind the texture from the first pass
    glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
    
    // Create a plane in front of current camera position
    float dist = 2.0f; // Distance from camera
    model = mat4(1.0f);
    
    // Position the screen at a fixed distance in front of wherever the camera is looking
    vec3 planePos = cameraPos + cameraFront * dist;
    
    // Make the plane face the camera
    model = glm::translate(model, planePos);
    
    // Rotate to face camera
    glm::mat4 rotMat = glm::lookAt(vec3(0.0f), -cameraFront, cameraUp);
    model = model * glm::inverse(rotMat);
    
    // Scale the plane to fill the view
    float scale = dist * tan(glm::radians(35.0f)) * 2.0f;
    model = glm::scale(model, vec3(scale * ((float)width/height), scale, 1.0f));
    
    // Use the actual camera matrices
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(70.0f), (float)width/height, 0.3f, 100.0f);
    
    setMatrices();
    
	// Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6); 
    glBindVertexArray(0);
}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update(float t)
{
	float deltaT = t - tPrev;

	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;
	angle += 0.1f * deltaT;

	if (this->m_animate){
		angle += rotSpeed * deltaT;
		if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
	}
	
	// Update camera position based on key input
	updateCamera(deltaT);
}

// Process keyboard input for camera movement
void SceneBasic_Uniform::processKeyInput(unsigned char key, bool pressed) {
    // Convert to lowercase for case-insensitive comparison
    if (key >= 'A' && key <= 'Z') {
        key = key - 'A' + 'a';
    }
    
    // Update key state
    keys[key] = pressed;
}

// Process mouse movement for camera orientation
void SceneBasic_Uniform::processMouseMovement(float xpos, float ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    
    // Update cameraFront vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Update camera position based on key states
void SceneBasic_Uniform::updateCamera(float deltaTime) {
    float velocity = cameraSpeed * deltaTime;
    
    if (keys['w']) {
        cameraPos += velocity * cameraFront;
    }
    if (keys['s']) {
        cameraPos -= velocity * cameraFront;
    }
    if (keys['a']) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    }
    if (keys['d']) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    }
}

void SceneBasic_Uniform::render()
{
    // First pass - render the scene to the framebuffer
    pass1();
	glFlush();
	pass2();
}

void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
    width = w;
    height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices() {
	mat4 mv = view * model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);
	prog.setUniform("ModelMatrix", model);
}