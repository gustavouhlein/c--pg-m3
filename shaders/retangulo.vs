#version 400

layout (location = 0) in vec3 vp;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(vp.x, vp.y, vp.z, 1.0);
}