float machineEpsilon = 1.19209e-07;

float nextFloatUp(float v) {
  // Handle infinity and negative zero for _NextFloatUp()_
  if (isinf(v) && v > 0.0f)
    return v;

  if (v == -0.0f)
    v = 0.0f;

  // Advance _v_ to next higher float
  uint ui = uint(v);

  if (v >= 0u)
    ++ui;
  else
    --ui;

  return float(ui);
}

float nextFloatDown(float v) {
  // Handle infinity and positive zero for _NextFloatDown()_
  if (isinf(v) && v < 0.0f)
    return v;
    
  if (v == 0.0f)
    v = -0.0f;

  uint ui = uint(v);

  if (v > 0u)
    --ui;
  else
    ++ui;

  return float(ui);
}

float gamma(int n) {
  return (n * machineEpsilon) / (1 - n * machineEpsilon);
}