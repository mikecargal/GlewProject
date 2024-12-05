#version 150

in vec4 vPosition;
in vec4 vColor;

uniform mat4 Model;
uniform mat4 Projection;
uniform mat4 View;
uniform sampler2D baseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

out vec4 pColor;


void
main()
{
    gl_Position = Projection * View * Model * vPosition;
	pColor = vColor;
}
