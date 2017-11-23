/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDClusterizerDirectModule_H
#define SVDClusterizerDirectModule_H

#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/reconstruction/NNWaveFitter.h>
#include <map>
#include <vector>
#include <memory>

namespace Belle2 {

  namespace SVD {

    /** SVD Direct Clusterizer
     *
     * This module produces clusters from SVDShaperDigits (signal samples taken on
     * individual strips) by first performing time and apmlitude fitting, and then
     * reconstructing particle hits in SVDSensors.
     * The module makes no use of SVDRecoDigits. If RecoDigits are desired, use the
     * SVDSignalReconstructor module.
     */
    class SVDClusterizerDirectModule : public Module {

    public:

      /** Container for a RelationArray Lookup table */
      typedef std::vector<const RelationElement*> RelationLookup;

      /** Constructor defining the parameters */
      SVDClusterizerDirectModule();

      /** Initialize the module */
      virtual void initialize();

      /** do the clustering */
      virtual void event();

    protected:

      /** Create lookup maps for relations
       * We do not use the RelationIndex as we know much more about the
       * relations: we know the relations get created in a consolidated way by
       * the Digitizer and that they already point in the right direction so we
       * only need to speed up finding the correct element. We just create a
       * vector from digit id to relationElement pointer for fast lookup
       * @param relation RelationArray to build the lookuptable for
       * @param lookup Lookuptable to fill
       * @param digits number of digits in this event
       */
      void createRelationLookup(const RelationArray& relation, RelationLookup& lookup,
                                size_t digits);

      /** Add the relation from a given SVDShaperDigit index to a map
       * @param lookup Lookuptable to use for the relation
       * @param relation map to add the entries to
       * @param index index of the SVDDigit
       */
      void fillRelationMap(const RelationLookup& lookup, std::map<unsigned int, float>&
                           relation, unsigned int index);

      // Data members
      //1. Collections
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;
      /** Name of the relation between SVDShaperDigits and MCParticles */
      std::string m_relShaperDigitMCParticleName;
      /** Name of the relation between SVDShaperDigits and SVDTrueHits */
      std::string m_relShaperDigitTrueHitName;
      /** Name of the relation between SVDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between SVDClusters and SVDTrueHits */
      std::string m_relClusterTrueHitName;
      /** Name of the relation between SVDClusters and SVDShaperDigits */
      std::string m_relClusterShaperDigitName;

      //2. Strip and time fitter data
      /** Name of the time fitter (db label) */
      std::string m_timeFitterName;
      /** Use peak widths and peak time calibrations?
       * Unitl this is also simulated, set to true only for testbeam data. */
      bool m_calibratePeak = false;


      //3. Clustering
      /** Noise (cluster member) cut in units of m_elNoise. */
      double m_cutAdjacent = 3.0;
      /** Seed cut in units of m_elNoise. */
      double m_cutSeed = 5.0;
      /** Cluster cut in units of m_elNoise */
      double m_cutCluster = 5.0;
      /** Size of the cluster at which we switch from Center of Gravity to Analog Head Tail */
      int m_sizeHeadTail = 3;

      // 4. Strip map and time fitter handles
      /** Time fitter */
      NNWaveFitter m_fitter;

      /** Calibrations: noise */
      SVDNoiseCalibrations m_noiseCal;
      /** Calibrations: pusle shape and gain */
      SVDPulseShapeCalibrations m_pulseShapeCal;

      // 5. Relation lookups
      /** Lookup table for SVDShaperDigit->MCParticle relation */
      RelationLookup m_mcRelation;
      /** Lookup table for SVDShaperDigit->SVDTrueHit relation */
      RelationLookup m_trueRelation;

    };//end class declaration

  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDClusterizerDirectModule_H
