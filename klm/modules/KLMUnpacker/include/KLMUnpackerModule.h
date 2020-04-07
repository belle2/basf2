/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/ElementNumbersSingleton.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/dataobjects/KLMDigitRaw.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/bklm/BKLMADCThreshold.h>
#include <klm/dbobjects/eklm/EKLMChannels.h>
#include <klm/dbobjects/KLMElectronicsMap.h>
#include <klm/dbobjects/KLMTimeConversion.h>

/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawKLM.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * KLM unpacker.
   */
  class KLMUnpackerModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMUnpackerModule();

    /**
     * Destructor.
     */
    virtual ~KLMUnpackerModule();

    /**
     * Initializer.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

    /**
     * This method is called for each event.
     */
    virtual void event() override;

    /**
     * This method is called if the current run ends.
     */
    virtual void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    virtual void terminate() override;

  private:

    /**
     * Unpack KLM digit.
     * @param[in] rawData           Data to be unpacked.
     * @param[in] copper            Copper identifier.
     * @param[in] hslb              HSLB number.
     * @param[in] daqSubdetector    Subdetector (as determied from DAQ data).
     * @param[in] klmDigitEventInfo KLMDigitEventInfo.
     */
    void unpackKLMDigit(const int* rawData, int copper, int hslb,
                        int daqSubdetector,
                        KLMDigitEventInfo* klmDigitEventInfo);

    /**
     * To be used to map electronics address to module id.
     *
     * @param copperId
     * Id of the copper board.
     *
     * @param finesseNum
     * The Finesse slot on the copper boards.
     *
     * @param lane
     * The lane number, giving for the rpcs the slot number in the crate.
     *
     * @param axis
     * The axis bit in the datapacket.
     */
    int electCooToInt(int copper, int finesse, int lane, int axis, int channel);

    /* Module parameters. */

    /** Name of KLMDigit store array. */
    std::string m_outputKLMDigitsName;

    /** Record raw data in dataobject format (for debugging). */
    bool m_WriteDigitRaws;

    /** Record wrong hits (for debugging). */
    bool m_WriteWrongHits;

    /** Debug electronics map (record DAQ channel instead of strip). */
    bool m_DebugElectronicsMap;

    /** Record DAQ channel for BKLM scintillators. */
    bool m_DAQChannelBKLMScintillators;

    /** Record DAQ channel for specific module. */
    int m_DAQChannelModule;

    /* EKLM parameters. */

    /**
     * Do not issue B2ERROR on wrong hits, with certain firmware versions
     * wrong strip numbers are expected.
     */
    bool m_IgnoreWrongHits;

    /**
     * Ignore hits with strip = 0. Such hits are normally expected for normal
     * firmware versions.
     */
    bool m_IgnoreStrip0;

    /* BKLM parameters. */

    /** The flag to keep the even packages. */
    bool m_keepEvenPackages = false;

    /** Load threshold from DataBase (true) or not (false). */
    bool m_loadThresholdFromDB = true;

    /** Threshold for the scintillator NPhotoelectrons .*/
    double m_scintThreshold = 140;

    /* Common database objects. */

    /** Electronics map. */
    DBObjPtr<KLMElectronicsMap> m_ElectronicsMap;

    /** Time conversion. */
    DBObjPtr<KLMTimeConversion> m_TimeConversion;

    /** Raw data. */
    StoreArray<RawKLM> m_RawKLMs;

    /** Digits. */
    StoreArray<KLMDigit> m_Digits;

    /** Event information. */
    StoreArray<KLMDigitEventInfo> m_DigitEventInfos;

    /** Out-of-range digits. */
    StoreArray<KLMDigit> m_klmDigitsOutOfRange;

    /** Raw digits. */
    StoreArray<KLMDigitRaw> m_klmDigitRaws;

    /* EKLM database objects. */

    /** Channels. */
    DBObjPtr<EKLMChannels> m_eklmChannels;

    /* BKLM database objects. */

    /** ADC offset and threshold read from database. */
    DBObjPtr<BKLMADCThreshold> m_bklmADCParams;

    /* Other common variables. */

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Trigger ctime of the previous event. */
    unsigned int m_triggerCTimeOfPreviousEvent;

    /* Other EKLM variables. */

    /** Element numbers. */
    const EKLM::ElementNumbersSingleton* m_eklmElementNumbers;

    /* Other BKLM variables. */

    /**
     * Counter for channels that were rejected due to unreasonable
     * channel number.
     */
    long m_rejectedCount = 0;

    /**
     * Warning message: number of channels rejected due to unreasonable
     * channel number.
     */
    std::map<std::string, long> m_rejected;

  };

}
