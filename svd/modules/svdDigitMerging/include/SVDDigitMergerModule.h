/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_DigitMergerModule_H
#define SVD_DigitMergerModule_H

#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <vxd/dataobjects/VxdID.h>
#include <string>

namespace Belle2 {

  namespace SVD {

    /** SVDDigitMergerModule: The module to merge SVDDigits into SVDShaperDigits.
     *
     * This module creates SVDShaperDigits StoreArray from SVDDigts
     * The module expects sorted SVDDigits on input. Use SVDDigitSorter if SVDDigits
     * may be unsorted.
     * The module also creates the relations between SVDShaperDigits and MCParticles or
     * SVDTrueHits.
     */
    class SVDDigitMergerModule : public Module {

    public:
      /** Container for a RelationArray Lookup table */
      typedef std::vector<const RelationElement*> RelationLookup;

      /** Constructor defining the parameters */
      SVDDigitMergerModule();

      /** Initialize the module */
      virtual void initialize();

      /** Merge this event's SVDDigits into SVDShaperDigits */
      virtual void event();

    protected:

    private:

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
      void createRelationLookup(const RelationArray& relation, RelationLookup& lookup, size_t digits);

      /** Add the relation from a given SVDDigit index to a map
       * @param lookup Lookuptable to use for the relation
       * @param relation map to add the entries to
       * @param index index of the SVDDigit
       */
      void fillRelationMap(const RelationLookup& lookup, std::map<unsigned int, float>&
                           relation, unsigned int index);

      // Data members
      /** Name of the collection to use for the SVDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between SVDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between SVDDigits and SVDTrueHits */
      std::string m_relDigitTrueHitName;
      /** Name of the relation between SVDShaperDigits and MCParticles */
      std::string m_relShaperDigitMCParticleName;
      /** Name of the relation between SVDShaperDigits and SVDTrueHits */
      std::string m_relShaperDigitTrueHitName;
      /** Name of the relation between SVDShaperDigits and SVDDigits */
      std::string m_relShaperDigitDigitName;

      // Relation lookups
      /** Lookup table for SVDDigit->MCParticle relation */
      RelationLookup m_mcRelation;
      /** Lookup table for SVDDigit->SVDTrueHit relation */
      RelationLookup m_trueRelation;

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVD_DigitMergerModule_H
