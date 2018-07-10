/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  class ECLCalDigit;
  class ECLTRGInformation;
  class TRGECLUnpackerStore;
  class TrgEclMapping;

  /**
   * Module to find the closest ECLCalDigits to an extrapolated track
   */
  class ECLTRGInformationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLTRGInformationModule();

    /** initialize */
    virtual void initialize();

    /** event */
    virtual void event();

    /** terminate */
    virtual void terminate();

  private:
    /** vector (8736+1 entries) with cell id to store array positions */
    std::vector< int > m_calDigitStoreArrPosition;

    /** TC mapping class */
    TrgEclMapping* m_trgmap;

    StoreArray<ECLCalDigit> m_eclCalDigits; /**< Required input array of ECLCalDigits  */
    StoreArray<TRGECLUnpackerStore> m_trgUnpackerStore; /**< Required input array of TRGECLUnpackerStore  */

    StoreObjPtr<ECLTRGInformation> m_eclTRGInformation; /**< Analysis level information per event holding TRG information*/
  };
}
