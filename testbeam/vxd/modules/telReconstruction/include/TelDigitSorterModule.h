/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TelDigitSorterModule_H
#define TelDigitSorterModule_H

#include <framework/core/Module.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <string>

namespace Belle2 {
  namespace TEL {
    /** \addtogroup modules
     * @{
     */

    /** The TelDigitSorter module.
     *
     * This module sorts the existing TelDigits collection and also updates the
     * corresponding Relation to MCParticles and TrueHits. This is needed for
     * unsorted pixel data as the Clusterizer expects sorted input.
     *
     * @see TelClusterizerModule
     */
    class TelDigitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      TelDigitSorterModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the sorting */
      virtual void event();

    private:
      /** Name of the collection to use for the TelDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the TelTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between TelDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between TelDigits and TelTrueHits */
      std::string m_relDigitTrueHitName;
      /** Copy of the Digits needed for sorting */
      std::vector<TelDigit> m_digitcopy;
    };//end class declaration

    /** @}*/

  } //end Tel namespace;
} // end namespace Belle2

#endif // TelDigitSorterModule_H
