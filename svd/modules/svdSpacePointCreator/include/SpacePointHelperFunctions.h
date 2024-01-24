/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

#include <svd/calibration/SVDHitTimeSelection.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dbobjects/SVDSpacePointSNRFractionSelector.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>

#include <vxd/dataobjects/VxdID.h>

#include <unordered_map>

#include <TH2.h>
#include <math.h>
#include <TFile.h>

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



  /** Store the input values for SVDSpacePoint selection from the given SVDCluster  */
  inline void storeInputVectorFromSingleCluster(const SVDCluster* cls,
                                                std::vector<float>& inputVector,
                                                const SVDNoiseCalibrations& noiseCal)
  {
    inputVector.clear();
    inputVector.resize(3, 0.0);

    auto shaperDigits = cls->getRelationsTo<SVDShaperDigit>();
    float noise = 0;
    for (auto iSD : shaperDigits) {
      auto samples = iSD.getSamples();

      inputVector[0] += samples[0];
      inputVector[1] += samples[1];
      inputVector[2] += samples[2];

      VxdID thisSensorID = iSD.getSensorID();
      bool thisSide = iSD.isUStrip();
      int thisCellID = iSD.getCellID();
      float thisNoise = noiseCal.getNoise(thisSensorID, thisSide, thisCellID);

      noise += thisNoise * thisNoise;
    }
    noise = sqrt(noise);
    inputVector[0] = inputVector[0] / noise;
    inputVector[1] = inputVector[1] / noise;
    inputVector[2] = inputVector[2] / noise;
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
                                       std::vector< std::vector<const SVDCluster*> >& foundCombinations, const SVDHitTimeSelection& hitTimeCut,
                                       const bool& useSVDGroupInfo,  const int& numberOfSignalGroups, const bool& formSingleSignalGroup,
                                       const SVDNoiseCalibrations& noiseCal, const DBObjPtr<SVDSpacePointSNRFractionSelector>& svdSpacePointSelectionFunction,
                                       bool useSVDSpacePointSNRFractionSelector)
  {

    for (const SVDCluster* uCluster : aSensor.clustersU) {
      if (! hitTimeCut.isClusterInTime(uCluster->getSensorID(), 1, uCluster->getClsTime())) {
        B2DEBUG(29, "Cluster rejected due to timing cut. Cluster time: " << uCluster->getClsTime());
        continue;
      }
      for (const SVDCluster* vCluster : aSensor.clustersV) {
        if (! hitTimeCut.isClusterInTime(vCluster->getSensorID(), 0, vCluster->getClsTime())) {
          B2DEBUG(29, "Cluster rejected due to timing cut. Cluster time: " << vCluster->getClsTime());
          continue;
        }

        if (! hitTimeCut.areClusterTimesCompatible(vCluster->getSensorID(), uCluster->getClsTime(), vCluster->getClsTime())) {
          B2DEBUG(29, "Cluster combination rejected due to timing cut. Cluster time U (" << uCluster->getClsTime() <<
                  ") is incompatible with Cluster time V (" << vCluster->getClsTime() << ")");
          continue;
        }

        if (useSVDGroupInfo) {
          const std::vector<int>& uTimeGroupId = uCluster->getTimeGroupId();
          const std::vector<int>& vTimeGroupId = vCluster->getTimeGroupId();

          if (int(uTimeGroupId.size()) && int(vTimeGroupId.size())) { // indirect check if the clusterizer module is disabled
            bool isContinue = true;
            for (auto& uitem : uTimeGroupId) {
              if (uitem < 0 || uitem >= numberOfSignalGroups) continue;
              for (auto& vitem : vTimeGroupId) {
                if (vitem < 0 || vitem >= numberOfSignalGroups) continue;
                if ((uitem == vitem) || formSingleSignalGroup) { isContinue = false; break; }
              }
              if (!isContinue) break;
            }

            if (isContinue) {
              B2DEBUG(29, "Cluster combination rejected due to different time-group Id.");
              continue;
            }
          }
        }

        if (useSVDSpacePointSNRFractionSelector) {
          std::vector<float> inputU;
          std::vector<float> inputV;

          storeInputVectorFromSingleCluster(uCluster, inputU, noiseCal);
          storeInputVectorFromSingleCluster(vCluster, inputV, noiseCal);

          bool pass = svdSpacePointSelectionFunction->passSNRFractionSelection(inputU, inputV);
          if (!pass) {
            B2DEBUG(29, "Cluster combination rejected due to SVDSpacePointSNRFractionSelector");
            continue;
          }
        }

        foundCombinations.push_back({uCluster, vCluster});


      }
    }




  }

  /** Function to set name of PDF for spacePoint quality estimation.
   *  Legacy naming convention for PDFs distributed by sensor type
   *  rather than sensor ID, not used after 2018 MC calibrated PDFs.
   */

  inline void spPDFName(const VxdID& sensor, int uSize, int vSize, int maxClusterSize, std::string& PDFName,
                        std::string& errorPDFName, bool useLegacyNaming)
  {
    if (useLegacyNaming == true) {

      if (uSize > maxClusterSize) uSize = maxClusterSize;
      if (vSize > maxClusterSize) vSize = maxClusterSize;

      std::string sensorName;

      if (sensor.getLayerNumber() == 3)  sensorName = "l3";
      if (sensor.getLayerNumber() > 3 && sensor.getSensorNumber() == 1)  sensorName = "trap";
      if (sensor.getLayerNumber() > 3 && sensor.getSensorNumber() > 1)  sensorName = "large";

      PDFName =  sensorName +  std::to_string(uSize)  + std::to_string(vSize);
      errorPDFName = "error" + PDFName;
    } else {

      if (uSize > maxClusterSize) uSize = maxClusterSize;
      if (vSize > maxClusterSize) vSize = maxClusterSize;

      int layer = sensor.getLayerNumber();
      int ladder = sensor.getLadderNumber();
      int sens = sensor.getSensorNumber();

      PDFName =  std::to_string(layer) + "." + std::to_string(ladder) + "." + std::to_string(sens) + "."  + std::to_string(
                   uSize) + "."  + std::to_string(vSize);
      errorPDFName = PDFName + "_Error";
    }



  }


  /**
   * Function to extract probability of correct (pair from signal hit) cluster pairing from preconfigured pdfs
   * Probability defined as Pcharge * Ptime * Pucluster * Pvcluster
   *
   */


  inline void calculatePairingProb(TFile* pdfFile, std::vector<const SVDCluster*>& clusters, double& prob, double& error,
                                   bool useLegacyNaming)
  {

    int maxSize;
    int pdfEntries = pdfFile->GetListOfKeys()->GetSize();
    if (useLegacyNaming == true) {
      maxSize = floor(sqrt((pdfEntries - 4) / 6)); //4(time+size)+3(sensors)*2(prob/error)*size^2(u/v combo.)
    } else {
      maxSize = floor(sqrt((pdfEntries - 4) / 344)); //4(time+size)+172(sensorType)*2(prob/error)*size^2(u/v combo.)
    }
    std::string chargeProbInput;
    std::string chargeErrorInput;

    spPDFName(clusters[0]->getSensorID(), clusters[0]->getSize(), clusters[1]->getSize(), maxSize,
              chargeProbInput, chargeErrorInput, useLegacyNaming);
    std::string timeProbInput = "timeProb";
    std::string timeErrorInput = "timeError";
    std::string sizeProbInput = "sizeProb";
    std::string sizeErrorInput = "sizeError";


    TH2F* chargePDF = nullptr;
    TH2F* chargeError = nullptr;
    TH2F* timePDF = nullptr;
    TH2F* timeError = nullptr;
    TH2F* sizePDF = nullptr;
    TH2F* sizeError = nullptr;

    pdfFile->GetObject(chargeProbInput.c_str(), chargePDF);
    pdfFile->GetObject(chargeErrorInput.c_str(), chargeError);
    pdfFile->GetObject(timeProbInput.c_str(), timePDF);
    pdfFile->GetObject(timeErrorInput.c_str(), timeError);
    pdfFile->GetObject(sizeProbInput.c_str(), sizePDF);
    pdfFile->GetObject(sizeErrorInput.c_str(), sizeError);

    int xChargeBin = chargePDF->GetXaxis()->FindFixBin(clusters[0]->getCharge());
    int yChargeBin = chargePDF->GetYaxis()->FindFixBin(clusters[1]->getCharge());

    int xTimeBin = timePDF->GetXaxis()->FindFixBin(clusters[0]->getClsTime());
    int yTimeBin = timePDF->GetYaxis()->FindFixBin(clusters[1]->getClsTime());


    int xSizeBin = sizePDF->GetXaxis()->FindFixBin(clusters[0]->getSize());
    int ySizeBin = sizePDF->GetYaxis()->FindFixBin(clusters[1]->getSize());

    double chargeProb = chargePDF->GetBinContent(xChargeBin, yChargeBin);
    double timeProb = timePDF->GetBinContent(xTimeBin, yTimeBin);
    double sizeProb = sizePDF->GetBinContent(xSizeBin, ySizeBin);
    double chargeProbError = chargePDF->GetBinContent(xChargeBin, yChargeBin);
    double timeProbError = timePDF->GetBinContent(xTimeBin, yTimeBin);
    double sizeProbError = sizePDF->GetBinContent(xSizeBin, ySizeBin);


    if (chargeProbError == 0) {
      B2DEBUG(21, "svdClusterProbabilityEstimator has not been run, spacePoint QI will return zero!");
    }

    prob = chargeProb * timeProb * sizeProb * clusters[0]->getQuality() * clusters[1]->getQuality();
    error = prob * sqrt(pow(timeProb * sizeProb * clusters[0]->getQuality() * clusters[1]->getQuality() * chargeProbError, 2) +
                        pow(chargeProb * sizeProb * clusters[0]->getQuality() * clusters[1]->getQuality() * timeProbError, 2) +
                        pow(chargeProb * timeProb * clusters[0]->getQuality() * clusters[1]->getQuality() * sizeProbError, 2) +
                        pow(chargeProb * timeProb * sizeProb * clusters[1]->getQuality() * clusters[0]->getQualityError(), 2) +
                        pow(chargeProb * timeProb * sizeProb * clusters[0]->getQuality() * clusters[1]->getQualityError(), 2));
  }

  /** finds all possible combinations of U and V Clusters for SVDClusters.
   *
   * first parameter is a storeArray containing SVDClusters.
   * second parameter is a storeArra containing SpacePoints (will be filled in the function).
   * third parameter tels the spacePoint where to get the name of the storeArray containing the related clusters
   * relationweights code the type of the cluster. +1 for u and -1 for v
   */
  template <class SpacePointType> void provideSVDClusterCombinations(const StoreArray<SVDCluster>& svdClusters,
      StoreArray<SpacePointType>& spacePoints, SVDHitTimeSelection& hitTimeCut, bool useQualityEstimator, TFile* pdfFile,
      bool useLegacyNaming, unsigned int numMaxSpacePoints, std::string m_eventLevelTrackingInfoName, const bool& useSVDGroupInfo,
      const int& numberOfSignalGroups, const bool& formSingleSignalGroup,
      const SVDNoiseCalibrations& noiseCal, const DBObjPtr<SVDSpacePointSNRFractionSelector>& svdSpacePointSelectionFunction,
      bool useSVDSpacePointSNRFractionSelector)
  {
    std::unordered_map<VxdID::baseType, ClustersOnSensor>
    activatedSensors; // collects one entry per sensor, each entry will contain all Clusters on it TODO: better to use a sorted vector/list?
    std::vector<std::vector<const SVDCluster*> >
    foundCombinations; // collects all combinations of Clusters which were possible (condition: 1u+1v-Cluster on the same sensor)

    // sort Clusters by sensor. After the loop, each entry of activatedSensors contains all U and V-type clusters on that sensor
    for (unsigned int i = 0; i < uint(svdClusters.getEntries()); ++i) {
      SVDCluster* currentCluster = svdClusters[i];

      activatedSensors[currentCluster->getSensorID().getID()].addCluster(currentCluster);
    }


    for (auto& aSensor : activatedSensors)
      findPossibleCombinations(aSensor.second, foundCombinations, hitTimeCut, useSVDGroupInfo, numberOfSignalGroups,
                               formSingleSignalGroup,
                               noiseCal, svdSpacePointSelectionFunction, useSVDSpacePointSNRFractionSelector);

    // Do not make space-points if their number would be too large to be considered by tracking
    if (foundCombinations.size() > numMaxSpacePoints) {
      StoreObjPtr<EventLevelTrackingInfo> m_eventLevelTrackingInfo(m_eventLevelTrackingInfoName);
      if (m_eventLevelTrackingInfo.isValid()) {
        m_eventLevelTrackingInfo->setSVDSpacePointCreatorAbortionFlag();
      }
      return;
    }

    for (auto& clusterCombi : foundCombinations) {
      SpacePointType* newSP = spacePoints.appendNew(clusterCombi);
      if (useQualityEstimator == true) {
        double probability;
        double error;
        calculatePairingProb(pdfFile, clusterCombi, probability, error, useLegacyNaming);
        newSP->setQualityEstimation(probability);
        newSP->setQualityEstimationError(error);
      }
      for (auto* cluster : clusterCombi) {
        newSP->addRelationTo(cluster, cluster->isUCluster() ? 1. : -1.);
      }
    }
  }


} //Belle2 namespace
