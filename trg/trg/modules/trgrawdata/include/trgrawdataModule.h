#ifndef TRGRAWDATAModule_h
#define TRGRAWDATAModule_h

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <sys/uio.h>

#include <rawdata/dataobjects/RawTRG.h>

#include <framework/pcore/EvtMessage.h>

#include <framework/datastore/StoreArray.h>
// #include <framework/core/Module.h>
#include <framework/core/HistoModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBBadrun.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>

#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCOPPER.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <TH2I.h>
#include <TH1I.h>

namespace Belle2 {

//class TRGRAWDATAModule : public Module {
  class TRGRAWDATAModule : public HistoModule {

  public:
    //! Costructor
    TRGRAWDATAModule();
    //! Destrunctor
    virtual ~TRGRAWDATAModule() {}

  public:
    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override {}
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override {}
    //! terminate
    virtual void terminate() override
    {

      bool m_ons[c_nModules] = {
        m_on_2d0, m_on_2d1, m_on_2d2, m_on_2d3,
        m_on_3d0, m_on_3d1, m_on_3d2, m_on_3d3,
        m_on_nn0, m_on_nn1, m_on_nn2, m_on_nn3,
        m_on_sl0, m_on_sl1, m_on_sl2, m_on_sl3,
        m_on_sl4, m_on_sl5, m_on_sl6, m_on_sl8,
        m_on_gdl, m_on_etf, m_on_grl
      };
      const char* moduleNames[c_nModules] = {
        "2D0", "2D1", "2D2", "2D3",
        "3D0", "3D1", "3D2", "3D3",
        "NN0", "NN1", "NN2", "NN3",
        "SL0", "SL1", "SL2", "SL3",
        "SL4", "SL5", "SL6", "SL8",
        "GDL", "ETF", "GRL"
      };
      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      printf(" %3s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s |\n",
             "", "hdr ok", "data ok", "hdr bad",
             "data bad", "bad#wd", "dd shift", "ccdisodr", "cc ok");
      printf("-%3s-|-%8s-|-%8s-|-%8s-|-%8s-|-%8s-|-%8s-|-%8s-|-%8s-|\n",
             "---", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      for (int i = 0; i < c_nModules; i++) {
        if (m_ons[i]) {
          printf(" %3s | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d |\n",
                 moduleNames[i],
                 cntr_nw3[i],
                 cntr_nwn[i],
                 cntr_nw3_badtrg[i] + cntr_nw3_badrvc[i] + cntr_nw3_badvet[i],
                 cntr_nwn_badtrg[i] + cntr_nwn_badrvc[i] +
                 cntr_nwn_badbbb[i] + cntr_nwn_badddd[i],
                 cntr_bad_nwd[i],
                 cntr_bad_ddd[i],
                 cntr_bad_odr[i],
                 cntr_good_odr[i]);
        }
      }
      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");

    }
    //! Define Histogram
    virtual void defineHisto() override;

    //! print a buffer
    virtual void printBuffer(int* buf, int nwords);

    //! print the contents of a RawFTSW event
    virtual void printFTSWEvent(RawDataBlock* raw_datablock, int i);

    /** number of modules */
    static const unsigned int c_nModules = 23;

    enum EModuleNum {
      e_2d0 = 0,
      e_2d1 = 1,
      e_2d2 = 2,
      e_2d3 = 3,
      e_3d0 = 4,
      e_3d1 = 5,
      e_3d2 = 6,
      e_3d3 = 7,
      e_nn0 = 8,
      e_nn1 = 9,
      e_nn2 = 10,
      e_nn3 = 11,
      e_sl0 = 12,
      e_sl1 = 13,
      e_sl2 = 14,
      e_sl3 = 15,
      e_sl4 = 16,
      e_sl5 = 17,
      e_sl6 = 18,
      e_sl8 = 19,
      e_gdl = 20,
      e_etf = 21,
      e_grl = 22
    };

  private:

    bool m_mydebug;
    bool m_cc_check;
    bool m_print_cc;
    bool m_print_dbmap;
    bool m_print_err;
    int m_hdr_nwd;

    int m_cpr_sl0;
    int m_cpr_sl1;
    int m_cpr_sl2;
    int m_cpr_sl3;
    int m_cpr_sl4;
    int m_cpr_sl5;
    int m_cpr_sl6;
    int m_cpr_sl7;
    int m_cpr_sl8;
    int m_cpr_2d0;
    int m_cpr_2d1;
    int m_cpr_2d2;
    int m_cpr_2d3;
    int m_cpr_3d0;
    int m_cpr_3d1;
    int m_cpr_3d2;
    int m_cpr_3d3;
    int m_cpr_nn0;
    int m_cpr_nn1;
    int m_cpr_nn2;
    int m_cpr_nn3;
    int m_cpr_gdl;
    int m_cpr_etf;
    int m_cpr_grl;

    int m_hslb_sl0;
    int m_hslb_sl1;
    int m_hslb_sl2;
    int m_hslb_sl3;
    int m_hslb_sl4;
    int m_hslb_sl5;
    int m_hslb_sl6;
    int m_hslb_sl7;
    int m_hslb_sl8;
    int m_hslb_2d0;
    int m_hslb_2d1;
    int m_hslb_2d2;
    int m_hslb_2d3;
    int m_hslb_3d0;
    int m_hslb_3d1;
    int m_hslb_3d2;
    int m_hslb_3d3;
    int m_hslb_nn0;
    int m_hslb_nn1;
    int m_hslb_nn2;
    int m_hslb_nn3;
    int m_hslb_gdl;
    int m_hslb_etf;
    int m_hslb_grl;

    int m_fmid_sl0;
    int m_fmid_sl1;
    int m_fmid_sl2;
    int m_fmid_sl3;
    int m_fmid_sl4;
    int m_fmid_sl5;
    int m_fmid_sl6;
    int m_fmid_sl7;
    int m_fmid_sl8;
    int m_fmid_2d0;
    int m_fmid_2d1;
    int m_fmid_2d2;
    int m_fmid_2d3;
    int m_fmid_3d0;
    int m_fmid_3d1;
    int m_fmid_3d2;
    int m_fmid_3d3;
    int m_fmid_nn0;
    int m_fmid_nn1;
    int m_fmid_nn2;
    int m_fmid_nn3;
    int m_fmid_gdl;
    int m_fmid_etf;
    int m_fmid_grl;

    int m_scale_sl0;
    int m_scale_sl1;
    int m_scale_sl2;
    int m_scale_sl3;
    int m_scale_sl4;
    int m_scale_sl5;
    int m_scale_sl6;
    int m_scale_sl7;
    int m_scale_sl8;
    int m_scale_2ds;
    int m_scale_3ds;
    int m_scale_nns;
    int m_scale_gdl;
    int m_scale_etf;
    int m_scale_grl;

    bool m_on_sl0;
    bool m_on_sl1;
    bool m_on_sl2;
    bool m_on_sl3;
    bool m_on_sl4;
    bool m_on_sl5;
    bool m_on_sl6;
    bool m_on_sl7;
    bool m_on_sl8;
    bool m_on_2d0;
    bool m_on_2d1;
    bool m_on_2d2;
    bool m_on_2d3;
    bool m_on_3d0;
    bool m_on_3d1;
    bool m_on_3d2;
    bool m_on_3d3;
    bool m_on_nn0;
    bool m_on_nn1;
    bool m_on_nn2;
    bool m_on_nn3;
    bool m_on_gdl;
    bool m_on_etf;
    bool m_on_grl;

    int m_nwd_sl0;
    int m_nwd_sl1;
    int m_nwd_sl2;
    int m_nwd_sl3;
    int m_nwd_sl4;
    int m_nwd_sl5;
    int m_nwd_sl6;
    int m_nwd_sl7;
    int m_nwd_sl8;
    int m_nwd_2ds;
    int m_nwd_3ds;
    int m_nwd_nns;
    int m_nwd_gdl;
    int m_nwd_etf;
    int m_nwd_grl;

    int m_nclk_gdl;
    int m_nclk_cdctrg;

    TH1I* h_2d0_fmid;
    TH1I* h_2d1_fmid;
    TH1I* h_2d2_fmid;
    TH1I* h_2d3_fmid;
    TH1I* h_3d0_fmid;
    TH1I* h_3d1_fmid;
    TH1I* h_3d2_fmid;
    TH1I* h_3d3_fmid;
    TH1I* h_nn0_fmid;
    TH1I* h_nn1_fmid;
    TH1I* h_nn2_fmid;
    TH1I* h_nn3_fmid;
    TH1I* h_sl0_fmid;
    TH1I* h_sl1_fmid;
    TH1I* h_sl2_fmid;
    TH1I* h_sl3_fmid;
    TH1I* h_sl4_fmid;
    TH1I* h_sl5_fmid;
    TH1I* h_sl6_fmid;
    TH1I* h_sl7_fmid;
    TH1I* h_sl8_fmid;
    TH1I* h_gdl_fmid;
    TH1I* h_etf_fmid;
    TH1I* h_grl_fmid;


    TH1I* h_2d0_wd;
    TH1I* h_2d1_wd;
    TH1I* h_2d2_wd;
    TH1I* h_2d3_wd;
    TH1I* h_3d0_wd;
    TH1I* h_3d1_wd;
    TH1I* h_3d2_wd;
    TH1I* h_3d3_wd;
    TH1I* h_nn0_wd;
    TH1I* h_nn1_wd;
    TH1I* h_nn2_wd;
    TH1I* h_nn3_wd;
    TH1I* h_sl0_wd;
    TH1I* h_sl1_wd;
    TH1I* h_sl2_wd;
    TH1I* h_sl3_wd;
    TH1I* h_sl4_wd;
    TH1I* h_sl5_wd;
    TH1I* h_sl6_wd;
    TH1I* h_sl7_wd;
    TH1I* h_sl8_wd;
    TH1I* h_gdl_wd;
    TH1I* h_etf_wd;
    TH1I* h_grl_wd;

    TH1I* h_2d0_nwd;
    TH1I* h_2d1_nwd;
    TH1I* h_2d2_nwd;
    TH1I* h_2d3_nwd;
    TH1I* h_3d0_nwd;
    TH1I* h_3d1_nwd;
    TH1I* h_3d2_nwd;
    TH1I* h_3d3_nwd;
    TH1I* h_nn0_nwd;
    TH1I* h_nn1_nwd;
    TH1I* h_nn2_nwd;
    TH1I* h_nn3_nwd;
    TH1I* h_sl0_nwd;
    TH1I* h_sl1_nwd;
    TH1I* h_sl2_nwd;
    TH1I* h_sl3_nwd;
    TH1I* h_sl4_nwd;
    TH1I* h_sl5_nwd;
    TH1I* h_sl6_nwd;
    TH1I* h_sl7_nwd;
    TH1I* h_sl8_nwd;
    TH1I* h_gdl_nwd;
    TH1I* h_etf_nwd;
    TH1I* h_grl_nwd;

    TH1I* h_2d0_cnttrg;
    TH1I* h_2d1_cnttrg;
    TH1I* h_2d2_cnttrg;
    TH1I* h_2d3_cnttrg;
    TH1I* h_3d0_cnttrg;
    TH1I* h_3d1_cnttrg;
    TH1I* h_3d2_cnttrg;
    TH1I* h_3d3_cnttrg;
    TH1I* h_nn0_cnttrg;
    TH1I* h_nn1_cnttrg;
    TH1I* h_nn2_cnttrg;
    TH1I* h_nn3_cnttrg;
    TH1I* h_sl0_cnttrg;
    TH1I* h_sl1_cnttrg;
    TH1I* h_sl2_cnttrg;
    TH1I* h_sl3_cnttrg;
    TH1I* h_sl4_cnttrg;
    TH1I* h_sl5_cnttrg;
    TH1I* h_sl6_cnttrg;
    TH1I* h_sl7_cnttrg;
    TH1I* h_sl8_cnttrg;
    TH1I* h_gdl_cnttrg;
    TH1I* h_etf_cnttrg;
    TH1I* h_grl_cnttrg;

    int cntr_good_odr[50];
    int cntr_bad_odr[50];
    int cntr_bad_ddd[50];
    int cntr_bad_nwd[50];

    int cntr_nw3[50];
    int cntr_nw3_badvet[50];
    int cntr_nw3_badtrg[50];
    int cntr_nw3_badrvc[50];

    int cntr_nwn[50];
    int cntr_nwn_badvet[50];
    int cntr_nwn_badtrg[50];
    int cntr_nwn_badrvc[50];

    int cntr_nwn_badbbb[50];
    int cntr_nwn_badddd[50];

    int cntr_nwe_badnwd[50];

  protected :
    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    std::vector<std::vector<int>> BitMap;
    int LeafBitMap[320];
    char LeafNames[320][100];

  protected :
    //!Compression parameter
    int m_compressionLevel;

    //! No. of sent events
    int n_basf2evt;

    //! # of FTSWs
    int m_nftsw;

    //! # of COPPERs
    int m_ncpr;

    //! counter
    int m_print_cnt;

  };

}

#endif
