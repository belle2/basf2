/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDDigitSorterModule_H
#define PXDDigitSorterModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <string>

namespace Belle2 {
  namespace PXD {

    /** The PXDDigitSorter module.
     *
     * This module sorts the existing PXDDigits collection and also updates the
     * corresponding Relation to MCParticles and TrueHits. This is needed for
     * unsorted pixel data as the Clusterizer expects sorted input.
     *
     * @see PXDClusterizerModule
     */
    class PXDDigitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDDigitSorterModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the sorting */
      virtual void event();

    private:
      /** Name of the collection to use for the PXDDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the PXDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between PXDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between PXDDigits and PXDTrueHits */
      std::string m_relDigitTrueHitName;
      /** Copy of the Digits needed for sorting */
      std::vector<PXDDigit> m_digitcopy;
      /** Mode: if true, merge duplicate pixels, otherwise delete all but the first occurence */
      bool m_mergeDuplicates;
      /** Name of the ignored pixels list xml */
      std::string m_ignoredPixelsListName;
      /** Ignored pixels list manager class */
      std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsList;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDDigitSorterModule_H
