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

  B2DEBUG(1, "Initialize");

  B2DEBUG(1, "input array name " << m_inputArrayName);


  StoreArray<SVDCluster> inputArray(m_inputArrayName);
  inputArray.isRequired();

  m_selectedClusters.registerSubset(inputArray, m_outputINArrayName);
  m_selectedClusters.inheritAllRelations();

  m_notSelectedClusters.registerSubset(inputArray, m_outputOUTArrayName);
  m_notSelectedClusters.inheritAllRelations();



}


void SVDClusterFilterModule::beginRun()
{
}

void SVDClusterFilterModule::event()
{
  B2DEBUG(1, std::endl << "NEW EVENT: " << m_layerNum << std::endl);

  StoreArray<SVDCluster> inputClusterArray(m_inputArrayName);

  int bufflayer = m_layerNum;
  m_selectedClusters.select([& bufflayer](const SVDCluster * aCluster) {
    unsigned short layernum = aCluster->getSensorID().getLayerNumber();
    B2DEBUG(1, "Cluster " << " layernum " << layernum << " layer to be removed " << bufflayer << std::endl);
    return (layernum != bufflayer);
  });

  m_notSelectedClusters.select([& bufflayer](const SVDCluster * aCluster) {
    unsigned short layernum = aCluster->getSensorID().getLayerNumber();
    B2DEBUG(1, "Cluster " << " layernum " << layernum << " layer to be removed " << bufflayer << std::endl);
    return (layernum == bufflayer);
  });

}

void SVDClusterFilterModule::endRun()
{
}

void SVDClusterFilterModule::terminate()
{
}
