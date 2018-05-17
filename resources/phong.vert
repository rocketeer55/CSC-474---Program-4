#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in int vertInd;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 Manim[73];

void main() {
	vec4 pos = M * Manim[vertInd] * vec4(vertPos, 1.0);


    gl_Position = P * V * pos;
}
