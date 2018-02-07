#version 120

attribute vec3 Position;
attribute vec2 TexCoord;

uniform mat4 gWVP;

varying vec2 TexCoord0;

void main()
{
    gl_Position = gWVP * vec4(Position, 1.0);
    TexCoord0 = TexCoord;
}
