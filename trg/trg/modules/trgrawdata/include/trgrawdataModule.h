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

#include <TDirectory.h>
#include <TH2I.h>
#include <TH1I.h>
#include <TString.h>

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
        m_on_gdl, m_on_etf, m_on_grl, m_on_top
      };
      /*
      const char* moduleNames[c_nModules] = {
        "2D0", "2D1", "2D2", "2D3",
        "3D0", "3D1", "3D2", "3D3",
        "NN0", "NN1", "NN2", "NN3",
        "SL0", "SL1", "SL2", "SL3",
        "SL4", "SL5", "SL6", "SL8",
        "GDL", "ETF", "GRL"
      };
      */

      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------", "--------", "--------", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      printf(" %3s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s |\n",
             "",
             "hdr ok",   // cntr_nw3[i],
             "data ok",  // cntr_nwn[i],
             "cc ok",    // cntr_good_odr[i]);
             "hdtrg",  // cntr_nw3_badtrg[i]
             "hdrvc",  // cntr_nw3_badrvc[i]
             "hdvet",  // cntr_nw3_badvet[i],
             "dttg", // cntr_nwn_badtrg[i]
             "dtrc", // cntr_nwn_badrvc[i]
             "dtbb", // cntr_nwn_badbbb[i]
             "dtdd", // cntr_nwn_badddd[i]
             "#wd",   // cntr_bad_nwd[i],
             "ddsft", // cntr_bad_ddd[i],
             "ccodr"  // cntr_bad_odr[i],
            );
      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------", "--------", "--------", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      for (int i = 0; i < c_nModules; i++) {
        if (m_ons[i]) {
          printf(" %3s | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d |\n",
                 moduleNames[i],
                 cntr_nw3[i],
                 cntr_nwn[i],
                 cntr_good_odr[i],
                 cntr_nw3_badtrg[i],
                 cntr_nw3_badrvc[i],
                 cntr_nw3_badvet[i],
                 cntr_nwn_badtrg[i],
                 cntr_nwn_badrvc[i],
                 cntr_nwn_badbbb[i],
                 cntr_nwn_badddd[i],
                 cntr_bad_nwd[i],
                 cntr_bad_ddd[i],
                 cntr_bad_odr[i]
                );
        }
      }
      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------", "--------", "--------", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      printf("dtbadbb : buf3dddd == 0xbbbb\n");
      printf("dtbaddd : buf3dddd != 0xbbbb, != 0xdddd\n");
      printf("ddsft : dddd not found in at least one clock. Can be 0xbbbb(=dummy buffer).\n");
      printf("dttg  : event number in 1st clock cycle is not cnttrg-1. Available for only 2Dfinder.\n");

    }

    //! Define Histogram
    virtual void defineHisto() override;

    /** number of modules */
    static const unsigned int c_nModules = 24;

    const char* moduleNames[c_nModules] = {
      "2D0", "2D1", "2D2", "2D3",
      "3D0", "3D1", "3D2", "3D3",
      "NN0", "NN1", "NN2", "NN3",
      "SL0", "SL1", "SL2", "SL3",
      "SL4", "SL5", "SL6", "SL8",
      "GDL", "ETF", "GRL", "TOP"
    };


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
      e_grl = 22,
      e_top = 23
    };

  private:

    int m_mydebug;
    bool m_cc_check;
    bool m_print_cc;
    bool m_print_dbmap;
    bool m_print_clkcyc_err;
    bool m_histRecord;

    int m_hdr_nwd_sl0;
    int m_hdr_nwd_sl1;
    int m_hdr_nwd_sl2;
    int m_hdr_nwd_sl3;
    int m_hdr_nwd_sl4;
    int m_hdr_nwd_sl5;
    int m_hdr_nwd_sl6;
    int m_hdr_nwd_sl7;
    int m_hdr_nwd_sl8;
    int m_hdr_nwd_2d0;
    int m_hdr_nwd_2d1;
    int m_hdr_nwd_2d2;
    int m_hdr_nwd_2d3;
    int m_hdr_nwd_3d0;
    int m_hdr_nwd_3d1;
    int m_hdr_nwd_3d2;
    int m_hdr_nwd_3d3;
    int m_hdr_nwd_nn0;
    int m_hdr_nwd_nn1;
    int m_hdr_nwd_nn2;
    int m_hdr_nwd_nn3;
    int m_hdr_nwd_gdl;
    int m_hdr_nwd_etf;
    int m_hdr_nwd_grl;
    int m_hdr_nwd_top;

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
    int m_cpr_top;

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
    int m_hslb_top;

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
    int m_fmid_top;

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
    int m_scale_top;

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
    bool m_on_top;

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
    int m_nwd_top;

    int m_nclk_cdctrg;
    int m_nclk_sl0;
    int m_nclk_sl1;
    int m_nclk_sl2;
    int m_nclk_sl3;
    int m_nclk_sl4;
    int m_nclk_sl5;
    int m_nclk_sl6;
    int m_nclk_sl7;
    int m_nclk_sl8;
    int m_nclk_2ds;
    int m_nclk_3ds;
    int m_nclk_nns;
    int m_nclk_gdl;
    int m_nclk_etf;
    int m_nclk_grl;
    int m_nclk_top;


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

  };

}

#endif
