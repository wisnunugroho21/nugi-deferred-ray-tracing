#include "../../general_struct.hpp"

#include <string>
#include <memory>
#include <vector>

namespace nugiEngine
{
  struct LoadedModel
  {
    std::shared_ptr<std::vector<Primitive>> primitives;
    std::shared_ptr<std::vector<Vertex>> vertices;
    std::shared_ptr<std::vector<uint32_t>> indices;
  };

  LoadedModel loadModelFromFile(const std::string &filePath, uint32_t transformIndex, uint32_t materialIndex, uint32_t vertexOffsetIndex);
}