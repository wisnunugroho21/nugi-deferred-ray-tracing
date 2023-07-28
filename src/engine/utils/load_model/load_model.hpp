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
  };

  LoadedModel loadModelFromFile(const std::string &filePath, uint32_t materialIndex, uint32_t offsetIndex);
}