#include "daq/slc/copper/FEEConfig.h"

#include <daq/slc/database/DBObject.h>

using namespace Belle2;

bool FEEConfig::read(const DBObject& obj)
{
  m_regs = RegList();
  if (obj.hasText("stream")) m_stream = obj.getText("stream");
  if (obj.hasObject("reg") && obj.hasObject("par")) {
    const DBObjectList& oregs(obj.getObjects("reg"));
    const DBObjectList& opars(obj.getObjects("par"));
    size_t len = (oregs.size() < opars.size()) ? oregs.size() : opars.size();
    for (size_t i = 0; i < len; i++) {
      const DBObject& oreg(oregs[i]);
      const DBObject& opar(opars[i]);
      Reg reg = {oreg.getText("name"), oreg.getInt("adr"),
                 oreg.getInt("size"), opar.getInt("val")
                };
      m_regs.push_back(reg);
    }
    return true;
  }
  return false;
}

