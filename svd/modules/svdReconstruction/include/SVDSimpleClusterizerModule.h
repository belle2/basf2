/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDSimpleClusterizerModule_H
#define SVDSimpleClusterizerModule_H

#include <framework/core/Module.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/reconstruction/SimpleClusterCandidate.h>

//neew calibration objects
//#include <svd/calibration/SVDPulseShapeCalibrations.h>
//#include <svd/calibration/SVDNoiseCalibrations.h>
//old calibration objects:
//#include <framework/database/DBObjPtr.h>
//#include <svd/dbobjects/SVDPulseShapeCalibrations.h>
//#include <svd/dbobjects/SVDNoiseCalibrations.h>

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
      //1. Collections and relations
      /** Name of the collection to use for the SVDRecoDigits */
      std::string m_storeRecoDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
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


      // 2. Clustering
      /** Seed cut in units of noise. */
      double m_cutSeed;
      /** Adjacent cut in units of noise. */
      double m_cutAdjacent;
      /** Cluster cut in units of m_elNoise */
      //      double m_cutCluster; //not included yet
      /** Size of the cluster at which we switch from Center of Gravity to Analog Head Tail */
      int m_sizeHeadTail;

      //old calibration objects
      //      DBObjPtr<SVDPulseShapeCalibrations> m_PulseShapeCal;
      //      DBObjPtr<SVDNoiseCalibrations> m_NoiseCal;
      //new calibration objects
      //      SVDPulseShapeCalibrations m_PulseShapeCal;
      //      SVDNoiseCalibrations m_NoiseCal;

      void writeClusters(SimpleClusterCandidate clusterCand);
    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDSimpleClusterizerModule_H
