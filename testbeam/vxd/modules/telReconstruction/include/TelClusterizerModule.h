/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TelClusterizerModule_H
#define TelClusterizerModule_H

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <testbeam/vxd/reconstruction/ClusterCache.h>
#include <testbeam/vxd/reconstruction/ClusterProjection.h>
#include <testbeam/vxd/reconstruction/NoiseMap.h>
#include <string>
#include <memory>

namespace Belle2 {
  class RelationArray;
  class RelationElement;

  namespace TEL {
    /** \addtogroup modules
     * @{
     */

    /** The TelClusterizer module.
     *
     * This module is responsible to cluster telescope digits and write them to
     * PXDCluster collections. It does this in a "streaming" way:
     * The hits are examined in an ordered way (sorted by row, then by column) and for each
     * pixel we only have to check the left neighbor and the three adjacent
     * hits in the last row. By caching the last row, each pixel gets examined
     * only once and the 4 adjacent pixels are accessed in constant time.
     * @see ClusterCache
     */
    class TelClusterizerModule : public Module {

    public:
      /** Container for a RelationArray Lookup table */
      typedef std::vector<const RelationElement*> RelationLookup;

      /** Constructor defining the parameters */
      TelClusterizerModule();

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
       * @param relation RelationArray to build the lookup table for
       * @param lookup Lookup table to fill
       * @param number of digits in this event
       */
      void createRelationLookup(const RelationArray& relation, RelationLookup& lookup, size_t digits);

      /** Add the relation from a given TelDigit index to a map
       * @param lookup Lookuptable to use for the relation
       * @param relation map to add the entries to
       * @param index index of the TelDigit
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
      void calculatePositionError(const ClusterCandidate& cls, ClusterProjection& primary, const ClusterProjection& secondary, double minPitch, double centerPitch, double maxPitch);

      /** Name of the collection to use for the TelDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the TelClusters */
      std::string m_storeClustersName;
      /** Name of the collection to use for the TelTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between TelDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between TelClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between TelClusters and TelDigits */
      std::string m_relClusterDigitName;
      /** Name of the relation between TelDigits and TelTrueHits */
      std::string m_relDigitTrueHitName;
      /** Name of the relation between TelClusters and TelTrueHits */
      std::string m_relClusterTrueHitName;

      /** LorentzAngle , FIXME: should be determined directly from B-Field */
      double m_tanLorentzAngle;
      /** Head-tail minimum cluster size. For smaller sizes, CoG is used. */
      int m_sizeHeadTail;

      /** Size of cluster Cache (0 = default) */
      int m_clusterCacheSize;
      /** cache of the last seen clusters to speed up clustering */
      std::unique_ptr<ClusterCache> m_cache;
      /** Noisemap for the currently active sensor */
      NoiseMap m_noiseMap;

      /** Lookuptable for TelDigit->MCParticle relation */
      RelationLookup m_mcRelation;
      /** Lookuptable for TelDigit->TelTrueHit relation */
      RelationLookup m_trueRelation;

    };//end class declaration

    /** @}*/

  } //end Tel namespace;
} // end namespace Belle2

#endif // TelClusterizerModule_H
