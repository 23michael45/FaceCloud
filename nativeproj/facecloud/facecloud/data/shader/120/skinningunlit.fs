#version 120

varying  vec2 TexCoord0;
varying  vec3 Normal0;    
varying  vec2 TexCoord1;                                                                

struct VSOutput
{
    vec2 TexCoord;
    vec3 Normal;    
    vec2 TexCoord_C2;                                                               
};

                                                                                               
uniform sampler2D gColorMap;   
uniform sampler2D gDetailMap;
uniform sampler2D gMaskMap;
                                                                                 
                                
//out vec4 FragColor;
                                                                
void main()
{                                    
    VSOutput In;
    In.TexCoord = TexCoord0;
    In.TexCoord_C2 = TexCoord1;
    In.Normal   = normalize(Normal0);
                                          
                                                                                                
                                                                                     
    vec4 color = texture2D(gColorMap, In.TexCoord.xy);                               
    vec4 mask = texture2D(gMaskMap, In.TexCoord.xy);
	vec4 detail = texture2D(gDetailMap, In.TexCoord_C2.xy);
			
    float blend = (mask.x + mask.y + mask.z)/3;	
	if(blend >= 0.991)
	{
		//color = vec4(0,0,0,0);
	}
    gl_FragColor  = (color * blend + detail * (1 - blend));
	
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
