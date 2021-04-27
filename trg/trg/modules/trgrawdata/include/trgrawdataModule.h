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

  static const int intNaN = std::numeric_limits<int>::quiet_NaN();

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

      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------", "--------", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      printf(" %3s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |\n",
//    printf(" %3s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s |\n",
             "",
             "a)dataOk",  // cntr_nwn[i],        Data event that has no problem
             "b)hdrOk",   // cntr_nw3[i],        Header event that has no problem (except cc check)
             "c)ccOk",    // cntr_good_odr[i]);  Data event that has good cc
             "d)hdrTag",  // cntr_nw3_badtrg[i]  Header event that has bad number
             "e)hdrL1",  // cntr_nw3_badrvc[i]   Header event that has L1 timing
             "f)hdrScl",  // cntr_nw3_badvet[i]  Header event that should not be header event
             "g)dataTag", // cntr_nwn_badtrg[i]    Data event that has bad bad event number in data
             "h)dataL1", // cntr_nwn_badrvc[i]     Data event that has bad L1 timing
             "i)bbbb", // cntr_nwn_badbbb[i]     Data event from dummy buffer
             "j)#wd",   // cntr_bad_nwd[i],       Data event that does not have expected #word
             "k)ddsft", // cntr_bad_ddd[i],       Data event that does not have expected dddd in clock cycle
             "l)ccodr"  // cntr_bad_odr[i],       Data event that has bad cc cycle
            );
      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------", "--------", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      for (unsigned i = 0; i < c_nModules; ++i) {
        if (m_ons[i]) {
          printf(" %3s | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d | %8d |\n",
                 moduleNames[i],
                 cntr_nwn[i],
                 cntr_nw3[i],
                 cntr_good_odr[i],
                 cntr_nw3_badtrg[i],
                 cntr_nw3_badrvc[i],
                 cntr_nw3_badvet[i],
                 cntr_nwn_badtrg[i],
                 cntr_nwn_badrvc[i],
                 cntr_nwn_badbbb[i],
                 cntr_bad_nwd[i],
                 cntr_bad_ddd[i],
                 cntr_bad_odr[i]
                );
        }
      }
      printf("-%3s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s---%8s--\n",
             "---", "--------", "--------", "--------", "--------", "--------", "--------", "--------",
             "--------", "--------", "--------", "--------", "--------");
      printf("%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n"
             "%-10s: %-s\n",
             "a)dataOk",  "Data event that has no problem (except cc check)",
             "b)hdrOk",   "Header event that has no problem",
             "c)ccOk",    "Data event that has good cc cycle",
             "d)hdrTag",  "Header event that has bad event number in header",
             "e)hdrL1",   "Header event that has bad L1 timing in header",
             "f)hdrScl",  "Header event that should not be header event",
             "g)dataTag", "Data event that has bad event number in data (can be dummy buffer)",
             "h)dataL1",  "Data event that has bad L1 timing in header",
             "i)bbbb",    "Data event from dummy buffer",
             "j)#wd",     "Data event that does not have expected #word",
             "k)ddsft",   "Data event that does not have expected dddd in clock cycle (can be dummy buffer)",
             "l)ccodr",   "Data event that has bad cc cycle");

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

    int m_hdr_nwd_sl0 = intNaN;
    int m_hdr_nwd_sl1 = intNaN;
    int m_hdr_nwd_sl2 = intNaN;
    int m_hdr_nwd_sl3 = intNaN;
    int m_hdr_nwd_sl4 = intNaN;
    int m_hdr_nwd_sl5 = intNaN;
    int m_hdr_nwd_sl6 = intNaN;
    //int m_hdr_nwd_sl7 = intNaN;
    int m_hdr_nwd_sl8 = intNaN;
    int m_hdr_nwd_2d0 = intNaN;
    int m_hdr_nwd_2d1 = intNaN;
    int m_hdr_nwd_2d2 = intNaN;
    int m_hdr_nwd_2d3 = intNaN;
    int m_hdr_nwd_3d0 = intNaN;
    int m_hdr_nwd_3d1 = intNaN;
    int m_hdr_nwd_3d2 = intNaN;
    int m_hdr_nwd_3d3 = intNaN;
    int m_hdr_nwd_nn0 = intNaN;
    int m_hdr_nwd_nn1 = intNaN;
    int m_hdr_nwd_nn2 = intNaN;
    int m_hdr_nwd_nn3 = intNaN;
    int m_hdr_nwd_gdl = intNaN;
    int m_hdr_nwd_etf = intNaN;
    int m_hdr_nwd_grl = intNaN;
    int m_hdr_nwd_top = intNaN;

    unsigned m_cpr_sl0 = intNaN;
    unsigned m_cpr_sl1 = intNaN;
    unsigned m_cpr_sl2 = intNaN;
    unsigned m_cpr_sl3 = intNaN;
    unsigned m_cpr_sl4 = intNaN;
    unsigned m_cpr_sl5 = intNaN;
    unsigned m_cpr_sl6 = intNaN;
    int m_cpr_sl7 = intNaN;
    unsigned m_cpr_sl8 = intNaN;
    unsigned m_cpr_2d0 = intNaN;
    unsigned m_cpr_2d1 = intNaN;
    unsigned m_cpr_2d2 = intNaN;
    unsigned m_cpr_2d3 = intNaN;
    unsigned m_cpr_3d0 = intNaN;
    unsigned m_cpr_3d1 = intNaN;
    unsigned m_cpr_3d2 = intNaN;
    unsigned m_cpr_3d3 = intNaN;
    unsigned m_cpr_nn0 = intNaN;
    unsigned m_cpr_nn1 = intNaN;
    unsigned m_cpr_nn2 = intNaN;
    unsigned m_cpr_nn3 = intNaN;
    unsigned m_cpr_gdl = intNaN;
    unsigned m_cpr_etf = intNaN;
    int m_cpr_grl = intNaN;
    unsigned m_cpr_top = intNaN;

    /*
    int m_hslb_sl0 = intNaN;
    int m_hslb_sl1 = intNaN;
    int m_hslb_sl2 = intNaN;
    int m_hslb_sl3 = intNaN;
    int m_hslb_sl4 = intNaN;
    int m_hslb_sl5 = intNaN;
    int m_hslb_sl6 = intNaN;
    int m_hslb_sl7 = intNaN;
    int m_hslb_sl8 = intNaN;
    int m_hslb_2d0 = intNaN;
    int m_hslb_2d1 = intNaN;
    int m_hslb_2d2 = intNaN;
    int m_hslb_2d3 = intNaN;
    int m_hslb_3d0 = intNaN;
    int m_hslb_3d1 = intNaN;
    int m_hslb_3d2 = intNaN;
    int m_hslb_3d3 = intNaN;
    int m_hslb_nn0 = intNaN;
    int m_hslb_nn1 = intNaN;
    int m_hslb_nn2 = intNaN;
    int m_hslb_nn3 = intNaN;
    int m_hslb_gdl = intNaN;
    int m_hslb_etf = intNaN;
    int m_hslb_grl = intNaN;
    int m_hslb_top = intNaN;
    */

    int m_fmid_sl0 = intNaN;
    int m_fmid_sl1 = intNaN;
    int m_fmid_sl2 = intNaN;
    int m_fmid_sl3 = intNaN;
    int m_fmid_sl4 = intNaN;
    int m_fmid_sl5 = intNaN;
    int m_fmid_sl6 = intNaN;
    int m_fmid_sl7 = intNaN;
    int m_fmid_sl8 = intNaN;
    int m_fmid_2d0 = intNaN;
    int m_fmid_2d1 = intNaN;
    int m_fmid_2d2 = intNaN;
    int m_fmid_2d3 = intNaN;
    int m_fmid_3d0 = intNaN;
    int m_fmid_3d1 = intNaN;
    int m_fmid_3d2 = intNaN;
    int m_fmid_3d3 = intNaN;
    int m_fmid_nn0 = intNaN;
    int m_fmid_nn1 = intNaN;
    int m_fmid_nn2 = intNaN;
    int m_fmid_nn3 = intNaN;
    unsigned m_fmid_gdl = intNaN;
    int m_fmid_etf = intNaN;
    int m_fmid_grl = intNaN;
    int m_fmid_top = intNaN;

    int m_scale_sl0 = intNaN;
    int m_scale_sl1 = intNaN;
    int m_scale_sl2 = intNaN;
    int m_scale_sl3 = intNaN;
    int m_scale_sl4 = intNaN;
    int m_scale_sl5 = intNaN;
    int m_scale_sl6 = intNaN;
    int m_scale_sl7 = intNaN;
    int m_scale_sl8 = intNaN;
    int m_scale_2ds = intNaN;
    int m_scale_3ds = intNaN;
    int m_scale_nns = intNaN;
    int m_scale_gdl = intNaN;
    int m_scale_etf = intNaN;
    int m_scale_grl = intNaN;
    int m_scale_top = intNaN;

    bool m_on_sl0 = false;
    bool m_on_sl1 = false;
    bool m_on_sl2 = false;
    bool m_on_sl3 = false;
    bool m_on_sl4 = false;
    bool m_on_sl5 = false;
    bool m_on_sl6 = false;
    bool m_on_sl7 = false;
    bool m_on_sl8 = false;
    bool m_on_2d0 = false;
    bool m_on_2d1 = false;
    bool m_on_2d2 = false;
    bool m_on_2d3 = false;
    bool m_on_3d0 = false;
    bool m_on_3d1 = false;
    bool m_on_3d2 = false;
    bool m_on_3d3 = false;
    bool m_on_nn0 = false;
    bool m_on_nn1 = false;
    bool m_on_nn2 = false;
    bool m_on_nn3 = false;
    bool m_on_gdl = false;
    bool m_on_etf = false;
    bool m_on_grl = false;
    bool m_on_top = false;

    int m_nwd_sl0 = intNaN;
    int m_nwd_sl1 = intNaN;
    int m_nwd_sl2 = intNaN;
    int m_nwd_sl3 = intNaN;
    int m_nwd_sl4 = intNaN;
    int m_nwd_sl5 = intNaN;
    int m_nwd_sl6 = intNaN;
    int m_nwd_sl7 = intNaN;
    int m_nwd_sl8 = intNaN;
    int m_nwd_2ds = intNaN;
    int m_nwd_3ds = intNaN;
    int m_nwd_nns = intNaN;
    int m_nwd_gdl = intNaN;
    int m_nwd_etf = intNaN;
    int m_nwd_grl = intNaN;
    int m_nwd_top = intNaN;

    int m_nclk_cdctrg = intNaN;
    int m_nclk_sl0    = intNaN;
    int m_nclk_sl1    = intNaN;
    int m_nclk_sl2    = intNaN;
    int m_nclk_sl3    = intNaN;
    int m_nclk_sl4    = intNaN;
    int m_nclk_sl5    = intNaN;
    int m_nclk_sl6    = intNaN;
    int m_nclk_sl7    = intNaN;
    int m_nclk_sl8    = intNaN;
    int m_nclk_2ds    = intNaN;
    int m_nclk_3ds    = intNaN;
    int m_nclk_nns    = intNaN;
    int m_nclk_gdl    = intNaN;
    int m_nclk_etf    = intNaN;
    int m_nclk_grl    = intNaN;
    int m_nclk_top    = intNaN;


    //TODO is zero default value good for these arrays
    int cntr_good_odr[50] = {};
    int cntr_bad_odr[50] = {};
    int cntr_bad_ddd[50] = {};
    int cntr_bad_nwd[50] = {};

    int cntr_nw3[50] = {};
    int cntr_nw3_badvet[50] = {};
    int cntr_nw3_badtrg[50] = {};
    int cntr_nw3_badrvc[50] = {};

    int cntr_nwn[50] = {};
    int cntr_nwn_badvet[50] = {};
    int cntr_nwn_badtrg[50] = {};
    int cntr_nwn_badrvc[50] = {};

    int cntr_nwn_badbbb[50] = {};
    int cntr_nwn_badddd[50] = {};

    int cntr_nwe_badnwd[50] = {};

  protected :
    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    DBObjPtr<TRGGDLDBUnpacker> m_unpacker;
    std::vector<std::vector<int>> BitMap;
    int LeafBitMap[320] = {};
    char LeafNames[320][100] = {};

  };

}

#endif
