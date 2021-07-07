/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <string>

namespace Belle2 {
  namespace PXD {

    /** The PXDRawHitMasking module.
     *
     * This module is a helper module to apply basic cuts on the RawHits.
     * Its is similar to what PXDRawHitSorter is doing, but here we stay
     * on the RawHits level for further raw analysis
     *
     * @see PXDRawHitSorter
     */
    class PXDRawHitMaskingModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDRawHitMaskingModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the filtering */
      virtual void event() override;

    private:
      StoreArray<PXDRawHit> m_pxdRawHit; /**< Required input for  PXDRawHit */
      StoreArray<PXDRawHit> m_pxdRawHitOut; /**< Required output for  PXDRawHit */
      /** Utility function to check pixel coordinates */
      inline bool goodHit(const PXDRawHit& rawhit) const
      {
        short u = rawhit.getColumn();
        bool goodU = (u == std::min(std::max(u, short(0)), short(249)));
        short v = rawhit.getRow();
        bool goodV = (v == std::min(std::max(v, short(0)), short(767)));
        return (goodU && goodV);
      }
      /** Name of the collection to use for PXDRawHits */
      std::string m_storeRawHitsName;
      /** Name of the collection to use for Output PXDRawHits */
      std::string m_storeRawHitsNameOut;
      /** Minimum charge for a digit to carry */
      int m_0cut;
      /** Discard out-of-range hits. */
      bool m_trimOutOfRange;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

