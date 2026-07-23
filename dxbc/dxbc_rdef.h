#pragma once

#include <string>
#include <vector>
#include <map>
#include "../util/util_byte_stream.h"

namespace dxbc_spv::dxbc {

struct RdefVariable {
  std::string name;
  uint32_t startOffset = 0u;
  uint32_t size = 0u;
};

struct RdefCBuffer {
  std::string name;
  uint32_t size = 0u;
  uint32_t bufferType = 0u;
  std::vector<RdefVariable> variables;
};

struct RdefResourceBinding {
  std::string name;
  uint32_t type = 0u;        // 0 = CBuffer, 2 = Texture, 3 = Sampler, 4 = UAV
  uint32_t bindPoint = 0u;   // Register index (e.g. 0 for cb0/t0/s0)
  uint32_t bindCount = 0u;
};

class RdefParser {
public:
  RdefParser() = default;
  explicit RdefParser(util::ByteReader reader);

  bool parse();

  const RdefCBuffer* findCBufferByName(const std::string& name) const;
  const RdefCBuffer* findCBufferByBinding(uint32_t bindPoint) const;
  const RdefResourceBinding* findBinding(uint32_t type, uint32_t bindPoint) const;

  const std::vector<RdefCBuffer>& getCBuffers() const { return m_cbuffers; }
  const std::vector<RdefResourceBinding>& getBindings() const { return m_bindings; }

private:
  util::ByteReader m_reader;
  std::vector<RdefCBuffer> m_cbuffers;
  std::vector<RdefResourceBinding> m_bindings;

  std::string readStringAt(size_t offset) const;
};

}
