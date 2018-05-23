//+
// File : svdSetMetaModule.cc
// Description : A module to set meta from FTSW
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
  setDescription("Module to set meta from SVD");

  // Parameters
  addParam("svdRawName", m_svdRawName, "Name of the SVD Raw", string(""));
}

svdSetMetaModule::~svdSetMetaModule()
{
}

void svdSetMetaModule::initialize()
{
  B2INFO("svdSetMetaModule: initialize() is called.");

  StoreArray<RawSVD> RawSVD(m_svdRawName);  RawSVD.isRequired();
}

void svdSetMetaModule::terminate()
{
  B2INFO("svdSetMetaModule: terminate() is called.");
}


void svdSetMetaModule::beginRun()
{
  B2INFO("svdSetMetaModule: beginRun() is called.");
}

void svdSetMetaModule::endRun()
{
  B2INFO("svdSetMetaModule: endRun() is called.");
}

void svdSetMetaModule::event()
{
  StoreArray<RawSVD> rawSVD(m_svdRawName);
  StoreObjPtr<EventMetaData> evtPtr;

  for (auto& it : rawSVD) {
    B2INFO("Set time for SVD: " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
           (it.GetTTCtimeTRGType(0) & 0xF) << " Meta " << evtPtr->getEvent());
    evtPtr->setTime(it.GetTTCtime(0));
    break;
  }
}
