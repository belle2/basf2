/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * ECL Data Quality Monitor (Second Module)                               *
 *                                                                        *
 * This module provides DQM histograms to check out ECL electronics logic *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitry Matvienko (d.v.matvienko@inp.nsk.su)              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// Module manager
#include <framework/core/HistoModule.h>

//STL
#include <map>
#include <vector>
#include <string>

//FRAMEWORK
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLDspData.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/utility/ECLChannelMapper.h>


class TH1F;
class TH2F;

namespace Belle2 {

  /**
   * This module is created to monitor ECL electronics logic in frame of DQM system.
   */
  class ECLDQMEXTENDEDModule : public HistoModule {  /**< derived from HistoModule class. */

  public:

    /** Constructor. */
    ECLDQMEXTENDEDModule();

    /** Destructor. */
    virtual ~ECLDQMEXTENDEDModule() override;

    /** Initialize the module. */
    virtual void initialize() override;
    /** Call when a run begins. */
    virtual void beginRun() override;
    /** Event processor. */
    virtual void event() override;
    /** Call when a run ends. */
    virtual void endRun() override;
    /** Terminate. */
    virtual void terminate() override;

    /** Function to define histograms. */
    virtual void defineHisto() override;

  private:
    /** Histogram directory in ROOT file. */
    std::string m_histogramDirectoryName;
    /** Key to initialize DSP coeffs. ('File' or 'DB' are acceptable). */
    std::string m_InitKey;
    /** Directory name consisting of DSP coeffs. */
    std::string m_DSPDirectoryName;
    /** Run with valid DSP coeffs. (only for 'm_InitKey == File'). */
    std::string m_RunName;
    /** Save detailed fit data for failed fits */
    bool m_SaveDetailedFitData;
    /** Use modified time determination algorithm in emulator, same as in ShaperDSP version >= 1.4.3 */
    bool m_adjusted_timing;

    /** DBArray for payload 'ECLDSPPars0'. */
    DBArray<ECLDspData> m_ECLDspDataArray0;
    /** DBArray for payload 'ECLDSPPars1'. */
    DBArray<ECLDspData> m_ECLDspDataArray1;
    /** DBArray for payload 'ECLDSPPars2'. */
    DBArray<ECLDspData> m_ECLDspDataArray2;

    /** ECL digits. */
    StoreArray<ECLDigit> m_ECLDigits;

    /** ECL trigger data. */
    StoreArray<ECLTrig> m_ECLTrigs;

    /** ECL DSP data. */
    StoreArray<ECLDsp> m_ECLDsps;

    /** Low amplitude threshold. */
    DBObjPtr<ECLCrystalCalib> m_calibrationThrA0;
    /** Hit threshold. */
    DBObjPtr<ECLCrystalCalib> m_calibrationThrAhard;
    /** Skip amplitude threshold. */
    DBObjPtr<ECLCrystalCalib> m_calibrationThrAskip;

    /** ECL channel mapper. */
    ECL::ECLChannelMapper mapper;

    /** Map to store DSP coeffs. for one shaper */
    std::map<std::string, std::vector<short int>> map_vec;
    /** Map to store auxiliary constants for one shaper. */
    std::map<std::string, short int> map_coef;
    /** Map to store DSP coeffs. for all shapers. */
    std::map<int, std::map<std::string, std::vector<short int>>> map_container_vec;
    /** Map to store auxiliary constants for all shapers. */
    std::map<int, std::map<std::string, short int>> map_container_coef;

    /** Vector to store Low amplitude thresholds. */
    std::vector<short int>  v_totalthrA0 = {};
    /** Vector to store hit thresholds. */
    std::vector<short int>  v_totalthrAhard = {};
    /** Vector to store skip amplitude threshold. */
    std::vector<short int>  v_totalthrAskip = {};

    /** Signal amplitude obtaining from DSP emulator. */
    int m_AmpFit{0};
    /** Signal time obtaining from DSP emulator. */
    int m_TimeFit{0};
    /** Quality flag obtaining from DSP emulator. */
    int m_QualityFit{0};

    /** Signal amplitude from ECL data. */
    int m_AmpData{0};
    /** Signal time from ECL data. */
    int m_TimeData{0};
    /** Quality flag from ECL data. */
    int m_QualityData{0};

    /** Cell ID number. */
    int m_CellId{0};
    /** Trigger time value. */
    int m_TrigTime{0};

    /** Histogram vector: Amplitude for time mismacthes (AmplitudeFit == AmplitudeData) w/ various QualityData values. */
    std::vector<TH1F*> h_amp_timefail = {};
    /** Histogram vector: Time for amplitude mismathces (TimeFit == TimeData) w/ various QualityData values. */
    std::vector<TH1F*> h_time_ampfail = {};
    /** Histogram vector: AmplitudeData for quality mismathes w/ various QualityFit (raw) and QualityData (column) values. */
    std::vector<std::vector<TH1F*>> h_amp_qualityfail = {};
    /** Histogram vector: TimeData for quality mismacthes w/ various QualitFit (raw) and QualityData (column) values. */
    std::vector<std::vector<TH1F*>> h_time_qualityfail = {};

    /**  Histogram: Amp. Control flags in bins of QualityData. */
    TH1F* h_ampfail_quality{nullptr};
    /** Histogram: Time control flags in bins of QualityData. */
    TH1F* h_timefail_quality{nullptr};

    /** Histogram: CellIDs w/ failed amplitudes. */
    TH1F* h_ampfail_cellid{nullptr};
    /** Histogram: CellIDs w/ failed times. */
    TH1F* h_timefail_cellid{nullptr};
    /** Histogram: CellIDs w/ failed amplitudes and times. */
    TH1F* h_amptimefail_cellid{nullptr};
    /** Histogram: CellIDs w/ failed qualities. */
    TH1F* h_qualityfail_cellid{nullptr};
    /** Histogram: ShaperIDs w/ failed amplitudes. */
    TH1F* h_ampfail_shaperid{nullptr};
    /** Histogram: ShaperIDs w/ failed times. */
    TH1F* h_timefail_shaperid{nullptr};
    /** Histogram: ShaperIDs w/ failed amplitudes and times. */
    TH1F* h_amptimefail_shaperid{nullptr};
    /** Histogram: ShaperIDs w/ failed qualities. */
    TH1F* h_qualityfail_shaperid{nullptr};
    /** Histogram: ShaperIDs w/ failed logic. */
    TH1F* h_fail_shaperid{nullptr};
    /** Histogram: CrateIDs w/ failed amplitudes. */
    TH1F* h_ampfail_crateid{nullptr};
    /** Histogram: CrateIDs w/ failed times. */
    TH1F* h_timefail_crateid{nullptr};
    /** Histogram: CrateIDs w/ failed amplitudes and times. */
    TH1F* h_amptimefail_crateid{nullptr};
    /** Histogram: CrateIDs w/ failed qualities. */
    TH1F* h_qualityfail_crateid{nullptr};
    /** Histogram: CrateIDs w/ failed logic. */
    TH1F* h_fail_crateid{nullptr};

    /** Histogram: Amplitude diff. (Emulator - Data) for amp. fails. */
    TH2F* h_ampdiff_cellid{nullptr};
    /** Histogram: Time diff.(Emulator-Data) for time fails. */
    TH2F* h_timediff_cellid{nullptr};
    /** Histogram: Amp. diff. (Emulator-Data) for amp. fails in bins of Shaper Id. */
    TH2F* h_ampdiff_shaperid{nullptr};
    /** Histogram: Time diff. (Emulator-Data) for time fails in bins of Shaper Id. */
    TH2F* h_timediff_shaperid{nullptr};
    /** Histogram: Amp. diff. (Emulator-Data) for amp. fails in bins of QualityData. */
    TH2F* h_ampdiff_quality{nullptr};
    /** Histogram: Time diff. (Emulator-Data) for time fails in bins of QualityData. */
    TH2F* h_timediff_quality{nullptr};
    /** Histogram: QualityFit vs QualityData for quality fails. */
    TH2F* h_quality_fit_data{nullptr};
    /** Histogram: Amp flag (0/1) w/ failed qualities in bins of QualityData. */
    TH2F* h_ampflag_qualityfail{nullptr};
    /**Histogram: Time flag (0/1) w/ failed qualities in bins of Quality Data. */
    TH2F* h_timeflag_qualityfail{nullptr};


    /** Convert a CellID number to the global Shaper number. */
    int conversion(int);
    /** Select from vector of DSP coeffs a subvector corresponding to accurate channel number. */
    short int* vectorsplit(std::vector<short int>&, int);
    /** Read calibration values for thresholds from DBObject. */
    void callbackCalibration(DBObjPtr<ECLCrystalCalib>&, std::vector<short int>&);
    /** Read DSP coeffs and auxiliary constants from DBObject. */
    void callbackCalibration(ECLDspData*, std::map<std::string, std::vector<short int>>&, std::map<std::string, short int>&);
    /** Get DSP coeffs and auxiliary constants from DB. */
    void initDspfromDB();
    /** Get DSP coeffs and auxiliary constants from Files. */
    void initDspfromFile();
    /** Call for DSP emulator. */
    void emulator(int, int, std::vector<int>);
  };
}; // end Belle2 namespace
