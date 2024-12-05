#pragma once
#include "Angel.h"

GLint f(GLint * original);
extern GLfloat globalTime;

typedef vec4 color4;
typedef vec4 point4;
GLuint globalTextureID0;
GLuint globalTextureID1;
GLuint globalTextureID2;
GLuint globalTextureID3;

typedef struct
{
	color4 diffuse;
	color4 specular;
	color4 ambient;
	float shininess;
} Material;


class Shape
{
public:
	vec4 position;
	vec4 velocity;
	vec3 rotation;
	vec3 rotationSpeed;
	vec3 scale;
	Material *cMaterial;
	GLuint shaderID;
	vec4 color;
	mat4 *projection;
	mat4 *camera;
	void (*shapeF)();
	void (*updateShape)(Shape* shape);
	Shape* next;
	GLuint textureID0;
	GLuint textureID1;
	GLuint textureID2;
	GLuint textureID3;


	Shape();
	void draw();
	void update();
	void append(Shape* shape);
};

void NoUpdate(Shape* shape);

Shape::Shape()
{
	position = vec4(0.0f);
	velocity = vec4(0.0f);
	rotation = vec3(0.0f);
	scale = vec3(1.0f);
	color = vec4(1, 0, 0, 1);
	rotationSpeed = vec3(0, 0, 0);
	updateShape = NoUpdate;
	next = NULL;
}

void Shape::draw()
{
	mat4 pTransform = Translate(position);
	mat4 rzTransform = RotateZ(rotation.z);
	mat4 ryTransform = RotateY(rotation.y);
	mat4 rxTransform = RotateX(rotation.x);
	mat4 sTransform = Scale(scale);
	glUseProgram(shaderID);
	
	GLint uniformLocation = glGetUniformLocation(shaderID, "uColor");
	GLint modelLocation = glGetUniformLocation(shaderID, "Model");
	glUniform4fv(uniformLocation, 1, color);
	glUniformMatrix4fv(modelLocation, 1, true, pTransform * rzTransform * ryTransform * rxTransform * sTransform);
	// Send the material to the shader
	/*
	glUniform4fv(glGetUniformLocation(shaderID, "MaterialAmbient"), 1, &cMaterial->ambient.x);// ambient.x);
	glUniform4fv(glGetUniformLocation(shaderID, "MaterialDiffuse"), 1, &cMaterial->diffuse.x);
	glUniform4fv(glGetUniformLocation(shaderID, "MaterialSpecular"), 1, &cMaterial->specular.x);
	glUniform1f(glGetUniformLocation(shaderID, "Shininess"), cMaterial->shininess);
	*/
	

	glEnable(GL_TEXTURE_2D);
	glUniform1i(glGetUniformLocation(shaderID, "baseTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	/*
	glUniform1i(glGetUniformLocation(shaderID, "normalTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureID1);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	glUniform1i(glGetUniformLocation(shaderID, "specularTexture"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureID2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	*/

	shapeF();
	if (next != NULL)
	{
		//printf("Draw next shape");
		next->draw();
	}

	glDisable(GL_TEXTURE_2D);

}

void Shape::update()
{
	updateShape(this);
	rotation += rotationSpeed;
	if (next != NULL)
	{
		//printf("Update next shape");
		next->update();
	}
}

void Shape::append(Shape* shape)
{
	if (next == NULL)
	{
		next = shape;
	}
	else
	{
		next->append(shape);
	}
	//printf("One shape added");
}

class Camera
{
public:
	GLfloat fov;
	GLfloat aspectRatio;
	GLfloat cNear;
	GLfloat cFar;
	point4 position;
	point4 objective;
	vec4 up;

	Camera();
	void use(GLuint shader);
};

Camera::Camera()
{
	fov = 45;
	aspectRatio = 1;
	cNear = 0.3;
	cFar = 20;
	position = point4(0, 0, 5, 1);
	objective = point4(0, 0, 0, 1);
	up = vec4(0, 1, 0, 0);
}

void Camera::use(GLuint shader)
{
	mat4 projection = Perspective(fov, aspectRatio, cNear, cFar);
	glUniformMatrix4fv(glGetUniformLocation(shader, "Projection"), 1, GL_TRUE, projection);
	mat4 camera = LookAt(position, objective, up);
	glUniformMatrix4fv(glGetUniformLocation(shader, "View"), 1, GL_TRUE, camera);
	glUniform4fv(glGetUniformLocation(shader, "CameraPosition"), 1, position);
}

class Viewport
{
public:
	Camera* camera;
	GLint* globalWidth, * globalHeight;
	GLint(*xPosition)(GLint standard);
	GLint(*yPosition)(GLint standard);
	GLint(*width)(GLint standard);
	GLint(*height)(GLint standard);

	Viewport(GLint(*xFunction)(GLint), GLint(*yFunction)(GLint), GLint(*widthFunction)(GLint), GLint(*heightFunction)(GLint));
	void use(GLuint shader);
};

Viewport::Viewport(GLint(*xFunction)(GLint), GLint(*yFunction)(GLint), GLint(*widthFunction)(GLint), GLint(*heightFunction)(GLint))
{
	xPosition = xFunction;
	yPosition = yFunction;
	width = widthFunction;
	height = heightFunction;
}

void Viewport::use(GLuint shader)
{
	glViewport(xPosition(*globalWidth), yPosition(*globalHeight), width(*globalWidth), height(*globalHeight));
	camera->aspectRatio = (GLfloat)width(*globalWidth) / (GLfloat)height(*globalHeight);
	camera->use(shader);
}

void NoUpdate(Shape* shape)
{

}

void Bounce(Shape* shape)
{
	if (shape->position.y > 1)
	{
		shape->velocity = -shape->velocity;
	}
	if (shape->position.y < -1)
	{
		shape->velocity = -shape->velocity;
	}
	shape->position += shape->velocity;
}

void Circle(Shape* shape)
{
	shape->position.x = sin(5 * globalTime);
	shape->position.y = cos(5 * globalTime);
}