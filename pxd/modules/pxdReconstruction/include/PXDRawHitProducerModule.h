/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRawHitProducerModule_H
#define PXDRawHitProducerModule_H

#include <framework/core/Module.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <string>

namespace Belle2 {
  namespace PXD {
    /** \addtogroup modules
     * @{
     */

    /** The PXDRawHitProducer module.
     *
     * This module is used only for testing.
     * It converts PXDDigits from a simulation to PXDRawHits and randomly assigns
     * them to frames. It doesn't take care of any relations.
     *
     */
    class PXDRawHitProducerModule : public Module {

    public:
      /** Constructor defining the parameters */
      PXDRawHitProducerModule();

      /** Initialize the module */
      virtual void initialize();
      /** do the sorting */
      virtual void event();

    private:
      /** Name of the collection to use for PXDRawHits */
      std::string m_storeRawHitsName;
      /** Name of the collection to use for the PXDDigits */
      std::string m_storeDigitsName;
    };//end class declaration

    /** @}*/

  } //end PXD namespace;
} // end namespace Belle2

#endif // PXDRawHitProducerModule_H
