/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <svd/reconstruction/SimpleClusterCandidate.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDClusterCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>

namespace Belle2 {

  namespace SVD {

    /** SVDSimpleClusterizerModule: The SVD SimpleClusterizer.
     *
     * This module produces clusters from SVDRecoDigits (strips fitted with basic fitter and calibrated)
     */
    class SVDSimpleClusterizerModule : public Module {

    public:

      /** Constructor defining the parameters */
      SVDSimpleClusterizerModule();

      /** Initialize the module */
      virtual void initialize() override;

      /** do the clustering */
      virtual void event() override;

    protected:


      // Data members
      //1. Collections and relations Names
      /** Name of the collection to use for the SVDRecoDigits */
      std::string m_storeRecoDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the relation between SVDRecoDigits and MCParticles */
      std::string m_relRecoDigitMCParticleName;
      /** Name of the relation between SVDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between SVDClusters and SVDRecoDigits */
      std::string m_relClusterRecoDigitName;
      /** Name of the relation between SVDRecoDigits and SVDTrueHits */
      std::string m_relRecoDigitTrueHitName;
      /** Name of the relation between SVDClusters and SVDTrueHits */
      std::string m_relClusterTrueHitName;

      /** Collection of SVDClusters */
      StoreArray<SVDCluster> m_storeClusters;
      /** Collection of SVDRecoDigits */
      StoreArray<SVDRecoDigit> m_storeDigits;
      /** Collection of SVDTrueHits */
      StoreArray<SVDTrueHit> m_storeTrueHits;
      /** Collection of MCParticles */
      StoreArray<MCParticle> m_storeMCParticles;



      // 2. Clustering
      /** Seed cut in units of noise. DEPRECATED - useDB */
      double m_cutSeed = 5;
      /** Adjacent cut in units of noise. DEPRECATED - useDB*/
      double m_cutAdjacent = 3;
      /** Size of the cluster at which we switch from Center of Gravity to Analog Head Tail */
      int m_sizeHeadTail = 3;
      /** Cluster cut in units of m_elNoise, not included (yet?) */
      double m_cutCluster = 0;
      /** if true takes the clusterizer cuts from the DB object*/
      bool m_useDB = true;
      /** selects the algorithm to compute the cluster tim
       *  0 = 6-sample CoG (default)
       *  1 = 3-sample CoG (TO DO: default if 3-sample acquisition mode)
       *  2 = 3-sample ELS
       */
      int m_timeAlgorithm = 0;

      //calibration objects
      SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShape calibrations db object*/
      SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibrations db object*/
      SVDClusterCalibrations m_ClusterCal; /**<SVDCluster calibrations db object*/

      SVD3SampleCoGTimeCalibrations m_3CoGTimeCal; /**< SVD 3-sample CoG Time calibrations db object*/
      SVD3SampleELSTimeCalibrations m_3ELSTimeCal; /**< SVD 3-sample ELS Time calibrations db object*/

      void writeClusters(SimpleClusterCandidate clusterCand); /**<write the cluster candidate to clusters*/
    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2
