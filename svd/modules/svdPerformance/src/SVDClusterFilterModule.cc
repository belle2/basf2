/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDClusterFilterModule.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <framework/datastore/StoreArray.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDClusterFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDClusterFilterModule::SVDClusterFilterModule() : Module()
{
  B2DEBUG(1, "Constructor");
  // Set module properties
  setDescription("generates a new StoreArray from the input StoreArray which has all specified Clusters removed");

  // Parameter definitions
  addParam("inputArrayName", m_inputArrayName, "StoreArray with the input clusters", std::string("SVDClusters"));
  addParam("outputINArrayName", m_outputINArrayName, "StoreArray with the output clusters", std::string(""));
  addParam("outputOUTArrayName", m_outputOUTArrayName, "StoreArray with the output clusters", std::string(""));
  addParam("layerNum", m_layerNum, "layer number", int(999));
  addParam("XShell", m_xShell, "X-Shell ID (+1 -> +X, -1 -> -X, 0 -> both)", int(0));
  addParam("YShell", m_yShell, "Y-Shell ID (+1 -> +Y, -1 -> -Y, 0 -> both)", int(0));


}

SVDClusterFilterModule::~SVDClusterFilterModule()
{
  B2DEBUG(1, "Destructor");
}


void SVDClusterFilterModule::initialize()
{

  B2INFO("inputArrayName: " << m_inputArrayName);
  B2INFO("outputINArrayName: " <<  m_outputINArrayName);
  B2INFO("outputOUTArrayName: " <<  m_outputOUTArrayName);
  B2INFO("layerNum: " << m_layerNum);
  B2INFO("X-Shell: " << m_xShell);
  B2INFO("Y-Shell: " << m_yShell);

  B2DEBUG(1, "Initialize");

  B2DEBUG(1, "input array name " << m_inputArrayName);


  StoreArray<SVDCluster> inputArray(m_inputArrayName);
  inputArray.isRequired();

  m_selectedClusters.registerSubset(inputArray, m_outputINArrayName);
  m_selectedClusters.inheritAllRelations();
  if (m_outputOUTArrayName != "") {
    m_notSelectedClusters.registerSubset(inputArray, m_outputOUTArrayName);
    m_notSelectedClusters.inheritAllRelations();
  }
  create_goodVxdID_set();
}


void SVDClusterFilterModule::beginRun()
{
}


void SVDClusterFilterModule::event()
{
  B2DEBUG(1, std::endl << "NEW EVENT: " << m_layerNum << std::endl);

  StoreArray<SVDCluster> inputClusterArray(m_inputArrayName);

  std::set<VxdID> goodVxdID = m_goodVxdID;

  m_selectedClusters.select([& goodVxdID](const SVDCluster * aCluster) {

    if (goodVxdID.find(aCluster->getSensorID()) == goodVxdID.end())
      B2DEBUG(10, "keeping " << aCluster->getSensorID().getLayerNumber() << "." << aCluster->getSensorID().getLadderNumber());

    return (goodVxdID.find(aCluster->getSensorID()) == goodVxdID.end());

  });

  if (m_outputOUTArrayName != "") {
    m_notSelectedClusters.select([& goodVxdID](const SVDCluster * aCluster) {

      if (goodVxdID.find(aCluster->getSensorID()) != goodVxdID.end())
        B2DEBUG(10, "rejecting " << aCluster->getSensorID().getLayerNumber() << "." << aCluster->getSensorID().getLadderNumber());

      return (goodVxdID.find(aCluster->getSensorID()) != goodVxdID.end());

    });
  }
}

void SVDClusterFilterModule::create_goodVxdID_set()
{

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    int currentLayer = layer.getLayerNumber();

    if (currentLayer != m_layerNum)
      continue;

    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {
        const VXD::SensorInfoBase& theSensorInfo = geoCache.getSensorInfo(sensor);
        const TVector3 globPos = theSensorInfo.pointToGlobal(TVector3(0, 0, 0));
        if (globPos.X()*m_xShell < 0)
          continue;

        if (globPos.Y()*m_yShell < 0)
          continue;

        m_goodVxdID.insert(sensor);

      }
  }

  B2DEBUG(10, "list of DUTs:");
  for (auto it = m_goodVxdID.begin(); it != m_goodVxdID.end(); it++)
    B2DEBUG(10, it->getLayerNumber() << "." << it->getLadderNumber() << "." << it->getSensorNumber());

}

void SVDClusterFilterModule::endRun()
{
}

void SVDClusterFilterModule::terminate()
{
}
