#version 120

varying vec2 TexCoord0;

//out vec4 FragColor;

uniform sampler2D gSampler;

void main()
{
    gl_FragColor = texture2D(gSampler, TexCoord0.xy);
	
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}