/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/gearbox/Const.h>

#include <TTree.h>

namespace Belle2 {

  /**
   * Module to produce ntuple from TOPDigits and TOPRawDigits
   */
  class TOPInterimFENtupleModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TOPInterimFENtupleModule();

    /**
     * Destructor
     */
    virtual ~TOPInterimFENtupleModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Module funcions to define histograms
     */
    virtual void defineHisto();

  private:

    /**
     * enum for maximum number of array elements (# of hits per event)
     */
    enum { c_NWindow = 4, c_NModule = 16, c_NSamplePerWindow = 64, c_NWaveformSample = 256,
           c_NSampleTBC = 256, c_NPixelPerModule = 512, c_NWindowRingBuffer = 512,
           c_NMaxHitPerChannel = 5, c_NMaxHitEvent = 5000, c_NTotalScrod = 64
         };

    TTree* m_tree = 0; /**< ntuple */
    unsigned m_calibrationChannel = 0; /**< asic channel number where the calibration pulses are routed */
    bool m_saveWaveform = false; /**< set true when you want to save waveform data */
    bool m_useDoublePulse = true; /**< set true when you require both of double calibration pulses for reference timing */
    // float m_averageSamplingRate = 2.71394; /**< sampling rate with assumption of uniform interval in a unit of GHz */
    float m_calibrationPulseThreshold1 =
      600; /**< minimum pulse height for the first calibration pulse to be qualified as calibration signals */
    float m_calibrationPulseThreshold2 =
      450; /**< minimum pulse height for the secon calibration pulse to be qualified as calibration signals */
    float m_calibrationPulseInterval = 21.85; /**< nominal DeltaT value (time interval of two calibration signals) in a unit of ns */
    float m_calibrationPulseIntervalRange = 2; /**< tolerable shift of DeltaT from its nominal before calibration in a unit of ns */
    float m_timePerWin = 23.581939; /**< time interval of onw window (=64 samples) [ns]  */
    int m_globalRefSlotNum =
      1;/**< slot number used to define "global" reference timing (a single reference timing in an event for all the channels of all the 16 modules.) slot01-asic0 (pixelId=1-8) is default. */
    int m_globalRefAsicNum =
      0;/**< asic number used to define "global" reference timing. This asic number is given as int((pixelId-1)/8). */

    int m_nHit = 0; /**< number of hits for the event */
    unsigned m_eventNumCopper[c_NModule] = {0}; /**< event number stored in COPPER */
    unsigned m_ttuTime[c_NModule] = {0}; /**< counter for TTclock, stored in COPPER */
    unsigned m_ttcTime[c_NModule] = {0}; /**< counter for TTclock, stored in COPPER */
    short m_slotNum[c_NMaxHitEvent] = {0}; /**< "m_moduleID" in TOPDigit, slot number */
    short m_pixelId[c_NMaxHitEvent] = {0}; /**< "m_pixelID" in TOPDigit */
    short m_channelId[c_NMaxHitEvent] = {0}; /**< "m_channel" in TOPDigit */
    bool m_isCalCh[c_NMaxHitEvent] = {0}; /**< true if the hit is in the calibration channel */
    unsigned m_eventNum = 0; /**< event number taken from EventMetaData */
    short m_winNum[c_NMaxHitEvent] = {0}; /**< "m_firstWindow" in TOPDigit */
    short m_trigWinNum[c_NMaxHitEvent] = {0}; /**< "m_lastWriteAddr" in TOPRawDigit, window # when trigger is issued  */
    short m_winNumList[c_NMaxHitEvent][c_NWindow] = {0}; /**< list of window numbers for recorded waveform, valid only when waveform analysis is enabled */
    bool m_windowsInOrder[c_NMaxHitEvent] = {0}; /**< "areWindowsInOrder()" ; false if the window number of all (4) windows taken from TOPRawWaveform::getReferenceWindows() are consecutive */
    unsigned char m_hitQuality[c_NMaxHitEvent] = {0}; /**< "m_quality" in TOPDigit, =0:junk, =1:good, =2:charge sharing, =3:cross talk, =4:cal. pulse, +10 if cal. pulse is properly identified for the asic, +100(+200) for the first(second) calibration signal itself  */
    bool m_isReallyJunk[c_NMaxHitEvent] = {0}; /**< true if pedestal jump or other invalid hit is detected, not saved */
    float m_time[c_NMaxHitEvent] = {0}; /**< time in a unit of ns, defined as m_rawTime+64*m_winNum. Converted into ns unit with assumption of uniform sampling interval. 0 ns at the start of the window number 0. When waveform analysis is enabled, discontinuous window numbers are considered. */
    float m_rawTime[c_NMaxHitEvent] = {0}; /**< "m_rawTime" [0-256] in new TOPDigit (update at May, 2017) in sample(time bin) unit */
    float m_refTime[c_NMaxHitEvent] = {0}; /**< time of the first calibration signal as reference timing. Chosen from a list of m_time for each asic. When waveform analysis is enabled, double pulse is required. Otherwise, hit timing in the calibration channel of a correcponding asic is used. */
    float m_globalRefTime =
      0; /**< refTime of the specific asic, which is specified by parameters "globalRefSlotNum" and "globalRefAsicNum" */
    unsigned short m_sample[c_NMaxHitEvent] = {0}; /**< (m_rawTDC+m_winNum*64)%256, for time base correction */
    float m_height[c_NMaxHitEvent] = {0}; /**< "m_pulseHeight" in new TOPDigit (update at May, 2017) */
    float m_integral[c_NMaxHitEvent] = {0}; /**< "m_integral" in TOPDigit, but not available */
    float m_width[c_NMaxHitEvent] = {0}; /**< "m_pulseWidth" in TOPDigit, full width at half maximum of the pulse, converted into unit of samples from ns */
    unsigned short m_peakSample[c_NMaxHitEvent] = {0}; /**< sample number for peak */
    char m_offlineFlag[c_NMaxHitEvent] = {0}; /**< =-1 if the channel does not have waveform information, =0 when the hit comes from online FE, >=1 when the hit comes from offline FE ; =-100 corresponding TOPRawDigit object is not found (problematic case) */
    short m_nHitOfflineFE[c_NMaxHitEvent] = {0}; /**< number of hits for each pixel */
    short m_waveformStartSample[c_NMaxHitEvent] = {0}; /**< start sample number of waveform segment (only for production FW) */
    unsigned short m_nWaveformSample[c_NMaxHitEvent] = {0}; /**< number of waveform samples */
    short m_waveform[c_NMaxHitEvent][c_NWaveformSample] = {0}; /**< waveform from TOPRawWaveform, if not exist, filled with -32767 */

    short m_nFEHeader = 0; /**< m_FEHeaders in TOPInterimFEInfo, the total # of FE headers found */
    short m_nEmptyFEHeader = 0; /**< m_emptyFEHeaders in TOPInterimFEInfo, the total # of empty FE headers */
    short m_nWaveform = 0; /**< m_waveforms in TOPInterimFEInfo, # of waveformes */
    unsigned m_errorFlag = 0; /**< m_errorFlags in TOPInterimFEInfo, defined in the TOPInterimFEInfo.h */

    unsigned m_eventErrorFlag = 0; /**< m_errorFlags in EventMetaData, 0x1 : CRC error */


    int m_nDebugInfo = 0; /**< number of ProductionEventDebug (in a unit of number of boardstack) */
    unsigned short m_scrodCtime[c_NTotalScrod] = { 0 };  /**< ctime recorded in scrod header */
    unsigned short m_phase[c_NTotalScrod] = { 0 };  /**< event phase */
    unsigned short m_asicMask[c_NTotalScrod] = { 0 };  /**< asic mask bit pattern */
    unsigned short m_eventQueuDepth[c_NTotalScrod] = { 0 };  /**< trigger FIRO queue depth */
    unsigned short m_eventNumberByte[c_NTotalScrod] = { 0 };  /**< least significant byte of FE event number */


    /**
     * Find reference timing. In case that the waveform analysis is enabled,
     * try to find a pair of calibration pulses and timing of the first one
     * is used as reference timing for the correcponding asic.
     * If not enabled, feature extracted timing for a calibration channel is used.
     */
    void getReferenceTiming();

  };

}  //namespace Belle2
