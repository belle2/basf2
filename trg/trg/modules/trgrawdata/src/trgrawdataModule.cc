/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <trg/trg/modules/trgrawdata/trgrawdataModule.h>

#include <rawdata/modules/PrintDataTemplate.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dbobjects/RunInfo.h>
#include <framework/datastore/DataStore.h>

#include <string.h>
#include <unistd.h>

#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>

using namespace std;
using namespace Belle2;
using namespace GDL;


REG_MODULE(TRGRAWDATA);

// TRGRAWDATAModule::TRGRAWDATAModule() : Module()
TRGRAWDATAModule::TRGRAWDATAModule() : HistoModule()
{

  setDescription("TRG rawdata analyer");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("histRecord", m_histRecord,
           "Recording event by event timing distribution histogram or not",
           false);
  addParam("mydebug", m_mydebug, "debug flag", 0);
  addParam("cc_check", m_cc_check, "cc mode", true);
  addParam("print_cc", m_print_cc, "Print Disordered clocks", true);
  addParam("print_clkcyc_err", m_print_clkcyc_err, "Print Errs of Disordered clocks", false);
  addParam("print_dbmap", m_print_dbmap, "Print Database Bit Map", false);

  addParam("hdr_nwd_2d0", m_hdr_nwd_2d0, "Number of word for 2D0 Header", 3);
  addParam("hdr_nwd_2d1", m_hdr_nwd_2d1, "Number of word for 2D1 Header", 3);
  addParam("hdr_nwd_2d2", m_hdr_nwd_2d2, "Number of word for 2D2 Header", 3);
  addParam("hdr_nwd_2d3", m_hdr_nwd_2d3, "Number of word for 2D3 Header", 3);
  addParam("hdr_nwd_3d0", m_hdr_nwd_3d0, "Number of word for 3D0 Header", 3);
  addParam("hdr_nwd_3d1", m_hdr_nwd_3d1, "Number of word for 3D1 Header", 3);
  addParam("hdr_nwd_3d2", m_hdr_nwd_3d2, "Number of word for 3D2 Header", 3);
  addParam("hdr_nwd_3d3", m_hdr_nwd_3d3, "Number of word for 3D3 Header", 3);
  addParam("hdr_nwd_nn0", m_hdr_nwd_nn0, "Number of word for NN0 Header", 3);
  addParam("hdr_nwd_nn1", m_hdr_nwd_nn1, "Number of word for NN1 Header", 3);
  addParam("hdr_nwd_nn2", m_hdr_nwd_nn2, "Number of word for NN2 Header", 3);
  addParam("hdr_nwd_nn3", m_hdr_nwd_nn3, "Number of word for NN3 Header", 3);
  addParam("hdr_nwd_sl0", m_hdr_nwd_sl0, "Number of word for TSF0 Header", 3);
  addParam("hdr_nwd_sl1", m_hdr_nwd_sl1, "Number of word for TSF1 Header", 3);
  addParam("hdr_nwd_sl2", m_hdr_nwd_sl2, "Number of word for TSF2 Header", 3);
  addParam("hdr_nwd_sl3", m_hdr_nwd_sl3, "Number of word for TSF3 Header", 3);
  addParam("hdr_nwd_sl4", m_hdr_nwd_sl4, "Number of word for TSF4 Header", 3);
  addParam("hdr_nwd_sl5", m_hdr_nwd_sl5, "Number of word for TSF5 Header", 3);
  addParam("hdr_nwd_sl6", m_hdr_nwd_sl6, "Number of word for TSF6 Header", 3);
  addParam("hdr_nwd_sl8", m_hdr_nwd_sl8, "Number of word for TSF8 Header", 3);
  addParam("hdr_nwd_etf", m_hdr_nwd_etf, "Number of word for ETF Header", 3);
  addParam("hdr_nwd_grl", m_hdr_nwd_grl, "Number of word for GRL Header", 3);
  addParam("hdr_nwd_top", m_hdr_nwd_top, "Number of word for TOP Header", 3);
  addParam("hdr_nwd_gdl", m_hdr_nwd_gdl, "Number of word for GDL Header", 6);

  addParam("cpr_2d0", m_cpr_2d0, "Copper number of 2D0", 0x11000001U);
  addParam("cpr_2d1", m_cpr_2d1, "Copper number of 2D1", 0x11000001U);
  addParam("cpr_2d2", m_cpr_2d2, "Copper number of 2D2", 0x11000002U);
  addParam("cpr_2d3", m_cpr_2d3, "Copper number of 2D3", 0x11000002U);
  addParam("cpr_3d0", m_cpr_3d0, "Copper number of 3D0", 0x11000003U);
  addParam("cpr_3d1", m_cpr_3d1, "Copper number of 3D1", 0x11000003U);
  addParam("cpr_3d2", m_cpr_3d2, "Copper number of 3D2", 0x11000004U);
  addParam("cpr_3d3", m_cpr_3d3, "Copper number of 3D3", 0x11000004U);
  addParam("cpr_nn0", m_cpr_nn0, "Copper number of NN0", 0x11000005U);
  addParam("cpr_nn1", m_cpr_nn1, "Copper number of NN1", 0x11000005U);
  addParam("cpr_nn2", m_cpr_nn2, "Copper number of NN2", 0x11000006U);
  addParam("cpr_nn3", m_cpr_nn3, "Copper number of NN3", 0x11000006U);
  addParam("cpr_sl0", m_cpr_sl0, "Copper number of SL0", 0x11000007U);
  addParam("cpr_sl1", m_cpr_sl1, "Copper number of SL1", 0x11000009U);//*
  addParam("cpr_sl2", m_cpr_sl2, "Copper number of SL2", 0x11000008U);
  addParam("cpr_sl3", m_cpr_sl3, "Copper number of SL3", 0x11000008U);
  addParam("cpr_sl4", m_cpr_sl4, "Copper number of SL4", 0x11000007U);//*
  addParam("cpr_sl5", m_cpr_sl5, "Copper number of SL5", 0x11000009U);
  addParam("cpr_sl6", m_cpr_sl6, "Copper number of SL6", 0x1100000aU);
  addParam("cpr_sl7", m_cpr_sl7, "Copper number of SL7", 0x1100000a);
  addParam("cpr_sl8", m_cpr_sl8, "Copper number of SL8", 0x1100000aU);
  addParam("cpr_gdl", m_cpr_gdl, "Copper number of GDL", 0x15000001U);
  addParam("cpr_etf", m_cpr_etf, "Copper number of ETF", 0x15000001U);
  addParam("cpr_grl", m_cpr_grl, "Copper number of GRL", 0x15000002);
  addParam("cpr_top", m_cpr_top, "Copper number of TOP", 0x12000001U);

  addParam("on_2d0", m_on_2d0, "ON/OFF of 2D0", true);
  addParam("on_2d1", m_on_2d1, "ON/OFF of 2D1", true);
  addParam("on_2d2", m_on_2d2, "ON/OFF of 2D2", true);
  addParam("on_2d3", m_on_2d3, "ON/OFF of 2D3", true);
  addParam("on_3d0", m_on_3d0, "ON/OFF of 3D0", true);
  addParam("on_3d1", m_on_3d1, "ON/OFF of 3D1", true);
  addParam("on_3d2", m_on_3d2, "ON/OFF of 3D2", true);
  addParam("on_3d3", m_on_3d3, "ON/OFF of 3D3", true);
  addParam("on_nn0", m_on_nn0, "ON/OFF of NN0", true);
  addParam("on_nn1", m_on_nn1, "ON/OFF of NN1", true);
  addParam("on_nn2", m_on_nn2, "ON/OFF of NN2", true);
  addParam("on_nn3", m_on_nn3, "ON/OFF of NN3", true);
  addParam("on_sl0", m_on_sl0, "ON/OFF of SL0", true);
  addParam("on_sl1", m_on_sl1, "ON/OFF of SL1", true);
  addParam("on_sl2", m_on_sl2, "ON/OFF of SL2", true);
  addParam("on_sl3", m_on_sl3, "ON/OFF of SL3", true);
  addParam("on_sl4", m_on_sl4, "ON/OFF of SL4", true);
  addParam("on_sl5", m_on_sl5, "ON/OFF of SL5", true);
  addParam("on_sl6", m_on_sl6, "ON/OFF of SL6", true);
  addParam("on_sl7", m_on_sl7, "ON/OFF of SL7", true);
  addParam("on_sl8", m_on_sl8, "ON/OFF of SL8", true);
  addParam("on_gdl", m_on_gdl, "ON/OFF of GDL", true);
  addParam("on_etf", m_on_etf, "ON/OFF of ETF", true);
  addParam("on_grl", m_on_grl, "ON/OFF of GRL", true);
  addParam("on_top", m_on_top, "ON/OFF of TOP", true);

  addParam("nwd_2ds", m_nwd_2ds, "N word of 2Ds", 3939);   // 2624x48
  addParam("nwd_3ds", m_nwd_3ds, "N word of 3Ds", 3939);   // 2624x48
  addParam("nwd_nns", m_nwd_nns, "N word of NNs", 0xc03);  // 2048x48
  addParam("nwd_sl0", m_nwd_sl0, "N word of SL0", 0xc03);  // 2048x48
  addParam("nwd_sl1", m_nwd_sl1, "N word of SL1", 0x1803); // 4096x48
  addParam("nwd_sl2", m_nwd_sl2, "N word of SL2", 0x1803); // 4096x48
  addParam("nwd_sl3", m_nwd_sl3, "N word of SL3", 0x1803); // 4096x48
  addParam("nwd_sl4", m_nwd_sl4, "N word of SL4", 0xc03);  // 2048x48
  addParam("nwd_sl5", m_nwd_sl5, "N word of SL5", 0x1803); // 4096x48
  addParam("nwd_sl6", m_nwd_sl6, "N word of SL6", 0xc03);  // 2048x48
  addParam("nwd_sl7", m_nwd_sl7, "N word of SL7", 0x603);  // 1024x48
  addParam("nwd_sl8", m_nwd_sl8, "N word of SL8", 0x603);  // 1024x48
  addParam("nwd_gdl", m_nwd_gdl, "N word of GDL", 646);   // 640x32
  addParam("nwd_etf", m_nwd_etf, "N word of ETF", 1539);  // 1024x48
  addParam("nwd_grl", m_nwd_grl, "N word of GRL", 0xc03); // 1024x48
  addParam("nwd_top", m_nwd_top, "N word of TOP", 0x303); // 1024*24

  addParam("nclk_2ds", m_nclk_2ds, "N clk of 2Ds", 48);
  addParam("nclk_3ds", m_nclk_3ds, "N clk of 3Ds", 48);
  addParam("nclk_nns", m_nclk_nns, "N clk of NNs", 48);
  addParam("nclk_sl0", m_nclk_sl0, "N clk of SL0", 48);
  addParam("nclk_sl1", m_nclk_sl1, "N clk of SL1", 48);
  addParam("nclk_sl2", m_nclk_sl2, "N clk of SL2", 48);
  addParam("nclk_sl3", m_nclk_sl3, "N clk of SL3", 48);
  addParam("nclk_sl4", m_nclk_sl4, "N clk of SL4", 48);
  addParam("nclk_sl5", m_nclk_sl5, "N clk of SL5", 48);
  addParam("nclk_sl6", m_nclk_sl6, "N clk of SL6", 48);
  addParam("nclk_sl7", m_nclk_sl7, "N clk of SL7", 48);
  addParam("nclk_sl8", m_nclk_sl8, "N clk of SL8", 48);
  addParam("nclk_gdl", m_nclk_gdl, "N clk of GDL", 32);
  addParam("nclk_etf", m_nclk_etf, "N clk of ETF", 48);
  addParam("nclk_grl", m_nclk_grl, "N clk of GRL", 48);
  addParam("nclk_top", m_nclk_top, "N clk of TOP", 24);

  addParam("nclk_cdctrg", m_nclk_cdctrg, "N clock of CDCTRG", 48);

  addParam("scale_2ds", m_scale_2ds, "Scale factor of 2Ds", 256);
  addParam("scale_3ds", m_scale_3ds, "Scale factor of 3Ds", 256);
  addParam("scale_nns", m_scale_nns, "Scale factor of NNs", 256);
  addParam("scale_sl0", m_scale_sl0, "Scale factor of SL0", 256);
  addParam("scale_sl1", m_scale_sl1, "Scale factor of SL1", 256);
  addParam("scale_sl2", m_scale_sl2, "Scale factor of SL2", 256);
  addParam("scale_sl3", m_scale_sl3, "Scale factor of SL3", 256);
  addParam("scale_sl4", m_scale_sl4, "Scale factor of SL4", 256);
  addParam("scale_sl5", m_scale_sl5, "Scale factor of SL5", 256);
  addParam("scale_sl6", m_scale_sl6, "Scale factor of SL6", 256);
  addParam("scale_sl7", m_scale_sl7, "Scale factor of SL7", 256);
  addParam("scale_sl8", m_scale_sl8, "Scale factor of SL8", 256);
  addParam("scale_gdl", m_scale_gdl, "Scale factor of GDL", 1);
  addParam("scale_etf", m_scale_etf, "Scale factor of ETF", 512);
  addParam("scale_grl", m_scale_grl, "Scale factor of GRL", 1);
  addParam("scale_top", m_scale_top, "Scale factor of TOP", 32);

  addParam("fmid_2d0", m_fmid_2d0, "Firmware ID of 2D0", 0x32442020);
  addParam("fmid_2d1", m_fmid_2d1, "Firmware ID of 2D1", 0x32442020);
  addParam("fmid_2d2", m_fmid_2d2, "Firmware ID of 2D2", 0x32442020);
  addParam("fmid_2d3", m_fmid_2d3, "Firmware ID of 2D3", 0x32442020);
  addParam("fmid_3d0", m_fmid_3d0, "Firmware ID of 3D0", 0x33440001);
  addParam("fmid_3d1", m_fmid_3d1, "Firmware ID of 3D1", 0x33440001);
  addParam("fmid_3d2", m_fmid_3d2, "Firmware ID of 3D2", 0x33440001);
  addParam("fmid_3d3", m_fmid_3d3, "Firmware ID of 3D3", 0x33440001);
  addParam("fmid_nn0", m_fmid_nn0, "Firmware ID of NN0", 0x4e4e5402);
  addParam("fmid_nn1", m_fmid_nn1, "Firmware ID of NN1", 0x4e4e5402);
  addParam("fmid_nn2", m_fmid_nn2, "Firmware ID of NN2", 0x4e4e5402);
  addParam("fmid_nn3", m_fmid_nn3, "Firmware ID of NN3", 0x4e4e5402);
  addParam("fmid_sl0", m_fmid_sl0, "Firmware ID of SL0", 0x54534630);
  addParam("fmid_sl1", m_fmid_sl1, "Firmware ID of SL1", 0x54534631);
  addParam("fmid_sl2", m_fmid_sl2, "Firmware ID of SL2", 0x54534632);
  addParam("fmid_sl3", m_fmid_sl3, "Firmware ID of SL3", 0x54534633);
  addParam("fmid_sl4", m_fmid_sl4, "Firmware ID of SL4", 0x54534634);
  addParam("fmid_sl5", m_fmid_sl5, "Firmware ID of SL5", 0x54534635);
  addParam("fmid_sl6", m_fmid_sl6, "Firmware ID of SL6", 0x54534636);
  addParam("fmid_sl7", m_fmid_sl7, "Firmware ID of SL7", 0x54534637);
  addParam("fmid_sl8", m_fmid_sl8, "Firmware ID of SL8", 0x54534d38);
  addParam("fmid_gdl", m_fmid_gdl, "Firmware ID of GDL", 0x47444c20U);
  addParam("fmid_etf", m_fmid_etf, "Firmware ID of ETF", 0x45544620);
  addParam("fmid_grl", m_fmid_grl, "Firmware ID of GRL", 0x47824c20);
  addParam("fmid_top", m_fmid_top, "Firmware ID of TOP", 0);

}

void TRGRAWDATAModule::initialize()
{
  B2INFO("trgrawdata: initialize() started.");
  if (! m_unpacker) {
    B2INFO("no database of gdl unpacker");
  }
  if (m_print_dbmap) {

    //const int nword_header = m_unpacker->get_nword_header(); //not used for now
    //const int n_clocks     = m_unpacker->getnClks(); //not used for now
    //const int nBits        = m_unpacker->getnBits(); //not used for now
    const int n_leafs      = m_unpacker->getnLeafs();
    const int n_leafsExtra = m_unpacker->getnLeafsExtra();
    //const int conf         = m_unpacker->getconf(); //not used for now

    int aBitMap[320][2] = {0};
    int aBitMap_extra[100][3] = { -1};
    for (int i = 0; i < n_leafsExtra; i++) {
      aBitMap_extra[i][0] = m_unpacker->getBitMap_extra(i, 0);
      aBitMap_extra[i][1] = m_unpacker->getBitMap_extra(i, 1);
      aBitMap_extra[i][2] = m_unpacker->getBitMap_extra(i, 2);
    }

    for (int i = 0; i < 200; i++) {
      LeafBitMap[i] = m_unpacker->getLeafMap(i);
      std::cout << "LeafBitMap[" << i << "] = " << m_unpacker->getLeafMap(i) << std::endl;
      strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
      std::cout << "LeafNames[" << i << "] = " << m_unpacker->getLeafnames(i) << std::endl;
    }
    for (int i = 0; i < n_leafs; i++) {
      aBitMap[i][0] = m_unpacker->getBitMap(i, 0);
      aBitMap[i][1] = m_unpacker->getBitMap(i, 1);
    }
    for (int i = 0; i < 200; i++) {
      int bin = m_unpacker->getLeafMap(i) + 1;
      if (0 < bin && bin <= n_leafs) {
        std::cout << "leaf(" << i
                  << "), bin(" << bin
                  << "), LeafNames[leaf](" << LeafNames[i]
                  << "), BitMap[bin-1][0](" << aBitMap[bin - 1][0]
                  << "), BitMap[bin-1][1](" << aBitMap[bin - 1][1]
                  << ")" << std::endl;
      }
    }
    // for leafsExtra
    for (int i = 0; i < 200; i++) {
      int bin = m_unpacker->getLeafMap(i) + 1;
      int j = bin - n_leafs - 1;
      if (n_leafs < bin && bin <= n_leafs + n_leafsExtra) {
        std::cout << "i(" << i
                  << "), bin(" << bin
                  << "), LeafNames[leaf](" << LeafNames[i]
                  << "), BitMap_extra[j][0](buf[" << aBitMap_extra[j][0]
                  << "]), BitMap_extra[j][1](downto " << aBitMap_extra[j][1]
                  << "), BitMap_extra[j][2](" << aBitMap_extra[j][2]
                  << " bit length)" << std::endl;
      }
    }

  }
  for (int i = 0; i < 50; i++) {
    cntr_bad_odr[i] = 0;
    cntr_bad_ddd[i] = 0;
    cntr_bad_nwd[i] = 0;

    cntr_nw3[i] = 0;
    cntr_nw3_badvet[i] = 0;
    cntr_nw3_badtrg[i] = 0;
    cntr_nw3_badrvc[i] = 0;

    cntr_nwn[i] = 0;
    cntr_nwn_badvet[i] = 0;
    cntr_nwn_badtrg[i] = 0;
    cntr_nwn_badrvc[i] = 0;

    cntr_nwn_badbbb[i] = 0;
    cntr_nwn_badddd[i] = 0;

    cntr_nwe_badnwd[i] = 0;

  }

}

void TRGRAWDATAModule::event()
{

  B2INFO("trgrawdata: event() started.");
  StoreArray<RawTRG> raw_trgarray;

  StoreObjPtr<EventMetaData> bevt;
  int _exp = bevt->getExperiment();
  int _run = bevt->getRun();
  unsigned _eve = bevt->getEvent();
  if (0)
    std::cout << "**************_eve(" << _eve
              << "), c_nModules(" << c_nModules
              << "), m_nclk_cdctrg(" << m_nclk_cdctrg
              << "), _exp(" << _exp
              << "), _run(" << _run
              << "), _eve(" << _eve
              << "), _Form(" << Form("he%010d", (int)_eve)
              << ")" << std::endl;

  TH2D* h_0 = new TH2D(Form("he%010d", (int)_eve),
                       Form("e%02dr%04d ev %08d", _exp, _run, (int)_eve),
                       c_nModules * 4, 0, c_nModules * 4,
                       m_nclk_cdctrg, 0, m_nclk_cdctrg);

  for (unsigned i = 0; i < c_nModules; ++i) {
    h_0->GetXaxis()->SetBinLabel(4 * i + 1, Form("%s hdevt", moduleNames[i]));
    h_0->GetXaxis()->SetBinLabel(4 * i + 2, Form("%s dtevt", moduleNames[i]));
    h_0->GetXaxis()->SetBinLabel(4 * i + 3, Form("%s L1rvc", moduleNames[i]));
    h_0->GetXaxis()->SetBinLabel(4 * i + 4, Form("%s cc", moduleNames[i]));
  }

  /*
    GDL only. To get gdlrvc12
   */
  unsigned gdlrvc12 = -1;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      int cprid = (int)raw_trgarray[i]->GetNodeID(j);
      unsigned eve20 = (_eve & 0xfffff);
      for (int hslb = 0; hslb < 2; hslb++) {
        int nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
        int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
        if (0x15000001 == cprid && hslb == 0) {

          if (m_mydebug) {
            printf("aa:GDL start: 0x%x%c exp(%d), run(%d), eve(%u), eve(0x%x), nword(%d)\n",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
          }

          unsigned buf0 = 0;
          if (nword > 0) buf0 = (unsigned)buf[0];
          unsigned buf1 = 0;
          if (nword > 1) buf1 = (unsigned)buf[1];
          unsigned buf2 = 0;
          if (nword > 2) buf2 = (unsigned)buf[2];
          //unsigned buf2cnttrg20 = (buf2 >> 12) & 0xfffff;
          //unsigned buf2rvc12    = (buf2 & 0xfff);

          if (nword < 3) {
            cntr_nwe_badnwd[e_gdl]++;
            printf("ab:GDL(0x%x%c) exp(%d), run(%d), eve(%u), eve(0x%x), nword(%d), ",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
            printf("buf0(0x%x), buf1(0x%x), buf2(0x%x)\n", buf0, buf1, buf2);
            continue;
          }

          if (nword < 8) {
            printf("ad:GDL(0x%x%c) exp(%d), run(%d), eve(%u), eve(0x%x), nword(%d)\n",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
            cntr_nwe_badnwd[e_gdl]++;
          } else {
            unsigned buf5 = (unsigned)buf[m_hdr_nwd_gdl - 1];
            // 626 = 646 -
            unsigned buf6 = (unsigned)buf[626]; // 1st wd in the last clock cycle
            unsigned buf7 = (unsigned)buf[627]; // 2nd wd in the last clock cycle
            unsigned lastdd = (buf6 >> 24) & 0xff;
            unsigned last_rvc = (buf6 >> 4) & 0xfff;
            unsigned cnttrg_data_15to12 = buf6 & (0xf);
            unsigned cnttrg_data_12 = (buf7 >> 20) & (0xfff);
            gdlrvc12 = (buf5 & 0xfff);
            unsigned gdlhdrcnttrg20 = (buf5 >> 12) & 0xfffff;

            //unsigned wdperclk = (m_nwd_gdl - m_hdr_nwd_gdl) / m_nclk_gdl;
            //unsigned ibuf_lastclk_msw  = m_nwd_gdl - wdperclk;
            //unsigned buf_lastclk_msw  = (unsigned)buf[ibuf_lastclk_msw];
            //unsigned buf_lastclk_msw2 = (unsigned)buf[ibuf_lastclk_msw + 1];
            //unsigned buf_lastclk_msw3 = (unsigned)buf[ibuf_lastclk_msw + 2];
            unsigned cnttrg_data_16 = (cnttrg_data_15to12 << 12) + cnttrg_data_12;
            unsigned eve16 = (_eve & 0xffff);

            h_0->SetBinContent(4 * (int)e_gdl + 1, m_nclk_gdl, gdlhdrcnttrg20);
            h_0->SetBinContent(4 * (int)e_gdl + 2, m_nclk_gdl, cnttrg_data_16);
            h_0->SetBinContent(4 * (int)e_gdl + 3, m_nclk_gdl, gdlrvc12);

            if (nword != m_nwd_gdl || buf0 != m_fmid_gdl || lastdd != 0xdd ||
                gdlhdrcnttrg20 != eve20 ||
                !(eve16 == cnttrg_data_16 + 1 || (eve16 == 0 && cnttrg_data_16 == 0xffff))
               ) {

              unsigned diag = 0;
              diag |= (nword  != m_nwd_gdl)  ? 1 : 0;
              diag |= (buf0   != m_fmid_gdl) ? 2 : 0;
              diag |= (lastdd != 0xdd)       ? 4 : 0;
              diag |= (gdlhdrcnttrg20 != eve20) ? 8 : 0;
              diag |= !(eve16 == cnttrg_data_16 + 1 || (eve16 == 0 && cnttrg_data_16 == 0xffff)) ? 16 : 0;
              printf("ae:GDL(0x%x%c) exp(%d), run(%d), evedaq(%u=0x%x), gdlhdrcnttrg20(%u,0x%x), "
                     "cnttrg_data16(%u=0x%x), l1rvc(%u)-lastrvc(%u)=%u, nword(%d), diag(%u)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, gdlhdrcnttrg20, gdlhdrcnttrg20,
                     cnttrg_data_16, cnttrg_data_16, gdlrvc12, last_rvc, gdlrvc12 - last_rvc, nword, diag); //TODO can the difference be negative?
              printf("\n");
              cntr_nwn_badtrg[e_gdl]++;
            } else {
              cntr_nwn[e_gdl]++;
            }
          }
        }
      }
    }
  }

  /* Other than GDL */
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      unsigned cprid = raw_trgarray[i]->GetNodeID(j);
      //unsigned eve16 = (_eve & 0xffff);
      unsigned eve20 = (_eve & 0xfffff);
      for (int hslb = 0; hslb < 2; hslb++) {
        unsigned e_mdl = -1;
        unsigned _nwd = -1;
        unsigned _scale = -1;
        unsigned _nclk = -1;
        unsigned _hdr_nwd = 3;
        if (m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) {
          e_mdl = e_2d0; _nwd = m_nwd_2ds; _scale = m_scale_2ds; _nclk = m_nclk_2ds; _hdr_nwd = m_hdr_nwd_2d0;
        } else if (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) {
          e_mdl = e_2d1; _nwd = m_nwd_2ds; _scale = m_scale_2ds; _nclk = m_nclk_2ds; _hdr_nwd = m_hdr_nwd_2d1;
        } else if (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) {
          e_mdl = e_2d2; _nwd = m_nwd_2ds; _scale = m_scale_2ds; _nclk = m_nclk_2ds; _hdr_nwd = m_hdr_nwd_2d2;
        } else if (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3) {
          e_mdl = e_2d3; _nwd = m_nwd_2ds; _scale = m_scale_2ds; _nclk = m_nclk_2ds; _hdr_nwd = m_hdr_nwd_2d3;
        } else if (m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) {
          e_mdl = e_3d0; _nwd = m_nwd_3ds; _scale = m_scale_3ds; _nclk = m_nclk_3ds; _hdr_nwd = m_hdr_nwd_3d0;
        } else if (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) {
          e_mdl = e_3d1; _nwd = m_nwd_3ds; _scale = m_scale_3ds; _nclk = m_nclk_3ds; _hdr_nwd = m_hdr_nwd_3d1;
        } else if (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) {
          e_mdl = e_3d2; _nwd = m_nwd_3ds; _scale = m_scale_3ds; _nclk = m_nclk_3ds; _hdr_nwd = m_hdr_nwd_3d2;
        } else if (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3) {
          e_mdl = e_3d3; _nwd = m_nwd_3ds; _scale = m_scale_3ds; _nclk = m_nclk_3ds; _hdr_nwd = m_hdr_nwd_3d3;
        } else if (m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) {
          e_mdl = e_nn0; _nwd = m_nwd_nns; _scale = m_scale_nns; _nclk = m_nclk_nns; _hdr_nwd = m_hdr_nwd_nn0;
        } else if (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) {
          e_mdl = e_nn1; _nwd = m_nwd_nns; _scale = m_scale_nns; _nclk = m_nclk_nns; _hdr_nwd = m_hdr_nwd_nn1;
        } else if (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) {
          e_mdl = e_nn2; _nwd = m_nwd_nns; _scale = m_scale_nns; _nclk = m_nclk_nns; _hdr_nwd = m_hdr_nwd_nn2;
        } else if (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3) {
          e_mdl = e_nn3; _nwd = m_nwd_nns; _scale = m_scale_nns; _nclk = m_nclk_nns; _hdr_nwd = m_hdr_nwd_nn3;
        } else if (m_cpr_sl0 == cprid && hslb == 0 && m_on_sl0) {
          e_mdl = e_sl0; _nwd = m_nwd_sl0; _scale = m_scale_sl0; _nclk = m_nclk_sl0; _hdr_nwd = m_hdr_nwd_sl0;
        } else if (m_cpr_sl1 == cprid && hslb == 0 && m_on_sl1) { //ex w/ 1
          e_mdl = e_sl1; _nwd = m_nwd_sl1; _scale = m_scale_sl1; _nclk = m_nclk_sl1; _hdr_nwd = m_hdr_nwd_sl1;
        } else if (m_cpr_sl2 == cprid && hslb == 0 && m_on_sl2) {
          e_mdl = e_sl2; _nwd = m_nwd_sl2; _scale = m_scale_sl2; _nclk = m_nclk_sl2; _hdr_nwd = m_hdr_nwd_sl2;
        } else if (m_cpr_sl3 == cprid && hslb == 1 && m_on_sl3) {
          e_mdl = e_sl3; _nwd = m_nwd_sl3; _scale = m_scale_sl3; _nclk = m_nclk_sl3; _hdr_nwd = m_hdr_nwd_sl3;
        } else if (m_cpr_sl4 == cprid && hslb == 1 && m_on_sl4) { //ex w/ 4
          e_mdl = e_sl4; _nwd = m_nwd_sl4; _scale = m_scale_sl4; _nclk = m_nclk_sl4; _hdr_nwd = m_hdr_nwd_sl4;
        } else if (m_cpr_sl5 == cprid && hslb == 1 && m_on_sl5) {
          e_mdl = e_sl5; _nwd = m_nwd_sl5; _scale = m_scale_sl5; _nclk = m_nclk_sl5; _hdr_nwd = m_hdr_nwd_sl5;
        } else if (m_cpr_sl6 == cprid && hslb == 0 && m_on_sl6) {
          e_mdl = e_sl6; _nwd = m_nwd_sl6; _scale = m_scale_sl6; _nclk = m_nclk_sl6; _hdr_nwd = m_hdr_nwd_sl6;
        } else if (m_cpr_sl8 == cprid && hslb == 0 && m_on_sl8) {
          e_mdl = e_sl8; _nwd = m_nwd_sl8; _scale = m_scale_sl8; _nclk = m_nclk_sl8; _hdr_nwd = m_hdr_nwd_sl8;
        } else if (m_cpr_etf == cprid && hslb == 1 && m_on_etf) {
          e_mdl = e_etf; _nwd = m_nwd_etf; _scale = m_scale_etf; _nclk = m_nclk_etf; _hdr_nwd = m_hdr_nwd_etf;
        } else if (m_cpr_top == cprid && hslb == 0 && m_on_top) {
          e_mdl = e_top; _nwd = m_nwd_top; _scale = m_scale_top; _nclk = m_nclk_top; _hdr_nwd = m_hdr_nwd_top;
        } else {
          continue;
        }

        unsigned nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
        int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
        unsigned buf0 = 0;
        if (nword > 0) buf0 = (unsigned)buf[0];
        unsigned buf1 = 0;
        if (nword > 1) buf1 = (unsigned)buf[1];
        unsigned buf2 = 0;
        if (nword > 2) buf2 = (unsigned)buf[2];
        unsigned buf2cnttrg20 = (buf2 >> 12) & 0xfffff;
        unsigned buf2rvc12    = (buf2 & 0xfff);

        if (nword < 3) {
          // err
          cntr_nwe_badnwd[e_mdl]++;
          printf("ah:%s(0x%x%c) exp(%d), run(%d), eve(%u), eve(0x%x), nword(%u), ",
                 moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
          printf("buf0(0x%x), buf1(0x%x), buf2(0x%x)\n", buf0, buf1, buf2);
          continue;
        } else if (nword == _hdr_nwd) {
          // header-only event
          if (gdlrvc12 != buf2rvc12) {
            printf("ai:%s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                   moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
            cntr_nw3_badrvc[e_mdl]++;
          } else if (eve20 != buf2cnttrg20) {
            printf("aj:%s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), cnttrg20(0x%x)\n",
                   moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20);
            cntr_nw3_badtrg[e_mdl]++;
          } else if ((_eve % _scale) == 0) {
            cntr_nw3_badvet[e_mdl]++;
          } else {
            cntr_nw3[e_mdl]++;
          }
          continue;
        } else if (nword != _nwd) {
          // err
          printf("bo:wrong nword: %s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), buf0(0x%x), buf2(0x%x)\n",
                 moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
          cntr_nwe_badnwd[e_mdl]++;
          continue;
        }

        unsigned buf3 = (unsigned)buf[_hdr_nwd];
        unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
        unsigned buf4 = (unsigned)buf[_hdr_nwd + 1];
        unsigned buf5 = (unsigned)buf[_hdr_nwd + 2];
        unsigned wdperclk = (_nwd - _hdr_nwd) / _nclk;
        //unsigned ibuf_lsw = _nwd - 1;
        unsigned ibuf_lastclk_msw  = _nwd - wdperclk;
        //unsigned buf_lastclk_msw  = (unsigned)buf[ibuf_lastclk_msw];
        unsigned buf_lastclk_msw2 = (unsigned)buf[ibuf_lastclk_msw + 1];
        unsigned buf_lastclk_msw3 = (unsigned)buf[ibuf_lastclk_msw + 2];
        //unsigned dddd_lastclk = ((buf_lastclk_msw >> 16) & 0xffff);
        unsigned datacnttrg32 = 0;
        unsigned buf4_msb16 = (buf4 & 0xffff);
        unsigned buf5_lsb16 = ((buf5 >> 16) & 0xffff);

        if (m_mydebug) {
          printf("af:Debug0: %s(0x%x%c) exp(%d), run(%d), eve(%u), eve(0x%x), nword(%u)\n",
                 moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
        }

        /*
         *
         * Module Start
         *
         *
         */

        if ((0x11000009 == cprid && hslb == 0) || // SL1
            (0x11000008 == cprid && hslb == 1) || // SL1
            (0x11000009 == cprid && hslb == 1)    // SL5
           ) {
          /* SL1 SL3 SL5 4096x48
             data_b2l_r(4095 downto 4048) <= x"dddd" & "00000" & revoclk & cntr125M(15 downto 0);--i=0..5
             data_b2l_r(4031 downto 4000) <= cnttrg;--i=6..9
             buf[3] = dddd * revoclk
             buf[4] = cntr125M & x"0000"
             buf[5] = cnttrg
          */
          datacnttrg32 = buf5;

        } else if ((0x11000007 == cprid && hslb == 0) || // SL0
                   (0x11000007 == cprid && hslb == 1) || // SL4
                   (0x1100000a == cprid && hslb == 0)    // SL6
                  ) {
          /* SL0 SL4 SL6 2048*48
             data_b2l_r(2047 downto 2000) <= x"dddd" &"00000"& revoclk & cntr125M(15 downto 0);
             data_b2l_r(1999 downto 1968) <= cnttrg;

             data_b2l(2047, 2016) <= buf[3]
             data_b2l(2015, 1984) <= buf[4]
             data_b2l(1983, 1952) <= buf[5]
             data_b2l(1999, 1968) <= cnttrg;
          */
          datacnttrg32 = (buf4_msb16 << 16) + buf5_lsb16;

        } else if ((m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) ||
                   (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) ||
                   (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) ||
                   (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3)) {
          /*
            2D2D2D: 2048 bit x 48 clock = 64 word x 48 clock
          */

          unsigned buf_lastclk_lsb  = (unsigned)buf[_nwd - 1];
          unsigned buf_lastclk_lsb2 = (unsigned)buf[_nwd - 2];
          //unsigned nworking = (buf_lastclk_lsb2 >> 24) & 0xff;
          //unsigned idbuf    = (buf_lastclk_lsb2 >> 16) & 0xff;
          //unsigned ihdr     = (buf_lastclk_lsb2 >> 8)  & 0xff;
          datacnttrg32 = ((buf_lastclk_lsb2 & 0xff) << 24) + (buf_lastclk_lsb >> 8);

        } else if ((m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) ||
                   (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) ||
                   (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) ||
                   (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3)) {
          /*
            3D3D3D: 2048 bit x 48 clock = 64 word x 48 clock
          */

          // data_b2l_r(2623 downto 2576) <= x"dddd" &"00000"& revoclk & cntr125M(15 downto 0);
          // data_b2l_r(2575 downto 2544) <= cnttrg;
          // [3] (2623, 2592)
          // [4] (2591, 2560)
          // [5] (2559, 2528)
          unsigned datacnttrg_msb16 = (buf_lastclk_msw2 & 0xffff);
          unsigned datacnttrg_lsb16 = ((buf_lastclk_msw3 >> 16) & 0xffff);
          datacnttrg32 = (datacnttrg_msb16 << 16) + datacnttrg_lsb16;

        } else if ((m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) ||
                   (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) ||
                   (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) ||
                   (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3)) {
          /*
            int e_mdl = -1;
            if (m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) e_mdl = e_nn0;
            if (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) e_mdl = e_nn1;
            if (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) e_mdl = e_nn2;
            if (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3) e_mdl = e_nn3;
          */
          /*
            NNNNNN: 2048 bit x 48 clock = 64 word x 48 clock
            data_b2l_r(1999 downto 1968) <= cnttrg;
          */

          // [3] 2047 2016
          // [4] 2015 1984
          // [5] 1983 1952
          // [3 + 64] = [m_hdr_nwd + wdperclk]
          // [3 + 65] = [m_hdr_nwd + wdperclk + 1]
          // [3 + 66] = [m_hdr_nwd + wdperclk + 2]
          // [3 + 64 * 47] = [m_hdr_nwd + wdperclk * (_nclk - 1)]
          // [3 + 65 * 47] = [m_hdr_nwd + wdperclk * (_nclk - 1) + 1]
          // [3 + 66 * 47] = [m_hdr_nwd + wdperclk * (_nclk - 1) + 2]
          unsigned datacnttrg_msb16 = (buf_lastclk_msw2 & 0xffff);
          unsigned datacnttrg_lsb16 = ((buf_lastclk_msw3 >> 16) & 0xffff);
          datacnttrg32 = (datacnttrg_msb16 << 16) + datacnttrg_lsb16;
        } else if (m_cpr_top == cprid && hslb == 0 && m_on_top) {
          // TOPTRG
        } // Modules

        bool something_bad = false;
        h_0->SetBinContent(4 * e_mdl + 1, _nclk, buf2cnttrg20);
        h_0->SetBinContent(4 * e_mdl + 2, _nclk, datacnttrg32);
        h_0->SetBinContent(4 * e_mdl + 3, _nclk, buf2rvc12);
        if (gdlrvc12 != buf2rvc12) {
          printf("ba:%s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                 moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
          cntr_nwn_badrvc[e_mdl]++;
          something_bad = true;
        }
        if (buf3dddd == 0xbbbb) {
          cntr_nwn_badbbb[e_mdl]++;
          printf("bv:%s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), cnttrg20(0x%x), dddd(0x%x), datacnttrg32(0x%x)\n",
                 moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, buf3dddd, datacnttrg32);
          something_bad = true;
        }
        if (buf3dddd != 0xdddd) {
          cntr_nwn_badddd[e_mdl]++;
          printf("bb:%s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), cnttrg20(0x%x), dddd(0x%x), datacnttrg32(0x%x)\n",
                 moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, buf3dddd, datacnttrg32);
          something_bad = true;
        }
        if (datacnttrg32 + 1 != _eve) {
          if (!(
                (m_cpr_top == cprid) ||
                (m_cpr_3d0 <= cprid && cprid <= m_cpr_3d3)
              )
             ) {
            printf("bc:%s(0x%x%c) exp(%d), run(%d), eve(%u,0x%x), nword(%u), cnttrg20(0x%x), dddd(0x%x), datacnttrg32(0x%x)\n",
                   moduleNames[e_mdl], cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, buf3dddd, datacnttrg32);
            cntr_nwn_badtrg[e_mdl]++;
            something_bad = true;
          }
        }
        if (! something_bad) cntr_nwn[e_mdl]++;

      } // for hslb
    }
  }

  if (m_cc_check) {
    for (int i = 0; i < raw_trgarray.getEntries(); i++) {
      for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
        unsigned cprid = raw_trgarray[i]->GetNodeID(j);
//      int _exp       = raw_trgarray[i]->GetExpNo(j);
//      int _run       = raw_trgarray[i]->GetRunNo(j);
        unsigned _eveRaw       = raw_trgarray[i]->GetEveNo(j);
        if ((0x11000001 <= cprid && cprid <= 0x1100000a) ||
            (0x15000001 <= cprid && cprid <= 0x15000002) ||
            (0x12000001 == cprid) // TOPTRG
           ) {
        } else {
          continue;
        }
        for (int hslb = 0; hslb < 2; hslb++) {
          int nclks = m_nclk_cdctrg;
          unsigned _hdr_nwd = 3;
          unsigned e_mdl = -1;
          unsigned _nwd = -1;
          if (m_cpr_top == cprid && hslb == 0 && m_on_top) {
            e_mdl = e_top; _nwd = m_nwd_top;
            nclks = m_nclk_top; _hdr_nwd = m_hdr_nwd_top;
          } else if (m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) {
            e_mdl = e_2d0; _nwd = m_nwd_2ds; _hdr_nwd = m_hdr_nwd_2d0;
          } else if (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) {
            e_mdl = e_2d1; _nwd = m_nwd_2ds; _hdr_nwd = m_hdr_nwd_2d1;
          } else if (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) {
            e_mdl = e_2d2; _nwd = m_nwd_2ds; _hdr_nwd = m_hdr_nwd_2d2;
          } else if (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3) {
            e_mdl = e_2d3; _nwd = m_nwd_2ds; _hdr_nwd = m_hdr_nwd_2d3;
          } else if (m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) {
            e_mdl = e_3d0; _nwd = m_nwd_3ds; _hdr_nwd = m_hdr_nwd_3d0;
          } else if (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) {
            e_mdl = e_3d1; _nwd = m_nwd_3ds; _hdr_nwd = m_hdr_nwd_3d1;
          } else if (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) {
            e_mdl = e_3d2; _nwd = m_nwd_3ds; _hdr_nwd = m_hdr_nwd_3d2;
          } else if (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3) {
            e_mdl = e_3d3; _nwd = m_nwd_3ds; _hdr_nwd = m_hdr_nwd_3d3;
          } else if (m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) {
            e_mdl = e_nn0; _nwd = m_nwd_nns; _hdr_nwd = m_hdr_nwd_nn0;
          } else if (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) {
            e_mdl = e_nn1; _nwd = m_nwd_nns; _hdr_nwd = m_hdr_nwd_nn1;
          } else if (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) {
            e_mdl = e_nn2; _nwd = m_nwd_nns; _hdr_nwd = m_hdr_nwd_nn2;
          } else if (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3) {
            e_mdl = e_nn3; _nwd = m_nwd_nns; _hdr_nwd = m_hdr_nwd_nn3;
          } else if (m_cpr_sl0 == cprid && hslb == 0 && m_on_sl0) {
            e_mdl = e_sl0; _nwd = m_nwd_sl0; _hdr_nwd = m_hdr_nwd_sl0;
          } else if (m_cpr_sl1 == cprid && hslb == 0 && m_on_sl1) {//ex w/ 4
            e_mdl = e_sl1; _nwd = m_nwd_sl1; _hdr_nwd = m_hdr_nwd_sl1;
          } else if (m_cpr_sl2 == cprid && hslb == 0 && m_on_sl2) {
            e_mdl = e_sl2; _nwd = m_nwd_sl2; _hdr_nwd = m_hdr_nwd_sl2;
          } else if (m_cpr_sl3 == cprid && hslb == 1 && m_on_sl3) {
            e_mdl = e_sl3; _nwd = m_nwd_sl3; _hdr_nwd = m_hdr_nwd_sl3;
          } else if (m_cpr_sl4 == cprid && hslb == 1 && m_on_sl4) {//ex w/ 1
            e_mdl = e_sl4; _nwd = m_nwd_sl4; _hdr_nwd = m_hdr_nwd_sl4;
          } else if (m_cpr_sl5 == cprid && hslb == 1 && m_on_sl5) {
            e_mdl = e_sl5; _nwd = m_nwd_sl5; _hdr_nwd = m_hdr_nwd_sl5;
          } else if (m_cpr_sl6 == cprid && hslb == 0 && m_on_sl6) {
            e_mdl = e_sl6; _nwd = m_nwd_sl6; _hdr_nwd = m_hdr_nwd_sl6;
          } else if (m_cpr_sl8 == cprid && hslb == 0 && m_on_sl8) {
            e_mdl = e_sl8; _nwd = m_nwd_sl8; _hdr_nwd = m_hdr_nwd_sl8;
          } else if (m_cpr_etf == cprid && hslb == 1 && m_on_etf) {
            e_mdl = e_etf; _nwd = m_nwd_etf; _hdr_nwd = m_hdr_nwd_etf;
          } else if (m_cpr_gdl == cprid && hslb == 0 && m_on_gdl) {
            e_mdl = e_gdl; _nwd = m_nwd_gdl;
            nclks = m_nclk_gdl; _hdr_nwd = 6;
          } else {
            continue;
          }
          unsigned nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
          if (nword == _hdr_nwd) continue;
          if (nword != _nwd) {
            printf("bu:Nword mismatch:nword(%u),expected(%u). eve(%u), %s(0x%x%c), nword(%u)\n",
                   nword, _nwd, _eveRaw, moduleNames[e_mdl], cprid, 'a' + hslb, nword);
          }
          int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
          int i47 = _hdr_nwd + (nclks - 1) * (_nwd - _hdr_nwd) / nclks;
          if (i47 > int(nword) - 1) {
            if (m_print_clkcyc_err)
              printf("bp:data truncation. eve(%u), %s(0x%x%c), nword(%u)\n",
                     _eveRaw, moduleNames[e_mdl], cprid, 'a' + hslb, nword);
            break;
          }
          unsigned buf47 = buf[i47];
          bool break_this_module = false;
          bool cc_disorder = false;

          unsigned prev_cc = buf47 & 0xffff;
          if (e_mdl == e_gdl) {
            prev_cc = (buf47 >> 4) & 0xfff;
          }
          char ccc[400];
          sprintf(ccc, "i47(%d),buf47(0x%x),%x", i47, buf47, prev_cc);
          std::string ccs(ccc);
          for (int clk = 0; clk < nclks; clk++) {
            unsigned ibuf = _hdr_nwd + clk * (nword - _hdr_nwd) / nclks;
            if (ibuf > nword - 1) {
              if (m_print_clkcyc_err)
                printf("bq:data truncation. eve(%u), %s(0x%x%c), nword(%u)\n",
                       _eveRaw, moduleNames[e_mdl], cprid, 'a' + hslb, nword);
              break_this_module = true;
              cntr_bad_nwd[e_mdl]++;
              break;
            }
            unsigned ddddcc = buf[ibuf];
            unsigned dddd = (ddddcc >> 16) & 0xffff;
            unsigned cc = ddddcc & 0xffff;
            if (e_mdl == e_gdl) {
              cc = (ddddcc >> 4) & 0xfff;
            }
            h_0->SetBinContent(4 * e_mdl + 4, clk + 1, cc);
            sprintf(ccc, ",%x", cc);
            if ((int)e_2d0 <= (int)e_mdl && (int)e_mdl <= (int)e_2d3 &&
                m_nwd_2ds == 3939/* =2624bit */) {
              // (downto) = wd   i
              // (      ) = 75 300
              // (      ) = 76 304
              // (      ) = 79 316
              // (79, 72) = 79 318 addr_o
              // (71, 64) = 79 319 addr_i
              // (63, 56) = 80 320 nworking
              // (55, 48) = 80 321 rennum
              // (47, 40) = 80 322 itarget
              // (39,  8) = 80 323-326 = cnttrg
              // ( 7,  0) = 81
              int i_addr = ibuf + 79; // addr_i and addr_o
              int val_addr = buf[i_addr];
              int addr_o = (val_addr >> 8) & 0xff;
              int addr_i =  val_addr     & 0xff;
              sprintf(ccc, ",%x(%d,%d)", cc, addr_i, addr_o);
            }
            ccs += ccc;
            if (e_mdl == e_gdl) {
              dddd = (ddddcc >> 24);
              if (dddd != 0xdd) {
                if (m_print_clkcyc_err)
                  printf("br:dddd not found. eve(%u), %s(0x%x%c), nword(%u)\n",
                         _eveRaw, moduleNames[e_mdl], cprid, 'a' + hslb, nword);
                break_this_module = true;
                cntr_bad_ddd[e_mdl]++;
                break;
              }
            } else if (dddd != 0xdddd) {
              if (m_print_clkcyc_err)
                printf("bs:dddd not found. eve(%u), %s(0x%x%c), nword(%u)\n",
                       _eveRaw, moduleNames[e_mdl], cprid, 'a' + hslb, nword);
              break_this_module = true;
              cntr_bad_ddd[e_mdl]++;
              break;
            }
            // 0x500 == 1280
            if (!(cc == prev_cc + 4 || prev_cc == cc + 4 * (nclks - 1) || prev_cc + 0x500 == cc + 4 * (nclks - 1)
                  || cc + 0x500 == prev_cc + 4)) {
              cc_disorder = true;
              ccs += "*";
            }
            prev_cc = cc;
          } // clk
          if (break_this_module) continue; // bad dddd
          if (cc_disorder) {
            cntr_bad_odr[e_mdl]++;
            if (m_print_cc) {
              printf("bt:ccdisorder: eve(%u), %s(0x%x%c), nword(%u), %s\n",
                     _eveRaw, moduleNames[e_mdl], cprid, 'a' + hslb, nword, ccs.c_str());
            }
          } else {
            cntr_good_odr[e_mdl]++;
          }
        } // hslb
      } // j < raw_trgarray[i]->GetNumEntries()
    } // i < raw_trgarray.getEntries()
  }

  if (! m_histRecord) {
    h_0->Delete();
  }
}

void TRGRAWDATAModule::defineHisto()
{
}
