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


DescriptorBinding BasicResourceMapping::mapDescriptor(ir::ScalarType, uint32_t, uint32_t) {
  DescriptorBinding binding = { };
  binding.set = m_setIndex;
  binding.binding = m_bindingOffset + m_descriptorIndex++;
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
