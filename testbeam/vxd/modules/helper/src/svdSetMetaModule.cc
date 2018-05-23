//+
// File : svdSetMetaModule.cc
// Description : A module to set meta time from FTSW (SVD)
//
// Author : Bjoern Spruck
// Date : 17 - June - 2016
//-

#include <testbeam/vxd/modules/helper/svdSetMetaModule.h>

//#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
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
  StoreArray<RawSVD> RawSVD(m_svdRawName);  RawSVD.isRequired();
}

void svdSetMetaModule::event()
{
  StoreArray<RawSVD> rawSVD(m_svdRawName);
  StoreObjPtr<EventMetaData> evtPtr;
  if (evtPtr.isValid()) {
    B2FATAL("EventMeta missing");
    return;
  }

  for (auto& it : rawSVD) {
    B2DEBUG(1, "Set time for SVD: " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " Meta " << evtPtr->getEvent());
    evtPtr->setTime((unsigned long long int)it.GetTTUtime(0) + (long)((double)it.GetTTCtime(0) / 0.127216)); // like in RawFTSWFormat
    // evtPtr->setTime(it.GetTTTimeNs(0);
    break;
  }
}
