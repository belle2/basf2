/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#ifndef SVDZSEMULATOR_H
#define SVDZSEMULATOR_H


#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <framework/datastore/SelectSubset.h>
#include <string>

namespace Belle2 {

  /** This module filters out strips that do not pass a ZS cut from the SVDShaperDigit StoreArray.
   * The strip is filtered out if less than nSamples are above a SN threshold.
   */

  class SVDZeroSuppressionEmulatorModule : public Module {
  public:

    /** Constructor defining the parameters */
    SVDZeroSuppressionEmulatorModule();

    virtual ~SVDZeroSuppressionEmulatorModule();

    /** Initialize the SVDZeroSuppressionEmulator.*/
    virtual void initialize() override;

    /** This method is the core of the SVDZeroSuppressionEmulator. */
    virtual void event() override;


    int m_nSample = 1; /**< minimum number of samples required to be above threshold*/
    float m_cutSN = 3; /**< SN ratio threshold*/
    bool m_createOutside = false; /**<if true a StoreArray of Strips zero-suppressed is created*/

  private:

    /** store arrays*/
    StoreArray<SVDShaperDigit> m_storeShaper;

    SelectSubset< SVDShaperDigit > m_selectorIN; /**< selector of the subset of the SVDShaperDigit passing ZS*/
    SelectSubset< SVDShaperDigit > m_selectorOUT; /**< selector of the subset of the SVDShaperDigit not passing ZS*/

    bool passesZS(const SVDShaperDigit*);  /**< returns true if the strip passes the ZS cut*/

    //calibration objects
    SVDNoiseCalibrations m_NoiseCal; /**<SVDNoise calibration db object*/

  protected:

    /** Name of the collections to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;
    std::string m_SVDShaperDigitsIN; /**< name of SVDShaperDigits passing ZS*/
    std::string m_SVDShaperDigitsOUT; /**< name of SVDShaperDigits NOT passing ZS*/
    bool m_FADCmode = true; /**< if true, same algorithm as on FADC is applied*/

  };
}
#endif
