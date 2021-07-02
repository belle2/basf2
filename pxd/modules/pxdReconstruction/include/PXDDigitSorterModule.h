/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <string>
#include <algorithm>

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
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:
      /** Utility function to check pixel coordinates */
      inline bool goodDigit(const PXDDigit* const digit) const
      {
        short u = digit->getUCellID();
        bool goodU = (u == std::min(std::max(u, short(0)), short(249)));
        short v = digit->getVCellID();
        bool goodV = (v == std::min(std::max(v, short(0)), short(767)));
        return (goodU && goodV);
      }

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
      /** if true, check digit data and discard malformed digits.*/
      bool m_trimDigits;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
