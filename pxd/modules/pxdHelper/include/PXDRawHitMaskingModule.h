/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
     * This module sorts the existing PXDRawHits collection and saves PXDDigits
     * ordered by row and column for each half-ladder. This
     * provides ordered input to PXDClusterizer.
     *
     * @see PXDClusterizerModule
     */
    class PXDRawHitMaskingModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDRawHitMaskingModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the sorting */
      virtual void event();

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

