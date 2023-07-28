#version 460

#include "core/struct.glsl"

layout(location = 0) in vec3 positionFrag;
layout(location = 1) in vec2 textCoordFrag;
layout(location = 2) in vec3 normalFrag;
layout(location = 3) flat in uint materialIndexFrag;

layout(location = 0) out vec4 positionResource;
layout(location = 1) out vec4 normalResource;
layout(location = 2) out vec4 albedoColorResource;
layout(location = 3) out vec4 materialResource;

layout(set = 0, binding = 2) buffer readonly MaterialSsbo {
  Material materials[];
};

layout(set = 0, binding = 3) uniform sampler2D colorTextureSamplers[1];

void main() {
	positionResource = vec4(positionFrag, 1.0f);
  normalResource = vec4(normalFrag, 1.0f);

  if (materials[materialIndexFrag].colorTextureIndex == 0u) {
    albedoColorResource = vec4(materials[materialIndexFrag].baseColor, 1.0f);
  } else {
    albedoColorResource = texture(colorTextureSamplers[materials[materialIndexFrag].colorTextureIndex - 1u], textCoordFrag);
  }

	materialResource = vec4(materials[materialIndexFrag].metallicness, materials[materialIndexFrag].roughness, materials[materialIndexFrag].fresnelReflect, 1.0f);
}