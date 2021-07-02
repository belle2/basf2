/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDMissingAPVsClusterCreatorModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDMissingAPVsClusterCreator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

std::string Belle2::SVD::SVDMissingAPVsClusterCreatorModule::m_xmlFileName = std::string("SVDChannelMapping.xml");

SVDMissingAPVsClusterCreatorModule::SVDMissingAPVsClusterCreatorModule()
  : Module()
  , m_mapping(m_xmlFileName)
{
  //Set module properties
  setDescription("This module produces clusters in the middle of a region read by a disabled APV. It can be run only after the SVDClusterizer because it does not register the SVDClusters StoreArray in the DataStore, but only add clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Clusters", m_storeClustersName,
           "SVDCluster collection name", string(""));

  addParam("time", m_time, "fake-cluster time", float(0));
  addParam("timeError", m_timeError, "fake-cluster time error", float(10));
  addParam("seedCharge", m_seedCharge, "fake-cluster seed charge (in e-)", float(10000));
  addParam("charge", m_charge, "fake-cluster charge (in e-)", float(20000));
  addParam("SNR", m_SNR, "fake-cluster SNR", float(15));
  addParam("size", m_size, "fake-cluster size", int(128));
  addParam("firstFrame", m_firstFrame, "first frame, needed to build the fake-cluster", int(0));
  addParam("nFakeClusters", m_nFakeClusters, "number of fake clusters equally distributed in the dead area", int(4));
}

void SVDMissingAPVsClusterCreatorModule::beginRun()
{

  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  m_map->prepareListOfMissingAPVs();
  B2DEBUG(29, " found a total of " << m_map->getNumberOfMissingAPVs() << " missing APVs in the channel mapping");

}

void SVDMissingAPVsClusterCreatorModule::initialize()
{
  //Register clusters only if the SVDClusters are not already registered
  // this makes some test fail. Commented out until we find a bug-fix.
  //  if(!m_storeClusters.isValid())
  //    m_storeClusters.registerInDataStore(m_storeClustersName);

  //Store names to speed up creation later
  m_storeClustersName = m_storeClusters.getName();

  // Report:
  B2DEBUG(29, "SVDMissingAPVsClusterCreator Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(29, " 1. COLLECTIONS:");
  B2DEBUG(29, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
}



void SVDMissingAPVsClusterCreatorModule::event()
{

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  for (int nAPV = 0; nAPV < m_map->getNumberOfMissingAPVs(); nAPV++) {

    SVDOnlineToOfflineMap::missingAPV tmp_missingAPV = (m_map->m_missingAPVs).at(nAPV);

    VxdID sensorID = tmp_missingAPV.m_sensorID;
    bool isU = tmp_missingAPV.m_isUSide;

    //position
    const VXD::SensorInfoBase& info = geo.getSensorInfo(sensorID);
    float floatingStrip = tmp_missingAPV.m_halfStrip;
    double pitch = isU ? info.getUPitch(0) : info.getVPitch(0);

    float halfChip_position = isU ? info.getUCellPosition(floatingStrip) : info.getVCellPosition(floatingStrip);
    const int Nstrips = 128; //number of strips in each APV
    float positionError = Nstrips * pitch / sqrt(12);

    //time
    float time = m_time;
    float timeError = m_timeError;

    //charge
    float seedCharge = m_seedCharge;
    float charge = m_charge;
    float size = m_size;
    float SNR = m_SNR;

    // Built m_nFakeClusters equally distributed
    float fakeCluster_width = pitch * Nstrips / m_nFakeClusters;
    float chip_halfWidth = pitch * Nstrips / 2;
    float firstStrip_position = halfChip_position - chip_halfWidth;

    //position of the first fake cluster:
    float fakeCluster_position = firstStrip_position + fakeCluster_width / 2;
    //store first fake cluster:
    m_storeClusters.appendNew(sensorID, isU, fakeCluster_position, positionError, time, timeError, charge, seedCharge, size,
                              SNR, m_firstFrame);
    //  Store all other Fake Clusters into DataStore
    for (int i = 1; i < m_nFakeClusters; i++) {
      fakeCluster_position = fakeCluster_position + fakeCluster_width;
      m_storeClusters.appendNew(sensorID, isU, fakeCluster_position, positionError, time, timeError, charge, seedCharge, size,
                                SNR, m_firstFrame);
    }
  }
  B2DEBUG(29, "Number of clusters: " << m_storeClusters.getEntries());
}

