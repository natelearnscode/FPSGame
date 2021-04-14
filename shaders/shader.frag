#version 150 core
in vec3 Color;
in vec3 normal;
in vec3 pos;
in vec3 TextCoord;
const vec3 lightDir = normalize(vec3(1,1,1));
out vec4 outColor;
const float ambient = .3;

//uniform sampler2D Texture;

void main() {
   vec3 diffuseC = Color*max(dot(lightDir,normal),0.0);
   vec3 ambC = Color*ambient;
   vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)!
   vec3 reflectDir = reflect(viewDir,normal);
   float spec = max(dot(reflectDir,-lightDir),0.0);
   if (dot(lightDir,normal) <= 0.0) spec = 0;
   vec3 specC = vec3(1.0,1.0,1.0)*pow(spec,4);
   outColor = vec4(diffuseC+ambC+specC, 1.0);
}