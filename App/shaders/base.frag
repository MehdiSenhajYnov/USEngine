#version 460
//! #extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 vTexCoord;

layout(set = 1, binding = 0) uniform sampler2D uTexture;

layout(location = 0) out vec4 oFragColor;

void main()
{
	oFragColor = texture(uTexture, vTexCoord);
}
