#version 120                                                                        
                                                                                    
attribute   vec3 Position;                                             
attribute   vec2 TexCoord;                                             
attribute   vec3 Normal;                                               
attribute   ivec4 BoneIDs;
attribute   vec4 Weights;
attribute   vec2 TexCoord_C2;  

varying  vec2 TexCoord0;
varying  vec2 TexCoord1;
varying  vec3 Normal0;                                                                   
varying  vec3 WorldPos0;                                                                 

const int MAX_BONES = 240;
uniform float gYOffset;
uniform vec2 gUVSize;
uniform mat4 gWVP;
uniform mat4 gWorld;
uniform mat4 gBones[MAX_BONES];


void main()
{       
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

    vec4 PosL    = BoneTransform * vec4(Position, 1.0);
    gl_Position  = gWVP * PosL;
    TexCoord0    = TexCoord;
	
	vec2 C2 = vec2(-PosL.x / gUVSize.x - 0.5,-PosL.y / gUVSize.y - gYOffset);
	
    TexCoord1    = C2;
    vec4 NormalL = BoneTransform * vec4(Normal, 0.0);
    Normal0      = (gWorld * NormalL).xyz;
    WorldPos0    = (gWorld * PosL).xyz;   


	gl_Position = 	gWVP * vec4((fract(TexCoord.x) - 0.5) * -gUVSize.x,(fract(TexCoord.y) - 0.5) * -gUVSize.y,0,1);
}
