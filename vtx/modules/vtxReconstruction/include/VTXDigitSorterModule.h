/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VTXDigitSorterModule_H
#define VTXDigitSorterModule_H

#include <framework/core/Module.h>
#include <vtx/dataobjects/VTXDigit.h>
#include <vtx/geometry/SensorInfo.h>
#include <string>
#include <algorithm>

namespace Belle2 {
  namespace VTX {

    /** The VTXDigitSorter module.
     *
     * This module sorts the existing VTXDigits collection and also updates the
     * corresponding Relation to MCParticles and TrueHits. This is needed for
     * unsorted pixel data as the Clusterizer expects sorted input.
     *
     * @see VTXClusterizerModule
     */
    class VTXDigitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      VTXDigitSorterModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:
      /** Utility function to check pixel coordinates */
      inline bool goodDigit(const VTXDigit* const digit, const SensorInfo&  info) const
      {
        short uCells = short(info.getUCells());
        short vCells = short(info.getVCells());
        short u = digit->getUCellID();
        bool goodU = (u == std::min(std::max(u, short(0)), uCells));
        short v = digit->getVCellID();
        bool goodV = (v == std::min(std::max(v, short(0)), vCells));
        return (goodU && goodV);
      }

      /** Name of the collection to use for the VTXDigits */
      std::string m_storeDigitsName;
      /** Name of the collection to use for the VTXTrueHits */
      std::string m_storeTrueHitsName;
      /** Name of the collection to use for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the relation between VTXDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between VTXDigits and VTXTrueHits */
      std::string m_relDigitTrueHitName;
      /** Copy of the Digits needed for sorting */
      std::vector<VTXDigit> m_digitcopy;
      /** Mode: if true, merge duplicate pixels, otherwise delete all but the first occurence */
      bool m_mergeDuplicates;
      /** if true, check digit data and discard malformed digits.*/
      bool m_trimDigits;
    };//end class declaration


  } //end VTX namespace;
} // end namespace Belle2

#endif // VTXDigitSorterModule_H
