// VERTEX SHADER

#version 330

uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

out vec4 color;
out vec2 texCoord0;
out vec4 position;
out vec3 normal;


// Light declarations
struct AMBIENT
{	
	int on;
	vec3 color;
};
uniform AMBIENT lightAmbient, lightEmissive;

//Calculates the Ambient Light
vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

struct DIRECTIONAL
{	
	int on;
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}


void main(void) 
{
  position = matrixModelView * vec4(aVertex, 1.0);
  gl_Position = matrixProjection * position;

// calculate light
	color = vec4(0, 0, 0, 0);
	normal = normalize(mat3(matrixModelView) * aNormal);

	texCoord0 = aTexCoord;

	//Contains The Colour Of The Light
	if (lightAmbient.on == 1)
		color += AmbientLight(lightAmbient);

	if (lightEmissive.on == 1)
		color += AmbientLight(lightEmissive);

	if (lightDir.on == 1)
		color += DirectionalLight(lightDir);


}
