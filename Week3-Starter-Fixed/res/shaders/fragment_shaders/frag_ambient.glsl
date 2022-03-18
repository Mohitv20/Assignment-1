#version 430

#include "../fragments/fs_common_inputs.glsl"

// We output a single color to the color buffer
layout(location = 0) out vec4 frag_color;

struct Material {
	sampler2D Diffuse;
	float     Shininess;
	float	  ambientstrength;
};

uniform Material u_Material;

#include "../fragments/multiple_point_lights.glsl"


#include "../fragments/frame_uniforms.glsl"
#include "../fragments/color_correction.glsl"

//Ambience Shader

void main() {
	// Normalize our input normal
	vec3 normal = normalize(inNormal);

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(u_Material.Diffuse, inUV);

	float ambSTR = 0.1;
	vec3 COLORL = vec3(1.0, 1.0, 1.0);

	vec3 ambient = ambSTR * COLORL * inColor * textureColor.rgb;
	frag_color = vec4(ambient, 1.0);
}