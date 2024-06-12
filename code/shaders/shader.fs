#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;  
in vec3 Normal;

uniform sampler2D texture_basecolor1;
uniform sampler2D texture_metallicRoughness1;
uniform vec3 emissiveFactor;
uniform sampler2D texture_emissiveTexture1;
uniform sampler2D texture_normal1;

void main()
{
	vec4 basecolor = texture(texture_basecolor1, TexCoords);
	vec4 metallicRoughness = texture(texture_metallicRoughness1, TexCoords);

	vec3 emissive = emissiveFactor * texture(texture_emissiveTexture1, TexCoords).rgb;

	// 从法线贴图范围[0,1]获取法线
	vec3 normal = texture(texture_normal1, TexCoords).rgb;
	// 将法线向量转换为范围[-1,1]
	normal = normalize(normal * 2.0 - 1.0);  

	FragColor = vec4(mix(basecolor.rgb, metallicRoughness.rgb, 0.1)+emissive,1.0);
}