/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDigitSorterModule_H
#define SVDDigitSorterModule_H

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/online/SVDIgnoredStripsMap.h>
#include <string>
#include <memory>

namespace Belle2 {
  namespace SVD {

    /** The SVDDigitSorter module.
     *
     * This module sorts the existing SVDDigits collection and also updates the
     * corresponding Relation to MCParticles and TrueHits. This is needed for
     * unsorted pixel data as the Clusterizer expects sorted input.
     *
     * @see SVDClusterizerModule
     */
    class SVDDigitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      SVDDigitSorterModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:
      /** Name of the collection to use for the SVDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between SVDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between SVDDigits and SVDTrueHits */
      std::string m_relDigitTrueHitName;
      /** Copy of the Digits needed for sorting */
      std::vector<SVDDigit> m_digitcopy;
      /** Mode: if true, merge duplicate pixels, otherwise delete all but the first occurence */
      bool m_mergeDuplicates;
      /** Minimum number of samples over threshold to accept the digit */
      int m_minSamples;
      /** Suppression threshold */
      double m_rejectionThreshold;
      /** Name of the ignored strips list xml */
      std::string m_ignoredStripsListName;
      /** Ignored strips list manager class */
      std::unique_ptr<SVDIgnoredStripsMap> m_ignoredStripsList;

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDDigitSorterModule_H
