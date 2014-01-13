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
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <string>

namespace Belle2 {
  namespace PXD {
    /** \addtogroup modules
     * @{
     */

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
    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDRawHitSorterModule_H
