/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef SVDHOTSTRIPSFILTER_H
#define SVDHOTSTRIPSFILTER_H


#include <framework/core/Module.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <framework/datastore/SelectSubset.h>
#include <string>

namespace Belle2 {

  /** This module filters out the hot strips from the SVDShaperDigit StoreArray.
   * The strip is filtered out if only one sample is above a SN threshold.
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


    int m_nSample; /**< maximum number of samples above threshold to mark the strip as hot*/
    float m_cutSN; /**< SN ratio threshold*/
    bool m_createOutside; /**<if true a StoreArray of Hot Strips is created*/

  private:

    /** store arrays*/
    StoreArray<SVDShaperDigit> m_storeShaper;

    SelectSubset< SVDShaperDigit > m_selectorIN; /**< selector of the subset of the SVDShaperDigit passing ZS*/
    SelectSubset< SVDShaperDigit > m_selectorOUT; /**< selector of the subset of the SVDShaperDigit not passing ZS*/

    bool passesZS(const SVDShaperDigit*);  /**< returns true if the strip is hot*/

    //calibration objects
    SVDNoiseCalibrations m_NoiseCal;

  protected:

    /** Name of the collections to use for the SVDShaperDigits */
    std::string m_storeShaperDigitsName;
    std::string m_SVDShaperDigitsIN;
    std::string m_SVDShaperDigitsOUT;

  };
}
#endif
