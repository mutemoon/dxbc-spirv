#include "spirv_mapping.h"

#include "../util/util_bit.h"
#include "../util/util_flags.h"

namespace dxbc_spv::spirv {

ResourceMapping::~ResourceMapping() {

}



BasicResourceMapping::BasicResourceMapping(uint32_t setIndex, uint32_t bindingOffset)
: m_setIndex(setIndex), m_bindingOffset(bindingOffset) {

}


BasicResourceMapping::~BasicResourceMapping() {

}


DescriptorBinding BasicResourceMapping::mapDescriptor(
      ir::ScalarType type, uint32_t, uint32_t regIndex) {
  /* Make binding a pure function of the D3D register namespace and index so
   * that consumers can recover (kind, regIndex) from the binding number
   * without any debug names. Each kind gets a fixed 16-slot bucket inside the
   * per-stage window selected by m_bindingOffset (VS shift 0, PS shift 100). */
  uint32_t typeBase = 0u;
  switch (type) {
    case ir::ScalarType::eCbv:         typeBase =  0u; break;
    case ir::ScalarType::eSrv:         typeBase = 16u; break;
    case ir::ScalarType::eSampler:     typeBase = 32u; break;
    case ir::ScalarType::eUav:         typeBase = 48u; break;
    case ir::ScalarType::eUavCounter:  typeBase = 64u; break;
    case ir::ScalarType::eInputTarget: typeBase = 80u; break;
    default: break;
  }

  DescriptorBinding binding = { };
  binding.set = m_setIndex;
  binding.binding = m_bindingOffset + typeBase + regIndex;
  return binding;
}


uint32_t BasicResourceMapping::mapPushData(
      ir::ShaderStageMask     stages) {
  /* Block covering all stages in the pipeline */
  if (stages != stages.first() || stages == ir::ShaderStage::eCompute)
    return 0u;

  return 64u + 32u * util::tzcnt(uint32_t(stages));
}


}
