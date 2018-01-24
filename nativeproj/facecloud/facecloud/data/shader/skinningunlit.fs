#version 330

in vec2 TexCoord0;
in vec3 Normal0;    
in vec2 TexCoord1;                                                                

struct VSOutput
{
    vec2 TexCoord;
    vec3 Normal;    
    vec2 TexCoord_C2;                                                               
};

                                                                                               
uniform sampler2D gColorMap;   
uniform sampler2D gDetailMap;
                                                                                 
                                
out vec4 FragColor;
                                                                
void main()
{                                    
    VSOutput In;
    In.TexCoord = TexCoord0;
    In.TexCoord_C2 = TexCoord1;
    In.Normal   = normalize(Normal0);
                                          
                                                                                                
                                                                                     
    vec4 color = texture(gColorMap, In.TexCoord.xy);
	vec4 detail = texture(gDetailMap, In.TexCoord_C2.xy);
			
    float blend = 0.5;					   
    FragColor = (color * blend + detail * (1 - blend));
}
