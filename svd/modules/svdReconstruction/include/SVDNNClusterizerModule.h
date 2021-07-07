/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDNNClusterizerModule_H
#define SVDNNClusterizerModule_H

#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/reconstruction/NNWaveFitter.h>
#include <map>
#include <vector>

namespace Belle2 {

  namespace SVD {

    /** SVD NN Clusterizer
     *
     * This module produces clusters from SVDRecoDigits, that is, SVD strip
     * signals calibrated and fitted by the neural network fitter.
     * The module uses the NN fitter class - not for NN fits, but to
     * estimate hit times and re-calculate amplitudes.
     * The module identifies groups of neighbouring fired strips, estimates
     * hit time and re-calculates strip signal amplitudes. Next it estimates
     * hit position.
     */
    class SVDNNClusterizerModule : public Module {

    public:

      /** Container for a RelationArray Lookup table */
      typedef std::vector<const RelationElement*> RelationLookup;

      /** Constructor defining the parameters */
      SVDNNClusterizerModule();

      /** Initialize the module */
      virtual void initialize() override;

      /** do the clustering */
      virtual void event() override;

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

      /** Add the relation from a given SVDRecoDigit index to a map
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
      /** Name of the collection to use for the SVDRecoDigits */
      std::string m_storeRecoDigitsName;
      /** Name of the collection to use for the SVDClusters */
      std::string m_storeClustersName;
      /** Name of the relation between SVDRecoDigits and MCParticles */
      std::string m_relRecoDigitMCParticleName;
      /** Name of the relation between SVDRecoDigits and SVDTrueHits */
      std::string m_relRecoDigitTrueHitName;
      /** Name of the relation between SVDClusters and MCParticles */
      std::string m_relClusterMCParticleName;
      /** Name of the relation between SVDClusters and SVDTrueHits */
      std::string m_relClusterTrueHitName;
      /** Name of the relation between SVDClusters and SVDRecoDigits */
      std::string m_relClusterRecoDigitName;

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
      /** Calibrations: noise */
      SVDNoiseCalibrations m_noiseCal;
      /** Calibrations: pusle shape and gain */
      SVDPulseShapeCalibrations m_pulseShapeCal;
      /** Time fitter */
      NNWaveFitter m_fitter;

      // 5. Relation lookups
      /** Lookup table for SVDRecoDigit->MCParticle relation */
      RelationLookup m_mcRelation;
      /** Lookup table for SVDRecoDigit->SVDTrueHit relation */
      RelationLookup m_trueRelation;

    };//end class declaration

  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDNNClusterizerModule_H
