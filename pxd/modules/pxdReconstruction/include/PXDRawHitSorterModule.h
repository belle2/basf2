/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRawHitSorterModule_H
#define PXDRawHitSorterModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/online/PXDIgnoredPixelsMap.h>
#include <string>

namespace Belle2 {
  namespace PXD {

    /** The PXDRawHitSorter module.
     *
     * This module sorts the existing PXDRawHits collection and saves PXDDigits
     * ordered by frame and then by row and column for each half-ladder. This
     * provides ordered input to PXDClusterizer.
     *
     * @see PXDClusterizerModule
     */
    class PXDRawHitSorterModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDRawHitSorterModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the sorting */
      virtual void event();

    private:
      StoreArray<PXDRawHit> m_pxdRawHit; /**< Required input for  PXDRawHit */
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
      /** Name of the collection to use for the PXDFrames */
      std::string m_storeFramesName;
      /** Mode: if true, merge duplicate pixels, otherwise only keep the first. */
      bool m_mergeDuplicates;
      /** Mode: if true, merge frames, otherwise keep separate frames */
      bool m_mergeFrames;
      /** Minimum charge for which a digit is created. */
      double m_0cut;
      /** Assign VXDID to data that don't have it. */
      bool m_assignID;
      /** Discard out-of-range hits. */
      bool m_trimOutOfRange;
      /** Name of the ignored pixels list xml */
      std::string m_ignoredPixelsListName;
      /** Ignored pixels list manager class */
      std::unique_ptr<PXDIgnoredPixelsMap> m_ignoredPixelsList;
    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDRawHitSorterModule_H
