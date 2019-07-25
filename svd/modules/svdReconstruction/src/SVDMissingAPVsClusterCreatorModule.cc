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
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
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
  setDescription("This module produces clusters in the middle of a region read by a disabled APV");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("Clusters", m_storeClustersName,
           "SVDCluster collection name", string(""));

  addParam("time", m_time, "cluster time", float(0));
  addParam("timeError", m_timeError, "cluster time error", float(10));
  addParam("seedCharge", m_seedCharge, "cluster seed charge (in e-)", float(10000));
  addParam("charge", m_charge, "cluster charge (in e-)", float(20000));
  addParam("SNR", m_SNR, "cluster SNR", float(15));
  addParam("size", m_size, "cluster size", int(128));
}

void SVDMissingAPVsClusterCreatorModule::beginRun()
{

  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  m_map->prepareListOfMissingAPVs();
  B2DEBUG(29, " found a total of " << m_map->getNumberOfMissingAPVs() << " missing APVs in the channel mapping");

}

void SVDMissingAPVsClusterCreatorModule::initialize()
{
  //Register collections
  m_storeClusters.registerInDataStore(m_storeClustersName);

  //Store names to speed up creation later
  m_storeClustersName = m_storeClusters.getName();

  // Report:
  B2DEBUG(1, "SVDMissingAPVsClusterCreator Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(1, " 1. COLLECTIONS:");
  B2DEBUG(1, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
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

    float position = isU ? info.getUCellPosition(floatingStrip) : info.getVCellPosition(floatingStrip);
    const int Nstrips = 128; //mnumber of strips in each APV
    float positionError = Nstrips * pitch / sqrt(12);

    //time
    float time = m_time;
    float timeError = m_timeError;

    //charge
    float seedCharge = m_seedCharge;
    float charge = m_charge;
    float size = m_size;
    float SNR = m_SNR;

    //  Store Cluster into Datastore
    m_storeClusters.appendNew(SVDCluster(
                                sensorID, isU, position, positionError, time, timeError, charge, seedCharge, size, SNR, -1
                              ));
  }


  B2DEBUG(1, "Number of clusters: " << m_storeClusters.getEntries());
}

