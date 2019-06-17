//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : trgrawdataModule.cc
// Section  : TRG GDL DST
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG GDL DST
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------

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
  //  setPropertyFlags(c_ParallelProcessingCertified);

  n_basf2evt = -1;
  m_compressionLevel = 0;
  m_ncpr = 0;
  m_nftsw = 0;
  m_print_cnt = 0;

  addParam("mydebug", m_mydebug, "debug flag", false);
  addParam("cc_check", m_cc_check, "cc mode", true);
  addParam("print_cc", m_print_cc, "Print Disordered clocks", true);
  addParam("print_err", m_print_err, "Print Disordered clocks", false);
  addParam("print_dbmap", m_print_dbmap, "Print Database Bit Map", false);

  addParam("hdr_nwd", m_hdr_nwd, "Number of word for Header", 3);

  addParam("cpr_2d0", m_cpr_2d0, "Copper number of 2D0", 0x11000001);
  addParam("cpr_2d1", m_cpr_2d1, "Copper number of 2D1", 0x11000001);
  addParam("cpr_2d2", m_cpr_2d2, "Copper number of 2D2", 0x11000002);
  addParam("cpr_2d3", m_cpr_2d3, "Copper number of 2D3", 0x11000002);
  addParam("cpr_3d0", m_cpr_3d0, "Copper number of 3D0", 0x11000003);
  addParam("cpr_3d1", m_cpr_3d1, "Copper number of 3D1", 0x11000003);
  addParam("cpr_3d2", m_cpr_3d2, "Copper number of 3D2", 0x11000004);
  addParam("cpr_3d3", m_cpr_3d3, "Copper number of 3D3", 0x11000004);
  addParam("cpr_nn0", m_cpr_nn0, "Copper number of NN0", 0x11000005);
  addParam("cpr_nn1", m_cpr_nn1, "Copper number of NN1", 0x11000005);
  addParam("cpr_nn2", m_cpr_nn2, "Copper number of NN2", 0x11000006);
  addParam("cpr_nn3", m_cpr_nn3, "Copper number of NN3", 0x11000006);
  addParam("cpr_sl0", m_cpr_sl0, "Copper number of SL0", 0x11000007);
  addParam("cpr_sl1", m_cpr_sl1, "Copper number of SL1", 0x11000007);
  addParam("cpr_sl2", m_cpr_sl2, "Copper number of SL2", 0x11000008);
  addParam("cpr_sl3", m_cpr_sl3, "Copper number of SL3", 0x11000008);
  addParam("cpr_sl4", m_cpr_sl4, "Copper number of SL4", 0x11000009);
  addParam("cpr_sl5", m_cpr_sl5, "Copper number of SL5", 0x11000009);
  addParam("cpr_sl6", m_cpr_sl6, "Copper number of SL6", 0x1100000a);
  addParam("cpr_sl7", m_cpr_sl7, "Copper number of SL7", 0x1100000a);
  addParam("cpr_sl8", m_cpr_sl8, "Copper number of SL8", 0x1100000a);
  addParam("cpr_gdl", m_cpr_gdl, "Copper number of GDL", 0x15000001);
  addParam("cpr_etf", m_cpr_etf, "Copper number of ETF", 0x15000001);
  addParam("cpr_grl", m_cpr_grl, "Copper number of GRL", 0x15000002);

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

  addParam("nwd_2ds", m_nwd_2ds, "N word of 2Ds", 0xc03);
  addParam("nwd_3ds", m_nwd_3ds, "N word of 3Ds", 0xc03);
  addParam("nwd_nns", m_nwd_nns, "N word of NNs", 0xc03);
  addParam("nwd_sl0", m_nwd_sl0, "N word of SL0", 0xc03);  // 2048x48
  addParam("nwd_sl1", m_nwd_sl1, "N word of SL1", 0x1803); // 4096x48
  addParam("nwd_sl2", m_nwd_sl2, "N word of SL2", 0x1803); // 4096x48
  addParam("nwd_sl3", m_nwd_sl3, "N word of SL3", 0x1803); // 4096x48
  addParam("nwd_sl4", m_nwd_sl4, "N word of SL4", 0xc03);  // 2048x48
  addParam("nwd_sl5", m_nwd_sl5, "N word of SL5", 0x1803); // 4096x48
  addParam("nwd_sl6", m_nwd_sl6, "N word of SL6", 0xc03);  // 2048x48
  addParam("nwd_sl7", m_nwd_sl7, "N word of SL7", 0xc03);  // 1024x48
  addParam("nwd_sl8", m_nwd_sl8, "N word of SL8", 0xc03);
  addParam("nwd_gdl", m_nwd_gdl, "N word of GDL", 646);
  addParam("nwd_etf", m_nwd_etf, "N word of ETF", 1539); // 1024x48
  addParam("nwd_grl", m_nwd_grl, "N word of GRL", 0xc03);

  addParam("nclk_cdctrg", m_nclk_cdctrg, "N clock of CDCTRG", 48);
  addParam("nclk_gdl",    m_nclk_gdl,    "N clock of GDL", 32);

  addParam("scale_2ds", m_scale_2ds, "Scale factor of 2Ds", 256);
  addParam("scale_3ds", m_scale_3ds, "Scale factor of 3Ds", 256);
  addParam("scale_nns", m_scale_nns, "Scale factor of NNs", 256);
  addParam("scale_sl0", m_scale_sl0, "Scale factor of SL0", 256);
  addParam("scale_sl1", m_scale_sl1, "Scale factor of SL1", 256); //4096x48
  addParam("scale_sl2", m_scale_sl2, "Scale factor of SL2", 256);
  addParam("scale_sl3", m_scale_sl3, "Scale factor of SL3", 256);
  addParam("scale_sl4", m_scale_sl4, "Scale factor of SL4", 256);
  addParam("scale_sl5", m_scale_sl5, "Scale factor of SL5", 256);
  addParam("scale_sl6", m_scale_sl6, "Scale factor of SL6", 256);
  addParam("scale_sl7", m_scale_sl7, "Scale factor of SL7", 256);
  addParam("scale_sl8", m_scale_sl8, "Scale factor of SL8", 256);
  addParam("scale_gdl", m_scale_gdl, "Scale factor of GDL", 1);
  addParam("scale_etf", m_scale_etf, "Scale factor of ETF", 512); // 1024x48
  addParam("scale_grl", m_scale_grl, "Scale factor of GRL", 1);

  addParam("fmid_2d0", m_fmid_2d0, "Firmware ID of 2D0", 0x55330000);
  addParam("fmid_2d1", m_fmid_2d1, "Firmware ID of 2D1", 0x55330000);
  addParam("fmid_2d2", m_fmid_2d2, "Firmware ID of 2D2", 0x55330000);
  addParam("fmid_2d3", m_fmid_2d3, "Firmware ID of 2D3", 0x55330000);
  addParam("fmid_3d0", m_fmid_3d0, "Firmware ID of 3D0", 0x55330000);
  addParam("fmid_3d1", m_fmid_3d1, "Firmware ID of 3D1", 0x55330000);
  addParam("fmid_3d2", m_fmid_3d2, "Firmware ID of 3D2", 0x55330000);
  addParam("fmid_3d3", m_fmid_3d3, "Firmware ID of 3D3", 0x55330000);
  addParam("fmid_nn0", m_fmid_nn0, "Firmware ID of NN0", 0x55330000);
  addParam("fmid_nn1", m_fmid_nn1, "Firmware ID of NN1", 0x55330000);
  addParam("fmid_nn2", m_fmid_nn2, "Firmware ID of NN2", 0x55);
  addParam("fmid_nn3", m_fmid_nn3, "Firmware ID of NN3", 0x55330000);
  addParam("fmid_sl0", m_fmid_sl0, "Firmware ID of SL0", 0x55330000);
  addParam("fmid_sl1", m_fmid_sl1, "Firmware ID of SL1", 0x55330000);
  addParam("fmid_sl2", m_fmid_sl2, "Firmware ID of SL2", 0x55330000);
  addParam("fmid_sl3", m_fmid_sl3, "Firmware ID of SL3", 0x55330000);
  addParam("fmid_sl4", m_fmid_sl4, "Firmware ID of SL4", 0xc03);
  addParam("fmid_sl5", m_fmid_sl5, "Firmware ID of SL5", 0xc03);
  addParam("fmid_sl6", m_fmid_sl6, "Firmware ID of SL6", 0xc03);
  addParam("fmid_sl7", m_fmid_sl7, "Firmware ID of SL7", 0xc03);
  addParam("fmid_sl8", m_fmid_sl8, "Firmware ID of SL8", 0xc03);
  addParam("fmid_gdl", m_fmid_gdl, "Firmware ID of GDL", 0x47444c20);
  addParam("fmid_etf", m_fmid_etf, "Firmware ID of ETF", 0xc03);
  addParam("fmid_grl", m_fmid_grl, "Firmware ID of GRL", 0xc03);

}

void TRGRAWDATAModule::initialize()
{
  B2INFO("trgrawdata: initialize() started.");
  if (! m_unpacker) {
    B2INFO("no database of gdl unpacker");
  }
  if (m_print_dbmap) {
    for (int i = 0; i < 80; i++) {
      LeafBitMap[i] = m_unpacker->getLeafMap(i);
      std::cout << "LeafBitMap[" << i << "] = " << m_unpacker->getLeafMap(i) << std::endl;
    }
    for (int i = 0; i < 80; i++) {
      strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
      std::cout << "LeafNames[" << i << "] = " << m_unpacker->getLeafnames(i) << std::endl;
    }
    int aBitMap[320][2];
//  BitMap = std::vector<std::vector<int>>(m_unpacker->getnLeafs(),
//         std::vector<int>(2, 0));
    for (int i = 0; i < m_unpacker->getnLeafs(); i++) {
//    BitMap[i][0] = m_unpacker->getBitMap(i, 0);
//    BitMap[i][1] = m_unpacker->getBitMap(i, 1);
      aBitMap[i][0] = m_unpacker->getBitMap(i, 0);
      aBitMap[i][1] = m_unpacker->getBitMap(i, 1);
    }
    for (int leaf = 0; leaf < 80; leaf++) {
      int bin = m_unpacker->getLeafMap(leaf) + 1;
      if (bin != 0) {
        std::cout << "leaf(" << leaf
                  << "), bin(" << bin
                  << "), LeafNames[leaf](" << LeafNames[leaf]
//  << "), BitMap[bin-1][0](" << BitMap[bin-1][0]
//  << "), BitMap[bin-1][1](" << BitMap[bin-1][1]
                  << "), BitMap[bin-1][0](" << aBitMap[bin - 1][0]
                  << "), BitMap[bin-1][1](" << aBitMap[bin - 1][1]
                  << ")" << std::endl;
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

  /* SL1 4096 bit * 48 clocks = 6144 words */
  /* 2D   2048 bit * 48 clocks = 3072 words */
  B2INFO("trgrawdata: event() started.");
  StoreArray<RawTRG> raw_trgarray;

  /*
    GDL only. To get gdlrvc12
   */
  unsigned gdlrvc12 = -1;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
      unsigned cprid = raw_trgarray[i]->GetNodeID(j);
      int _exp = raw_trgarray[i]->GetExpNo(j);
      int _run = raw_trgarray[i]->GetRunNo(j);
      int _eve = raw_trgarray[i]->GetEveNo(j);
      unsigned eve20 = (_eve & 0xfffff);
      for (int hslb = 0; hslb < 2; hslb++) {
        int nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
        int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
        if (0x15000001 == cprid && hslb == 0) {

          if (m_mydebug) {
            printf("GDL start: 0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d)\n",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
          }
          unsigned buf0 = 0;
          if (nword > 0) buf0 = (unsigned)buf[0];
          unsigned buf1 = 0;
          if (nword > 1) buf1 = (unsigned)buf[1];
          unsigned buf2 = 0;
          if (nword > 2) buf2 = (unsigned)buf[2];
          unsigned buf2cnttrg20 = (buf2 >> 12) & 0xfffff;
          unsigned buf2rvc12    = (buf2 & 0xfff);

          if (nword < 3) {
            cntr_nwe_badnwd[e_gdl]++;
            printf("0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d), ",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
            printf("buf0(0x%x), buf1(0x%x), buf2(0x%x)\n", buf0, buf1, buf2);
            continue;
          }

          if (nword < 8) {
            printf("0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d)\n",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
            cntr_nwe_badnwd[e_gdl]++;
          } else {
            unsigned buf5 = (unsigned)buf[5];
            unsigned buf6 = (unsigned)buf[6];
            unsigned buf7 = (unsigned)buf[7];
            unsigned buf6dd = (buf6 >> 24) & 0xff;
            unsigned cnttrg_data_15to12 = buf6 & (0xf);
            unsigned cnttrg_data_12 = (buf7 >> 20) & (0xfff);
            unsigned cnttrg_data_16 = (cnttrg_data_15to12 << 12) + cnttrg_data_12;
            unsigned eve16 = (_eve & 0xffff);
            gdlrvc12 = (buf5 & 0xfff);
            unsigned buf5cnttrg20 = (buf5 >> 12) & 0xfffff;
            if (nword != m_nwd_gdl || buf0 != m_fmid_gdl || buf6dd != 0xdd ||
                buf5cnttrg20 != eve20 ||
                !(eve16 == cnttrg_data_16 + 1 || (eve16 == 0 && cnttrg_data_16 == 0xffff))
               ) {

              unsigned diag = 0;
              diag |= (nword  != m_nwd_gdl)  ? 1 : 0;
              diag |= (buf0   != m_fmid_gdl) ? 2 : 0;
              diag |= (buf6dd != 0xdd)       ? 4 : 0;
              printf("0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d), diag(%d)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, diag);
              // if (nword > 2) printf(", 0x%x 0x%x 0x%x", buf[0], buf[1], buf[2]);
              // if (nword > 3) printf(" 0x%x", buf[3]);
              // if (nword > 4) printf(" 0x%x", buf[4]);
              // if (nword > 5) printf(" 0x%x", buf[5]);
              // if (nword > 6) printf(" 0x%x", buf[6]);
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
      int _exp = raw_trgarray[i]->GetExpNo(j);
      int _run = raw_trgarray[i]->GetRunNo(j);
      int _eve = raw_trgarray[i]->GetEveNo(j);
      int exprun = _exp * 1000000 + _run;
      unsigned eve20 = (_eve & 0xfffff);
      for (int hslb = 0; hslb < 2; hslb++) {
        unsigned iboard = -1;
        unsigned _nwd = -1;
        if (m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) {
          iboard = e_2d0; _nwd = m_nwd_2ds;
        } else if (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) {
          iboard = e_2d1; _nwd = m_nwd_2ds;
        } else if (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) {
          iboard = e_2d2; _nwd = m_nwd_2ds;
        } else if (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3) {
          iboard = e_2d3; _nwd = m_nwd_2ds;
        } else if (m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) {
          iboard = e_3d0; _nwd = m_nwd_3ds;
        } else if (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) {
          iboard = e_3d1; _nwd = m_nwd_3ds;
        } else if (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) {
          iboard = e_3d2; _nwd = m_nwd_3ds;
        } else if (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3) {
          iboard = e_3d3; _nwd = m_nwd_3ds;
        } else if (m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) {
          iboard = e_nn0; _nwd = m_nwd_nns;
        } else if (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) {
          iboard = e_nn1; _nwd = m_nwd_nns;
        } else if (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) {
          iboard = e_nn2; _nwd = m_nwd_nns;
        } else if (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3) {
          iboard = e_nn3; _nwd = m_nwd_nns;
        } else if (m_cpr_sl0 == cprid && hslb == 0 && m_on_sl0) {
          iboard = e_sl0; _nwd = m_nwd_sl0;
        } else if (m_cpr_sl1 == cprid && hslb == 1 && m_on_sl1) {
          iboard = e_sl1; _nwd = m_nwd_sl1;
        } else if (m_cpr_sl2 == cprid && hslb == 0 && m_on_sl2) {
          iboard = e_sl2; _nwd = m_nwd_sl2;
        } else if (m_cpr_sl3 == cprid && hslb == 1 && m_on_sl3) {
          iboard = e_sl3; _nwd = m_nwd_sl3;
        } else if (m_cpr_sl4 == cprid && hslb == 0 && m_on_sl4) {
          iboard = e_sl4; _nwd = m_nwd_sl4;
        } else if (m_cpr_sl5 == cprid && hslb == 1 && m_on_sl5) {
          iboard = e_sl5; _nwd = m_nwd_sl5;
        } else if (m_cpr_sl6 == cprid && hslb == 0 && m_on_sl6) {
          iboard = e_sl6; _nwd = m_nwd_sl6;
        } else if (m_cpr_sl8 == cprid && hslb == 0 && m_on_sl8) {
          iboard = e_sl8; _nwd = m_nwd_sl8;
        } else if (m_cpr_etf == cprid && hslb == 1 && m_on_etf) {
          iboard = e_etf; _nwd = m_nwd_etf;
        } else {
          continue;
        }
        int nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
        int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
        if (m_mydebug) {
          printf("Debug0: 0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d)\n",
                 cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
        }
        if (
          //  (cprid <= 0x15000001) || // ETF
          (0x11000001 <= cprid && cprid <= 0x1100000a)) {

          if (!(
                // (m_cpr_gdl == cprid && hslb == 0 && m_on_gdl) ||
                // (m_cpr_grl == cprid && hslb == 0 && m_on_grl) ||
                // (m_cpr_etf == cprid && hslb == 1 && m_on_etf) ||
                (m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) ||
                (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) ||
                (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) ||
                (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3) ||
                (m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) ||
                (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) ||
                (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) ||
                (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3) ||
                (m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) ||
                (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) ||
                (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) ||
                (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3) ||
                (m_cpr_sl0 == cprid && hslb == 0 && m_on_sl0) ||
                (m_cpr_sl1 == cprid && hslb == 1 && m_on_sl1) ||
                (m_cpr_sl2 == cprid && hslb == 0 && m_on_sl2) ||
                (m_cpr_sl3 == cprid && hslb == 1 && m_on_sl3) ||
                (m_cpr_sl4 == cprid && hslb == 0 && m_on_sl4) ||
                (m_cpr_sl5 == cprid && hslb == 1 && m_on_sl5) ||
                (m_cpr_sl6 == cprid && hslb == 0 && m_on_sl6) ||
                (m_cpr_sl8 == cprid && hslb == 0 && m_on_sl8)
              )
             ) {
            continue;
          }
          if (m_mydebug) {
            printf("Debug1: 0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d)\n",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
          }
          unsigned buf0 = 0;
          if (nword > 0) buf0 = (unsigned)buf[0];
          unsigned buf1 = 0;
          if (nword > 1) buf1 = (unsigned)buf[1];
          unsigned buf2 = 0;
          if (nword > 2) buf2 = (unsigned)buf[2];
          unsigned buf2cnttrg20 = (buf2 >> 12) & 0xfffff;
          unsigned buf2rvc12    = (buf2 & 0xfff);

          if (nword < 3) {
            cntr_nwe_badnwd[iboard]++;
            printf("0x%x%c exp(%d), run(%d), eve(%d), eve(0x%x), nword(%d), ",
                   cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword);
            printf("buf0(0x%x), buf1(0x%x), buf2(0x%x)\n", buf0, buf1, buf2);
            continue;
          }

          /*
           *
           * Module Start
           *
           *
           */

          if ((exprun < 7004026 && 0x11000007 == cprid && hslb == 1) ||
              (exprun > 7004025 && 0x11000009 == cprid && hslb == 0)
             ) {
            /* SL1
               data_b2l_r(4095 downto 4048) <= x"dddd" & "00000" & revoclk & cnttrg(15 downto 0);
               No itarget, rennum
            */
            if (nword == 3) {
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20) {
                if (0) {
                  // if(m_sl1_dump){
                  printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x)\n",
                         cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20);
                }
                cntr_nw3_badtrg[iboard]++;
              } else if ((_eve % m_scale_sl1) == 0) {
                cntr_nw3_badvet[iboard]++;
              } else {
                cntr_nw3[iboard]++;
              }
            } else if (nword == 6147) {
              unsigned eve16 = (_eve & 0xffff);
              unsigned buf3 = (unsigned)buf[3];
              unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
              unsigned buf4 = (unsigned)buf[4];
              unsigned buf4_cnttrg16 = ((buf4 >> 16) & 0xffff);
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nwn_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20 ||
                         !(buf4_cnttrg16 + 1 == eve16 || (buf4_cnttrg16 == 0xffff && eve16 == 0))
                        ) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x), buf4cnttrg16(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, buf4_cnttrg16);
                cntr_nwn_badtrg[iboard]++;
              } else if (buf3dddd == 0xbbbb) {
                cntr_nwn_badbbb[iboard]++;
              } else if (buf3dddd != 0xdddd) {
                cntr_nwn_badddd[iboard]++;
              } else {
                cntr_nwn[iboard]++;
              }
            } else {
              printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), buf0(0x%x), buf2(0x%x)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
              cntr_nwe_badnwd[iboard]++;
            }
          } else if (0x11000009 == cprid && hslb == 1) {
            /* SL5
               data_b2l_r(4095 downto 4048) <= x"dddd" & "00000" & revoclk & cntr125M(15 downto 0);--i=0..5
               data_b2l_r(4047 downto 4016) <= cnttrg;--i=6..9
               data_b2l_r(4015 downto 4000) <= (others => '0'); -- Don't use this bits.
               buf[3] = dddd * revoclk
               buf[4] = cntr125M & cnttrg(31,16)
               buf[5] = cnttrg(15,0), itarget, rennum
            */
            if (nword == 3) {
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20) {
                if (0) {
                  // if(m_sl1_dump){
                  printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x)\n",
                         cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20);
                }
                cntr_nw3_badtrg[iboard]++;
              } else if ((_eve % m_scale_sl5) == 0) {
                cntr_nw3_badvet[iboard]++;
              } else {
                cntr_nw3[iboard]++;
              }
            } else if (nword == 6147) {
              unsigned eve16 = (_eve & 0xffff);
              unsigned buf3 = (unsigned)buf[3];
              unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
              unsigned buf4 = (unsigned)buf[4];
              unsigned buf5 = (unsigned)buf[5];
              unsigned buf4_cnttrg_msb16 = (buf4 & 0xffff);
              unsigned buf5_cnttrg_lsb16 = ((buf5 >> 16) & 0xffff);
              unsigned datacnttrg32 = (buf4_cnttrg_msb16 << 16) + buf5_cnttrg_lsb16;
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20 || datacnttrg32 + 1 != _eve) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x), datacnttrg32(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, datacnttrg32);
                cntr_nwn_badtrg[iboard]++;
              } else if (buf3dddd == 0xbbbb) {
                cntr_nwn_badbbb[iboard]++;
              } else if (buf3dddd != 0xdddd) {
                cntr_nwn_badddd[iboard]++;
              } else {
                cntr_nwn[iboard]++;
              }
            } else {
              printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), buf0(0x%x), buf2(0x%x)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
              cntr_nwe_badnwd[iboard]++;
            }
          } else if (0x11000007 <= cprid && cprid <= 0x1100000a) {
            /* Other SL */
            if (nword == 3) {
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20) {
                if (0) {
                  // if(m_sl1_dump){
                  printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x)\n",
                         cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20);
                }
                cntr_nw3_badtrg[iboard]++;
              } else if ((_eve % m_scale_sl5) == 0) {
                cntr_nw3_badvet[iboard]++;
              } else {
                cntr_nw3[iboard]++;
              }
            } else if ((cprid == 0x11000007 && hslb == 0 && nword == m_nwd_sl0) ||
                       (cprid == 0x11000008 && hslb == 0 && nword == m_nwd_sl2) ||
                       (cprid == 0x11000008 && hslb == 1 && nword == m_nwd_sl3) ||
                       (cprid == 0x11000009 && hslb == 0 && nword == m_nwd_sl4) ||
                       (cprid == 0x1100000a && hslb == 0 && nword == m_nwd_sl6)
                      ) {
              unsigned eve16 = (_eve & 0xffff);
              unsigned buf3 = (unsigned)buf[3];
              unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
              unsigned buf4 = (unsigned)buf[4];
              unsigned buf5 = (unsigned)buf[5];
              unsigned buf4_cnttrg_msb16 = (buf4 & 0xffff);
              unsigned buf5_cnttrg_lsb16 = ((buf5 >> 16) & 0xffff);
              unsigned datacnttrg32 = (buf4_cnttrg_msb16 << 16) + buf5_cnttrg_lsb16;
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nwn_badrvc[iboard]++;
              } else if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nwn_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20 || datacnttrg32 + 1 != _eve) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x), datacnttrg32(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, datacnttrg32);
                cntr_nwn_badtrg[iboard]++;
              } else if (buf3dddd == 0xbbbb) {
                cntr_nwn_badbbb[iboard]++;
              } else if (buf3dddd != 0xdddd) {
                cntr_nwn_badddd[iboard]++;
              } else {
                cntr_nwn[iboard]++;
              }
            } else {
              printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), buf0(0x%x), buf2(0x%x)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
              cntr_nwe_badnwd[iboard]++;
            }
          } else if ((m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) ||
                     (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) ||
                     (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) ||
                     (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3)) {
            /*
               2D: 2048 bit x 48 clock = 64 word x 48 clock
            */

            if (nword == 3) {
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if (eve20 != buf2cnttrg20) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20);
                cntr_nw3_badtrg[iboard]++;
              } else if ((_eve % m_scale_2ds) == 0) {
                cntr_nw3_badvet[iboard]++;
              } else {
                cntr_nw3[iboard]++;
              }
            } else if (nword == 3075) {
              unsigned buf3 = (unsigned)buf[3]; // ok
              unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
              unsigned buf65 = (unsigned)buf[65];
              unsigned dbuf = (buf65 & 0xffff);
              unsigned data_cnttrg32 = (unsigned)buf[66];
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nwn_badrvc[iboard]++;
              } else if (buf3dddd == 0xbbbb) {
                cntr_nwn_badbbb[iboard]++;
              } else if (buf3dddd != 0xdddd) {
                cntr_nwn_badddd[iboard]++;
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x), dddd(0x%x), datacnttrg32(0x%x), dbuf(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, buf3dddd, data_cnttrg32, dbuf);
              } else if (data_cnttrg32 + 1 != _eve) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), cnttrg20(0x%x), dddd(0x%x), datacnttrg32(0x%x), dbuf(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf2cnttrg20, buf3dddd, data_cnttrg32, dbuf);
                cntr_nwn_badtrg[iboard]++;
              } else {
                cntr_nwn[iboard]++;
              }
            } else {
              printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), buf0(0x%x), buf2(0x%x)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
              cntr_nwe_badnwd[iboard]++;
            }

          } else if ((m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) ||
                     (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) ||
                     (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) ||
                     (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3)) {
            /*
               3D: 2048 bit x 48 clock = 64 word x 48 clock
            */
            if (nword == 3) {
              // if(eve20 != buf2cnttrg20){
              if (gdlrvc12 != buf2rvc12) {
                printf("event slip: 0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if ((_eve % m_scale_3ds) == 0) {
                cntr_nw3_badvet[iboard]++;
              } else {
                cntr_nw3[iboard]++;
              }
            } else if (nword == 3075) {
              unsigned buf3 = (unsigned)buf[3];
              unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
              // unsigned buf65 = (unsigned)buf[65];
              // unsigned dbuf = (buf65 & 0xffff);
              // unsigned data_cnttrg32 = (unsigned)buf[66];
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nwn_badrvc[iboard]++;
              } else if (buf3dddd == 0xbbbb) {
                cntr_nwn_badbbb[iboard]++;
              } else if (buf3dddd != 0xdddd) {
                cntr_nwn_badddd[iboard]++;
              } else if (gdlrvc12 != buf2rvc12) {
                printf("event slip: 0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), dddd(0x%x), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf3dddd, gdlrvc12, buf2rvc12);
                cntr_nwn_badtrg[iboard]++;
              } else {
                cntr_nwn[iboard]++;
              }
            } else {
              printf("wrong nword: 0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), buf0(0x%x), buf2(0x%x)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
              cntr_nwe_badnwd[iboard]++;
            }

          } else if ((m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) ||
                     (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) ||
                     (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) ||
                     (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3)) {
            /*
               NN: 2048 bit x 48 clock = 64 word x 48 clock
            */

            if (nword == 3) {
              // if(eve20 != buf2cnttrg20){
              if (gdlrvc12 != buf2rvc12) {
                printf("event slip: 0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nw3_badrvc[iboard]++;
              } else if ((_eve % m_scale_nns) == 0) {
                cntr_nw3_badvet[iboard]++;
              } else {
                cntr_nw3[iboard]++;
              }
            } else if (nword == 3075) {
              unsigned buf3 = (unsigned)buf[3];
              unsigned buf3dddd = ((buf3 >> 16) & 0xffff);
              // unsigned buf65 = (unsigned)buf[65];
              // unsigned dbuf = (buf65 & 0xffff);
              // unsigned data_cnttrg32 = (unsigned)buf[66];
              if (gdlrvc12 != buf2rvc12) {
                printf("0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, gdlrvc12, buf2rvc12);
                cntr_nwn_badrvc[iboard]++;
              } else if (buf3dddd == 0xbbbb) {
                cntr_nwn_badbbb[iboard]++;
              } else if (buf3dddd != 0xdddd) {
                cntr_nwn_badddd[iboard]++;
              } else if (gdlrvc12 != buf2rvc12) {
                printf("event slip: 0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), dddd(0x%x), gdlrvc12(0x%x), buf2rvc12(0x%x)\n",
                       cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf3dddd, gdlrvc12, buf2rvc12);
                cntr_nwn_badtrg[iboard]++;
              } else {
                cntr_nwn[iboard]++;
              }
            } else {
              printf("wrong nword: 0x%x%c exp(%d), run(%d), eve(%d,0x%x), nword(%d), buf0(0x%x), buf2(0x%x)\n",
                     cprid, 'a' + hslb, _exp, _run, _eve, _eve, nword, buf0, buf2);
              cntr_nwe_badnwd[iboard]++;
            }

          } // Modules
        } // if TRG board
      } // for hslb
    }
  }
  // 2D 0xc03 = 2048x48
  // 3D 0xc03 = 2048x48
  // NN 0xc03 = 2048x48
  // TSF0 0xc03  = 2048x48
  // TSF1 0x1803 = 4096x48
  // TSF2 0x1803 = 4096x48
  // TSF3 0x1803 = 4096x48
  // TSF4 0xc03  = 2048x48
  // TSF5 0x1803 = 4096x48
  // TSF6 0xc03  = 2048x48
  if (m_cc_check)
    for (int i = 0; i < raw_trgarray.getEntries(); i++) {
      for (int j = 0; j < raw_trgarray[i]->GetNumEntries(); j++) {
        unsigned cprid = raw_trgarray[i]->GetNodeID(j);
        if ((0x11000001 <= cprid && cprid <= 0x1100000a) ||
            (0x15000001 <= cprid && cprid <= 0x15000002)) {
        } else {
          continue;
        }
        int _exp = raw_trgarray[i]->GetExpNo(j);
        int _run = raw_trgarray[i]->GetRunNo(j);
        int _eve = raw_trgarray[i]->GetEveNo(j);
        int exprun = _exp * 1000000 + _run;
        unsigned eve20 = (_eve & 0xfffff);
        for (int hslb = 0; hslb < 2; hslb++) {
          int nclks = m_nclk_cdctrg;
          int _hdr_nwd = 3;
          unsigned iboard = -1;
          unsigned _nwd = -1;
          if (m_cpr_2d0 == cprid && hslb == 0 && m_on_2d0) {
            iboard = e_2d0; _nwd = m_nwd_2ds;
          } else if (m_cpr_2d1 == cprid && hslb == 1 && m_on_2d1) {
            iboard = e_2d1; _nwd = m_nwd_2ds;
          } else if (m_cpr_2d2 == cprid && hslb == 0 && m_on_2d2) {
            iboard = e_2d2; _nwd = m_nwd_2ds;
          } else if (m_cpr_2d3 == cprid && hslb == 1 && m_on_2d3) {
            iboard = e_2d3; _nwd = m_nwd_2ds;
          } else if (m_cpr_3d0 == cprid && hslb == 0 && m_on_3d0) {
            iboard = e_3d0; _nwd = m_nwd_3ds;
          } else if (m_cpr_3d1 == cprid && hslb == 1 && m_on_3d1) {
            iboard = e_3d1; _nwd = m_nwd_3ds;
          } else if (m_cpr_3d2 == cprid && hslb == 0 && m_on_3d2) {
            iboard = e_3d2; _nwd = m_nwd_3ds;
          } else if (m_cpr_3d3 == cprid && hslb == 1 && m_on_3d3) {
            iboard = e_3d3; _nwd = m_nwd_3ds;
          } else if (m_cpr_nn0 == cprid && hslb == 0 && m_on_nn0) {
            iboard = e_nn0; _nwd = m_nwd_nns;
          } else if (m_cpr_nn1 == cprid && hslb == 1 && m_on_nn1) {
            iboard = e_nn1; _nwd = m_nwd_nns;
          } else if (m_cpr_nn2 == cprid && hslb == 0 && m_on_nn2) {
            iboard = e_nn2; _nwd = m_nwd_nns;
          } else if (m_cpr_nn3 == cprid && hslb == 1 && m_on_nn3) {
            iboard = e_nn3; _nwd = m_nwd_nns;
          } else if (m_cpr_sl0 == cprid && hslb == 0 && m_on_sl0) {
            iboard = e_sl0; _nwd = m_nwd_sl0;
          } else if ((exprun < 7004026 && cprid == 0x11000007 && hslb == 1 && m_on_sl1) ||
                     (exprun > 7004025 && cprid == 0x11000009 && hslb == 0 && m_on_sl1)) {
            iboard = e_sl1; _nwd = m_nwd_sl1;
          } else if (m_cpr_sl2 == cprid && hslb == 0 && m_on_sl2) {
            iboard = e_sl2; _nwd = m_nwd_sl2;
          } else if (m_cpr_sl3 == cprid && hslb == 1 && m_on_sl3) {
            iboard = e_sl3; _nwd = m_nwd_sl3;
          } else if ((exprun < 7004026 && cprid == 0x11000009 && hslb == 0 && m_on_sl4) ||
                     (exprun > 7004025 && cprid == 0x11000007 && hslb == 1 && m_on_sl4)) {
            iboard = e_sl4; _nwd = m_nwd_sl4;
          } else if (m_cpr_sl5 == cprid && hslb == 1 && m_on_sl5) {
            iboard = e_sl5; _nwd = m_nwd_sl5;
          } else if (m_cpr_sl6 == cprid && hslb == 0 && m_on_sl6) {
            iboard = e_sl6; _nwd = m_nwd_sl6;
          } else if (m_cpr_sl8 == cprid && hslb == 0 && m_on_sl8) {
            iboard = e_sl8; _nwd = m_nwd_sl8;
          } else if (m_cpr_etf == cprid && hslb == 1 && m_on_etf) {
            iboard = e_etf; _nwd = m_nwd_etf;
          } else if (m_cpr_gdl == cprid && hslb == 0 && m_on_gdl) {
            iboard = e_gdl; _nwd = m_nwd_gdl;
            nclks = m_nclk_gdl;
            _hdr_nwd = 6;
          } else {
            continue;
          }
          int nword = raw_trgarray[i]->GetDetectorNwords(j, hslb);
          if (nword == _hdr_nwd) continue;
          int* buf  = raw_trgarray[i]->GetDetectorBuffer(j, hslb);
          int i47 = _hdr_nwd + (nclks - 1) * (_nwd - _hdr_nwd) / nclks;
          if (i47 > nword - 1) {
            if (m_print_err)
              printf("data truncation. eve(%d), 0x%x%c, nword(%d)\n",
                     _eve, cprid, 'a' + hslb, nword);
            break;
          }
          unsigned buf47clk = buf[i47];
          bool break_this_module = false;
          bool cc_disorder = false;
//nsigned nwd = 0;

          unsigned prev_cc = buf47clk & 0xffff;
          if (iboard == e_gdl) {
            prev_cc = (buf47clk >> 4) & 0xfff;
          }
          char ccc[10];
          sprintf(ccc, "%x", prev_cc);
          std::string ccs(ccc);
          for (int clk = 0; clk < nclks; clk++) {
            unsigned ibuf = _hdr_nwd + clk * (nword - _hdr_nwd) / nclks;
            if (ibuf > nword - 1) {
              if (m_print_err)
                printf("data truncation. eve(%d), 0x%x%c, nword(%d)\n",
                       _eve, cprid, 'a' + hslb, nword);
              break_this_module = true;
              cntr_bad_nwd[iboard]++;
              break;
            }
            unsigned ddddcc = buf[ibuf];
            unsigned dddd = (ddddcc >> 16) & 0xffff;
            unsigned cc = ddddcc & 0xffff;
            if (iboard == e_gdl) {
              cc = (ddddcc >> 4) & 0xfff;
            }
            sprintf(ccc, ",%x", cc);
            ccs += ccc;
            if (iboard == e_gdl) {
              dddd = (ddddcc >> 24);
              if (dddd != 0xdd) {
                if (m_print_err)
                  printf("dddd not found. eve(%d), 0x%x%c, nword(%d)\n",
                         _eve, cprid, 'a' + hslb, nword);
                break_this_module = true;
                cntr_bad_ddd[iboard]++;
                break;
              }
            } else if (dddd != 0xdddd) {
              if (m_print_err)
                printf("dddd not found. eve(%d), 0x%x%c, nword(%d)\n",
                       _eve, cprid, 'a' + hslb, nword);
              break_this_module = true;
              cntr_bad_ddd[iboard]++;
              break;
            }
            // 0x500 == 1280
            if (!(cc == prev_cc + 4 || prev_cc == cc + 4 * (nclks - 1) || prev_cc + 0x500 == cc + 4 * (nclks - 1)
                  || cc + 0x500 == prev_cc + 4)) {
              cc_disorder = true;
              ccs += "*";
            }
            prev_cc = cc;
          }
          if (break_this_module) break; // bad dddd
          if (cc_disorder) {
            cntr_bad_odr[iboard]++;
            if (m_print_cc) {
              printf("ccdisorder: eve(%d), 0x%x%c, nword(%d), %s\n",
                     _eve, cprid, 'a' + hslb, nword, ccs.c_str());
            }
          } else {
            cntr_good_odr[iboard]++;
          }
        }
      }
    }


  //
  // TLU data
  //
  /*
  StoreArray<RawTLU> raw_tluarray;
  for (int i = 0; i < raw_tluarray.getEntries(); i++) {
    for (int j = 0; j < raw_tluarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawTLU) : Block # %d ", i);
      printFTSWEvent(raw_tluarray[ i ], j);
    }
  }
  */


}

void TRGRAWDATAModule::defineHisto()
{

  h_2d0_fmid = new TH1I("h_2d0_fmid", "bit shift events for 2d0", 1000, 0, 1000);
  h_2d1_fmid = new TH1I("h_2d1_fmid", "bit shift events for 2d1", 1000, 0, 1000);
  h_2d2_fmid = new TH1I("h_2d2_fmid", "bit shift events for 2d2", 1000, 0, 1000);
  h_2d3_fmid = new TH1I("h_2d3_fmid", "bit shift events for 2d3", 1000, 0, 1000);
  h_3d0_fmid = new TH1I("h_3d0_fmid", "bit shift events for 3d0", 1000, 0, 1000);
  h_3d1_fmid = new TH1I("h_3d1_fmid", "bit shift events for 3d1", 1000, 0, 1000);
  h_3d2_fmid = new TH1I("h_3d2_fmid", "bit shift events for 3d2", 1000, 0, 1000);
  h_3d3_fmid = new TH1I("h_3d3_fmid", "bit shift events for 3d3", 1000, 0, 1000);
  h_nn0_fmid = new TH1I("h_nn0_fmid", "bit shift events for nn0", 1000, 0, 1000);
  h_nn1_fmid = new TH1I("h_nn1_fmid", "bit shift events for nn1", 1000, 0, 1000);
  h_nn2_fmid = new TH1I("h_nn2_fmid", "bit shift events for nn2", 1000, 0, 1000);
  h_nn3_fmid = new TH1I("h_nn3_fmid", "bit shift events for nn3", 1000, 0, 1000);
  h_sl0_fmid = new TH1I("h_sl0_fmid", "bit shift events for sl0", 1000, 0, 1000);
  h_sl1_fmid = new TH1I("h_sl1_fmid", "bit shift events for sl1", 1000, 0, 1000);
  h_sl2_fmid = new TH1I("h_sl2_fmid", "bit shift events for sl2", 1000, 0, 1000);
  h_sl3_fmid = new TH1I("h_sl3_fmid", "bit shift events for sl3", 1000, 0, 1000);
  h_sl4_fmid = new TH1I("h_sl4_fmid", "bit shift events for sl4", 1000, 0, 1000);
  h_sl5_fmid = new TH1I("h_sl5_fmid", "bit shift events for sl5", 1000, 0, 1000);
  h_sl6_fmid = new TH1I("h_sl6_fmid", "bit shift events for sl6", 1000, 0, 1000);
  h_sl7_fmid = new TH1I("h_sl7_fmid", "bit shift events for sl7", 1000, 0, 1000);
  h_sl8_fmid = new TH1I("h_sl8_fmid", "bit shift events for sl8", 1000, 0, 1000);
  h_gdl_fmid = new TH1I("h_gdl_fmid", "bit shift events for gdl", 1000, 0, 1000);
  h_etf_fmid = new TH1I("h_etf_fmid", "bit shift events for etf", 1000, 0, 1000);
  h_grl_fmid = new TH1I("h_grl_fmid", "bit shift events for grl", 1000, 0, 1000);

  h_2d0_wd = new TH1I("h_2d0_wd", "evtno of wrong #word events for 2d0", 1000, 0, 1000);
  h_2d1_wd = new TH1I("h_2d1_wd", "evtno of wrong #word events for 2d1", 1000, 0, 1000);
  h_2d2_wd = new TH1I("h_2d2_wd", "evtno of wrong #word events for 2d2", 1000, 0, 1000);
  h_2d3_wd = new TH1I("h_2d3_wd", "evtno of wrong #word events for 2d3", 1000, 0, 1000);
  h_3d0_wd = new TH1I("h_3d0_wd", "evtno of wrong #word events for 3d0", 1000, 0, 1000);
  h_3d1_wd = new TH1I("h_3d1_wd", "evtno of wrong #word events for 3d1", 1000, 0, 1000);
  h_3d2_wd = new TH1I("h_3d2_wd", "evtno of wrong #word events for 3d2", 1000, 0, 1000);
  h_3d3_wd = new TH1I("h_3d3_wd", "evtno of wrong #word events for 3d3", 1000, 0, 1000);
  h_nn0_wd = new TH1I("h_nn0_wd", "evtno of wrong #word events for nn0", 1000, 0, 1000);
  h_nn1_wd = new TH1I("h_nn1_wd", "evtno of wrong #word events for nn1", 1000, 0, 1000);
  h_nn2_wd = new TH1I("h_nn2_wd", "evtno of wrong #word events for nn2", 1000, 0, 1000);
  h_nn3_wd = new TH1I("h_nn3_wd", "evtno of wrong #word events for nn3", 1000, 0, 1000);
  h_sl0_wd = new TH1I("h_sl0_wd", "evtno of wrong #word events for sl0", 1000, 0, 1000);
  h_sl1_wd = new TH1I("h_sl1_wd", "evtno of wrong #word events for sl1", 1000, 0, 1000);
  h_sl2_wd = new TH1I("h_sl2_wd", "evtno of wrong #word events for sl2", 1000, 0, 1000);
  h_sl3_wd = new TH1I("h_sl3_wd", "evtno of wrong #word events for sl3", 1000, 0, 1000);
  h_sl4_wd = new TH1I("h_sl4_wd", "evtno of wrong #word events for sl4", 1000, 0, 1000);
  h_sl5_wd = new TH1I("h_sl5_wd", "evtno of wrong #word events for sl5", 1000, 0, 1000);
  h_sl6_wd = new TH1I("h_sl6_wd", "evtno of wrong #word events for sl6", 1000, 0, 1000);
  h_sl7_wd = new TH1I("h_sl7_wd", "evtno of wrong #word events for sl7", 1000, 0, 1000);
  h_sl8_wd = new TH1I("h_sl8_wd", "evtno of wrong #word events for sl8", 1000, 0, 1000);
  h_gdl_wd = new TH1I("h_gdl_wd", "evtno of wrong #word events for gdl", 1000, 0, 1000);
  h_etf_wd = new TH1I("h_etf_wd", "evtno of wrong #word events for etf", 1000, 0, 1000);
  h_grl_wd = new TH1I("h_grl_wd", "evtno of wrong #word events for grl", 1000, 0, 1000);

  h_2d0_nwd = new TH1I("h_2d0_nwd", "#words for wrong #word events for 2d0", 1000, 0, 1000);
  h_2d1_nwd = new TH1I("h_2d1_nwd", "#words for wrong #word events for 2d1", 1000, 0, 1000);
  h_2d2_nwd = new TH1I("h_2d2_nwd", "#words for wrong #word events for 2d2", 1000, 0, 1000);
  h_2d3_nwd = new TH1I("h_2d3_nwd", "#words for wrong #word events for 2d3", 1000, 0, 1000);
  h_3d0_nwd = new TH1I("h_3d0_nwd", "#words for wrong #word events for 3d0", 1000, 0, 1000);
  h_3d1_nwd = new TH1I("h_3d1_nwd", "#words for wrong #word events for 3d1", 1000, 0, 1000);
  h_3d2_nwd = new TH1I("h_3d2_nwd", "#words for wrong #word events for 3d2", 1000, 0, 1000);
  h_3d3_nwd = new TH1I("h_3d3_nwd", "#words for wrong #word events for 3d3", 1000, 0, 1000);
  h_nn0_nwd = new TH1I("h_nn0_nwd", "#words for wrong #word events for nn0", 1000, 0, 1000);
  h_nn1_nwd = new TH1I("h_nn1_nwd", "#words for wrong #word events for nn1", 1000, 0, 1000);
  h_nn2_nwd = new TH1I("h_nn2_nwd", "#words for wrong #word events for nn2", 1000, 0, 1000);
  h_nn3_nwd = new TH1I("h_nn3_nwd", "#words for wrong #word events for nn3", 1000, 0, 1000);
  h_sl0_nwd = new TH1I("h_sl0_nwd", "#words for wrong #word events for sl0", 1000, 0, 1000);
  h_sl1_nwd = new TH1I("h_sl1_nwd", "#words for wrong #word events for sl1", 1000, 0, 1000);
  h_sl2_nwd = new TH1I("h_sl2_nwd", "#words for wrong #word events for sl2", 1000, 0, 1000);
  h_sl3_nwd = new TH1I("h_sl3_nwd", "#words for wrong #word events for sl3", 1000, 0, 1000);
  h_sl4_nwd = new TH1I("h_sl4_nwd", "#words for wrong #word events for sl4", 1000, 0, 1000);
  h_sl5_nwd = new TH1I("h_sl5_nwd", "#words for wrong #word events for sl5", 1000, 0, 1000);
  h_sl6_nwd = new TH1I("h_sl6_nwd", "#words for wrong #word events for sl6", 1000, 0, 1000);
  h_sl7_nwd = new TH1I("h_sl7_nwd", "#words for wrong #word events for sl7", 1000, 0, 1000);
  h_sl8_nwd = new TH1I("h_sl8_nwd", "#words for wrong #word events for sl8", 1000, 0, 1000);
  h_gdl_nwd = new TH1I("h_gdl_nwd", "#words for wrong #word events for gdl", 1000, 0, 1000);
  h_etf_nwd = new TH1I("h_etf_nwd", "#words for wrong #word events for etf", 1000, 0, 1000);
  h_grl_nwd = new TH1I("h_grl_nwd", "#words for wrong #word events for grl", 1000, 0, 1000);

  h_2d0_cnttrg = new TH1I("h_2d0_cnttrg", "evtno of wrong cnttrg events for 2d0", 1000, 0, 1000);
  h_2d1_cnttrg = new TH1I("h_2d1_cnttrg", "evtno of wrong cnttrg events for 2d1", 1000, 0, 1000);
  h_2d2_cnttrg = new TH1I("h_2d2_cnttrg", "evtno of wrong cnttrg events for 2d2", 1000, 0, 1000);
  h_2d3_cnttrg = new TH1I("h_2d3_cnttrg", "evtno of wrong cnttrg events for 2d3", 1000, 0, 1000);
  h_3d0_cnttrg = new TH1I("h_3d0_cnttrg", "evtno of wrong cnttrg events for 3d0", 1000, 0, 1000);
  h_3d1_cnttrg = new TH1I("h_3d1_cnttrg", "evtno of wrong cnttrg events for 3d1", 1000, 0, 1000);
  h_3d2_cnttrg = new TH1I("h_3d2_cnttrg", "evtno of wrong cnttrg events for 3d2", 1000, 0, 1000);
  h_3d3_cnttrg = new TH1I("h_3d3_cnttrg", "evtno of wrong cnttrg events for 3d3", 1000, 0, 1000);
  h_nn0_cnttrg = new TH1I("h_nn0_cnttrg", "evtno of wrong cnttrg events for nn0", 1000, 0, 1000);
  h_nn1_cnttrg = new TH1I("h_nn1_cnttrg", "evtno of wrong cnttrg events for nn1", 1000, 0, 1000);
  h_nn2_cnttrg = new TH1I("h_nn2_cnttrg", "evtno of wrong cnttrg events for nn2", 1000, 0, 1000);
  h_nn3_cnttrg = new TH1I("h_nn3_cnttrg", "evtno of wrong cnttrg events for nn3", 1000, 0, 1000);
  h_sl0_cnttrg = new TH1I("h_sl0_cnttrg", "evtno of wrong cnttrg events for sl0", 1000, 0, 1000);
  h_sl1_cnttrg = new TH1I("h_sl1_cnttrg", "evtno of wrong cnttrg events for sl1", 1000, 0, 1000);
  h_sl2_cnttrg = new TH1I("h_sl2_cnttrg", "evtno of wrong cnttrg events for sl2", 1000, 0, 1000);
  h_sl3_cnttrg = new TH1I("h_sl3_cnttrg", "evtno of wrong cnttrg events for sl3", 1000, 0, 1000);
  h_sl4_cnttrg = new TH1I("h_sl4_cnttrg", "evtno of wrong cnttrg events for sl4", 1000, 0, 1000);
  h_sl5_cnttrg = new TH1I("h_sl5_cnttrg", "evtno of wrong cnttrg events for sl5", 1000, 0, 1000);
  h_sl6_cnttrg = new TH1I("h_sl6_cnttrg", "evtno of wrong cnttrg events for sl6", 1000, 0, 1000);
  h_sl7_cnttrg = new TH1I("h_sl7_cnttrg", "evtno of wrong cnttrg events for sl7", 1000, 0, 1000);
  h_sl8_cnttrg = new TH1I("h_sl8_cnttrg", "evtno of wrong cnttrg events for sl8", 1000, 0, 1000);
  h_gdl_cnttrg = new TH1I("h_gdl_cnttrg", "evtno of wrong cnttrg events for gdl", 1000, 0, 1000);
  h_etf_cnttrg = new TH1I("h_etf_cnttrg", "evtno of wrong cnttrg events for etf", 1000, 0, 1000);
  h_grl_cnttrg = new TH1I("h_grl_cnttrg", "evtno of wrong cnttrg events for grl", 1000, 0, 1000);

}

void TRGRAWDATAModule::printFTSWEvent(RawDataBlock* raw_datablock, int i)
{
  int* buf  = raw_datablock->GetBuffer(i);
  int nwords =  raw_datablock->GetBlockNwords(i);
  printf("*******FTSW data**********: nwords %d\n", nwords);
  printBuffer(buf, nwords);



  RawFTSW rawftsw;
  int delete_flag = 0; // No need to free the buffer
  int num_event = 1;
  int num_nodes = 1;
  rawftsw.SetBuffer(buf, nwords, delete_flag, num_event, num_nodes);



  timeval tv;
  int n = 0;
  rawftsw.GetTTTimeVal(n , &tv);
  printf("eve %u TLU %d: %d %d %.8x: tv %d %d\n",
         rawftsw.GetEveNo(n),
         rawftsw.Get15bitTLUTag(n),
         rawftsw.GetBlockNwords(n),
         rawftsw.GetNwordsHeader(n),
         rawftsw.GetFTSWNodeID(n),
         (int)(tv.tv_sec), (int)(tv.tv_usec)
        );

  m_nftsw++;
  return;
}

void TRGRAWDATAModule::printBuffer(int* buf, int nwords)
{
  //  printf("%.8x :", 0);
  for (int j = 0; j < nwords; j++) {
    printf(" %.8x", buf[ j ]);
    if ((j + 1) % 10 == 0) {
      //      printf("\n%.8x :", j + 1);
      printf("\n");
    }
    m_print_cnt++;
  }
  printf("\n");

  return;
}

