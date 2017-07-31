
attribute vec3 glVertex;

uniform mat4 glModelViewMatrix;
uniform mat4 glProjectionMatrix;

varying vec3 ls_Position;

void main(void)
{
	ls_Position = glVertex;
	gl_Position = glProjectionMatrix * glModelViewMatrix * vec4(glVertex, 1.0);
}