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
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/KLMDigitEventInfo.h>
#include <klm/dataobjects/KLMDigitRaw.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dbobjects/KLMElectronicsMap.h>
#include <klm/dbobjects/KLMScintillatorFEEParameters.h>
#include <klm/time/KLMTime.h>

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
    ~KLMUnpackerModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /**
     * Create KLM digit.
     * @param[in] raw               Raw data.
     * @param[in] klmDigitRaw       Raw digit.
     * @param[in] klmDigitEventInfo Event information.
     * @param[in] subdetector       Subdetector.
     * @param[in] section           Section.
     * @param[in] sector            Sector.
     * @param[in] layer             Layer.
     * @param[in] plane             Plane.
     * @param[in] strip             Strip.
     * @param[in] lastStrip         Last strip (for multiple-strip hits).
     */
    void createDigit(
      const KLM::RawData* raw, const KLMDigitRaw* klmDigitRaw,
      KLMDigitEventInfo* klmDigitEventInfo, int subdetector, int section,
      int sector, int layer, int plane, int strip, int lastStrip);

    /**
     * Unpack KLM digit.
     * @param[in] rawData           Data to be unpacked.
     * @param[in] copper            Copper identifier.
     * @param[in] hslb              HSLB number.
     * @param[in] daqSubdetector    Subdetector (as determined from DAQ data).
     * @param[in] klmDigitEventInfo KLMDigitEventInfo.
     */
    void unpackKLMDigit(const int* rawData, int copper, int hslb,
                        int daqSubdetector,
                        KLMDigitEventInfo* klmDigitEventInfo);

    /**
     * Map a PCIe40 channel to the old and corresponding (COPPER, HSLB) address.
     * From Yamada-san, the map between (COPPER, HSLB) and PCIe40 is:
     * cpr7001 a,b,c,d -> PCIe40 ch.   0-3
     * cpr7002 a,b,c,d -> PCIe40 ch.   4-7
     * cpr7003 a,b,c,d -> PCIe40 ch.  8-11
     * cpr7004 a,b,c,d -> PCIe40 ch. 12-15
     * cpr8001 a,b,c,d -> PCIe40 ch. 16-19
     * cpr8002 a,b,c,d -> PCIe40 ch. 20-23
     * cpr8003 a,b,c,d -> PCIe40 ch. 24-27
     * cpr8004 a,b,c,d -> PCIe40 ch. 28-31
     * @param[in] channel PCIe40 channel.
     * @param[out] copper COPPER identifier.
     * @param[out] hslb   HSLB identifier.
     */
    void convertPCIe40ToCOPPER(int channel, unsigned int* copper, int* hslb) const;

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

    /** Flag to keep the even packages. */
    bool m_keepEvenPackages = false;

    /* Common database objects. */

    /** Electronics map. */
    DBObjPtr<KLMElectronicsMap> m_ElectronicsMap;

    /** Scintillator FEE parameters. */
    DBObjPtr<KLMScintillatorFEEParameters> m_FEEParameters;

    /** Time conversion. */
    KLMTime* m_Time;

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

    /* Other common variables. */

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Trigger ctime of the previous event. */
    unsigned int m_triggerCTimeOfPreviousEvent;

  };

}
