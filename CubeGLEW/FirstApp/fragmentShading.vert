#version 150

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
out vec3 fL;
out vec3 fE;
out vec3 fN;
out vec2 fTexCoord;

uniform mat4 Model;
uniform mat4 Projection;
uniform mat4 View;

uniform sampler2D baseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

uniform vec4 LightPosition;


void
main()
{

	vec3 pos = (View * Model * vPosition).xyz;

	//fL = normalize(LightPosition.xyz - pos);
	fL = normalize((View * LightPosition).xyz - pos);
	fE = normalize( -pos );
	fN = normalize( Model * vec4(vNormal, 0.0)).xyz;

	fTexCoord = vTexCoord;

    gl_Position = Projection * View * Model * vPosition;
}
