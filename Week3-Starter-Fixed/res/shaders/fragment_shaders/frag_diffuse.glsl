#version 430

#include "../fragments/fs_common_inputs.glsl"

// We output a single color to the color buffer
layout (location = 0) in vec3 inPos; 
layout (location = 1) in vec3 aNormal;
layout(location = 0) out vec4 frag_color;

struct Material {
	sampler2D Diffuse;
	float     Shininess;
	float	  ambientstrength;
};

uniform vec3 lightPos;
uniform Material u_Material;

#include "../fragments/multiple_point_lights.glsl"


#include "../fragments/frame_uniforms.glsl"
#include "../fragments/color_correction.glsl"

//Diffuse Shader

void main() {
	// Normalize our input normal
	vec3 normal = normalize(inNormal);

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	float ambSTR = 0.1;
	vec3 COLORL = vec3(1.0, 1.0, 1.0);

    vec3 lighting = normalize(lightPos - inPos);  

	vec3 ambient = ambSTR * COLORL * inColor * textureColor.rgb;
	float d = max(dot(normal, lighting), 0.0);
    vec3 diffuse = d * lighting * inColor * textureColor.rgb;
	frag_color = vec4(ambient + diffuse, 1.0);
//diffuse + ambient
}

