//+
// File : svdSetMetaModule.cc
// Description : A module to set meta time from FTSW (SVD)
//
// Author : Bjoern Spruck
// Date : 17 - June - 2016
//-

#include <testbeam/vxd/modules/helper/svdSetMetaModule.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(svdSetMeta)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// Implementations
svdSetMetaModule::svdSetMetaModule() : Module()
{
  // Module description
  setDescription("Module to set meta time from SVD");

  // Parameters
  addParam("svdRawName", m_svdRawName, "Name of the SVD Raw", string(""));
}

void svdSetMetaModule::initialize()
{
  m_evtPtr.isRequired();
  m_rawSVD.isRequired(m_svdRawName);
}

void svdSetMetaModule::event()
{
  if (not m_evtPtr.isValid()) {
    B2FATAL("EventMeta missing in this event");
    return;
  }

  if (m_evtPtr->getTime() != 0) return; // time already set

  for (auto& it : m_rawSVD) {
    B2DEBUG(1, "Set time for SVD: " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " Meta " << m_evtPtr->getEvent());
    m_evtPtr->setTime((unsigned long long int)it.GetTTUtime(0) + (long)((double)it.GetTTCtime(0) / 0.127216)); // like in RawFTSWFormat
    // evtPtr->setTime(it.GetTTTimeNs(0); // if update to RawCopper happened
    break;
  }
}
