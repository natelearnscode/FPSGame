#version 150 core
#define NR_POINT_LIGHTS 4  

in vec3 Color;
in vec3 normal;
in vec3 pos;
in vec2 TextCoord;
in vec3 lightDir;
in vec3 pointLight;

out vec4 outColor;
const float ambient = 0.3;

uniform sampler2D Texture;

void main() {
	vec3 norm = normalize(normal);

	//Directional light contribution
	vec3 ambientC = ambient * Color;
	vec3 diffuseC = max(dot(-lightDir, norm),0.0) * Color;
	vec3 finalColor = ambientC + diffuseC; 
	vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)!
	vec3 reflectDir = reflect(viewDir,norm);
	float spec = max(dot(reflectDir,lightDir),0.0);
	if (dot(-lightDir,norm) <= 0.0) spec = 0; //No highlight if we are not facing the light
	vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
	//vec3 finalColor = ambientC + diffuseC + specC; 


	//Point Light Contribution
	vec3 pointLightDir = normalize(pointLight - pos);
	diffuseC = max(dot(pointLightDir, norm),0.0) * Color;
	spec = max(dot(reflectDir,pointLightDir),0.0);
	if (dot(-lightDir,norm) <= 0.0) spec = 0; //No highlight if we are not facing the light
	specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
	//finalColor += diffuseC;

	outColor = texture(Texture, TextCoord) * vec4(finalColor, 1.0);
}