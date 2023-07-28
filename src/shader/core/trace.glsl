// ------------- Point Light -------------

HitRecord hitPointLight(PointLight light, Ray r, float dirMin, float tMax) {
  HitRecord hit;
  hit.isHit = false;

  vec3 lightDirection = light.position - r.origin;
  vec3 lightNormal = normalize(lightDirection);

  if (dot(normalize(r.direction), lightNormal) < 0.99f || length(lightDirection) < dirMin) {
    return hit;
  }

  float t = length(lightDirection / r.direction);
  if (t > tMax) {
    return hit;
  }

  hit.isHit = true;
  hit.t = t;
  hit.point = light.position;
  hit.normal = setFaceNormal(r.direction, lightNormal);

  return hit;
}

// ------------- Triangle Light -------------

HitRecord hitTriangleLight(TriangleLight light, Ray r, float dirMin, float tMax) {
  HitRecord hit;
  hit.isHit = false;

  vec3 v0v1 = light.point1.xyz - light.point0.xyz;
  vec3 v0v2 = light.point2.xyz - light.point0.xyz;
  vec3 pvec = cross(r.direction, v0v2);
  float det = dot(v0v1, pvec);
  
#ifdef BACKFACE_CULLING
  if (det < KEPSILON) {
    return hit;
  }
#else
  if (abs(det) < KEPSILON) {
    return hit;
  }
#endif
    
  float invDet = 1.0f / det;

  vec3 tvec = r.origin - light.point0.xyz;
  float u = dot(tvec, pvec) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return hit;
  }

  vec3 qvec = cross(tvec, v0v1);
  float v = dot(r.direction, qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f) {
    return hit;
  }
  
  float t = dot(v0v2, qvec) * invDet;

  if (t > tMax || length(t * r.direction) < dirMin) {
    return hit;
  }

  vec3 outwardNormal = normalize(cross(v0v1, v0v2));

  hit.isHit = true;
  hit.t = t;
  hit.point = rayAt(r, t);
  hit.normal = setFaceNormal(r.direction, outwardNormal);

  return hit;
}

// ------------- Triangle -------------

HitRecord hitTriangle(uvec3 triIndices, Ray r, float dirMin, float tMax) {
  HitRecord hit;
  hit.isHit = false;

  vec3 v0v1 = vertices[triIndices.y].position.xyz - vertices[triIndices.x].position.xyz;
  vec3 v0v2 = vertices[triIndices.z].position.xyz - vertices[triIndices.x].position.xyz;
  vec3 pvec = cross(r.direction, v0v2);
  float det = dot(v0v1, pvec);
  
#ifdef BACKFACE_CULLING
  if (det < KEPSILON) {
    return hit;
  }
#else
  if (abs(det) < KEPSILON) {
    return hit;
  }
#endif
    
  float invDet = 1.0f / det;

  vec3 tvec = r.origin - vertices[triIndices.x].position.xyz;
  float u = dot(tvec, pvec) * invDet;
  if (u < 0.0f || u > 1.0f) {
    return hit;
  }

  vec3 qvec = cross(tvec, v0v1);
  float v = dot(r.direction, qvec) * invDet;
  if (v < 0.0f || u + v > 1.0f) {
    return hit;
  }
  
  float t = dot(v0v2, qvec) * invDet;

  if (t > tMax || length(t * r.direction) < dirMin) {
    return hit;
  }

  vec3 outwardNormal = normalize(cross(v0v1, v0v2));

  hit.isHit = true;
  hit.t = t;
  hit.point = rayAt(r, t);
  hit.normal = setFaceNormal(r.direction, outwardNormal);

  return hit;
}

// ------------- Primitive -------------

HitRecord hitPrimitive(uvec3 triIndices, Ray r, float dirMin, float tMax, uint transformIndex, uint materialIndex) {
  HitRecord hit = hitTriangle(triIndices, r, dirMin, tMax);

  if (!hit.isHit) {
    return hit;
  }

  hit.point = (transformations[transformIndex].pointMatrix * vec4(hit.point, 1.0f)).xyz;
  hit.normal = normalize(mat3(transformations[transformIndex].normalMatrix) * hit.normal);

  hit.color = materials[materialIndex].baseColor;
  hit.metallicness = materials[materialIndex].metallicness;
  hit.roughness = materials[materialIndex].roughness;
  hit.fresnelReflect = materials[materialIndex].fresnelReflect;

  return hit;
}

// ------------- Bvh -------------

bool intersectAABB(Ray r, vec3 boxMin, vec3 boxMax) {
  vec3 dirMin = (boxMin - r.origin) / r.direction;
  vec3 tMax = (boxMax - r.origin) / r.direction;
  vec3 t1 = min(dirMin, tMax);
  vec3 t2 = max(dirMin, tMax);

  // Update t2 to ensure robust ray-bounds intersection
  // t2 *= 1 + 2 * gamma(3);

  float tNear = max(max(t1.x, t1.y), t1.z);
  float tFar = min(min(t2.x, t2.y), t2.z);

  return tNear < tFar;
}

HitRecord hitPrimitiveBvh(Ray r, float dirMin, float tMax, uint firstBvhIndex, uint firstPrimitiveIndex, uint transformIndex) {
  HitRecord hit;
  hit.isHit = false;
  hit.t = tMax;

  uint stack[30];
  stack[0] = 1u;

  int stackIndex = 1;  

  r.origin = (transformations[transformIndex].pointInverseMatrix * vec4(r.origin, 1.0f)).xyz;
  r.direction = mat3(transformations[transformIndex].dirInverseMatrix) * r.direction;

  while(stackIndex > 0 && stackIndex <= 30) {
    stackIndex--;
    uint currentNode = stack[stackIndex];
    if (currentNode == 0u) {
      continue;
    }

    if (!intersectAABB(r, primitiveBvhNodes[currentNode - 1u + firstBvhIndex].minimum, primitiveBvhNodes[currentNode - 1u + firstBvhIndex].maximum)) {
      continue;
    }

    uint primIndex = primitiveBvhNodes[currentNode - 1u + firstBvhIndex].leftObjIndex;
    if (primIndex >= 1u) {
      HitRecord tempHit = hitPrimitive(primitives[primIndex - 1u + firstPrimitiveIndex].indices, r, 
        dirMin, hit.t, transformIndex, primitives[primIndex - 1u + firstPrimitiveIndex].materialIndex);

      if (tempHit.isHit) {
        hit = tempHit;
        hit.hitIndex = primIndex - 1u + firstPrimitiveIndex;
        // hit.uv = getTotalTextureCoordinate(primitives[hit.hitIndex].indices, hit.uv);
      }
    }

    primIndex = primitiveBvhNodes[currentNode - 1u + firstBvhIndex].rightObjIndex;    
    if (primIndex >= 1u) {
      HitRecord tempHit = hitPrimitive(primitives[primIndex - 1u + firstPrimitiveIndex].indices, r, 
        dirMin, hit.t, transformIndex, primitives[primIndex - 1u + firstPrimitiveIndex].materialIndex);

      if (tempHit.isHit) {
        hit = tempHit;
        hit.hitIndex = primIndex - 1u + firstPrimitiveIndex;
        // hit.uv = getTotalTextureCoordinate(primitives[hit.hitIndex].indices, hit.uv);
      }
    }

    uint bvhNode = primitiveBvhNodes[currentNode - 1u + firstBvhIndex].leftNode;
    if (bvhNode >= 1u) {
      stack[stackIndex] = bvhNode;
      stackIndex++;
    }

    bvhNode = primitiveBvhNodes[currentNode - 1u + firstBvhIndex].rightNode;
    if (bvhNode >= 1u) {
      stack[stackIndex] = bvhNode;
      stackIndex++;
    }
  }

  return hit;
}

HitRecord hitObjectBvh(Ray r, float dirMin, float tMax) {
  HitRecord hit;
  hit.isHit = false;
  hit.t = tMax;

  uint stack[30];
  stack[0] = 1u;

  int stackIndex = 1;
  while(stackIndex > 0 && stackIndex <= 30) {
    stackIndex--;
    uint currentNode = stack[stackIndex];
    if (currentNode == 0u) {
      continue;
    }

    if (!intersectAABB(r, objectBvhNodes[currentNode - 1u].minimum, objectBvhNodes[currentNode - 1u].maximum)) {
      continue;
    }

    uint objIndex = objectBvhNodes[currentNode - 1u].leftObjIndex;
    if (objIndex >= 1u) {
      HitRecord tempHit = hitPrimitiveBvh(r, dirMin, hit.t, objects[objIndex - 1u].firstBvhIndex, objects[objIndex - 1u].firstPrimitiveIndex, objects[objIndex - 1u].transformIndex);

      if (tempHit.isHit) {
        hit = tempHit;
      }
    }

    objIndex = objectBvhNodes[currentNode - 1u].rightObjIndex;
    if (objIndex >= 1u) {
      HitRecord tempHit = hitPrimitiveBvh(r, dirMin, hit.t, objects[objIndex - 1u].firstBvhIndex, objects[objIndex - 1u].firstPrimitiveIndex, objects[objIndex - 1u].transformIndex);

      if (tempHit.isHit) {
        hit = tempHit;
      }
    }

    uint bvhNode = objectBvhNodes[currentNode - 1u].leftNode;
    if (bvhNode >= 1u) {
      stack[stackIndex] = bvhNode;
      stackIndex++;
    }

    bvhNode = objectBvhNodes[currentNode - 1u].rightNode;
    if (bvhNode >= 1u) {
      stack[stackIndex] = bvhNode;
      stackIndex++;
    }
  }

  return hit;
}

// ------------- Light BVH -------------

HitRecord hitLightBvh(Ray r, float dirMin, float tMax) {
  HitRecord hit;
  hit.isHit = false;
  hit.t = tMax;

  for (uint i = 0; i < ubo.numLights; i++) {
    HitRecord tempHit = hitTriangleLight(lights[i], r, dirMin, hit.t);

    if (tempHit.isHit) {
      hit = tempHit;
      hit.hitIndex = i;
    }
  }

  return hit;
}

/* HitRecord hitLightBvh(Ray r, float dirMin, float tMax) {
  HitRecord hit;
  hit.isHit = false;
  hit.t = tMax;

  uint stack[30];
  stack[0] = 1u;

  int stackIndex = 1;
  while(stackIndex > 0 && stackIndex <= 30) {
    stackIndex--;
    uint currentNode = stack[stackIndex];
    if (currentNode == 0u) {
      continue;
    }

    if (!intersectAABB(r, lightBvhNodes[currentNode - 1u].minimum, lightBvhNodes[currentNode - 1u].maximum)) {
      continue;
    }

    uint lightIndex = lightBvhNodes[currentNode - 1u].leftObjIndex;
    if (lightIndex >= 1u) {
      HitRecord tempHit = hitPointLight(lights[lightIndex - 1u], r, dirMin, hit.t);

      if (tempHit.isHit) {
        hit = tempHit;
        hit.hitIndex = lightIndex - 1u;
      }
    }

    lightIndex = lightBvhNodes[currentNode - 1u].rightObjIndex;    
    if (lightIndex >= 1u) {
      HitRecord tempHit = hitPointLight(lights[lightIndex - 1u], r, dirMin, hit.t);

      if (tempHit.isHit) {
        hit = tempHit;
        hit.hitIndex = lightIndex - 1u;
      }
    }

    uint bvhNode = lightBvhNodes[currentNode - 1u].leftNode;
    if (bvhNode >= 1u) {
      stack[stackIndex] = bvhNode;
      stackIndex++;
    }

    bvhNode = lightBvhNodes[currentNode - 1u].rightNode;
    if (bvhNode >= 1u) {
      stack[stackIndex] = bvhNode;
      stackIndex++;
    }
  }

  return hit;
} */