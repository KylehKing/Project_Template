#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include <glm/ext/matrix_clip_space.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;
SceneBasic_Uniform::SceneBasic_Uniform() :
	tPrev(0),
	plane(50.0f, 50.0f, 1, 1), 
	teapot(14, glm::mat4(1.0f)){
	//mesh = ObjMesh::load("media/pig_triangulated.obj", true);
}

void SceneBasic_Uniform::initScene()
{
    compile();
	glEnable(GL_DEPTH_TEST);
	model = mat4(1.0f);
	view = glm::lookAt(vec3(0.0f, 4.0f, 6.0f), vec3(0.0f, 0.1f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(-35.0f), vec3(1.0f, 0.0f, 0.0f));
	//model = glm::rotate(model, glm::radians(15.0f), vec3(0.0f, 1.0f, 0.0f));
	projection = mat4(1.0f);
	angle = 0.0f;

	prog.setUniform("Light.L", vec3(0.9f));
	prog.setUniform("Light.La", vec3(0.5f));
	prog.setUniform("Fog.MaxDist", 20.0f);
	prog.setUniform("Fog.MinDist", 10.0f);
	prog.setUniform("Fog.Color", vec3(0.5f, 0.5f, 0.5f));
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

void SceneBasic_Uniform::update( float t )
{
	float deltaT = t - tPrev;
	if (tPrev == 0.0f) deltaT = 0.0f;
	tPrev = t;
	angle += 0.1f * deltaT;
	if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	vec4 lightPos = vec4(10.0f*cos(angle), 10.0f, 10.0f*sin(angle), 1.0f);
	prog.setUniform("Light.Position", vec4(view*lightPos));
		
	prog.setUniform("Material.Kd", vec3(0.2f, 0.55f, 0.9f));
	prog.setUniform("Material.Ka", vec3(0.2f * 0.3f, 0.55f * 0.3f, 0.9f * 0.3f));
	prog.setUniform("Material.Ks", vec3(0.95f, 0.95f, 0.95f));
	prog.setUniform("Material.Shininess", 100.0f);
	
	float dist = 0.0f;
	for (int i = 0; i < 5; i++) {
		model = mat4(1.0f);
		model = glm::translate(model, vec3(dist*0.6f-1.0f, 0.0f, -dist));
		model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
		setMatrices();
		teapot.render();
		dist += 7.0f;
	}

	prog.setUniform("Material.Kd", vec3(0.7f, 0.7f, 0.7f));
	prog.setUniform("Material.Ks", vec3(0.0f, 0.0f, 0.0f));
	prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
	prog.setUniform("Material.Shininess", 180.0f);

	model = mat4(1.0f);
	setMatrices();
	plane.render();
}

void SceneBasic_Uniform::resize(int w, int h)
{
	glViewport(0, 0, w, h);
    width = w;
    height = h;
	projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::setMatrices() {
	mat4 mv = view*model;
	prog.setUniform("ModelViewMatrix", mv);
	prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
	prog.setUniform("MVP", projection * mv);
}