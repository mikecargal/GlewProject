/* sierpinski gasket using vertex buffer objects */

#include "Angel.h"
#include "Shape.h"
#include "SOIL.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
// #include <GLUT/freeglut.h>
#endif

const int NumPoints = 5000;
int Index = 0;

GLuint currentShader;
int count = 0;
GLfloat aspectRatio = 1;

// GLfloat rotation;
typedef struct
{
	int firstPoint;
	int numPoints;
	int pointsPerStrip;
	int numStrips;
	int mode;
} Sphere;

int width, height;
GLfloat xw, yw;
Shape *shapes = NULL;
Shape *current;
Shape *first, *second, *third;
vec4 yellow = vec4(1, 1, 0, 1);
vec4 white = vec4(1, 1, 1, 1);
vec4 magenta = vec4(1, 0, 1, 1);
// vec4 cyan = vec4(0, 1, 1, 1);
vec4 eye = {0, 0, 5, 1};
vec4 at = {0, 0, 0, 1};
// vec4 up = { 0.707f,0.707f,0,0 };
vec4 up = {0, 1, 0, 0};
mat4 projection = mat4(1);
mat4 camera = mat4(1);
bool animate = true;

// typedef vec4 color4;
// typedef vec4 point4;

Sphere sphere;

point4 light_position(10.0, 0.0, 10.0, 1.0);
// Initialize shader lighting parameters

color4 light_ambient(0.1, 0.1, 0.1, 1.0);
color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
color4 light_specular(1.0, 1.0, 1.0, 1.0);

color4 material_ambient(0.0215, 0.1745, 0.0215, 1.0);
color4 material_diffuse(0.07568, 0.61424, 0.07568, 1.0);
color4 material_specular(0.633, 0.727811, 0.633, 1.0);
float material_shininess = 0.6;

color4 emerald_ambient(0.0215, 0.1745, 0.0215, 1.0);
color4 emerald_diffuse(0.07568, 0.61424, 0.07568, 1.0);
color4 emerald_specular(0.633, 0.727811, 0.633, 1.0);
float emerald_shininess = 76.8;

color4 ruby_ambient(0.1745, 0.01175, 0.01175, 1.0);
color4 ruby_diffuse(0.61424, 0.04136, 0.04136, 1.0);
color4 ruby_specular(0.727811, 0.626959, 0.626959, 1.0);
float ruby_shininess = 76.8;

color4 cyan_rubber_ambient(0.0, 0.05, 0.05, 1.0);
color4 cyan_rubber_diffuse(0.4, 0.5, 0.5, 1.0);
color4 cyan_rubber_specular(0.04, 0.7, 0.7, 1.0);
float cyan_rubber_shininess = .078125 * 128;

color4 gold_ambient(0.24725, 0.1995, 0.0745, 1.0);
color4 gold_diffuse(0.75164, 0.60648, 0.22648, 1.0);
color4 gold_specular(0.628281, 0.555802, 0.366065, 1.0);
float gold_shininess = .4 * 128;

/*
typedef struct
{
	color4 diffuse;
	color4 specular;
	color4 ambient;
	float shininess;
} Material;
*/

Material emerald = {emerald_diffuse, emerald_specular, emerald_ambient, emerald_shininess};
Material ruby = {ruby_diffuse, ruby_specular, ruby_ambient, ruby_shininess};
Material cyan = {cyan_rubber_diffuse, cyan_rubber_specular, cyan_rubber_ambient, cyan_rubber_shininess};
Material gold = {gold_diffuse, gold_specular, gold_ambient, gold_shininess};

typedef struct
{
	point4 position;
	vec3 rotation;
	vec3 scale;
} Transform;

/*
typedef struct
{
	point4 position;
	color4 diffuse;
	color4 specular;
	color4 ambient;
} LightStruct;

typedef struct
{
	point4 eye;
	point4 at;
	vec4 up;
} CameraStruct;
*/

typedef struct
{
	GLfloat fov;
	GLfloat aspectRatio;
	GLfloat cNear;
	GLfloat cFar;
	point4 position;
	point4 objective;
	vec4 up;
} CameraStruct;
Material *cMaterial;

Camera *cameraOne, *cameraTwo;
Viewport *viewportOne, *viewportTwo;
GLfloat globalTime;

void drawTriangle()
{
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void drawDiamond()
{
	glDrawArrays(GL_TRIANGLES, 36, 24);
}

void drawCube()
{
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

point4 vertices[13] = {
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.0, 0.5, 0.0, 1.0), // pyramid start
	point4(0.5, -0.5, 0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, -0.5, 0.5, 1.0),
};

// Vertex positions for the pyramid
float verticesP[] = {
	// Front face
	0.0f, 0.5f, 0.0f,	// top vertex
	-0.5f, -0.5f, 0.5f, // bottom left vertex
	0.5f, -0.5f, 0.5f,	// bottom right vertex

	// Right face
	0.0f, 0.5f, 0.0f,	// top vertex
	0.5f, -0.5f, 0.5f,	// bottom left vertex
	0.5f, -0.5f, -0.5f, // bottom right vertex

	// Back face
	0.0f, 0.5f, 0.0f,	 // top vertex
	0.5f, -0.5f, -0.5f,	 // bottom left vertex
	-0.5f, -0.5f, -0.5f, // bottom right vertex

	// Left face
	0.0f, 0.5f, 0.0f,	 // top vertex
	-0.5f, -0.5f, -0.5f, // bottom left vertex
	-0.5f, -0.5f, 0.5f	 // bottom right vertex
};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0), // black
	color4(1.0, 0.0, 0.0, 1.0), // red
	color4(1.0, 1.0, 0.0, 1.0), // yellow
	color4(0.0, 1.0, 0.0, 1.0), // green
	color4(0.0, 0.0, 1.0, 1.0), // blue
	color4(1.0, 0.0, 1.0, 1.0), // magenta
	color4(1.0, 1.0, 1.0, 1.0), // white
	color4(0.0, 1.0, 1.0, 1.0)	// cyan
};

point4 points[NumPoints];
color4 colors[NumPoints];
vec3 normals[NumPoints];
vec3 tangents[NumPoints];
vec2 texCoords[NumPoints];

// quad generates two triangles for each face and assigns colors
//    to the vertices
void quad(int a, int b, int c, int d)
{
	vec3 normal = -cross(vertices[c] - vertices[a], vertices[b] - vertices[a]);
	vec3 tangent = vec3(vertices[c].x - vertices[a].x, vertices[c].y - vertices[a].y, vertices[c].z - vertices[a].z);
	printf("%f %f %f %f\n", vertices[a].x, vertices[a].y, vertices[a].z, vertices[a].w);
	printf("%f %f %f %f\n", vertices[b].x, vertices[b].y, vertices[b].z, vertices[b].w);
	printf("%f %f %f %f\n", vertices[c].x, vertices[c].y, vertices[c].z, vertices[c].w);
	printf("%f %f %f %f\n", vertices[d].x, vertices[d].y, vertices[d].z, vertices[d].w);
	printf("\nnormal: x = %f, y = %f, z = %f.\n\n", normal.x, normal.y, normal.z);
	printf("\ntangent: x = %f, y = %f, z = %f.\n\n", tangent.x, tangent.y, tangent.z);
	colors[Index] = vertex_colors[a];
	points[Index] = vertices[a];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[b];
	points[Index] = vertices[b];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[c];
	points[Index] = vertices[c];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[a];
	points[Index] = vertices[a];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[c];
	points[Index] = vertices[c];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[d];
	points[Index] = vertices[d];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
}

void tri(int a, int b, int c)
{
	vec3 normal = -cross(vertices[c] - vertices[a], vertices[b] - vertices[a]);
	vec3 tangent = vec3(vertices[c].x - vertices[a].x, vertices[c].y - vertices[a].y, vertices[c].z - vertices[a].z);
	printf("%f %f %f %f\n", vertices[a].x, vertices[a].y, vertices[a].z, vertices[a].w);
	printf("%f %f %f %f\n", vertices[b].x, vertices[b].y, vertices[b].z, vertices[b].w);
	printf("%f %f %f %f\n", vertices[c].x, vertices[c].y, vertices[c].z, vertices[c].w);
	printf("\nnormal: x = %f, y = %f, z = %f.\n\n", normal.x, normal.y, normal.z);
	printf("\ntangent: x = %f, y = %f, z = %f.\n\n", tangent.x, tangent.y, tangent.z);
	colors[Index] = vertex_colors[a];
	points[Index] = vertices[a];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[b];
	points[Index] = vertices[b];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[c];
	points[Index] = vertices[c];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[a];
	points[Index] = vertices[a];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
	colors[Index] = vertex_colors[c];
	points[Index] = vertices[c];
	normals[Index] = normal;
	tangents[Index] = tangent;
	Index++;
}

// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
	quad(9, 10, 11, 12);
	tri(8, 9, 10);
	tri(8, 10, 11);
	tri(8, 12, 11);
	tri(8, 12, 9);
}

/*
Create a sphere centered at the origin, with radius r, and precision n
Draw a point for zero radius spheres
Use CCW facet ordering
"method" is 0 for quads, 1 for triangles
(quads look nicer in wireframe mode)
Partial spheres can be created using theta1->theta2, phi1->phi2
in radians 0 < theta < 2pi, -pi/2 < phi < pi/2
*/
Sphere CreateSphere(double r, int n, int method, double theta1, double theta2, double phi1, double phi2)
{
	Sphere sphere;
	int i, j;
	vec3 e, e2;
	vec4 p, p2;
	vec4 tangent, tangent2;
	double jdivn, j1divn, idivn, dosdivn, unodivn = 1 / (double)n, ndiv2 = (double)n / 2, t1, t2, t3, cost1, cost2, cte1, cte3;
	cte3 = (theta2 - theta1) / n;
	cte1 = (phi2 - phi1) / ndiv2;
	dosdivn = 2 * unodivn;
	/* Handle special cases */
	if (r < 0)
		r = -r;
	if (n < 0)
	{
		n = -n;
		ndiv2 = -ndiv2;
	}
	if (n < 4 || r <= 0)
	{
		points[Index] = vec4(0.0, 0.0, 0.0, 1.0);
		sphere.firstPoint = Index++;
		sphere.mode = GL_POINTS;
		sphere.numPoints = 1;
		sphere.numStrips = 1;
		sphere.pointsPerStrip = 1;
		return sphere;
	}

	// printf("<<<< New >>>>\n");
	t2 = phi1;
	cost2 = cos(phi1);
	j1divn = 0;
	sphere.firstPoint = Index;
	sphere.mode = method;
	sphere.numStrips = ndiv2;
	// printf("*** Start a planet. ***\n");
	for (j = 0; j < ndiv2; j++)
	{
		t1 = t2;	// t1 = phi1 + j * cte1;
		t2 += cte1; // t2 = phi1 + (j + 1) * cte1;
		t3 = theta1 - cte3;
		cost1 = cost2; // cost1=cos(t1);
		cost2 = cos(t2);
		e.y = sin(t1);
		e2.y = sin(t2);
		p.y = r * e.y;
		p2.y = r * e2.y;
		tangent.y = -cost1;
		tangent2.y = -cost2;
		// vec4 lastPoint = p;

		// if (method == 0)
		//    glBegin(GL_QUAD_STRIP);
		// else
		//    glBegin(GL_TRIANGLE_STRIP);

		idivn = 0;
		jdivn = j1divn;
		j1divn += dosdivn; //=2*(j+1)/(double)n;
		for (i = 0; i <= n; i++)
		{
			// t3 = theta1 + i * (theta2 - theta1) / n;
			t3 += cte3;
			e.x = cost1 * cos(t3);
			// e.y = sin(t1);
			e.z = cost1 * sin(t3);
			p.x = r * e.x;
			// p.y = c.y + r * e.y;
			p.z = r * e.z;
			normals[Index] = e;
			p.w = 1.0;
			// tangent = p - lastPoint;
			// tangent = lastPoint - p;
			tangent.x = sin(t1) * cos(t3);
			tangent.z = sin(t1) * sin(t3);
			// lastPoint = p;
			tangents[Index] = vec3(tangent.x, tangent.y, tangent.z); // vec3(tempTangent.x, tempTangent.y, tempTangent.z);
			points[Index] = p;
			texCoords[Index].x = 1.0 - idivn;
			texCoords[Index].y = jdivn;
			Index++;
			// glNormal3f(e.x,e.y,e.z);
			// glTexCoord2f(idivn,jdivn);
			// glVertex3f(p.x,p.y,p.z);

			e2.x = cost2 * cos(t3);
			// e.y = sin(t2);
			e2.z = cost2 * sin(t3);
			p2.x = r * e2.x;
			// p.y = c.y + r * e.y;
			p2.z = r * e2.z;
			normals[Index] = e2;
			p2.w = 1.0;
			// tangent = p2 - lastPoint;
			// tangent = lastPoint - p2;
			tangent2.x = sin(t2) * cos(t3);
			tangent2.z = sin(t2) * sin(t3);
			// lastPoint = p2;
			tangents[Index] = vec3(tangent2.x, tangent2.y, tangent2.z); // vec3(tempTangent.x, tempTangent.y, tempTangent.z);
			points[Index] = p2;
			texCoords[Index].x = 1.0 - idivn;
			texCoords[Index].y = j1divn;
			Index++;
			// glNormal3f(e2.x,e2.y,e2.z);
			// glTexCoord2f(idivn,j1divn);
			// glVertex3f(p2.x,p2.y,p2.z);
			idivn += unodivn;
		}
		// glEnd();
	}
	sphere.numPoints = Index - sphere.firstPoint;
	sphere.pointsPerStrip = 2 * n + 2;
	return sphere;
}

void drawSphere()
{
	int currentVertex = sphere.firstPoint;
	for (int i = 0; i < sphere.numStrips; i++)
	{
		glDrawArrays(sphere.mode, currentVertex, sphere.pointsPerStrip);
		currentVertex += sphere.pointsPerStrip;
	}
}

// Viewport functions
GLint zero(GLint standard)
{
	return 0;
}

GLint one(GLint standard)
{
	return standard;
}

GLint half(GLint standard)
{
	return standard / 2;
}

GLint quarter(GLint standard)
{
	return standard / 4;
}

//----------------------------------------------------------------------------
/* This function initializes an array of 3d vectors
   and sends it to the graphics card along with shaders
   properly connected to them.
*/

void loadTexture(GLuint texture, char *filename)
{
	texture = SOIL_load_OGL_texture(
		filename,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT);
	if (texture == NULL)
	{
		printf("[Texture loader] \"%s\" failed to load!\n", filename);
	}
}

void init(void)
{
	globalTime = 0;
	// Specifiy the vertices for a triangle

	colorcube();
	double size = 1.0;
	sphere = CreateSphere(size, 36, GL_TRIANGLE_STRIP, 0.0, 2 * M_PI, -M_PI / 2, M_PI / 2);
	globalTextureID0 = SOIL_load_OGL_texture(
		"img_test.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_DDS_LOAD_DIRECT);
	// loadTexture(globalTextureID0, "img_test.png");
	if (globalTextureID0 == NULL)
	{
		printf("Globaltexture is NULL");
	}

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals) + sizeof(tangents), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals), sizeof(tangents), tangents);

	// Load shaders and use the resulting shader program
	// currentShader = InitShader( "simpleShader.vert", "simpleShader.frag" );
	// currentShader = InitShader("vertexShading.vert", "vertexShading.frag");
	currentShader = InitShader("fragmentShading.vert", "fragmentShading.frag");
	// currentShader = InitShader("shading.vert", "shading.frag");
	//  make thIS shader the current shader
	glUseProgram(currentShader);

	// Initialize the vertex position attribute from the vertex shader
	GLuint vPositionLocation = glGetAttribLocation(currentShader, "vPosition");
	glEnableVertexAttribArray(vPositionLocation);
	glVertexAttribPointer(vPositionLocation, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// Initialize the vertex color attribute from the vertex shader
	GLuint vColorPosition = glGetAttribLocation(currentShader, "vColor");
	glEnableVertexAttribArray(vColorPosition);
	glVertexAttribPointer(vColorPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	// Initialize the vertex normal attribute from the vertex shader
	GLuint vNormalPosition = glGetAttribLocation(currentShader, "vNormal");
	glEnableVertexAttribArray(vNormalPosition);
	glVertexAttribPointer(vNormalPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(colors)));

	// Initialize the vertex normal attribute from the vertex shader
	GLuint vTangentPosition = glGetAttribLocation(currentShader, "vTangent");
	glEnableVertexAttribArray(vTangentPosition);
	glVertexAttribPointer(vTangentPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(colors) + sizeof(normals)));

	GLuint vTexCoord = glGetAttribLocation(currentShader, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
						  BUFFER_OFFSET(sizeof(points) + sizeof(colors) + sizeof(normals) + sizeof(tangents)));

	// This enables hidden surface removal
	glEnable(GL_DEPTH_TEST);

	projection = Perspective(30, aspectRatio, 0.3, 20.0);
	camera = LookAt(eye, at, up);

	glClearColor(0.5, 0.5, 0.5, 1.0); // gray background

	width = 512;
	height = 512;

	first = new Shape();
	first->shaderID = currentShader;
	first->color = white;
	first->textureID0 = globalTextureID0;
	first->shapeF = drawCube;
	// first->shapeF = drawSphere;
	first->projection = &projection;
	first->camera = &camera;
	first->cMaterial = &gold;
	// first->rotationSpeed = vec3(1.0f, 0.01f, 0.05f);
	// first->rotationSpeed = vec3(0);
	first->rotationSpeed = vec3(0.2, 0.5, 0.1);
	first->velocity = vec4(0.1f, .1f, -0.10, 0);
	// first->updateShape = Bounce;

	shapes = first;
	current = first;

	first = new Shape();
	first->shaderID = currentShader;
	first->color = yellow;
	// first->shapeF = drawCube;
	first->shapeF = drawSphere;
	first->projection = &projection;
	first->camera = &camera;
	first->cMaterial = &emerald;
	// first->rotationSpeed = vec3(1.0f, 0.01f, 0.05f);
	// first->rotationSpeed = vec3(0);
	first->rotationSpeed = vec3(0.1, 0.3, 0.2);
	first->position = vec3(1, 1, 0);
	first->velocity = vec4(0, .1f, 0, 0);
	// first->updateShape = Circle;
	shapes->append(first);

	first = new Shape();
	first->shaderID = currentShader;
	first->color = yellow;
	// first->shapeF = drawCube;
	first->shapeF = drawDiamond;
	first->projection = &projection;
	first->camera = &camera;
	first->cMaterial = &ruby;
	// first->rotationSpeed = vec3(1.0f, 0.01f, 0.05f);
	// first->rotationSpeed = vec3(0);
	first->rotationSpeed = vec3(0.1, 0.3, 0.2);
	first->position = vec3(-1, 1, 0);
	first->velocity = vec4(0, .1f, 0, 0);
	// first->updateShape = Circle;
	shapes->append(first);

	// cMaterial = &emerald;
	// cMaterial = &gold;
	// cMaterial = &ruby;
	// cMaterial = &cyan;

	// shapes = first;
	cameraOne = new Camera();
	viewportOne = new Viewport(zero, zero, half, one);
	viewportOne->globalWidth = &width;
	viewportOne->globalHeight = &height;
	viewportOne->camera = cameraOne;

	cameraTwo = new Camera();
	cameraTwo->fov = 60;
	cameraTwo->position = vec4(-5, 5, -5, 1);

	viewportTwo = new Viewport(half, zero, half, one);
	viewportTwo->globalWidth = &width;
	viewportTwo->globalHeight = &height;
	viewportTwo->camera = cameraTwo;
	cameraTwo->objective = first->position;
}

//----------------------------------------------------------------------------
/* This function handles the display and it is automatically called by GLUT
   once it is declared as the display function. The application should not
   call it directly.
*/

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window

	// cameraOne->use(currentShader);
	viewportOne->use(currentShader);

	// send the light color parameters to the shader
	glUniform4fv(glGetUniformLocation(first->shaderID, "LightAmbient"), 1, light_ambient);
	glUniform4fv(glGetUniformLocation(first->shaderID, "LightDiffuse"), 1, light_diffuse);
	glUniform4fv(glGetUniformLocation(first->shaderID, "LightSpecular"), 1, light_specular);
	glUniform4fv(glGetUniformLocation(first->shaderID, "LightPosition"), 1, light_position);
	shapes->draw();

	viewportTwo->use(currentShader);
	glClear(GL_DEPTH_BUFFER_BIT);
	shapes->draw();

	// glFlush();									// flush the buffer
	glutSwapBuffers();
}

//----------------------------------------------------------------------------
/* This function handles the keyboard and it is called by GLUT once it is
   declared as the keyboard function. The application should not call it
   directly.
*/

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033: // escape key
		glutPostRedisplay();
		exit(EXIT_SUCCESS); // terminates the program
		break;
	case '1':
		current = first;
		// translation = current->position;
		// rotation = current->rotation;
		// scale = current->scale;
		break;
	case '2':
		current = second;
		// translation = current->position;
		// rotation = current->rotation;
		// scale = current->scale;
		break;
	case '3':
		current = third;
		// translation = current->position;
		// rotation = current->rotation;
		// scale = current->scale;
		break;
	case 'n':
		current = new Shape();
		current->shaderID = currentShader;
		current->color = yellow;
		current->shapeF = drawDiamond;
		break;
	case 'a':
		animate = !animate;
		break;
	case 'w':
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_POINT);
		break;
	case 'f':
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_POINT);
		break;
	case 't':
		printf("Global Clock: %f\n", globalTime);
		break;
	default:
		printf("Mouse Position: %i, %i\n", x, y);
		break;
	}
}

void specialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 - Mouse Position: %i, %i\n", x, y);
		break;
	case GLUT_KEY_UP:
		// printf("UP - Mouse Position: %i, %i\n", x, y);
		eye.y *= 1.1f;
		break;
	case GLUT_KEY_DOWN:
		eye.y *= 0.9f;
		break;
	case GLUT_KEY_RIGHT:
		// current->rotation.z -= 45;
		// rotation = RotateZ(angle);
		eye.x *= 1.1f;
		printf("RIGHT - Mouse Position: %i, %i\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		eye.x *= 0.9f;
		break;
	case GLUT_KEY_HOME:
		printf("HOME - Mouse Position: %i, %i\n", x, y);
		break;
	default:
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	xw = 2 * ((GLfloat)x / width) - 1;
	yw = 2 * (height - (GLfloat)y) / height - 1;
	printf("RMB - World Position: %f, %f\n", xw, yw);
	current->position.x = xw;
	current->position.y = yw;
	// translation = Translate(xw, yw, 0.0);
	switch (button)
	{
	case GLUT_RIGHT_BUTTON:
		// printf("RMB - Mouse Position: %i, %i\n", x, y);
		// printf("RMB - World Position: %f, %f\n", xw, yw);
		break;
	case GLUT_LEFT_BUTTON:
		// printf("LMB - Mouse Position: %i, %i\n", x, y);
		break;
	case GLUT_MIDDLE_BUTTON:
		// printf("MMB - Mouse Position: %i, %i\n", x, y);
		break;
	}
	switch (state)
	{
	case GLUT_UP:
		// printf("Button Up - Mouse Position: %i, %i\n", x, y);
		break;
	case GLUT_DOWN:
		// printf("Button Down - Mouse Position: %i, %i\n", x, y);
		break;
	}
}

void mouseMove(int x, int y)
{
	// printf("MOVING - Mouse Position: %i, %i\n", x, y);
	xw = 2 * ((GLfloat)x / width) - 1;
	yw = 2 * (height - (GLfloat)y) / height - 1;
	// printf("RMB - World Position: %f, %f\n", xw, yw);
	current->position.x = xw;
	current->position.y = yw;
	// translation = Translate(xw, yw, 0.0);
}

//----------------------------------------------------------------------------
/*
	This is the reshape callback
*/
void reshapeFunction(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	// glViewport(0, 0, width/2, height);
	//  = (GLfloat)(width/2) / (GLfloat)height;
	// cameraOne->aspectRatio = aspectRatio;
}

//----------------------------------------------------------------------------
/*
	This will change variables for animation
*/
void idleFunction()
{
	shapes->update();

	glutPostRedisplay();
}

void frameFunction(int value)
{
	globalTime += value * .001;
	if (animate)
	{
		shapes->update();
	}

	glutPostRedisplay();
	glutTimerFunc(value, frameFunction, value);
}

//----------------------------------------------------------------------------
/* This is the main function that calls all the functions to initialize
   and setup the OpenGL environment through GLUT and GLEW.
*/

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	// Initialize the display mode to a buffer with Red, Green, Blue and Alpha channels
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Set the window size
	glutInitWindowSize(512, 512);
	// Here you set the OpenGL version
	glutInitContextVersion(3, 2);
	// Use only one of the next two lines
	// glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutCreateWindow("Simple GLSL example");

	// Uncomment if you are using GLEW
	// glewInit();

	// initialize the array and send it to the graphics card
	init();

	// provide the function that handles the display
	glutDisplayFunc(display);
	// provide the functions that handles the keyboard
	glutKeyboardFunc(keyboard);
	// provide the function for special keys handling
	glutSpecialFunc(specialKeys);
	// provide the function for idle handling
	// glutIdleFunc(idleFunction);
	// provide the function for frame timing
	glutTimerFunc(33, frameFunction, 33);
	// provide the function for the mouse handling
	glutMouseFunc(mouse);
	// provide the function for the mouse moving
	glutMotionFunc(mouseMove);
	// provide the callback function for window resizing
	glutReshapeFunc(reshapeFunction);

	// Wait for input from the user (the only meaningful input is the key escape)
	glutMainLoop();
	return 0;
}