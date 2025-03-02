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
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

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