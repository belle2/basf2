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
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>
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

    SVDHotStripsCalibrations m_HotStripsCalib; /**<SVDHotStrips calibration db object*/

    /** store arrays*/
    StoreArray<SVDShaperDigit> m_storeShaper;

    SelectSubset< SVDShaperDigit > m_selectorIN; /**< selector of the subset of the good SVDShaperDigit*/
    SelectSubset< SVDShaperDigit > m_selectorOUT; /**< selector of the subset of the hot SVDShaperDigit*/

  protected:

    /** Name of the collections to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;
    std::string m_SVDShaperDigitsMasked; /**< masked SVDShaperDigits name*/
    std::string m_SVDShaperDigitsUnmasked; /**< unmasked (good) SVDShaperDigits name*/

  };
}

