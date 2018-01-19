#version 330

in vec2 TexCoord0;

out vec4 FragColor;

uniform sampler2D gSampler;

void main()
{
    FragColor = texture2D(gSampler, TexCoord0.xy);
	
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}