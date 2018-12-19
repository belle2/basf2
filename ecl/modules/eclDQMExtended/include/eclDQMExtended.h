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
#include <boost/filesystem.hpp>
#include <regex>
#include <map>
#include <vector>
#include <string>
#include <iostream>

//FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

//ECL
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/dbobjects/ECLDspData.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>


class TH1F;
class TH2F;

namespace Belle2 {

  class ECLDspData;
  class ECLCrystalCalib;

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

    /** DBArray for payload 'ECLDSPPars0'. */
    DBArray<ECLDspData> m_ECLDspDataArray0;
    /** DBArray for payload 'ECLDSPPars1'. */
    DBArray<ECLDspData> m_ECLDspDataArray1;
    /** DBArray for payload 'ECLDSPPars2'. */
    DBArray<ECLDspData> m_ECLDspDataArray2;

    /** Low amplitude threshold. */
    DBObjPtr<ECLCrystalCalib> m_calibrationThrA0;
    /** Hit threshold. */
    DBObjPtr<ECLCrystalCalib> m_calibrationThrAhard;

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
    std::vector<short int>  v_totalthrA0;
    /** Vector to store hit thresholds. */
    std::vector<short int>  v_totalthrAhard;

    /** Signal amplitude obtaining from DSP emulator. */
    int m_AmpFit;
    /** Signal time obtaining from DSP emulator. */
    int m_TimeFit;
    /** Quality flag obtaining from DSP emulator. */
    int m_QualityFit;

    /** Signal amplitude from ECL data. */
    int m_AmpData;
    /** Signal time from ECL data. */
    int m_TimeData;
    /** Quality flag from ECL data. */
    int m_QualityData;

    /** Cell ID number. */
    int m_CellId;
    /** Trigger time value. */
    int m_TrigTime;

    /** Histogram: Amplitude for time mismatches w/ QualityData == 0. */
    TH1F* h_amp_timefail_q0;
    /** Histogram: Amplitude for time mismathes w/ QualityData == 3. */
    TH1F* h_amp_timefail_q3;
    /** Histogram: Amplitude for QualityFit == 0 && QualityData == 3. */
    TH1F* h_amp_qf0_qd3;
    /** Histogram: Amplitude for QualityFit == 3 && QualityData == 0. */
    TH1F* h_amp_qf3_qd0;
    /** Histogram: Time for QualityFit == 0 && QualityData == 3. */
    TH1F* h_time_qf0_qd3;
    /** Histogram: Time for QualityFit == 3 && QualityData == 0. */
    TH1F* h_time_qf3_qd0;

    /**  Histogram: Amp. Control flags in bins of QualityData. */
    TH1F* h_ampfail_quality;
    /** Histogram: Time control flags in bins of QualityData. */
    TH1F* h_timefail_quality;
    /** Histogram: CellIDs w/ failed amplitudes. */
    TH1F* h_ampfail_cellid;
    /** Histogram: CellIDs w/ failed times. */
    TH1F* h_timefail_cellid;
    /** Histogram: CellIDs w/ failed qualities. */
    TH1F* h_qualityfail_cellid;

    /** Histogram: Amplitude diff. (Emulator - Data) for amp. fails. */
    TH2F* h_ampdiff_cellid;
    /** Histogram: Time diff.(Emulator-Data) for time fails. */
    TH2F* h_timediff_cellid;
    /** Histogram: Amp. diff. (Emulator-Data) for amp. fails in bins of QualityData. */
    TH2F* h_ampdiff_quality;
    /** Histogram: Time diff. (Emulator-Data) for time fails in bins of QualityData. */
    TH2F* h_timediff_quality;
    /** Histogram: Amp. diff. (Emulator-Data) for amp. fails in bins of Shaper Id. */
    TH2F* h_ampdiff_shaper;
    /** Histogram: Time diff. (Emulator-Data) for time fails in bins of Shaper Id. */
    TH2F* h_timediff_shaper;
    /** Histogram: QualityFit vs QualityData for quality fails. */
    TH2F* h_quality_fit_data;
    /** Histogram: Amp flag (0/1) w/ failed qualities in bins of QualityData. */
    TH2F* h_ampflag_qualityfail;
    /**Histogram: Time flag (0/1) w/ failed qualities in bins of Quality Data. */
    TH2F* h_timeflag_qualityfail;


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
