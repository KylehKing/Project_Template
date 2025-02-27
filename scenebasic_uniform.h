#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/teapot.h"
#include <glm/glm.hpp>
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/cube.h"

#include <glm/ext/matrix_transform.hpp>
#include <map>
#include <string>

class SceneBasic_Uniform : public Scene
{
private:
    //Cube cube;
    //Teapot teapot;
    std::unique_ptr<ObjMesh> ogre;
    std::unique_ptr<ObjMesh> corridor;
   
    float rotSpeed;
	float tPrev;
    float angle;

    // Camera variables
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraSpeed;
    
    // Mouse control variables
    float yaw;
    float pitch;
    float lastX;
    float lastY;
    bool firstMouse;
    float mouseSensitivity;
    
    // Keyboard state
    bool keys[256];
    
    // Fog variables
    float fogMinDist;
    float fogMaxDist;
    glm::vec3 fogColor;

    GLSLProgram prog;
    void setMatrices();

    // Map to store texture IDs
    std::map<std::string, GLuint> textures;

    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
    
    // Camera control methods
    void processKeyInput(unsigned char key, bool pressed);
    void updateCamera(float deltaTime);
    
    // Mouse control methods
    void processMouseMovement(float xpos, float ypos);
};

#endif // SCENEBASIC_UNIFORM_H
