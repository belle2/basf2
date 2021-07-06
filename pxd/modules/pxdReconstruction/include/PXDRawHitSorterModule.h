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
#include <framework/datastore/StoreObjPtr.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDDAQStatus.h>
#include <string>

namespace Belle2 {
  namespace PXD {

    /** The PXDRawHitSorter module.
     *
     * This module sorts the existing PXDRawHits collection and saves PXDDigits
     * ordered by row and column for each half-ladder. This
     * provides ordered input to PXDClusterizer.
     *
     * @see PXDClusterizerModule
     */
    class PXDRawHitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDRawHitSorterModule();

      /** Initialize the module */
      virtual void initialize() override;
      /** do the sorting */
      virtual void event() override;

    private:
      /** Required input for PXDRawHit */
      StoreArray<PXDRawHit> m_storeRawHits;
      /** Required input for PXD Daq Status */
      StoreObjPtr<PXDDAQStatus> m_storeDaqStatus;
      /** Output array for PXDDigits. */
      StoreArray<PXDDigit> m_storeDigits;

      /** Utility function to check pixel coordinates */
      inline bool goodHit(const PXDRawHit* const rawhit) const
      {
        short u = rawhit->getColumn();
        bool goodU = (u == std::min(std::max(u, short(0)), short(249)));
        short v = rawhit->getRow();
        bool goodV = (v == std::min(std::max(v, short(0)), short(767)));
        return (goodU && goodV);
      }
      /** Name of the collection to use for PXDRawHits */
      std::string m_storeRawHitsName;
      /** Name of the collection to use for the PXDDigits */
      std::string m_storeDigitsName;
      /** Mode: if true, merge duplicate pixels, otherwise only keep the first. */
      bool m_mergeDuplicates;
      /** Minimum charge for a digit to carry */
      int m_0cut;
      /** Discard out-of-range hits. */
      bool m_trimOutOfRange;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

