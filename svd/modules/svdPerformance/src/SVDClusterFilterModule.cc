/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
  addParam("layerNum", m_layerNum, "layer number (0 -> no selection)", int(0));
  addParam("XShell", m_xShell, "X-Shell ID (+1 -> +X, -1 -> -X, 0 -> both)", int(0));
  addParam("YShell", m_yShell, "Y-Shell ID (+1 -> +Y, -1 -> -Y, 0 -> both)", int(0));
  addParam("minSNR", m_minClSNR, "minimum cluster SNR", float(0));

}

SVDClusterFilterModule::~SVDClusterFilterModule()
{
  B2DEBUG(1, "Destructor");
}


void SVDClusterFilterModule::initialize()
{

  B2DEBUG(10, "inputArrayName: " << m_inputArrayName);
  B2DEBUG(1, "outputINArrayName: " <<  m_outputINArrayName);
  B2DEBUG(1, "outputOUTArrayName: " <<  m_outputOUTArrayName);
  B2DEBUG(1, "layerNum: " << m_layerNum);
  B2DEBUG(1, "X-Shell: " << m_xShell);
  B2DEBUG(1, "Y-Shell: " << m_yShell);
  B2DEBUG(1, "minSNR: " << m_minClSNR);


  StoreArray<SVDCluster> inputArray(m_inputArrayName);
  inputArray.isRequired();

  m_selectedClusters.registerSubset(inputArray, m_outputINArrayName);
  m_selectedClusters.inheritAllRelations();
  if (m_outputOUTArrayName != "") {
    m_notSelectedClusters.registerSubset(inputArray, m_outputOUTArrayName);
    m_notSelectedClusters.inheritAllRelations();
  }
  create_outVxdID_set();
}


void SVDClusterFilterModule::beginRun()
{
}


void SVDClusterFilterModule::event()
{
  B2DEBUG(1, std::endl << "NEW EVENT: " << m_layerNum << std::endl);

  StoreArray<SVDCluster> inputClusterArray(m_inputArrayName);

  m_selectedClusters.select([this](const SVDCluster * aCluster) {


    bool inVxdID = (this->m_outVxdID).find(aCluster->getSensorID()) == (this->m_outVxdID).end();
    bool aboveSNR = aCluster->getSNR() > this->m_minClSNR;

    if (!inVxdID)
      B2DEBUG(10, "not selected VxdID (OUT): " << aCluster->getSensorID().getLayerNumber() << "." <<
              aCluster->getSensorID().getLadderNumber());

    if (!aboveSNR)
      B2DEBUG(10, "below SNR (OUT):" << aCluster->getSNR());

    if (aboveSNR && inVxdID)
      B2DEBUG(10, "keeping " << aCluster->getSensorID().getLayerNumber() << "." << aCluster->getSensorID().getLadderNumber() << " SNR = "
              << aCluster->getSNR());

    return (inVxdID && aboveSNR);

  });

  if (m_outputOUTArrayName != "") {
    m_notSelectedClusters.select([this](const SVDCluster * aCluster) {

      bool inVxdID = (this->m_outVxdID).find(aCluster->getSensorID()) == (this->m_outVxdID).end();
      bool aboveSNR = aCluster->getSNR() > this->m_minClSNR;

      if (inVxdID)
        B2DEBUG(10, "selected VxdID (IN): " << aCluster->getSensorID().getLayerNumber() << "." <<
                aCluster->getSensorID().getLadderNumber());

      if (aboveSNR)
        B2DEBUG(10, "above SNR (IN):" << aCluster->getSNR());

      if (!(aboveSNR && inVxdID))
        B2DEBUG(10, "rejecting " << aCluster->getSensorID().getLayerNumber() << "." << aCluster->getSensorID().getLadderNumber() <<
                " SNR = " << aCluster->getSNR());

      return (!(inVxdID && aboveSNR));

    });
  }
}

void SVDClusterFilterModule::create_outVxdID_set()
{

  VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

  for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD)) {
    int currentLayer = layer.getLayerNumber();

    bool layer_IN = true;

    if ((m_layerNum != 0) && (currentLayer != m_layerNum))
      layer_IN = false;

    for (auto ladder : geoCache.getLadders(layer))
      for (Belle2::VxdID sensor :  geoCache.getSensors(ladder)) {

        bool xShell_IN = true;
        bool yShell_IN = true;

        const VXD::SensorInfoBase& theSensorInfo = geoCache.getSensorInfo(sensor);
        const TVector3 globPos = theSensorInfo.pointToGlobal(TVector3(0, 0, 0));
        if (globPos.X()*m_xShell < 0)
          xShell_IN = false;

        if (globPos.Y()*m_yShell < 0)
          xShell_IN = false;

        if (!(layer_IN && xShell_IN && yShell_IN))
          m_outVxdID.insert(sensor);

      }
  }

  B2DEBUG(10, "list of VxdID OUT:");
  for (auto it = m_outVxdID.begin(); it != m_outVxdID.end(); it++)
    B2DEBUG(10, it->getLayerNumber() << "." << it->getLadderNumber() << "." << it->getSensorNumber());

}

void SVDClusterFilterModule::endRun()
{
}

void SVDClusterFilterModule::terminate()
{
}
