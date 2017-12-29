/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDNNShapeReconstructorModule_H
#define SVDNNShapeReconstructorModule_H

#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/dbobjects/SVDLocalRunBadStrips.h>
#include <svd/reconstruction/NNWaveFitter.h>
#include <map>
#include <vector>
#include <memory>

namespace Belle2 {

  namespace SVD {

    /** The SVD NNShapeReconstructor.
     *
     * This module produces SVDRecoDigits from SVDShaperDigits (signal samples taken
     * on individual strips) by first performing time and apmlitude fitting, and then
     * calibrating the recovered signals.
     */
    class SVDNNShapeReconstructorModule : public Module {

    public:

      /** Container for a RelationArray Lookup table */
      typedef std::vector<const RelationElement*> RelationLookup;

      /** Constructor defining the parameters */
      SVDNNShapeReconstructorModule();

      /** Initialize the module */
      virtual void initialize();

      /** do the clustering */
      virtual void event();

    protected:

      /** Create lookup maps for relations
       * FIXME: This has to be significantly simplified here, we just copy the
       * relations, since there is a 1:1 correspondence. We don't even have to do
       * the copying digit-by-digit.
       *
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
      /** Name of the collection to use for the SVDRecoDigits */
      std::string m_storeRecoDigitsName;
      /** Name of the relation between SVDShaperDigits and MCParticles */
      std::string m_relShaperDigitMCParticleName;
      /** Name of the relation between SVDShaperDigits and SVDTrueHits */
      std::string m_relShaperDigitTrueHitName;
      /** Name of the relation between SVDRecoDigits and MCParticles */
      std::string m_relRecoDigitMCParticleName;
      /** Name of the relation between SVDRecoDigits and SVDTrueHits */
      std::string m_relRecoDigitTrueHitName;
      /** Name of the relation between SVDRecoDigits and SVDShaperDigits */
      std::string m_relRecoDigitShaperDigitName;

      //2. Strip and time fitter data
      /** Name of the time fitter data xml */
      std::string m_timeFitterName;
      /** Use peak widths and peak time calibrations?
       * Unitl this is also simulated, set to true only for testbeam data. */
      bool m_calibratePeak = false;

      /** Calibrations: noise */
      SVDNoiseCalibrations m_noiseCal;
      /** Calibrations: pusle shape and gain */
      SVDPulseShapeCalibrations m_pulseShapeCal;

      /** Time fitter */
      NNWaveFitter m_fitter;

      // Relation lookups
      /** Lookup table for SVDShaperDigit->MCParticle relation */
      RelationLookup m_mcRelation;
      /** Lookup table for SVDShaperDigit->SVDTrueHit relation */
      RelationLookup m_trueRelation;

      // Write SVDRecoDigits? (no in normal operation)
      bool m_writeRecoDigits = false;

      // Zero-suppression cut
      float m_cutAdjacent = 3.0;

    };//end class declaration

  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDNNShapeReconstructorModule_H
