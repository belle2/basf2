#ifndef TRCGDLDQMMODULE_h
#define TRCGDLDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/gdl/dataobjects/TRGGDLUnpackerStore.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <trg/gdl/TrgBit.h>
#include <framework/database/DBObjPtr.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <string>

#include <TH2I.h>
#include <TH1I.h>
#include "trg/ecl/TrgEclMapping.h"
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/ECLCluster.h>
#include "trg/ecl/dataobjects/TRGECLCluster.h"


namespace Belle2 {


  class TRGGDLDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGGDLDQMModule();
    //! Destrunctor
    virtual ~TRGGDLDQMModule() {}

  public:
    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    virtual void terminate() override {}
    //! Define Histogram
    virtual void defineHisto() override;

  protected:
    //number of skims
    static const int nskim_gdldqm = 11;
    int start_skim_gdldqm = 0;
    int end_skim_gdldqm = 0;
    int m_skim = -1;

    //! timestamp diff from gdlL1 to comL1 in LSB8nsec
    TH1I* h_c8_gdlL1TocomL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(top) to gdlL1 in LSB8nsec
    TH1I* h_c8_topTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(ecl) to gdlL1 in LSB8nsec
    TH1I* h_c8_eclTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(cdc) to gdlL1 in LSB8nsec
    TH1I* h_c8_cdcTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from fit on fam to GDL in LSB8nsec
    TH1I* h_c8_ecl8mToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(top) to GDL in LSB8nsec
    TH1I* h_c8_topToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(ecl) to GDL in LSB8nsec
    TH1I* h_c8_eclToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from t0(cdc) to GDL in LSB8nsec
    TH1I* h_c8_cdcToGDL[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to comL1 in LSB2nsec
    TH1I* h_c2_cdcTocomL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to comL1 in nsec
    TH1D* h_ns_cdcTocomL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to gdlL1 in nsec
    TH1D* h_ns_cdcTogdlL1[nskim_gdldqm] = {nullptr};
    //! timestamp diff from top_timing to ecl_timing in nsec
    TH1D* h_ns_topToecl[nskim_gdldqm] = {nullptr};
    //! timestamp diff from top_timing to cdc_timing in nsec
    TH1D* h_ns_topTocdc[nskim_gdldqm] = {nullptr};
    //! timestamp from cdc_timing to ecl_timing to L1 in LSB2nsec
    TH1I* h_c2_cdcToecl[nskim_gdldqm] = {nullptr};
    //! timestamp diff from cdc_timing to ecl_timing in nsec
    TH1D* h_ns_cdcToecl[nskim_gdldqm] = {nullptr};

    //! input bits
    // TH1I* h_inp;
    // TH1I* h_inp_rise[N_BITS_RESERVED][nskim_gdldqm];
    // TH1I* h_inp_fall[N_BITS_RESERVED][nskim_gdldqm];
    //! itd bits
    TH1I* h_itd[nskim_gdldqm] = {nullptr};
    TH1I* h_itd_rise[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_itd_fall[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    //! ftd bits
    TH1I* h_ftd[nskim_gdldqm] = {nullptr};
    TH1I* h_ftd_rise[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_ftd_fall[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    //! psn bits
    TH1I* h_psn[nskim_gdldqm] = {nullptr};
    TH1I* h_psn_rise[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_psn_fall[N_BITS_RESERVED][nskim_gdldqm] = {nullptr};
    TH1I* h_psn_extra[nskim_gdldqm] = {nullptr};
    TH1I* h_psn_overlap[nskim_gdldqm] = {nullptr};
    TH1I* h_psn_nooverlap[nskim_gdldqm] = {nullptr};
    TH1I* h_psn_pure_extra[nskim_gdldqm] = {nullptr};
    //! timtype
    TH1I* h_timtype[nskim_gdldqm] = {nullptr};
    std::vector<int> h_0_vec;
    ////! event by event psnm timing distribution
    std::vector<int> h_p_vec;
    ////! event by event ftdl timing distribution
    std::vector<int> h_f_vec;
    ////! event by event input timing distribution
    std::vector<int> h_i_vec;

    TDirectory* oldDir = nullptr;
    TDirectory* dirDQM = nullptr;

    bool m_eventByEventTimingHistRecord;
    bool m_dumpVcdFile;
    bool m_bitNameOnBinLabel;
    bool m_generatePostscript;
    unsigned m_vcdEventStart = 0;
    unsigned m_vcdNumberOfEvents = 0;
    std::string m_bitConditionToDumpVcd;
    std::string m_postScriptName;
    void fillRiseFallTimings(void);
    void fillOutputExtra(void);
    void fillOutputOverlap(void);
    void fillOutputPureExtra(void);

    void genVcd(void);
    bool anaBitCondition(void);
    bool isFired(std::string bitname);
    bool isFired_quick(const std::string& bitname, const bool& isPsnm);
    int getinbitnum(const char* c) const;
    int getoutbitnum(const char* c) const;
    unsigned n_clocks = 0;
    unsigned evtno = 0;
    unsigned _exp = 0;
    unsigned _run = 0;
    std::vector<int> skim;

    static const int n_output_extra = 73;
    static const char* output_extra[n_output_extra];
    static const int n_output_overlap = 10;
    static const char* output_overlap[n_output_overlap];
    static const int n_output_pure_extra = 13;
    static const char* output_pure_extra[n_output_pure_extra];

    //condition database for unpacker
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    int LeafBitMap[320] = {0};
    char LeafNames[320][100] = {0};
    int _e_timtype = 0;
    int _e_gdll1rvc = 0;
    int _e_coml1rvc = 0;
    int _e_toptiming = 0;
    int _e_ecltiming = 0;
    int _e_cdctiming = 0;
    int _e_toprvc = 0;
    int _e_eclrvc = 0;
    int _e_cdcrvc = 0;
    int ee_psn[10] = {0};
    int ee_ftd[10] = {0};
    int ee_itd[10] = {0};
    int n_leafs = 0;
    int n_leafsExtra = 0;
    int nconf = 0;
    int nword_input  = 0;
    int nword_output = 0;

    //condition database for input bits
    DBObjPtr<TRGGDLDBInputBits>m_dbinput;
    unsigned  n_inbit = 0;
    char inbitname[320][100] = {""};

    //condition database for output bits
    DBObjPtr<TRGGDLDBFTDLBits> m_dbftdl;
    unsigned  n_outbit = 0;
    char outbitname[320][100]  = {""};


    //name of skim from hlt/softwaretrigger/scripts/softwaretrigger/db_access.py
    std::string skim_menu[nskim_gdldqm] = {
      "all",
      "software_trigger_cut&skim&accept_hadron",
      "software_trigger_cut&skim&accept_tau_tau",
      "software_trigger_cut&skim&accept_mumu_1trk",
      "software_trigger_cut&skim&accept_mumu_2trk",
      "software_trigger_cut&skim&accept_gamma_gamma",
      "software_trigger_cut&skim&accept_bhabha",
      "software_trigger_cut&skim&accept_hadronb",
      "software_trigger_cut&skim&accept_hadronb1",
      "software_trigger_cut&skim&accept_hadronb2",
      "software_trigger_cut&skim&accept_mumutight"
    };

    //name of histgrams
    std::string skim_smap[nskim_gdldqm] = {
      "all",
      "hadron",
      "tautau",
      "mumu1trk",
      "mumu2trk",
      "gammagamma",
      "bhabha",
      "hadronb",
      "hadronb1",
      "hadronb2",
      "mumutight"
    };

  private:

    //ecltrg<->ecl mappint
    TrgEclMapping* trgeclmap = nullptr;

    //Input store array of GDL data
    StoreArray<TRGGDLUnpackerStore> entAry;

    //Input store array of metadata
    StoreObjPtr<EventMetaData> bevt;

    //Input store array of HLT
    StoreObjPtr<SoftwareTriggerResult> result_soft;

    //Input store array of track
    StoreArray<Track> Tracks;

    //Input store array of ECL
    //StoreArray<ECLCluster> ECLClusters;
    StoreArray<ECLCalDigit> m_ECLCalDigitData;
    StoreArray<ECLDigit>    m_ECLDigitData;

  };

}

#endif
