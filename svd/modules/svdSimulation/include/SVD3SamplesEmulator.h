/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa & Luigi Corona                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD3SAMPLESEMULATORMODULE_H
#define SVD3SAMPLESEMULATORMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <string>

namespace Belle2 {
  /**This module takes the SVDShaperDigit as input and select three consecutive samples starting from the one choosen by the user. The modules creates a new StoreArray of the class ShaperDigit whit three samples only, selected from the original ShaperDigits. The three samples are stored in the first three positions of the APVSamples store array, and the last three are set to 0.
   */
  class SVD3SamplesEmulatorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVD3SamplesEmulatorModule();

    /** if required */
    virtual ~SVD3SamplesEmulatorModule();

    /** initializes the module */
    virtual void beginRun() override;

    /** processes the event */
    virtual void event() override;

    /** end the run */
    virtual void endRun() override;

    /** terminates the module */
    virtual void terminate() override;

    /** init the module */
    virtual void initialize() override;

  private:

    std::string m_shaperDigitInput;  /**< StoreArray with the 6-samples input shaperdigits with DAQMode = 2*/
    Int_t m_startingSample = 0;  /**< Starting sample from which select the three samples of the initial six*/
    std::string m_outputArrayName;  /**< StoreArray with the 3-samples output shaperdigits, with DAQMode = 1*/

  };
}

#endif /* SVD3SAMPLESEMULATORMODULE_H */
