// ------------- Point Light ------------- 

vec3 pointLightFaceNormal(PointLight light, vec3 rayDirection, vec3 origin) {
  vec3 outwardNormal = normalize(light.position - origin);
  return setFaceNormal(rayDirection, outwardNormal);
}

float pointLightArea() {
  return 1.0f;
}

vec3 pointLightRandomDirection(PointLight light, vec3 origin) {
  return light.position - origin;
}

// ------------- Area Light -------------

vec3 areaLightFaceNormal(AreaLight light, vec3 rayDirection) {
  vec3 v0v1 = light.point1 - light.point0;
  vec3 v0v2 = light.point2 - light.point0;

  vec3 outwardNormal = normalize(cross(v0v1, v0v2));
  return setFaceNormal(rayDirection, outwardNormal);
}

float areaLightArea(AreaLight light) {
  vec3 v0v1 = light.point1 - light.point0;
  vec3 v0v2 = light.point2 - light.point0;

  vec3 pvec = cross(v0v1, v0v2);
  return 0.5 * sqrt(dot(pvec, pvec)); 
}

vec3 areaLightRandomDirection(AreaLight light, vec3 origin, uint additionalRandomSeed) {
  vec3 a = light.point1 - light.point0;
  vec3 b = light.point2 - light.point0;

  float u1 = randomFloat(additionalRandomSeed);
  float u2 = randomFloat(additionalRandomSeed + 1);

  if (u1 + u2 > 1) {
    u1 = 1 - u1;
    u2 = 1 - u2;
  }

  vec3 randomLight = u1 * a + u2 * b + light.point0;
  return randomLight - origin;
}

// ------------- Triangle -------------

vec3 triangleOffsetRayOrigin(vec3 hitPoint, vec3 point0, vec3 point1, vec3 point2,
  vec2 hitUV, vec3 triangleNormal, vec3 rayDirection) 
{
  vec3 hitPointError = abs(hitUV.s * point0) + abs(hitUV.t * point1) + abs((1.0f - hitUV.s - hitUV.t) * point2);
  hitPointError *= gamma(7);

  float d = dot(abs(triangleNormal), hitPointError);
  vec3 offset = d * triangleNormal;

  if (dot(rayDirection, triangleNormal) < 0.0f) {
    offset *= -1.0f;
  }

  vec3 po = hitPoint + offset;

  if (offset.x > 0) po.x = nextFloatUp(po.x);
  else if (offset.x < 0) po.x = nextFloatDown(po.x);

  if (offset.y > 0) po.y = nextFloatUp(po.y);
  else if (offset.y < 0) po.y = nextFloatDown(po.y);

  if (offset.z > 0) po.z = nextFloatUp(po.z);
  else if (offset.z < 0) po.z = nextFloatDown(po.z);

  return po;
}

vec3 triangleFaceNormal(uvec3 triIndices, vec3 rayDirection) {
  vec3 v0v1 = vertices[triIndices.y].position.xyz - vertices[triIndices.x].position.xyz;
  vec3 v0v2 = vertices[triIndices.z].position.xyz - vertices[triIndices.x].position.xyz;

  vec3 outwardNormal = normalize(cross(v0v1, v0v2));
  return setFaceNormal(rayDirection, outwardNormal);
}

float triangleArea(uvec3 triIndices) {
  vec3 v0v1 = vertices[triIndices.y].position.xyz - vertices[triIndices.x].position.xyz;
  vec3 v0v2 = vertices[triIndices.z].position.xyz - vertices[triIndices.x].position.xyz;

  vec3 pvec = cross(v0v1, v0v2);
  return 0.5 * sqrt(dot(pvec, pvec)); 
}

vec3 triangleRandomDirection(uvec3 triIndices, vec3 origin, uint additionalRandomSeed) {
  vec3 a = vertices[triIndices.y].position.xyz - vertices[triIndices.x].position.xyz;
  vec3 b = vertices[triIndices.z].position.xyz - vertices[triIndices.x].position.xyz;

  float u1 = randomFloat(additionalRandomSeed);
  float u2 = randomFloat(additionalRandomSeed + 1);

  if (u1 + u2 > 1) {
    u1 = 1 - u1;
    u2 = 1 - u2;
  }

  vec3 randomTriangle = u1 * a + u2 * b + vertices[triIndices.x].position.xyz;
  return randomTriangle - origin;
}