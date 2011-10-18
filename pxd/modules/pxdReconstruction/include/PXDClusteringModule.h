/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDClusteringModule_H
#define PXDClusteringModule_H

#include <framework/core/Module.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/VxdID.h>
#include <pxd/modules/pxdReconstruction/ClusterCache.h>
#include <pxd/modules/pxdReconstruction/Pixel.h>
#include <boost/format.hpp>
#include <string>

namespace Belle2 {

  namespace PXD {

    /** The PXDClustering module.
     *
     * This module is responsible to cluster all hits found in the PXD and
     * write them to the apropriate collections. It does this in a "streaming" way:
     * The hits are examined in an ordered way (sorted by row, then by column) and for each
     * pixel we only have to check the left neighbor and the three adjacent
     * hits in the last row By caching the last row, each pixel gets examined
     * only once and the 4 adjacent pixels are accessed in constant time.
     * @see ClusterCache
     */
    class PXDClusteringModule : public Module {

    public:
      /** Container to sort the hits by row and column */
      typedef std::set<Pixel> Sensor;

      /** Constructor defining the parameters */
      PXDClusteringModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the clustering */
      virtual void event();

    protected:
      /** Find the cluster a given pixel belongs to.
       * For this to work correctly, the pixels have to be passed sorted by sensor,row,column
       * @param px Pixel to find the cluster for
       */
      void findCluster(const Pixel &px);
      /** Write clusters to collection.
       * This method will check all cluster candidates and write valid ones to the datastore
       */
      void writeClusters(VxdID sensorID);

      /** Noise in number of electrons */
      double m_elNoise;
      /** Seed cut in sigma */
      double m_cutSeed;
      /** Noise cut in sigma */
      double m_cutAdjacent;
      /** Cluster cut in sigma */
      double m_cutCluster;
      /** Name of the collection to use for the PXDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the PXDClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the PXDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between PXDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between PXDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between PXDClusters and PXDDigits */
      std::string m_relClusterDigitName;
      /** Name of the relation between PXDDigits and PXDTrueHits */
      std::string m_relDigitTrueHitName;
      /** Name of the relation between PXDClusters and PXDTrueHits */
      std::string m_relClusterTrueHitName;

      /** Size of the cluster at which we switch from Center of Gravity to Analog Head Tail */
      int m_sizeHeadTail;
      /** LorentzAngle , FIXME: should be determined directly from B-Field */
      double m_tanLorentzAngle;
      /** Pointer to the geometry info of the currently active Sensor */
      SensorInfo *m_geometry;
      /** Assume the PXDDigits to be sorted so we can skip sorting them */
      bool m_assumeSorted;

      /** List of all cluster candidates */
      std::deque<ClusterCandidate> m_clusters;
      /** cache of the last seen clusters to speed up clustering */
      ClusterCache m_cache;

    };//end class declaration

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDClusteringModule_H
