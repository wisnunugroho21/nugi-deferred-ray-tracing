// ------------- Basic -------------

vec3 rayAt(Ray r, float t) {
  return r.origin + t * r.direction;
}

vec3[3] buildOnb(vec3 normal) {
  vec3 a = abs(normalize(normal).x) > 0.9 ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);

  vec3 z = normalize(normal);
  vec3 y = normalize(cross(z, a));
  vec3 x = cross(z, y);

  return vec3[3](x, y, z);
}

vec3 setFaceNormal(vec3 r_direction, vec3 outwardNormal) {
  return dot(normalize(r_direction), outwardNormal) < 0.0f ? outwardNormal : -1.0f * outwardNormal;
}

/* vec2 getTotalTextureCoordinate(uvec3 triIndices, vec2 uv) {
  float u = (1.0f - uv.x - uv.y) * vertices[triIndices.x].texel.s + uv.x * vertices[triIndices.y].texel.s + uv.y * vertices[triIndices.z].texel.s;
  float v = (1.0f - uv.x - uv.y) * vertices[triIndices.x].texel.t + uv.x * vertices[triIndices.y].texel.t + uv.y * vertices[triIndices.z].texel.t;

  return vec2(u, v);
} */