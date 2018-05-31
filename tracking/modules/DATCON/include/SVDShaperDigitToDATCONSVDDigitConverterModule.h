/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
// #include <framework/datastore/DataStore.h>
// #include <framework/datastore/RelationArray.h>
// #include <framework/datastore/RelationIndex.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>

#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/geometry/SensorInfo.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/dataobjects/DATCONSVDDigit.h>


namespace Belle2 {

  /** SVDShaperDigitToDATCONSVDDigitConverterModule:
   *
   * This module converts SVDShaperDigits to DATCONSVDDigits and creates the
   * relations needed.
   */
  class SVDShaperDigitToDATCONSVDDigitConverterModule : public Module {

  public:
    /**
     * Constructor of the module.
     */
    SVDShaperDigitToDATCONSVDDigitConverterModule();

    /**
     * Destructor of the module.
     */
    virtual ~SVDShaperDigitToDATCONSVDDigitConverterModule() = default;

    /**
     *Initializes the Module.
     */
    virtual void initialize() override;
    virtual void event() override;                /**< Event function */

    std::string m_storeSVDShaperDigitsListName;   /**< SVDShaperDigits list name */
    std::string m_storeDATCONSVDDigitsListName;   /**< DATCONSVDDigits list name */
    std::string m_storeTrueHitsListName;          /**< SVDTrueHits list name */
    std::string m_storeMCParticlesListName;       /**< MCParticles list name */

  private:

    StoreArray<SVDShaperDigit> storeSVDShaperDigits;    /**< SVDShaperDigits StoreArray */
    StoreArray<DATCONSVDDigit> storeDATCONSVDDigits;    /**< DATCONSVDDigits StoreArray */
    StoreArray<SVDTrueHit> storeTrueHits;               /**< SVDTrueHits StoreArray */
    StoreArray<MCParticle> storeMCParticles;            /**< MCParticles StoreArray */

  };//end class declaration

} // end namespace Belle2
