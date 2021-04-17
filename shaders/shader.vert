#version 150 core
in vec3 position;
//const vec3 inColor = vec3(1.0f,1.0f,1.0f);
in vec3 inNormal;
in vec2 inTextCoord;
out vec3 Color;
out vec3 normal;
out vec3 pos;
out vec2 TextCoord;
out vec3 lightDir;
out vec3 pointLight;
const vec3 inLightDir = normalize(vec3(0,-1,0));
const vec3 inPointLight = vec3(1,1,1);

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 inColor;

void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(position,1.0);
   pos = (view * model * vec4(position,1.0)).xyz;
   lightDir = vec3(view * vec4(inLightDir,0.0)); //It's a vector!
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);  //Or Just: normal = normalize(view*model* vec4(inNormal,0.0)).xyz; //faster, but wrong if skew or non-uniform scale in model matrix
   normal = normalize(norm4.xyz);
   pointLight = vec3(view * vec4(inPointLight, 1.0));
   TextCoord = inTextCoord;
}