#version 150 

// per-fragment interpolated values from the vertex shader
in  vec3 normal;
in  vec3 LightDir;
in  vec3 CameraDir;

out vec4 fColor;

uniform vec4 LightAmbient, LightDiffuse, LightSpecular;
uniform vec4 MaterialAmbient, MaterialDiffuse, MaterialSpecular;
uniform float Shininess;

uniform sampler2D baseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

void main() 
{
	vec4 specular = vec4(0.0);
	float nxDir = max(0.0, dot(normalize(normal), normalize(LightDir)));
	vec4 diffuse = LightDiffuse * MaterialDiffuse * nxDir;
	if(nxDir > 0.0)
	{
		vec3 halfVector = normalize(LightDir + CameraDir);
		float nxHalf = max(0.0, dot(normal, halfVector));
		float specularPower = pow(nxHalf, Shininess);
		specular = LightSpecular * MaterialSpecular * specularPower;
	}
	vec4 ambient = LightAmbient * MaterialAmbient;
	fColor =  ambient + diffuse + specular;
} 

