#version 150 core
in vec3 position;
//"in vec3 inColor;"
const vec3 inColor = vec3(0.f,0.7f,0.f);
in vec3 inNormal;
out vec3 Color;
out vec3 normal;
out vec3 pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(position,1.0);
   pos = (view * model * vec4(position,1.0)).xyz;
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);  //Or Just: normal = normalize(view*model* vec4(inNormal,0.0)).xyz; //faster, but wrong if skew or non-uniform scale in model matrix
   normal = normalize(norm4.xyz);
}