/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once


#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <framework/datastore/SelectSubset.h>
#include <string>

namespace Belle2 {

  /** This module removes the strips to be masked read form the SVDHotStripsCalibration
   */

  class SVDStripMaskingModule : public Module {
  public:

    /** Constructor defining the parameters */
    SVDStripMaskingModule();

    virtual ~SVDStripMaskingModule();

    /** Initialize the SVDStripMasking.*/
    virtual void initialize() override;

    /** This method is the core of the SVDStripMasking. */
    virtual void event() override;


    bool m_createOutside; /**<if true a StoreArray of Hot Strips is created*/

  private:

    /** store arrays*/
    StoreArray<SVDShaperDigit> m_storeShaper;

    SelectSubset< SVDShaperDigit > m_selectorIN; /**< selector of the subset of the good SVDShaperDigit*/
    SelectSubset< SVDShaperDigit > m_selectorOUT; /**< selector of the subset of the hot SVDShaperDigit*/

  protected:

    /** Name of the collections to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;
    std::string m_SVDShaperDigitsMasked;
    std::string m_SVDShaperDigitsUnmasked;

  };
}

