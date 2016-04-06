/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdClusterRescue/PXDClusterRescueNNModule.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <TVector3.h>

#include <math.h>

using namespace Belle2;

REG_MODULE(PXDClusterRescueNN)

PXDClusterRescueNNModule::PXDClusterRescueNNModule() : Module()
{
  // set description
  setDescription("Differ between background and signal PXD clusters by using NeuroBayes neural network for binary classification.");

  // add module parameters
  addParam("filenameExpertise", m_filenameExpertise, "Filename of Neurobayes teacher expertise input.",
           std::string("PXDClusterNeuroBayesExpertise.nb"));
  addParam("classThreshold", m_classThreshold,
           "Threshold for NeuroBayes expert classification. Expert output > classThreshold is interpreted as signal.", float(0.5));
  addParam("namePXDClusterInput", m_namePXDClusterInput, "Name of input PXD clusters in DataStore.", std::string(""));
  addParam("namePXDClusterOutput", m_namePXDClusterOutput,
           "Name of output PXD clusters in DataStore. If input name equals output name the DataStore object is replaced.",
           std::string("PXDClusterFiltered"));
}

PXDClusterRescueNNModule::~PXDClusterRescueNNModule()
{
}

void PXDClusterRescueNNModule::initialize()
{
  // set PXDCluster data on required
  StoreArray<PXDCluster>::required();

  // setup PXDClusters from DataStore
  StoreArray<PXDCluster> pxdClusters(m_namePXDClusterInput.c_str());

  if (m_namePXDClusterInput != m_namePXDClusterOutput) { // setup PXDCluster object if it should not be replaced
    // register subset
    m_pxdClustersSubset.registerSubset(pxdClusters, m_namePXDClusterOutput.c_str());

    // inherit relations from loaded PXDClusters
    m_pxdClustersSubset.inheritAllRelations();
  } else { // replace PXDCluster object
    // register subset
    m_pxdClustersSubset.registerSubset(pxdClusters);
  }

  // initialise NeuroBayes expert
  m_NBExpert = new Expert(m_filenameExpertise.c_str());
}

void PXDClusterRescueNNModule::beginRun()
{
}

void PXDClusterRescueNNModule::event()
{
  // create lambda function for classification of PXDClusters
  auto selectPXDClustersNeuroBayes = [this](const PXDCluster * pxdCluster) -> bool {
    std::vector<float> pxdClusterVariables(m_NumTrainingVariables);
    float NBOutput;

    // load trainings variables from PXDCluster
    getPXDClusterTrainingVariables(pxdCluster, pxdClusterVariables);

    // do classifcation with NeuroBayes neural network
    NBOutput = m_NBExpert->nb_expert(pxdClusterVariables.data());
    if (NBOutput > m_classThreshold) return true;
    else return false;
  };

  // do classification with NeuroBayes neural network on loaded PXDClusters
  m_pxdClustersSubset.select(selectPXDClustersNeuroBayes);
}

void PXDClusterRescueNNModule::endRun()
{
}

void PXDClusterRescueNNModule::terminate()
{
  // delete NeuroBayes teacher instance
  delete m_NBExpert;
}

void PXDClusterRescueNNModule::getPXDClusterTrainingVariables(const PXDCluster* pxdCluster, std::vector<float>& trainingVariables)
{
  // get pixels from cluster
  RelationVector<PXDDigit> pixels = pxdCluster->getRelationsTo<PXDDigit>();

  // get pixels charge as vector
  unsigned int pixelsNum = pixels.size();
  std::vector<float> pixelsCharge(pixelsNum);

  for (unsigned int p = 0; p < pixelsNum; p++) {
    pixelsCharge[p] = pixels[p]->getCharge();
  }

  // get single pixel charges and
  // get min pixel charge, mean pixel charge
  float pixelsChargeMin = pixels[0]->getCharge();
  float pixelsChargeMean = 0;

  for (unsigned int p = 0; p < pixelsNum; p++) {
    if (pixelsCharge[p] < pixelsChargeMin) pixelsChargeMin = pixelsCharge[p];
    pixelsChargeMean += pixelsCharge[p];
  }
  pixelsChargeMean = float(pixelsChargeMean) / float(pixelsNum);

  // get variance pixel charge
  float pixelsChargeVar = 0;

  for (unsigned int p = 0; p < pixelsNum; p++) {
    pixelsChargeVar += std::pow(pixelsCharge[p] - pixelsChargeMean, 2);
  }
  pixelsChargeVar = float(pixelsChargeVar) / float(pixelsNum);

  // get global coordinates
  VXD::GeoCache& m_vxdGeometry = VXD::GeoCache::getInstance();
  VXD::SensorInfoBase pxdSensorInfo = m_vxdGeometry.getSensorInfo(pxdCluster->getSensorID());
  TVector3 localPoint(pxdCluster->getU(), pxdCluster->getV(), 0);
  TVector3 globalPoint = pxdSensorInfo.pointToGlobal(localPoint);

  float coordinateZ = globalPoint.Z();
  float coordinateR = std::sqrt(std::pow(globalPoint.X(), 2) + std::pow(globalPoint.Y(), 2));
  float coordinatePhi = std::atan2(globalPoint.Y(), globalPoint.X());

  // write training variables to array
  trainingVariables[0] = pxdCluster->getCharge();
  trainingVariables[1] = pixelsChargeMin;
  trainingVariables[2] = pxdCluster->getSeedCharge();
  trainingVariables[3] = pixelsChargeMean;
  trainingVariables[4] = pixelsChargeVar;
  trainingVariables[5] = pxdCluster->getSize();
  trainingVariables[6] = pxdCluster->getUSize();
  trainingVariables[7] = pxdCluster->getVSize();
  trainingVariables[8] = coordinateZ;
  trainingVariables[9] = coordinateR;
  trainingVariables[10] = coordinatePhi;
}

int PXDClusterRescueNNModule::getNumTrainingVariables()
{
  return m_NumTrainingVariables;
}
