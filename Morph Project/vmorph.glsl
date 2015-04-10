attribute vec4 vertices1;
attribute vec4 vertices2;
attribute vec4 vertices3;

attribute vec4 vColor, vColor2;
varying vec4 color;

uniform float time;
uniform float t;

void main()
{
    float s =  0.5 * (1.0 + sin(t * time));

	gl_Position = mix( vertices1, mix (vertices3, vertices2, s), s);	

	color = vColor;
}
