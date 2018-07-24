/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Giulia Casarosa                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <unordered_map>

#include <type_traits>

#include <TH2.h>
#include <math.h>

namespace Belle2 {


  /** small struct for storing all clusters of the same sensor in one container.
   *
   * members should only be filled using the single addCluster-function described below.
   */
  struct ClustersOnSensor {

  public:

    /** member function to automatically add the cluster to its corresponding entry */
    inline void addCluster(const SVDCluster* entry)
    {
      vxdID = entry->getSensorID();
      if (entry->isUCluster() == true) { clustersU.push_back(entry); return; }
      clustersV.push_back(entry);
    }

    /** Id of sensor, TODO can be removed if struct is used in a map */
    VxdID vxdID;

    /** stores all SVDclusters of U type.
     *
     * Each entry stores a pointer to its SVDCluster.
     */
    std::vector<const SVDCluster*> clustersU;

    /** stores all SVDclusters of V type.
     *
     * Each entry stores a pointer to its SVDCluster.
     */
    std::vector<const SVDCluster*> clustersV;

  };





  /** simply store one spacePoint for each existing SVDCluster.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   */
  template <class SpacePointType> void provideSVDClusterSingles(const StoreArray<SVDCluster>& svdClusters,
      StoreArray<SpacePointType>& spacePoints)
  {
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      const SVDCluster* currentCluster = svdClusters[i];
      std::vector<const SVDCluster*> currentClusterCombi = { currentCluster };
      SpacePointType* newSP = spacePoints.appendNew(currentClusterCombi);
      newSP->addRelationTo(currentCluster);
    }
  }



  /** stores all possible 2-Cluster-combinations.
   *
   * first parameter is a struct containing all clusters on current sensor.
   * second parameter is the container which collects all combinations found.
   *
   * for each u cluster, a v cluster is combined to a possible combination.
   * Condition which has to be fulfilled: the first entry is always an u cluster, the second always a v-cluster
   */
  inline void findPossibleCombinations(const Belle2::ClustersOnSensor& aSensor,
                                       std::vector< std::vector<const SVDCluster*> >& foundCombinations, float minClusterTime)
  {

    for (const SVDCluster* uCluster : aSensor.clustersU) {
      if (uCluster->getClsTime() < minClusterTime) {
        B2DEBUG(1, "Cluster rejected due to timing cut. Cluster time: " << uCluster->getClsTime());
        continue;
      }
      for (const SVDCluster* vCluster : aSensor.clustersV) {
        if (vCluster->getClsTime() < minClusterTime) {
          B2DEBUG(1, "Cluster rejected due to timing cut. Cluster time: " << uCluster->getClsTime());
          continue;
        }
        foundCombinations.push_back({uCluster, vCluster});


      }
    }




  }

  /**
   * Function to extract probability of correct (pair from signal hit) cluster pairing from preconfigured pdfs
   * Probability defined as Pcharge * Ptime * Pucluster * Pvcluster
   *
   */


  void calculatePairingProb(TFile* pdfFile,  std::vector<const SVDCluster*>& clusters, double& prob, double& error)
  {

    double uCharge = clusters[0]->getCharge();
    double uTime = clusters[0]->getClsTime();
    int uSize =  clusters[0]->getSize();

    double vCharge = clusters[1]->getCharge();
    double vTime = clusters[1]->getClsTime();
    int vSize =  clusters[1]->getSize();

    //Pdfs only divided up to size 5-strips, 4(time+size)+172(sensors)*2(prob/error)*5^2(hit comb.)
    int pdfEntries = pdfFile->GetListOfKeys()->GetSize();
    if (uSize > sqrt((pdfEntries - 4) / 344)) uSize =  floor(sqrt((pdfEntries - 4) / 344));
    if (vSize > sqrt((pdfEntries - 4) / 344)) vSize =  floor(sqrt((pdfEntries - 4) / 344));

    std::string sensorID = clusters[0]->getSensorID();
    std::string chargeProbInput =  sensorID + "." + std::to_string(uSize) + "." + std::to_string(vSize);
    std::string chargeErrorInput = chargeProbInput + "_Error";
    std::string timeProbInput = "timeProb";
    std::string timeErrorInput = "timeError";

    TH2F* chargePDF = nullptr;
    TH2F* chargeError = nullptr;
    TH2F* timePDF = nullptr;
    TH2F* timeError = nullptr;

    pdfFile->GetObject(chargeProbInput.c_str(), chargePDF);
    pdfFile->GetObject(chargeErrorInput.c_str(), chargeError);
    pdfFile->GetObject(timeProbInput.c_str(), timePDF);
    pdfFile->GetObject(timeErrorInput.c_str(), timeError);

    int xChargeBin = chargePDF->GetXaxis()->FindFixBin(uCharge);
    int yChargeBin = chargePDF->GetYaxis()->FindFixBin(vCharge);

    int xTimeBin = timePDF->GetXaxis()->FindFixBin(uTime);
    int yTimeBin = timePDF->GetYaxis()->FindFixBin(vTime);

    double chargeProb = chargePDF->GetBinContent(xChargeBin, yChargeBin);
    double timeProb = timePDF->GetBinContent(xTimeBin, yTimeBin);
    double chargeProbError = chargePDF->GetBinContent(xChargeBin, yChargeBin);
    double timeProbError = timePDF->GetBinContent(xTimeBin, yTimeBin);

    if (chargeProbError == 0) {
      B2DEBUG(1, "svdClusterProbabilityEstimator has not been run, spacePoint QI will return zero!");
    }

    prob = chargeProb * timeProb * clusters[0]->getQuality() * clusters[1]->getQuality();
    error = prob * sqrt(pow(timeProb * clusters[0]->getQuality() * clusters[1]->getQuality() * chargeProbError , 2) +
                        pow(chargeProb * clusters[0]->getQuality() * clusters[1]->getQuality() * timeProbError , 2) +
                        pow(chargeProb * timeProb * clusters[1]->getQuality() * clusters[0]->getQualityError() , 2) +
                        pow(chargeProb * timeProb * clusters[0]->getQuality() * clusters[1]->getQualityError() , 2));
  }


  /** finds all possible combinations of U and V Clusters for SVDClusters.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   * relationweights code the type of the cluster. +1 for u and -1 for v
   */
  template <class SpacePointType> void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
      StoreArray<SpacePointType>& spacePoints, float minClusterTime, bool useQualityEstimator, TFile* pdfFile)
  {
    std::unordered_map<VxdID::baseType, ClustersOnSensor>
    activatedSensors; // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
    std::vector<std::vector<const SVDCluster*> >
    foundCombinations; // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)

    double probability;
    double error;

    // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SVDCluster* currentCluster = svdClusters[i];

      activatedSensors[currentCluster->getSensorID().getID()].addCluster(currentCluster);
    }


    for (auto& aSensor : activatedSensors) {
      findPossibleCombinations(aSensor.second, foundCombinations, minClusterTime);
    }

    for (auto& clusterCombi : foundCombinations) {
      SpacePointType* newSP = spacePoints.appendNew(clusterCombi);
      if (useQualityEstimator == true) {
        calculatePairingProb(pdfFile, clusterCombi, probability, error);
        newSP->setQualityEstimation(probability);
        newSP->setQualityEstimationError(error);
      }
      for (auto* cluster : clusterCombi) {
        newSP->addRelationTo(cluster, cluster->isUCluster() ? 1. : -1.);
      }
    }
  }

} //Belle2 namespace
