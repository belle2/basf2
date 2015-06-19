/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
// svd:
#include <svd/dataobjects/SVDCluster.h>


// tracking:
#include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmBase.h>

// root:
#include <TMath.h>

// stl:
#include <string>


namespace Belle2 {
  /** INFO
   * This file contains all the algorithms retrieving infos from Clusters.
   * */

  /** to cluster helper functions for AnalyzingAlgorithms */
  namespace AnalyzingAlgorithmHelper {

    /** helper function to retrieve SVDClusters from a given TC */
    template <class TrackCandType>
    std::vector<const Belle2::SVDCluster*> getSVDClusters(TrackCandType* aTC, bool wantUCluster)
    {
      std::vector<const Belle2::SVDCluster*> clusters;
      for (const Belle2::SVDCluster* aHit : *aTC) {
        auto relatedClusters = aHit->getRelationsTo<SVDCluster>("ALL");
        for (const Belle2::SVDCluster* aCluster : relatedClusters) {
          if (aCluster->isUCluster() == wantUCluster) { clusters.push_back(aCluster); }
        }
      }
      return clusters;
    }


    /** helper function to retrieve the number of unique Clusters of first tc given, compared with second tc
    *
    * WARNING: unique clusters of secondTC are ignored on purpose!
    * if you want both, just re-run the function with switched sides for first and second TC
    */
    template <class ClusterType>
    std::vector<const ClusterType*> getUniqueClusters(std::vector<const ClusterType*>& firstTC,
                                                      std::vector<const ClusterType*>& secondTC)
    {
      std::vector<const ClusterType*> uniqueClusters;
      for (const auto* firstCluster : firstTC) {
        for (const auto* secondCluster : secondTC) {
          if (*firstCluster == *secondCluster) { uniqueClusters.push_back(firstCluster); break; }
        }
      }
      return uniqueClusters;
    }
  }



// residual type algorithms: (uses difference between test- and refTC):

  /** Class for storing an algorithm to find out how many u-type-clusters the testTC lost compared to the refTC */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmLostUClusters : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmLostUClusters() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmLostUClusters")) {}

    /** returns how many u-type-clusters the testTC lost compared to the refTC */
    virtual DataType calcData(const TCInfoType& aTC) /// TODO
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      std::vector<const SVDCluster*> uClustersRef = AnalyzingAlgorithmHelper::getSVDClusters(tcs.refTC.tc, true);
      std::vector<const SVDCluster*> uClustersTest = AnalyzingAlgorithmHelper::getSVDClusters(tcs.testTC.tc, true);

      return AnalyzingAlgorithmHelper::getUniqueClusters(uClustersRef, uClustersTest).size();
    }
  };



  /** Class for storing an algorithm to find out how many v-type-clusters the testTC lost compared to the refTC */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmLostVClusters : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmLostVClusters() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmLostVClusters")) {}

    /** returns how many v-type-clusters the testTC lost compared to the refTC */
    virtual DataType calcData(const TCInfoType& aTC) /// TODO
    {
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      std::vector<const SVDCluster*> vClustersRef = AnalyzingAlgorithmHelper::getSVDClusters(tcs.refTC.tc, false);
      std::vector<const SVDCluster*> vClustersTest = AnalyzingAlgorithmHelper::getSVDClusters(tcs.testTC.tc, false);

      return AnalyzingAlgorithmHelper::getUniqueClusters(vClustersRef, vClustersTest).size();
    }
  };



  /** Class for storing an algorithm to find out the energy deposit of u-type-clusters the testTC lost compared to the refTC */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmLostUEDep : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmLostUEDep() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmLostUEDep")) {}

    /** returns the energy deposit of u-type-clusters the testTC lost compared to the refTC */
    virtual std::vector<DataType> calcData(const TCInfoType& aTC) /// TODO !!!
    {
      std::vector<DataType> lostEdep;
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      std::vector<const SVDCluster*> uClustersRef = AnalyzingAlgorithmHelper::getSVDClusters(tcs.refTC.tc, true);
      std::vector<const SVDCluster*> uClustersTest = AnalyzingAlgorithmHelper::getSVDClusters(tcs.testTC.tc, true);

      std::vector<const SVDCluster*> lostuClusters = AnalyzingAlgorithmHelper::getUniqueClusters(uClustersRef, uClustersTest);

      for (const SVDCluster* aCluster : lostuClusters) {
        lostEdep.push_back(aCluster->getCharge());
      }
      return lostEdep;
    }
  };



  /** Class for storing an algorithm to find out the energy deposit of v-type-clusters the testTC lost compared to the refTC */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmLostVEDep : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmLostVEDep() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmLostVEDep")) {}

    /** returns the energy deposit of v-type-clusters the testTC lost compared to the refTC */
    virtual std::vector<DataType> calcData(const TCInfoType& aTC) /// TODO
    {
      std::vector<DataType> lostEdep;
      const auto tcs = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectPairOfTCs(aTC);
      std::vector<const SVDCluster*> vClustersRef = AnalyzingAlgorithmHelper::getSVDClusters(tcs.refTC.tc, false);
      std::vector<const SVDCluster*> vClustersTest = AnalyzingAlgorithmHelper::getSVDClusters(tcs.testTC.tc, false);

      std::vector<const SVDCluster*> lostvClusters = AnalyzingAlgorithmHelper::getUniqueClusters(vClustersRef, vClustersTest);

      for (const SVDCluster* aCluster : lostvClusters) {
        lostEdep.push_back(aCluster->getCharge());
      }
      return lostEdep;
    }
  };



// value type algorithms: (uses info of a single TC):



  /** Class for storing an algorithm to find out how many u-type-clusters the given TC had */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmTotalUClusters : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmTotalUClusters() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmTotalUClusters")) {}

    /** returns how many u-type-clusters the given TC had */
    virtual DataType calcData(const TCInfoType& aTC) /// TODO
    {
      TCInfoType& thisTC = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC);
      std::vector<const SVDCluster*> uClusters = AnalyzingAlgorithmHelper::getSVDClusters(thisTC.tc, true);

      return uClusters.size();
    }
  };



  /** Class for storing an algorithm to find out how many v-type-clusters the given TC had */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmTotalVClusters : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmTotalVClusters() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmTotalVClusters")) {}

    /** returns how many v-type-clusters the given TC had */
    virtual DataType calcData(const TCInfoType& aTC) /// TODO
    {
      TCInfoType& thisTC = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC);
      std::vector<const SVDCluster*> vClusters = AnalyzingAlgorithmHelper::getSVDClusters(thisTC.tc, false);

      return vClusters.size();
    }
  };



  /** Class for storing an algorithm to find out the energy deposit of u-type-clusters the given TC had */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmTotalUEDep : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmTotalUEDep() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmTotalUEDep")) {}

    /** returns the energy deposit of u-type-clusters the given TC had */
    virtual std::vector<DataType> calcData(const TCInfoType& aTC) /// TODO
    {
      std::vector<DataType> totalEDep;
      TCInfoType& thisTC = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC);
      std::vector<const SVDCluster*> uClusters = AnalyzingAlgorithmHelper::getSVDClusters(thisTC.tc, true);

      for (const SVDCluster* aCluster : uClusters) {
        totalEDep.push_back(aCluster->getCharge());
      }
      return totalEDep;
    }
  };



  /** Class for storing an algorithm to find out the energy deposit of v-type-clusters the given TC had */
  template <class DataType, class TCInfoType, class VectorType>
  class AnalyzingAlgorithmTotalVEDep : public AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType> {
  public:
    /** constructor */
    AnalyzingAlgorithmTotalVEDep() : AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>
      (std::string("AnalyzingAlgorithmTotalVEDep")) {}

    /** returns the energy deposit of v-type-clusters the given TC had */
    virtual std::vector<DataType> calcData(const TCInfoType& aTC) /// TODO
    {
      std::vector<DataType> totalEDep;
      TCInfoType& thisTC = AnalyzingAlgorithmBase<DataType, TCInfoType, VectorType>::chooseCorrectTC(aTC);
      std::vector<const SVDCluster*> vClusters = AnalyzingAlgorithmHelper::getSVDClusters(thisTC.tc, false);

      for (const SVDCluster* aCluster : vClusters) {
        totalEDep.push_back(aCluster->getCharge());
      }
      return totalEDep;
    }
  };
}
