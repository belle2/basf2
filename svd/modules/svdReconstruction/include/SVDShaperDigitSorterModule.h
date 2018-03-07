/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDShaperDigitSorterModule_H
#define SVDShaperDigitSorterModule_H

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <string>

namespace Belle2 {
  namespace SVD {

    /** The SVDShaperDigitSorter module.
     *
     * This module sorts the existing SVDShaperDigits collection and also updates the
     * corresponding relations to MCParticles and TrueHits. This is needed for
     * background overlay, as the Clusterizer expects sorted input.
     * The module does only sorting and no filtering whatsoever.
     * The module is only required for background overlay, as both SVDDigitizer
     * and SVDUnpacker produce sorted arrays of digits.
     *
     * @see SVDClusterizerModule
     */
    class SVDShaperDigitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      SVDShaperDigitSorterModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:
      /** Name of the collection to use for the SVDShaperDigits */
      std::string m_storeShaperDigitsName;
      /** Name of the collection to use for the SVDTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between SVDShaperDigits and MCParticles */
      std::string m_relShaperDigitMCParticleName;
      /** Name of the relation between SVDShaperDigits and SVDTrueHits */
      std::string m_relShaperDigitTrueHitName;
      /** Copy of the ShaperDigits needed for sorting */
      std::vector<SVDShaperDigit> m_digitcopy;

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDShaperDigitSorterModule_H
