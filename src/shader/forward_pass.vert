#version 460

#include "core/struct.glsl"

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 textCoord;
layout(location = 2) in vec4 normal;
layout(location = 3) in uint materialIndex;
layout(location = 4) in uint transformIndex;

layout(location = 0) out vec3 positionFrag;
layout(location = 1) out vec2 textCoordFrag;
layout(location = 2) out vec3 normalFrag;
layout(location = 3) flat out uint materialIndexFrag;

layout(set = 0, binding = 0) uniform readonly RasterUbo {
	mat4 projection;
	mat4 view;
} ubo;

layout(set = 0, binding = 1) buffer readonly TransformationSsbo {
  Transformation transformations[];
};

void main() {
	vec4 positionWorld = transformations[transformIndex].pointMatrix * position;
	gl_Position = ubo.projection * ubo.view * positionWorld;

	positionFrag = positionWorld.xyz;
	textCoordFrag = textCoord.xy;
	normalFrag = normalize(mat3(transformations[transformIndex].normalMatrix) * normal.xyz);
	materialIndexFrag = materialIndex;
}