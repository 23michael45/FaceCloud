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
uniform sampler2D gMaskMap;
                                                                                 
                                
out vec4 FragColor;
                                                                
void main()
{                                    
    VSOutput In;
    In.TexCoord = TexCoord0;
    In.TexCoord_C2 = TexCoord1;
    In.Normal   = normalize(Normal0);
                                          
                                                                                                
                                                                                     
    vec4 color = texture(gColorMap, In.TexCoord.xy);                           
    vec4 mask = texture(gMaskMap, In.TexCoord.xy);
	vec4 detail = texture(gDetailMap, In.TexCoord_C2.xy);     
	
	
	
	
	
    float colormask = (detail.x + detail.y + detail.z)/3;	
	if(colormask == 0)
	{
		
		FragColor = vec4(0,0,0,0);
		
	}
	else{
		float dist = sqrt((In.TexCoord.x - 0.5) * (In.TexCoord.x - 0.5) + (In.TexCoord.y - 0.25) * (In.TexCoord.y - 0.25));
	
		if(dist < 0.2)
		{
			
			FragColor = vec4(1,1,1,1);
			
		}
		else
		{
			
			FragColor = vec4(0,0,0,0);
		}
	}
}
