/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVD3SAMPLESEMULATORMODULE_H
#define SVD3SAMPLESEMULATORMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDEventInfo.h>

#include <string>
#include <cmath>

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
    StoreArray<SVDShaperDigit> m_ShaperDigit; /**< StoreArray with the 6-samples input shaperdigits with DAQMode = 2*/
    std::string m_shaperDigitInputName;  /**< name of the input ShaperDigits StoreArray */
    StoreObjPtr<SVDEventInfo> m_storeSVDEvtInfo;  /**<storage for SVDEventInfo object required for the module */
    StoreObjPtr<SVDEventInfo> m_storeSVDEvtInfo3samples; /**<storage for SVDEventInfo object produced for 3 samples */
    Int_t m_startingSample = 0;  /**< Starting sample from which select the three samples of the initial six*/
    std::string m_outputArrayName;  /**< StoreArray with the 3-samples output shaperdigits, with DAQMode = 1*/
    std::string m_svdEventInfoName; /**< Name of the input SVDEventInfo object */
    std::string m_svdEventInfoOutName; /**< Name of the output SVDEventInfo object */
    bool m_chooseStartingSample; /**< Set it True if you want to choose the starting sample manually */
    bool m_chooseRelativeShift; /**< Set it True if you want to choose the relative shift manually */
    int m_relativeShift; /**< Relative shift */
    /** return the starting sample */
    int getFirstSample(const SVDModeByte
                       modeByte); /**< return the starting sample from the information of the Trigger Bin and relative shift */
  };
}

#endif /* SVD3SAMPLESEMULATORMODULE_H */
