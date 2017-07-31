
#ifdef GL_ES
precision highp float;
#endif

uniform vec3 glColour;

void main(void)
{
	gl_FragColor = vec4(glColour, 1);
}
