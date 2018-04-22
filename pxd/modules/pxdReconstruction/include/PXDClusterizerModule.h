/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDClusterizerModule_H
#define PXDClusterizerModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <pxd/reconstruction/ClusterCache.h>
#include <pxd/reconstruction/ClusterProjection.h>
#include <pxd/reconstruction/NoiseMap.h>
#include <pxd/reconstruction/PXDClusterShape.h>
#include <string>
#include <memory>

namespace Belle2 {
  class RelationArray;
  class RelationElement;
  class PXDClusterShape;
  class PXDClusterShapeType;

  namespace PXD {

    /** The PXDClusterizer module.
     *
     * This module is responsible to cluster all hits found in the PXD and
     * write them to the appropriate collections. It does this in a "streaming" way:
     * The hits are examined in an ordered way (sorted by v Id, then by u Id) and for each
     * pixel we only have to check the left neighbor and the three adjacent
     * hits in the last row. By caching the last row, each pixel gets examined
     * only once and the 4 adjacent pixels are accessed in constant time.
     * @see ClusterCache
     */
    class PXDClusterizerModule : public Module {

    public:
      /** Container for a RelationArray Lookup table */
      typedef std::vector<const RelationElement*> RelationLookup;

      /** Constructor defining the parameters */
      PXDClusterizerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the clustering */
      virtual void event();

    private:
      /** Create lookup maps for Relations
       * We do not use the RelationIndex as we know much more about the
       * relations: we know the relations get created in a consolidated way by
       * the Digitizer and that they already point in the right direction so we
       * only need to speed up finding the correct element. We just create a
       * vector from digit id to relationElement pointer for fast lookup
       * @param relation RelationArray to build the lookuptable for
       * @param lookup Lookuptable to fill
       * @param number of digits in this event
       */
      void createRelationLookup(const RelationArray& relation, RelationLookup& lookup, size_t digits);

      /** Add the relation from a given PXDDigit index to a map
       * @param lookup Lookuptable to use for the relation
       * @param relation map to add the entries to
       * @param index index of the PXDDigit
       */
      void fillRelationMap(const RelationLookup& lookup, std::map<unsigned int, float>& relation, unsigned int index);

      /** Write clusters to collection.
       * This method will check all cluster candidates and write valid ones to the datastore
       */
      void writeClusters(VxdID sensorID);

      /** Calculate position and error for a given cluster.
       * @param cls ClusterCandidate of the cluster
       * @param primary Projection of the cluster to calculate the position and error for
       * @param secondary Projection of the cluster in the other direction
       * @param minPitch Pitch at the minimum cell of the cluster
       * @param centerPitch Pitch between pixel cells
       * @param maxPitch Pitch at the maximum cell of the cluster
       */
      void calculatePositionError(const ClusterCandidate& cls, ClusterProjection& primary, const ClusterProjection& secondary,
                                  double minPitch, double centerPitch, double maxPitch);

      /** Noise in ADU */
      double m_elNoise;
      /** Seed cut in sigma */
      double m_cutSeed;
      /** Noise cut in sigma */
      double m_cutAdjacent;
      /** Cluster cut in sigma */
      double m_cutCluster;
      /** Signal in ADU for Adjacent cut, basically m_elNoise*m_cutAdjacent */
      double m_cutAdjacentSignal;
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

      /** Do not aply recognition of cluster shape and set its ID */
      bool m_notUseClusterShape;

      /** Size of cluster Cache (0 = default) */
      int m_clusterCacheSize;
      /** cache of the last seen clusters to speed up clustering */
      std::unique_ptr<ClusterCache> m_cache;
      /** Noise map for the currently active sensor */
      NoiseMap m_noiseMap;

      /** Lookup table for PXDDigit->MCParticle relation */
      RelationLookup m_mcRelation;
      /** Lookup table for PXDDigit->PXDTrueHit relation */
      RelationLookup m_trueRelation;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDClusterizerModule_H
