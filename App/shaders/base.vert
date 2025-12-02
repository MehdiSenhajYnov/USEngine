#version 460
//! #extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexCoord;

layout(location = 0) out vec2 vTexCoord;

layout(push_constant) uniform constants
{
	mat4 view;
	mat4 projection;
} PushConstants;

layout(set = 0, binding = 0) uniform ModelMatrix
{
	mat4 matrix;
} model;

void main()
{
	gl_Position = PushConstants.projection * PushConstants.view * model.matrix * vec4(iPosition, 1.0f);
	vTexCoord = iTexCoord;
}
