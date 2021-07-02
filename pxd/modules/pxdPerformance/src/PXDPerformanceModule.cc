/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdPerformance/PXDPerformanceModule.h>
//#include <pxd/utilities/PXD2TrackEvent.h>

//#include <framework/datastore/DataStore.h>
//#include <framework/datastore/StoreObjPtr.h>
//#include <framework/datastore/StoreArray.h>
//#include <framework/datastore/RelationArray.h>

//#include <framework/dataobjects/FileMetaData.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDPerformance)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDPerformanceModule::PXDPerformanceModule() :
  Module()
{
  //Set module properties
  setDescription("PXD performance module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing

  addParam("pxdClustersFromTracksName", m_storeClustersFromTracksName, "name of StoreArray with PXD track cluster",
           std::string("PXDClustersFromTracks"));
}

PXDPerformanceModule::~PXDPerformanceModule()
{
}

void PXDPerformanceModule::initialize()
{
  //Required data stores
  m_pxdClustersFromTracks.isRequired(m_storeClustersFromTracksName);
  m_tracks.isRequired(m_storeTracksName);
  m_recoTracks.isRequired(m_storeRecoTracksName);
  m_pxdIntercepts.isRequired(m_storeRecoTracksName);

  // Add new StoreArrays
  m_pxd2TrackEvents.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

}

void PXDPerformanceModule::beginRun()
{
  B2DEBUG(20, "||| PXDPerformanceModule Parameters:");
  B2DEBUG(20, "    PXDClustersFromTracksName = " << m_storeClustersFromTracksName);
  B2DEBUG(20, "    PXDInterceptsName         = " << m_storeInterceptsName);
  B2DEBUG(20, "    RecoTracksName            = " << m_storeRecoTracksName);
  B2DEBUG(20, "    TracksName                = " << m_storeTracksName);
}

void PXDPerformanceModule::event()
{
  PXD2TrackEvent eventHolder;
  B2DEBUG(20, "PXD2TrackEvent created");
  bool good = eventHolder.setValues(m_recoTracks,
                                    m_storeRecoTracksName,
                                    m_storeInterceptsName,
                                    m_storeClustersFromTracksName
                                   );
  B2DEBUG(20, "PXD2TrackEvent update status: " << good);
  if (good) {
    m_pxd2TrackEvents.appendNew(eventHolder);
    B2DEBUG(20, "PXD2TrackEvent is appended.");
  }

}

void PXDPerformanceModule::endRun()
{
}


void PXDPerformanceModule::terminate()
{
}
