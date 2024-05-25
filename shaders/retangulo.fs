#version 400

uniform vec3 cor; 
out vec4 cor_frg; 

void main(){ 
	cor_frg = vec4(cor, 1.0); 
}


