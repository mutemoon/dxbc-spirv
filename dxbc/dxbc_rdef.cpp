#include "dxbc_rdef.h"
#include "dxbc_container.h"
#include <cstring>

namespace dxbc_spv::dxbc {

RdefParser::RdefParser(util::ByteReader reader)
: m_reader(reader) {
}

std::string RdefParser::readStringAt(size_t offset) const {
  if (offset >= m_reader.getSize())
    return "";
  const char* strData = reinterpret_cast<const char*>(m_reader.getData(offset));
  size_t maxLen = m_reader.getSize() - offset;
  size_t len = 0;
  while (len < maxLen && strData[len] != '\0')
    len++;
  return std::string(strData, len);
}

static uint32_t readU32At(util::ByteReader reader, size_t offset) {
  if (offset + sizeof(uint32_t) > reader.getSize())
    return 0;
  uint32_t val = 0;
  std::memcpy(&val, reader.getData(offset), sizeof(uint32_t));
  return val;
}

bool RdefParser::parse() {
  if (m_reader.getSize() < 20)
    return false;

  uint32_t cbCount = readU32At(m_reader, 0);
  uint32_t cbOffset = readU32At(m_reader, 4);
  uint32_t resCount = readU32At(m_reader, 8);
  uint32_t resOffset = readU32At(m_reader, 12);
  uint32_t target = readU32At(m_reader, 16);

  uint32_t smVer = target & 0xFFFF;
  size_t varRecSize = (smVer >= 0x0500) ? 40 : 24;

  // Parse Resource Bindings
  for (uint32_t i = 0; i < resCount; ++i) {
    size_t entryOff = resOffset + i * 32;
    if (entryOff + 32 > m_reader.getSize())
      break;

    uint32_t nameOff = readU32At(m_reader, entryOff + 0);
    uint32_t type = readU32At(m_reader, entryOff + 4);
    uint32_t bindPoint = readU32At(m_reader, entryOff + 20);
    uint32_t bindCount = readU32At(m_reader, entryOff + 24);

    RdefResourceBinding res;
    res.name = readStringAt(nameOff);
    res.type = type;
    res.bindPoint = bindPoint;
    res.bindCount = bindCount;
    m_bindings.push_back(std::move(res));
  }

  // Parse Constant Buffers
  for (uint32_t i = 0; i < cbCount; ++i) {
    size_t entryOff = cbOffset + i * 24;
    if (entryOff + 24 > m_reader.getSize())
      break;

    uint32_t nameOff = readU32At(m_reader, entryOff + 0);
    uint32_t varCount = readU32At(m_reader, entryOff + 4);
    uint32_t varOffset = readU32At(m_reader, entryOff + 8);
    uint32_t size = readU32At(m_reader, entryOff + 12);
    uint32_t bufferType = readU32At(m_reader, entryOff + 20);

    RdefCBuffer cb;
    cb.name = readStringAt(nameOff);
    cb.size = size;
    cb.bufferType = bufferType;

    for (uint32_t v = 0; v < varCount; ++v) {
      size_t vOff = varOffset + v * varRecSize;
      if (vOff + varRecSize > m_reader.getSize())
        break;

      uint32_t vNameOff = readU32At(m_reader, vOff + 0);
      uint32_t vStartOff = readU32At(m_reader, vOff + 4);
      uint32_t vSize = readU32At(m_reader, vOff + 8);

      RdefVariable var;
      var.name = readStringAt(vNameOff);
      var.startOffset = vStartOff;
      var.size = vSize;
      cb.variables.push_back(std::move(var));
    }

    m_cbuffers.push_back(std::move(cb));
  }

  return true;
}

const RdefCBuffer* RdefParser::findCBufferByName(const std::string& name) const {
  for (const auto& cb : m_cbuffers) {
    if (cb.name == name)
      return &cb;
  }
  return nullptr;
}

const RdefCBuffer* RdefParser::findCBufferByBinding(uint32_t bindPoint) const {
  for (const auto& res : m_bindings) {
    if (res.type == 0 && res.bindPoint == bindPoint) {
      return findCBufferByName(res.name);
    }
  }
  return nullptr;
}

const RdefResourceBinding* RdefParser::findBinding(uint32_t type, uint32_t bindPoint) const {
  for (const auto& res : m_bindings) {
    if (res.bindPoint == bindPoint) {
      if (type == 0 && (res.type == 0 || res.type == 1)) return &res;
      if (type == 3 && res.type == 3) return &res;
      if (type == 2 && (res.type == 2 || res.type == 1 || res.type == 5 || res.type == 7)) return &res;
      if (type == 4 && (res.type == 4 || (res.type >= 6 && res.type <= 11))) return &res;
    }
  }
  return nullptr;
}

}
