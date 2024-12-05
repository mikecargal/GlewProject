#version 150

out vec4  fColor;
in vec3 fL;
in vec3 fE;
in vec3 fN;
in vec2 fTexCoord;

uniform sampler2D baseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

uniform vec4 LightAmbient, LightDiffuse, LightSpecular;
uniform vec4 MaterialAmbient, MaterialDiffuse, MaterialSpecular;
uniform float Shininess;

vec4 colorme(vec2 texCoord)
{
	return vec4(texCoord.x, texCoord.y, 0, 1);
}

vec4 checkered(vec2 texCoord)
{
	if(texCoord.x > 0.5 && texCoord.y > 0.5 || texCoord.x < 0.5 && texCoord.y < 0.5)
	{
		return vec4(1, 1, 1, 1);
	}
	else
	{
		return vec4(0, 0, 0, 1);
	}

}

vec4 circle(vec2 texCoord)
{
	if(distance(texCoord, vec2(0.5, 0.5)) < 0.1)
	{
		return vec4(0, 0, 0, 1);
	}
	else
	{
		return vec4(1, 1, 1, 1);
	}
}

vec4 donut(vec2 texCoord)
{
	if(distance(texCoord, vec2(0.5, 0.5)) < 0.5 && distance(texCoord, vec2(0.5, 0.5)) > 0.4)
	{
		return vec4(0, 0, 0, 1);
	}
	else
	{
		return vec4(1, 1, 1, 1);
	}
}

vec3 disturbeNormal(vec2 texCoord)
{
	return vec3(0, 0, 1);
}



void
main()
{
	vec3 L = normalize(fL);
	vec3 E = normalize(fE);
	vec3 N = normalize(fN);
	vec3 H = normalize( L + E );

	N = disturbeNormal(fTexCoord);

	vec4 ambient = MaterialAmbient * LightAmbient *  texture2D(baseTexture, fTexCoord);
	float Kd = max( dot(L, N), 0.0);
	//vec4 diffuse = Kd * LightDiffuse * MaterialDiffuse * texture2D(baseTexture, fTexCoord);
	vec4 diffuse = texture2D(baseTexture, fTexCoord);
	float Ks = pow( max(dot(N, H), 0.0), Shininess );
	vec4 specular = Ks * MaterialSpecular * LightSpecular * texture2D(baseTexture, fTexCoord);

	if( dot(L, N) < 0.0)
	{
		specular = vec4(0.0, 0.0, 0.0, 1.0);
	}
	fColor = ambient + diffuse + specular;
	fColor.a = 1.0;
}
