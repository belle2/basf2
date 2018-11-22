//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGCDCT3DUnpackerModule.h
// Section  :
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRGCDCT3DUnpacker Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#ifndef TRGCDCT3DUNPACKER_H
#define TRGCDCT3DUNPACKER_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/cdc/dataobjects/TRGCDCT3DUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  namespace TRGCDCT3DUNPACKERSPACE {

    /** num of leafs in data_b2l **/
    const int nLeafs = 306;
    /** num of leafs for others **/
    const int nLeafsExtra = 4;
    /** num of clk time window **/
    const int nClks = 48 ;
    /** num of b2l bits **/
    const int nBits = 2048;
    /** leaf names **/
    const char* LeafNames[nLeafs + nLeafsExtra] = {
      "t3d_2doldtrk",
      "t3dtrk0_evtTime_delay",
      "t3dtrk0_evtTimeValid_delay",
      "t3dtrk0_charge",
      "t3dtrk0_rho",
      "t3dtrk0_phi0",
      "t3dtrk0ts0_id",
      "t3dtrk0ts0_lr",
      "t3dtrk0ts0_rt",
      "t3dtrk0ts1_id",
      "t3dtrk0ts1_lr",
      "t3dtrk0ts1_rt",
      "t3dtrk0ts2_id",
      "t3dtrk0ts2_lr",
      "t3dtrk0ts2_rt",
      "t3dtrk0ts3_id",
      "t3dtrk0ts3_lr",
      "t3dtrk0ts3_rt",
      "t3dtrk1_evtTime_delay",
      "t3dtrk1_evtTimeValid_delay",
      "t3dtrk1_charge",
      "t3dtrk1_rho",
      "t3dtrk1_phi0",
      "t3dtrk1ts0_id",
      "t3dtrk1ts0_lr",
      "t3dtrk1ts0_rt",
      "t3dtrk1ts1_id",
      "t3dtrk1ts1_lr",
      "t3dtrk1ts1_rt",
      "t3dtrk1ts2_id",
      "t3dtrk1ts2_lr",
      "t3dtrk1ts2_rt",
      "t3dtrk1ts3_id",
      "t3dtrk1ts3_lr",
      "t3dtrk1ts3_rt",
      "t3d_phase",
      "t3d_validTS",
      "t3d_2dfnf",
      "t3dtrk0_z0_s",
      "t3dtrk0_cot_s",
      "t3dtrk0_zchisq",
      "t3dtrk1_z0_s",
      "t3dtrk1_cot_s",
      "t3dtrk1_zchisq",
      "t3dtrk2_z0_s",
      "t3dtrk2_cot_s",
      "t3dtrk2_zchisq",
      "t3dtrk3_z0_s",
      "t3dtrk3_cot_s",
      "t3dtrk3_zchisq",
      "t3dtrk_rd_req",
      "tsf1_cc",
      "tsf1ts0_id",
      "tsf1ts0_rt",
      "tsf1ts0_lr",
      "tsf1ts0_pr",
      "tsf1ts1_id",
      "tsf1ts1_rt",
      "tsf1ts1_lr",
      "tsf1ts1_pr",
      "tsf1ts2_id",
      "tsf1ts2_rt",
      "tsf1ts2_lr",
      "tsf1ts2_pr",
      "tsf1ts3_id",
      "tsf1ts3_rt",
      "tsf1ts3_lr",
      "tsf1ts3_pr",
      "tsf1ts4_id",
      "tsf1ts4_rt",
      "tsf1ts4_lr",
      "tsf1ts4_pr",
      "tsf1ts5_id",
      "tsf1ts5_rt",
      "tsf1ts5_lr",
      "tsf1ts5_pr",
      "tsf1ts6_id",
      "tsf1ts6_rt",
      "tsf1ts6_lr",
      "tsf1ts6_pr",
      "tsf1ts7_id",
      "tsf1ts7_rt",
      "tsf1ts7_lr",
      "tsf1ts7_pr",
      "tsf1ts8_id",
      "tsf1ts8_rt",
      "tsf1ts8_lr",
      "tsf1ts8_pr",
      "tsf1ts9_id",
      "tsf1ts9_rt",
      "tsf1ts9_lr",
      "tsf1ts9_pr",
      "tsf3_cc",
      "tsf3ts0_id",
      "tsf3ts0_rt",
      "tsf3ts0_lr",
      "tsf3ts0_pr",
      "tsf3ts1_id",
      "tsf3ts1_rt",
      "tsf3ts1_lr",
      "tsf3ts1_pr",
      "tsf3ts2_id",
      "tsf3ts2_rt",
      "tsf3ts2_lr",
      "tsf3ts2_pr",
      "tsf3ts3_id",
      "tsf3ts3_rt",
      "tsf3ts3_lr",
      "tsf3ts3_pr",
      "tsf3ts4_id",
      "tsf3ts4_rt",
      "tsf3ts4_lr",
      "tsf3ts4_pr",
      "tsf3ts5_id",
      "tsf3ts5_rt",
      "tsf3ts5_lr",
      "tsf3ts5_pr",
      "tsf3ts6_id",
      "tsf3ts6_rt",
      "tsf3ts6_lr",
      "tsf3ts6_pr",
      "tsf3ts7_id",
      "tsf3ts7_rt",
      "tsf3ts7_lr",
      "tsf3ts7_pr",
      "tsf3ts8_id",
      "tsf3ts8_rt",
      "tsf3ts8_lr",
      "tsf3ts8_pr",
      "tsf3ts9_id",
      "tsf3ts9_rt",
      "tsf3ts9_lr",
      "tsf3ts9_pr",
      "tsf5_cc",
      "tsf5ts0_id",
      "tsf5ts0_rt",
      "tsf5ts0_lr",
      "tsf5ts0_pr",
      "tsf5ts1_id",
      "tsf5ts1_rt",
      "tsf5ts1_lr",
      "tsf5ts1_pr",
      "tsf5ts2_id",
      "tsf5ts2_rt",
      "tsf5ts2_lr",
      "tsf5ts2_pr",
      "tsf5ts3_id",
      "tsf5ts3_rt",
      "tsf5ts3_lr",
      "tsf5ts3_pr",
      "tsf5ts4_id",
      "tsf5ts4_rt",
      "tsf5ts4_lr",
      "tsf5ts4_pr",
      "tsf5ts5_id",
      "tsf5ts5_rt",
      "tsf5ts5_lr",
      "tsf5ts5_pr",
      "tsf5ts6_id",
      "tsf5ts6_rt",
      "tsf5ts6_lr",
      "tsf5ts6_pr",
      "tsf5ts7_id",
      "tsf5ts7_rt",
      "tsf5ts7_lr",
      "tsf5ts7_pr",
      "tsf5ts8_id",
      "tsf5ts8_rt",
      "tsf5ts8_lr",
      "tsf5ts8_pr",
      "tsf5ts9_id",
      "tsf5ts9_rt",
      "tsf5ts9_lr",
      "tsf5ts9_pr",
      "tsf7_cc",
      "tsf7ts0_id",
      "tsf7ts0_rt",
      "tsf7ts0_lr",
      "tsf7ts0_pr",
      "tsf7ts1_id",
      "tsf7ts1_rt",
      "tsf7ts1_lr",
      "tsf7ts1_pr",
      "tsf7ts2_id",
      "tsf7ts2_rt",
      "tsf7ts2_lr",
      "tsf7ts2_pr",
      "tsf7ts3_id",
      "tsf7ts3_rt",
      "tsf7ts3_lr",
      "tsf7ts3_pr",
      "tsf7ts4_id",
      "tsf7ts4_rt",
      "tsf7ts4_lr",
      "tsf7ts4_pr",
      "tsf7ts5_id",
      "tsf7ts5_rt",
      "tsf7ts5_lr",
      "tsf7ts5_pr",
      "tsf7ts6_id",
      "tsf7ts6_rt",
      "tsf7ts6_lr",
      "tsf7ts6_pr",
      "tsf7ts7_id",
      "tsf7ts7_rt",
      "tsf7ts7_lr",
      "tsf7ts7_pr",
      "tsf7ts8_id",
      "tsf7ts8_rt",
      "tsf7ts8_lr",
      "tsf7ts8_pr",
      "tsf7ts9_id",
      "tsf7ts9_rt",
      "tsf7ts9_lr",
      "tsf7ts9_pr",
      "t2d_cc",
      "t2d_fnf",
      "t2d0_charge",
      "t2d0_rho_s",
      "t2d0_phi",
      "t2d0ts0_id",
      "t2d0ts0_rt",
      "t2d0ts0_lr",
      "t2d0ts0_pr",
      "t2d0ts2_id",
      "t2d0ts2_rt",
      "t2d0ts2_lr",
      "t2d0ts2_pr",
      "t2d0ts4_id",
      "t2d0ts4_rt",
      "t2d0ts4_lr",
      "t2d0ts4_pr",
      "t2d0ts6_id",
      "t2d0ts6_rt",
      "t2d0ts6_lr",
      "t2d0ts6_pr",
      "t2d0ts8_id",
      "t2d0ts8_rt",
      "t2d0ts8_lr",
      "t2d0ts8_pr",
      "t2d1_charge",
      "t2d1_rho_s",
      "t2d1_phi",
      "t2d1ts0_id",
      "t2d1ts0_rt",
      "t2d1ts0_lr",
      "t2d1ts0_pr",
      "t2d1ts2_id",
      "t2d1ts2_rt",
      "t2d1ts2_lr",
      "t2d1ts2_pr",
      "t2d1ts4_id",
      "t2d1ts4_rt",
      "t2d1ts4_lr",
      "t2d1ts4_pr",
      "t2d1ts6_id",
      "t2d1ts6_rt",
      "t2d1ts6_lr",
      "t2d1ts6_pr",
      "t2d1ts8_id",
      "t2d1ts8_rt",
      "t2d1ts8_lr",
      "t2d1ts8_pr",
      "t2d2_charge",
      "t2d2_rho_s",
      "t2d2_phi",
      "t2d2ts0_id",
      "t2d2ts0_rt",
      "t2d2ts0_lr",
      "t2d2ts0_pr",
      "t2d2ts2_id",
      "t2d2ts2_rt",
      "t2d2ts2_lr",
      "t2d2ts2_pr",
      "t2d2ts4_id",
      "t2d2ts4_rt",
      "t2d2ts4_lr",
      "t2d2ts4_pr",
      "t2d2ts6_id",
      "t2d2ts6_rt",
      "t2d2ts6_lr",
      "t2d2ts6_pr",
      "t2d2ts8_id",
      "t2d2ts8_rt",
      "t2d2ts8_lr",
      "t2d2ts8_pr",
      "t2d3_charge",
      "t2d3_rho_s",
      "t2d3_phi",
      "t2d3ts0_id",
      "t2d3ts0_rt",
      "t2d3ts0_lr",
      "t2d3ts0_pr",
      "t2d3ts2_id",
      "t2d3ts2_rt",
      "t2d3ts2_lr",
      "t2d3ts2_pr",
      "t2d3ts4_id",
      "t2d3ts4_rt",
      "t2d3ts4_lr",
      "t2d3ts4_pr",
      "t2d3ts6_id",
      "t2d3ts6_rt",
      "t2d3ts6_lr",
      "t2d3ts6_pr",
      "t2d3ts8_id",
      "firmid", "firmver", "evt", "clk"
    };

    /** enum of leafs **/
    enum EBits {
      e_t3d_2doldtrk,
      e_t3dtrk0_evtTime_delay,
      e_t3dtrk0_evtTimeValid_delay,
      e_t3dtrk0_charge,
      e_t3dtrk0_rho,
      e_t3dtrk0_phi0,
      e_t3dtrk0ts0_id,
      e_t3dtrk0ts0_lr,
      e_t3dtrk0ts0_rt,
      e_t3dtrk0ts1_id,
      e_t3dtrk0ts1_lr,
      e_t3dtrk0ts1_rt,
      e_t3dtrk0ts2_id,
      e_t3dtrk0ts2_lr,
      e_t3dtrk0ts2_rt,
      e_t3dtrk0ts3_id,
      e_t3dtrk0ts3_lr,
      e_t3dtrk0ts3_rt,
      e_t3dtrk1_evtTime_delay,
      e_t3dtrk1_evtTimeValid_delay,
      e_t3dtrk1_charge,
      e_t3dtrk1_rho,
      e_t3dtrk1_phi0,
      e_t3dtrk1ts0_id,
      e_t3dtrk1ts0_lr,
      e_t3dtrk1ts0_rt,
      e_t3dtrk1ts1_id,
      e_t3dtrk1ts1_lr,
      e_t3dtrk1ts1_rt,
      e_t3dtrk1ts2_id,
      e_t3dtrk1ts2_lr,
      e_t3dtrk1ts2_rt,
      e_t3dtrk1ts3_id,
      e_t3dtrk1ts3_lr,
      e_t3dtrk1ts3_rt,
      e_t3d_phase,
      e_t3d_validTS,
      e_t3d_2dfnf,
      e_t3dtrk0_z0_s,
      e_t3dtrk0_cot_s,
      e_t3dtrk0_zchisq,
      e_t3dtrk1_z0_s,
      e_t3dtrk1_cot_s,
      e_t3dtrk1_zchisq,
      e_t3dtrk2_z0_s,
      e_t3dtrk2_cot_s,
      e_t3dtrk2_zchisq,
      e_t3dtrk3_z0_s,
      e_t3dtrk3_cot_s,
      e_t3dtrk3_zchisq,
      e_t3dtrk_rd_req,
      e_tsf1_cc,
      e_tsf1ts0_id,
      e_tsf1ts0_rt,
      e_tsf1ts0_lr,
      e_tsf1ts0_pr,
      e_tsf1ts1_id,
      e_tsf1ts1_rt,
      e_tsf1ts1_lr,
      e_tsf1ts1_pr,
      e_tsf1ts2_id,
      e_tsf1ts2_rt,
      e_tsf1ts2_lr,
      e_tsf1ts2_pr,
      e_tsf1ts3_id,
      e_tsf1ts3_rt,
      e_tsf1ts3_lr,
      e_tsf1ts3_pr,
      e_tsf1ts4_id,
      e_tsf1ts4_rt,
      e_tsf1ts4_lr,
      e_tsf1ts4_pr,
      e_tsf1ts5_id,
      e_tsf1ts5_rt,
      e_tsf1ts5_lr,
      e_tsf1ts5_pr,
      e_tsf1ts6_id,
      e_tsf1ts6_rt,
      e_tsf1ts6_lr,
      e_tsf1ts6_pr,
      e_tsf1ts7_id,
      e_tsf1ts7_rt,
      e_tsf1ts7_lr,
      e_tsf1ts7_pr,
      e_tsf1ts8_id,
      e_tsf1ts8_rt,
      e_tsf1ts8_lr,
      e_tsf1ts8_pr,
      e_tsf1ts9_id,
      e_tsf1ts9_rt,
      e_tsf1ts9_lr,
      e_tsf1ts9_pr,
      e_tsf3_cc,
      e_tsf3ts0_id,
      e_tsf3ts0_rt,
      e_tsf3ts0_lr,
      e_tsf3ts0_pr,
      e_tsf3ts1_id,
      e_tsf3ts1_rt,
      e_tsf3ts1_lr,
      e_tsf3ts1_pr,
      e_tsf3ts2_id,
      e_tsf3ts2_rt,
      e_tsf3ts2_lr,
      e_tsf3ts2_pr,
      e_tsf3ts3_id,
      e_tsf3ts3_rt,
      e_tsf3ts3_lr,
      e_tsf3ts3_pr,
      e_tsf3ts4_id,
      e_tsf3ts4_rt,
      e_tsf3ts4_lr,
      e_tsf3ts4_pr,
      e_tsf3ts5_id,
      e_tsf3ts5_rt,
      e_tsf3ts5_lr,
      e_tsf3ts5_pr,
      e_tsf3ts6_id,
      e_tsf3ts6_rt,
      e_tsf3ts6_lr,
      e_tsf3ts6_pr,
      e_tsf3ts7_id,
      e_tsf3ts7_rt,
      e_tsf3ts7_lr,
      e_tsf3ts7_pr,
      e_tsf3ts8_id,
      e_tsf3ts8_rt,
      e_tsf3ts8_lr,
      e_tsf3ts8_pr,
      e_tsf3ts9_id,
      e_tsf3ts9_rt,
      e_tsf3ts9_lr,
      e_tsf3ts9_pr,
      e_tsf5_cc,
      e_tsf5ts0_id,
      e_tsf5ts0_rt,
      e_tsf5ts0_lr,
      e_tsf5ts0_pr,
      e_tsf5ts1_id,
      e_tsf5ts1_rt,
      e_tsf5ts1_lr,
      e_tsf5ts1_pr,
      e_tsf5ts2_id,
      e_tsf5ts2_rt,
      e_tsf5ts2_lr,
      e_tsf5ts2_pr,
      e_tsf5ts3_id,
      e_tsf5ts3_rt,
      e_tsf5ts3_lr,
      e_tsf5ts3_pr,
      e_tsf5ts4_id,
      e_tsf5ts4_rt,
      e_tsf5ts4_lr,
      e_tsf5ts4_pr,
      e_tsf5ts5_id,
      e_tsf5ts5_rt,
      e_tsf5ts5_lr,
      e_tsf5ts5_pr,
      e_tsf5ts6_id,
      e_tsf5ts6_rt,
      e_tsf5ts6_lr,
      e_tsf5ts6_pr,
      e_tsf5ts7_id,
      e_tsf5ts7_rt,
      e_tsf5ts7_lr,
      e_tsf5ts7_pr,
      e_tsf5ts8_id,
      e_tsf5ts8_rt,
      e_tsf5ts8_lr,
      e_tsf5ts8_pr,
      e_tsf5ts9_id,
      e_tsf5ts9_rt,
      e_tsf5ts9_lr,
      e_tsf5ts9_pr,
      e_tsf7_cc,
      e_tsf7ts0_id,
      e_tsf7ts0_rt,
      e_tsf7ts0_lr,
      e_tsf7ts0_pr,
      e_tsf7ts1_id,
      e_tsf7ts1_rt,
      e_tsf7ts1_lr,
      e_tsf7ts1_pr,
      e_tsf7ts2_id,
      e_tsf7ts2_rt,
      e_tsf7ts2_lr,
      e_tsf7ts2_pr,
      e_tsf7ts3_id,
      e_tsf7ts3_rt,
      e_tsf7ts3_lr,
      e_tsf7ts3_pr,
      e_tsf7ts4_id,
      e_tsf7ts4_rt,
      e_tsf7ts4_lr,
      e_tsf7ts4_pr,
      e_tsf7ts5_id,
      e_tsf7ts5_rt,
      e_tsf7ts5_lr,
      e_tsf7ts5_pr,
      e_tsf7ts6_id,
      e_tsf7ts6_rt,
      e_tsf7ts6_lr,
      e_tsf7ts6_pr,
      e_tsf7ts7_id,
      e_tsf7ts7_rt,
      e_tsf7ts7_lr,
      e_tsf7ts7_pr,
      e_tsf7ts8_id,
      e_tsf7ts8_rt,
      e_tsf7ts8_lr,
      e_tsf7ts8_pr,
      e_tsf7ts9_id,
      e_tsf7ts9_rt,
      e_tsf7ts9_lr,
      e_tsf7ts9_pr,
      e_t2d_cc,
      e_t2d_fnf,
      e_t2d0_charge,
      e_t2d0_rho_s,
      e_t2d0_phi,
      e_t2d0ts0_id,
      e_t2d0ts0_rt,
      e_t2d0ts0_lr,
      e_t2d0ts0_pr,
      e_t2d0ts2_id,
      e_t2d0ts2_rt,
      e_t2d0ts2_lr,
      e_t2d0ts2_pr,
      e_t2d0ts4_id,
      e_t2d0ts4_rt,
      e_t2d0ts4_lr,
      e_t2d0ts4_pr,
      e_t2d0ts6_id,
      e_t2d0ts6_rt,
      e_t2d0ts6_lr,
      e_t2d0ts6_pr,
      e_t2d0ts8_id,
      e_t2d0ts8_rt,
      e_t2d0ts8_lr,
      e_t2d0ts8_pr,
      e_t2d1_charge,
      e_t2d1_rho_s,
      e_t2d1_phi,
      e_t2d1ts0_id,
      e_t2d1ts0_rt,
      e_t2d1ts0_lr,
      e_t2d1ts0_pr,
      e_t2d1ts2_id,
      e_t2d1ts2_rt,
      e_t2d1ts2_lr,
      e_t2d1ts2_pr,
      e_t2d1ts4_id,
      e_t2d1ts4_rt,
      e_t2d1ts4_lr,
      e_t2d1ts4_pr,
      e_t2d1ts6_id,
      e_t2d1ts6_rt,
      e_t2d1ts6_lr,
      e_t2d1ts6_pr,
      e_t2d1ts8_id,
      e_t2d1ts8_rt,
      e_t2d1ts8_lr,
      e_t2d1ts8_pr,
      e_t2d2_charge,
      e_t2d2_rho_s,
      e_t2d2_phi,
      e_t2d2ts0_id,
      e_t2d2ts0_rt,
      e_t2d2ts0_lr,
      e_t2d2ts0_pr,
      e_t2d2ts2_id,
      e_t2d2ts2_rt,
      e_t2d2ts2_lr,
      e_t2d2ts2_pr,
      e_t2d2ts4_id,
      e_t2d2ts4_rt,
      e_t2d2ts4_lr,
      e_t2d2ts4_pr,
      e_t2d2ts6_id,
      e_t2d2ts6_rt,
      e_t2d2ts6_lr,
      e_t2d2ts6_pr,
      e_t2d2ts8_id,
      e_t2d2ts8_rt,
      e_t2d2ts8_lr,
      e_t2d2ts8_pr,
      e_t2d3_charge,
      e_t2d3_rho_s,
      e_t2d3_phi,
      e_t2d3ts0_id,
      e_t2d3ts0_rt,
      e_t2d3ts0_lr,
      e_t2d3ts0_pr,
      e_t2d3ts2_id,
      e_t2d3ts2_rt,
      e_t2d3ts2_lr,
      e_t2d3ts2_pr,
      e_t2d3ts4_id,
      e_t2d3ts4_rt,
      e_t2d3ts4_lr,
      e_t2d3ts4_pr,
      e_t2d3ts6_id,
      e_t2d3ts6_rt,
      e_t2d3ts6_lr,
      e_t2d3ts6_pr,
      e_t2d3ts8_id,
      e_firmid, e_firmver, e_evt, e_clk
    };

    /** bus bit map. (a downto a-b) **/
    const int BitMap[nLeafs][2] = {
      1714, 5, // t3d_2doldtrk (1714 downto 1709)
      1708, 8, // t3dtrk0_evtTime_delay (1708 downto 1700)
      1699, 0, // t3dtrk0_evtTimeValid_delay (1699 downto 1699)
      1698, 0, // t3dtrk0_charge (1698 downto 1698)
      1697, 10, // t3dtrk0_rho (1697 downto 1687)
      1686, 12, // t3dtrk0_phi0 (1686 downto 1674)
      1673, 7, // t3dtrk0ts0_id (1673 downto 1666)
      1665, 1, // t3dtrk0ts0_lr (1665 downto 1664)
      1663, 8, // t3dtrk0ts0_rt (1663 downto 1655)
      1654, 7, // t3dtrk0ts1_id (1654 downto 1647)
      1646, 1, // t3dtrk0ts1_lr (1646 downto 1645)
      1644, 8, // t3dtrk0ts1_rt (1644 downto 1636)
      1635, 8, // t3dtrk0ts2_id (1635 downto 1627)
      1626, 1, // t3dtrk0ts2_lr (1626 downto 1625)
      1624, 8, // t3dtrk0ts2_rt (1624 downto 1616)
      1615, 8, // t3dtrk0ts3_id (1615 downto 1607)
      1606, 1, // t3dtrk0ts3_lr (1606 downto 1605)
      1604, 8, // t3dtrk0ts3_rt (1604 downto 1596)
      1595, 8, // t3dtrk1_evtTime_delay (1595 downto 1587)
      1586, 0, // t3dtrk1_evtTimeValid_delay (1586 downto 1586)
      1585, 0, // t3dtrk1_charge (1585 downto 1585)
      1584, 10, // t3dtrk1_rho (1584 downto 1574)
      1573, 12, // t3dtrk1_phi0 (1573 downto 1561)
      1560, 7, // t3dtrk1ts0_id (1560 downto 1553)
      1552, 1, // t3dtrk1ts0_lr (1552 downto 1551)
      1550, 8, // t3dtrk1ts0_rt (1550 downto 1542)
      1541, 7, // t3dtrk1ts1_id (1541 downto 1534)
      1533, 1, // t3dtrk1ts1_lr (1533 downto 1532)
      1531, 8, // t3dtrk1ts1_rt (1531 downto 1523)
      1522, 8, // t3dtrk1ts2_id (1522 downto 1514)
      1513, 1, // t3dtrk1ts2_lr (1513 downto 1512)
      1511, 8, // t3dtrk1ts2_rt (1511 downto 1503)
      1502, 8, // t3dtrk1ts3_id (1502 downto 1494)
      1493, 1, // t3dtrk1ts3_lr (1493 downto 1492)
      1491, 8, // t3dtrk1ts3_rt (1491 downto 1483)
      1482, 1, // t3d_phase (1482 downto 1481)
      1480, 3, // t3d_validTS (1480 downto 1477)
      1476, 5, // t3d_2dfnf (1476 downto 1471)
      1470, 10, // t3dtrk0_z0_s (1470 downto 1460)
      1459, 10, // t3dtrk0_cot_s (1459 downto 1449)
      1448, 3, // t3dtrk0_zchisq (1448 downto 1445)
      1444, 10, // t3dtrk1_z0_s (1444 downto 1434)
      1433, 10, // t3dtrk1_cot_s (1433 downto 1423)
      1422, 3, // t3dtrk1_zchisq (1422 downto 1419)
      1418, 10, // t3dtrk2_z0_s (1418 downto 1408)
      1407, 10, // t3dtrk2_cot_s (1407 downto 1397)
      1396, 3, // t3dtrk2_zchisq (1396 downto 1393)
      1392, 10, // t3dtrk3_z0_s (1392 downto 1382)
      1381, 10, // t3dtrk3_cot_s (1381 downto 1371)
      1370, 3, // t3dtrk3_zchisq (1370 downto 1367)
      1366, 0, // t3dtrk_rd_req (1366 downto 1366)
      1365, 8, // tsf1_cc (1365 downto 1357)
      1356, 7, // tsf1ts0_id (1356 downto 1349)
      1348, 8, // tsf1ts0_rt (1348 downto 1340)
      1339, 1, // tsf1ts0_lr (1339 downto 1338)
      1337, 1, // tsf1ts0_pr (1337 downto 1336)
      1335, 7, // tsf1ts1_id (1335 downto 1328)
      1327, 8, // tsf1ts1_rt (1327 downto 1319)
      1318, 1, // tsf1ts1_lr (1318 downto 1317)
      1316, 1, // tsf1ts1_pr (1316 downto 1315)
      1314, 7, // tsf1ts2_id (1314 downto 1307)
      1306, 8, // tsf1ts2_rt (1306 downto 1298)
      1297, 1, // tsf1ts2_lr (1297 downto 1296)
      1295, 1, // tsf1ts2_pr (1295 downto 1294)
      1293, 7, // tsf1ts3_id (1293 downto 1286)
      1285, 8, // tsf1ts3_rt (1285 downto 1277)
      1276, 1, // tsf1ts3_lr (1276 downto 1275)
      1274, 1, // tsf1ts3_pr (1274 downto 1273)
      1272, 7, // tsf1ts4_id (1272 downto 1265)
      1264, 8, // tsf1ts4_rt (1264 downto 1256)
      1255, 1, // tsf1ts4_lr (1255 downto 1254)
      1253, 1, // tsf1ts4_pr (1253 downto 1252)
      1251, 7, // tsf1ts5_id (1251 downto 1244)
      1243, 8, // tsf1ts5_rt (1243 downto 1235)
      1234, 1, // tsf1ts5_lr (1234 downto 1233)
      1232, 1, // tsf1ts5_pr (1232 downto 1231)
      1230, 7, // tsf1ts6_id (1230 downto 1223)
      1222, 8, // tsf1ts6_rt (1222 downto 1214)
      1213, 1, // tsf1ts6_lr (1213 downto 1212)
      1211, 1, // tsf1ts6_pr (1211 downto 1210)
      1209, 7, // tsf1ts7_id (1209 downto 1202)
      1201, 8, // tsf1ts7_rt (1201 downto 1193)
      1192, 1, // tsf1ts7_lr (1192 downto 1191)
      1190, 1, // tsf1ts7_pr (1190 downto 1189)
      1188, 7, // tsf1ts8_id (1188 downto 1181)
      1180, 8, // tsf1ts8_rt (1180 downto 1172)
      1171, 1, // tsf1ts8_lr (1171 downto 1170)
      1169, 1, // tsf1ts8_pr (1169 downto 1168)
      1167, 7, // tsf1ts9_id (1167 downto 1160)
      1159, 8, // tsf1ts9_rt (1159 downto 1151)
      1150, 1, // tsf1ts9_lr (1150 downto 1149)
      1148, 1, // tsf1ts9_pr (1148 downto 1147)
      1146, 8, // tsf3_cc (1146 downto 1138)
      1137, 7, // tsf3ts0_id (1137 downto 1130)
      1129, 8, // tsf3ts0_rt (1129 downto 1121)
      1120, 1, // tsf3ts0_lr (1120 downto 1119)
      1118, 1, // tsf3ts0_pr (1118 downto 1117)
      1116, 7, // tsf3ts1_id (1116 downto 1109)
      1108, 8, // tsf3ts1_rt (1108 downto 1100)
      1099, 1, // tsf3ts1_lr (1099 downto 1098)
      1097, 1, // tsf3ts1_pr (1097 downto 1096)
      1095, 7, // tsf3ts2_id (1095 downto 1088)
      1087, 8, // tsf3ts2_rt (1087 downto 1079)
      1078, 1, // tsf3ts2_lr (1078 downto 1077)
      1076, 1, // tsf3ts2_pr (1076 downto 1075)
      1074, 7, // tsf3ts3_id (1074 downto 1067)
      1066, 8, // tsf3ts3_rt (1066 downto 1058)
      1057, 1, // tsf3ts3_lr (1057 downto 1056)
      1055, 1, // tsf3ts3_pr (1055 downto 1054)
      1053, 7, // tsf3ts4_id (1053 downto 1046)
      1045, 8, // tsf3ts4_rt (1045 downto 1037)
      1036, 1, // tsf3ts4_lr (1036 downto 1035)
      1034, 1, // tsf3ts4_pr (1034 downto 1033)
      1032, 7, // tsf3ts5_id (1032 downto 1025)
      1024, 8, // tsf3ts5_rt (1024 downto 1016)
      1015, 1, // tsf3ts5_lr (1015 downto 1014)
      1013, 1, // tsf3ts5_pr (1013 downto 1012)
      1011, 7, // tsf3ts6_id (1011 downto 1004)
      1003, 8, // tsf3ts6_rt (1003 downto 995)
      994, 1, // tsf3ts6_lr (994 downto 993)
      992, 1, // tsf3ts6_pr (992 downto 991)
      990, 7, // tsf3ts7_id (990 downto 983)
      982, 8, // tsf3ts7_rt (982 downto 974)
      973, 1, // tsf3ts7_lr (973 downto 972)
      971, 1, // tsf3ts7_pr (971 downto 970)
      969, 7, // tsf3ts8_id (969 downto 962)
      961, 8, // tsf3ts8_rt (961 downto 953)
      952, 1, // tsf3ts8_lr (952 downto 951)
      950, 1, // tsf3ts8_pr (950 downto 949)
      948, 7, // tsf3ts9_id (948 downto 941)
      940, 8, // tsf3ts9_rt (940 downto 932)
      931, 1, // tsf3ts9_lr (931 downto 930)
      929, 1, // tsf3ts9_pr (929 downto 928)
      927, 8, // tsf5_cc (927 downto 919)
      918, 7, // tsf5ts0_id (918 downto 911)
      910, 8, // tsf5ts0_rt (910 downto 902)
      901, 1, // tsf5ts0_lr (901 downto 900)
      899, 1, // tsf5ts0_pr (899 downto 898)
      897, 7, // tsf5ts1_id (897 downto 890)
      889, 8, // tsf5ts1_rt (889 downto 881)
      880, 1, // tsf5ts1_lr (880 downto 879)
      878, 1, // tsf5ts1_pr (878 downto 877)
      876, 7, // tsf5ts2_id (876 downto 869)
      868, 8, // tsf5ts2_rt (868 downto 860)
      859, 1, // tsf5ts2_lr (859 downto 858)
      857, 1, // tsf5ts2_pr (857 downto 856)
      855, 7, // tsf5ts3_id (855 downto 848)
      847, 8, // tsf5ts3_rt (847 downto 839)
      838, 1, // tsf5ts3_lr (838 downto 837)
      836, 1, // tsf5ts3_pr (836 downto 835)
      834, 7, // tsf5ts4_id (834 downto 827)
      826, 8, // tsf5ts4_rt (826 downto 818)
      817, 1, // tsf5ts4_lr (817 downto 816)
      815, 1, // tsf5ts4_pr (815 downto 814)
      813, 7, // tsf5ts5_id (813 downto 806)
      805, 8, // tsf5ts5_rt (805 downto 797)
      796, 1, // tsf5ts5_lr (796 downto 795)
      794, 1, // tsf5ts5_pr (794 downto 793)
      792, 7, // tsf5ts6_id (792 downto 785)
      784, 8, // tsf5ts6_rt (784 downto 776)
      775, 1, // tsf5ts6_lr (775 downto 774)
      773, 1, // tsf5ts6_pr (773 downto 772)
      771, 7, // tsf5ts7_id (771 downto 764)
      763, 8, // tsf5ts7_rt (763 downto 755)
      754, 1, // tsf5ts7_lr (754 downto 753)
      752, 1, // tsf5ts7_pr (752 downto 751)
      750, 7, // tsf5ts8_id (750 downto 743)
      742, 8, // tsf5ts8_rt (742 downto 734)
      733, 1, // tsf5ts8_lr (733 downto 732)
      731, 1, // tsf5ts8_pr (731 downto 730)
      729, 7, // tsf5ts9_id (729 downto 722)
      721, 8, // tsf5ts9_rt (721 downto 713)
      712, 1, // tsf5ts9_lr (712 downto 711)
      710, 1, // tsf5ts9_pr (710 downto 709)
      708, 8, // tsf7_cc (708 downto 700)
      699, 7, // tsf7ts0_id (699 downto 692)
      691, 8, // tsf7ts0_rt (691 downto 683)
      682, 1, // tsf7ts0_lr (682 downto 681)
      680, 1, // tsf7ts0_pr (680 downto 679)
      678, 7, // tsf7ts1_id (678 downto 671)
      670, 8, // tsf7ts1_rt (670 downto 662)
      661, 1, // tsf7ts1_lr (661 downto 660)
      659, 1, // tsf7ts1_pr (659 downto 658)
      657, 7, // tsf7ts2_id (657 downto 650)
      649, 8, // tsf7ts2_rt (649 downto 641)
      640, 1, // tsf7ts2_lr (640 downto 639)
      638, 1, // tsf7ts2_pr (638 downto 637)
      636, 7, // tsf7ts3_id (636 downto 629)
      628, 8, // tsf7ts3_rt (628 downto 620)
      619, 1, // tsf7ts3_lr (619 downto 618)
      617, 1, // tsf7ts3_pr (617 downto 616)
      615, 7, // tsf7ts4_id (615 downto 608)
      607, 8, // tsf7ts4_rt (607 downto 599)
      598, 1, // tsf7ts4_lr (598 downto 597)
      596, 1, // tsf7ts4_pr (596 downto 595)
      594, 7, // tsf7ts5_id (594 downto 587)
      586, 8, // tsf7ts5_rt (586 downto 578)
      577, 1, // tsf7ts5_lr (577 downto 576)
      575, 1, // tsf7ts5_pr (575 downto 574)
      573, 7, // tsf7ts6_id (573 downto 566)
      565, 8, // tsf7ts6_rt (565 downto 557)
      556, 1, // tsf7ts6_lr (556 downto 555)
      554, 1, // tsf7ts6_pr (554 downto 553)
      552, 7, // tsf7ts7_id (552 downto 545)
      544, 8, // tsf7ts7_rt (544 downto 536)
      535, 1, // tsf7ts7_lr (535 downto 534)
      533, 1, // tsf7ts7_pr (533 downto 532)
      531, 7, // tsf7ts8_id (531 downto 524)
      523, 8, // tsf7ts8_rt (523 downto 515)
      514, 1, // tsf7ts8_lr (514 downto 513)
      512, 1, // tsf7ts8_pr (512 downto 511)
      510, 7, // tsf7ts9_id (510 downto 503)
      502, 8, // tsf7ts9_rt (502 downto 494)
      493, 1, // tsf7ts9_lr (493 downto 492)
      491, 1, // tsf7ts9_pr (491 downto 490)
      489, 8, // t2d_cc (489 downto 481)
      480, 5, // t2d_fnf (480 downto 475)
      474, 1, // t2d0_charge (474 downto 473)
      472, 6, // t2d0_rho_s (472 downto 466)
      465, 6, // t2d0_phi (465 downto 459)
      458, 7, // t2d0ts0_id (458 downto 451)
      450, 8, // t2d0ts0_rt (450 downto 442)
      441, 1, // t2d0ts0_lr (441 downto 440)
      439, 1, // t2d0ts0_pr (439 downto 438)
      437, 7, // t2d0ts2_id (437 downto 430)
      429, 8, // t2d0ts2_rt (429 downto 421)
      420, 1, // t2d0ts2_lr (420 downto 419)
      418, 1, // t2d0ts2_pr (418 downto 417)
      416, 7, // t2d0ts4_id (416 downto 409)
      408, 8, // t2d0ts4_rt (408 downto 400)
      399, 1, // t2d0ts4_lr (399 downto 398)
      397, 1, // t2d0ts4_pr (397 downto 396)
      395, 7, // t2d0ts6_id (395 downto 388)
      387, 8, // t2d0ts6_rt (387 downto 379)
      378, 1, // t2d0ts6_lr (378 downto 377)
      376, 1, // t2d0ts6_pr (376 downto 375)
      374, 7, // t2d0ts8_id (374 downto 367)
      366, 8, // t2d0ts8_rt (366 downto 358)
      357, 1, // t2d0ts8_lr (357 downto 356)
      355, 1, // t2d0ts8_pr (355 downto 354)
      353, 1, // t2d1_charge (353 downto 352)
      351, 6, // t2d1_rho_s (351 downto 345)
      344, 6, // t2d1_phi (344 downto 338)
      337, 7, // t2d1ts0_id (337 downto 330)
      329, 8, // t2d1ts0_rt (329 downto 321)
      320, 1, // t2d1ts0_lr (320 downto 319)
      318, 1, // t2d1ts0_pr (318 downto 317)
      316, 7, // t2d1ts2_id (316 downto 309)
      308, 8, // t2d1ts2_rt (308 downto 300)
      299, 1, // t2d1ts2_lr (299 downto 298)
      297, 1, // t2d1ts2_pr (297 downto 296)
      295, 7, // t2d1ts4_id (295 downto 288)
      287, 8, // t2d1ts4_rt (287 downto 279)
      278, 1, // t2d1ts4_lr (278 downto 277)
      276, 1, // t2d1ts4_pr (276 downto 275)
      274, 7, // t2d1ts6_id (274 downto 267)
      266, 8, // t2d1ts6_rt (266 downto 258)
      257, 1, // t2d1ts6_lr (257 downto 256)
      255, 1, // t2d1ts6_pr (255 downto 254)
      253, 7, // t2d1ts8_id (253 downto 246)
      245, 8, // t2d1ts8_rt (245 downto 237)
      236, 1, // t2d1ts8_lr (236 downto 235)
      234, 1, // t2d1ts8_pr (234 downto 233)
      232, 1, // t2d2_charge (232 downto 231)
      230, 6, // t2d2_rho_s (230 downto 224)
      223, 6, // t2d2_phi (223 downto 217)
      216, 7, // t2d2ts0_id (216 downto 209)
      208, 8, // t2d2ts0_rt (208 downto 200)
      199, 1, // t2d2ts0_lr (199 downto 198)
      197, 1, // t2d2ts0_pr (197 downto 196)
      195, 7, // t2d2ts2_id (195 downto 188)
      187, 8, // t2d2ts2_rt (187 downto 179)
      178, 1, // t2d2ts2_lr (178 downto 177)
      176, 1, // t2d2ts2_pr (176 downto 175)
      174, 7, // t2d2ts4_id (174 downto 167)
      166, 8, // t2d2ts4_rt (166 downto 158)
      157, 1, // t2d2ts4_lr (157 downto 156)
      155, 1, // t2d2ts4_pr (155 downto 154)
      153, 7, // t2d2ts6_id (153 downto 146)
      145, 8, // t2d2ts6_rt (145 downto 137)
      136, 1, // t2d2ts6_lr (136 downto 135)
      134, 1, // t2d2ts6_pr (134 downto 133)
      132, 7, // t2d2ts8_id (132 downto 125)
      124, 8, // t2d2ts8_rt (124 downto 116)
      115, 1, // t2d2ts8_lr (115 downto 114)
      113, 1, // t2d2ts8_pr (113 downto 112)
      111, 1, // t2d3_charge (111 downto 110)
      109, 6, // t2d3_rho_s (109 downto 103)
      102, 6, // t2d3_phi (102 downto 96)
      95, 7, // t2d3ts0_id (95 downto 88)
      87, 8, // t2d3ts0_rt (87 downto 79)
      78, 1, // t2d3ts0_lr (78 downto 77)
      76, 1, // t2d3ts0_pr (76 downto 75)
      74, 7, // t2d3ts2_id (74 downto 67)
      66, 8, // t2d3ts2_rt (66 downto 58)
      57, 1, // t2d3ts2_lr (57 downto 56)
      55, 1, // t2d3ts2_pr (55 downto 54)
      53, 7, // t2d3ts4_id (53 downto 46)
      45, 8, // t2d3ts4_rt (45 downto 37)
      36, 1, // t2d3ts4_lr (36 downto 35)
      34, 1, // t2d3ts4_pr (34 downto 33)
      32, 7, // t2d3ts6_id (32 downto 25)
      24, 8, // t2d3ts6_rt (24 downto 16)
      15, 1, // t2d3ts6_lr (15 downto 14)
      13, 1, // t2d3ts6_pr (13 downto 12)
      11, 7 // t2d3ts8_id (11 downto 4)
    };

    void
    setLeafPointersArray(TRGCDCT3DUnpackerStore* store, int** bitArray)
    {
      bitArray[e_t3d_2doldtrk    ] = &(store->m_t3d_2doldtrk);
      bitArray[e_t3dtrk0_evtTime_delay    ] = &(store->m_t3dtrk0_evtTime_delay);
      bitArray[e_t3dtrk0_evtTimeValid_delay    ] = &(store->m_t3dtrk0_evtTimeValid_delay);
      bitArray[e_t3dtrk0_charge    ] = &(store->m_t3dtrk0_charge);
      bitArray[e_t3dtrk0_rho    ] = &(store->m_t3dtrk0_rho);
      bitArray[e_t3dtrk0_phi0    ] = &(store->m_t3dtrk0_phi0);
      bitArray[e_t3dtrk0ts0_id    ] = &(store->m_t3dtrk0ts0_id);
      bitArray[e_t3dtrk0ts0_lr    ] = &(store->m_t3dtrk0ts0_lr);
      bitArray[e_t3dtrk0ts0_rt    ] = &(store->m_t3dtrk0ts0_rt);
      bitArray[e_t3dtrk0ts1_id    ] = &(store->m_t3dtrk0ts1_id);
      bitArray[e_t3dtrk0ts1_lr    ] = &(store->m_t3dtrk0ts1_lr);
      bitArray[e_t3dtrk0ts1_rt    ] = &(store->m_t3dtrk0ts1_rt);
      bitArray[e_t3dtrk0ts2_id    ] = &(store->m_t3dtrk0ts2_id);
      bitArray[e_t3dtrk0ts2_lr    ] = &(store->m_t3dtrk0ts2_lr);
      bitArray[e_t3dtrk0ts2_rt    ] = &(store->m_t3dtrk0ts2_rt);
      bitArray[e_t3dtrk0ts3_id    ] = &(store->m_t3dtrk0ts3_id);
      bitArray[e_t3dtrk0ts3_lr    ] = &(store->m_t3dtrk0ts3_lr);
      bitArray[e_t3dtrk0ts3_rt    ] = &(store->m_t3dtrk0ts3_rt);
      bitArray[e_t3dtrk1_evtTime_delay    ] = &(store->m_t3dtrk1_evtTime_delay);
      bitArray[e_t3dtrk1_evtTimeValid_delay    ] = &(store->m_t3dtrk1_evtTimeValid_delay);
      bitArray[e_t3dtrk1_charge    ] = &(store->m_t3dtrk1_charge);
      bitArray[e_t3dtrk1_rho    ] = &(store->m_t3dtrk1_rho);
      bitArray[e_t3dtrk1_phi0    ] = &(store->m_t3dtrk1_phi0);
      bitArray[e_t3dtrk1ts0_id    ] = &(store->m_t3dtrk1ts0_id);
      bitArray[e_t3dtrk1ts0_lr    ] = &(store->m_t3dtrk1ts0_lr);
      bitArray[e_t3dtrk1ts0_rt    ] = &(store->m_t3dtrk1ts0_rt);
      bitArray[e_t3dtrk1ts1_id    ] = &(store->m_t3dtrk1ts1_id);
      bitArray[e_t3dtrk1ts1_lr    ] = &(store->m_t3dtrk1ts1_lr);
      bitArray[e_t3dtrk1ts1_rt    ] = &(store->m_t3dtrk1ts1_rt);
      bitArray[e_t3dtrk1ts2_id    ] = &(store->m_t3dtrk1ts2_id);
      bitArray[e_t3dtrk1ts2_lr    ] = &(store->m_t3dtrk1ts2_lr);
      bitArray[e_t3dtrk1ts2_rt    ] = &(store->m_t3dtrk1ts2_rt);
      bitArray[e_t3dtrk1ts3_id    ] = &(store->m_t3dtrk1ts3_id);
      bitArray[e_t3dtrk1ts3_lr    ] = &(store->m_t3dtrk1ts3_lr);
      bitArray[e_t3dtrk1ts3_rt    ] = &(store->m_t3dtrk1ts3_rt);
      bitArray[e_t3d_phase    ] = &(store->m_t3d_phase);
      bitArray[e_t3d_validTS    ] = &(store->m_t3d_validTS);
      bitArray[e_t3d_2dfnf    ] = &(store->m_t3d_2dfnf);
      bitArray[e_t3dtrk0_z0_s    ] = &(store->m_t3dtrk0_z0_s);
      bitArray[e_t3dtrk0_cot_s    ] = &(store->m_t3dtrk0_cot_s);
      bitArray[e_t3dtrk0_zchisq    ] = &(store->m_t3dtrk0_zchisq);
      bitArray[e_t3dtrk1_z0_s    ] = &(store->m_t3dtrk1_z0_s);
      bitArray[e_t3dtrk1_cot_s    ] = &(store->m_t3dtrk1_cot_s);
      bitArray[e_t3dtrk1_zchisq    ] = &(store->m_t3dtrk1_zchisq);
      bitArray[e_t3dtrk2_z0_s    ] = &(store->m_t3dtrk2_z0_s);
      bitArray[e_t3dtrk2_cot_s    ] = &(store->m_t3dtrk2_cot_s);
      bitArray[e_t3dtrk2_zchisq    ] = &(store->m_t3dtrk2_zchisq);
      bitArray[e_t3dtrk3_z0_s    ] = &(store->m_t3dtrk3_z0_s);
      bitArray[e_t3dtrk3_cot_s    ] = &(store->m_t3dtrk3_cot_s);
      bitArray[e_t3dtrk3_zchisq    ] = &(store->m_t3dtrk3_zchisq);
      bitArray[e_t3dtrk_rd_req    ] = &(store->m_t3dtrk_rd_req);
      bitArray[e_tsf1_cc    ] = &(store->m_tsf1_cc);
      bitArray[e_tsf1ts0_id    ] = &(store->m_tsf1ts0_id);
      bitArray[e_tsf1ts0_rt    ] = &(store->m_tsf1ts0_rt);
      bitArray[e_tsf1ts0_lr    ] = &(store->m_tsf1ts0_lr);
      bitArray[e_tsf1ts0_pr    ] = &(store->m_tsf1ts0_pr);
      bitArray[e_tsf1ts1_id    ] = &(store->m_tsf1ts1_id);
      bitArray[e_tsf1ts1_rt    ] = &(store->m_tsf1ts1_rt);
      bitArray[e_tsf1ts1_lr    ] = &(store->m_tsf1ts1_lr);
      bitArray[e_tsf1ts1_pr    ] = &(store->m_tsf1ts1_pr);
      bitArray[e_tsf1ts2_id    ] = &(store->m_tsf1ts2_id);
      bitArray[e_tsf1ts2_rt    ] = &(store->m_tsf1ts2_rt);
      bitArray[e_tsf1ts2_lr    ] = &(store->m_tsf1ts2_lr);
      bitArray[e_tsf1ts2_pr    ] = &(store->m_tsf1ts2_pr);
      bitArray[e_tsf1ts3_id    ] = &(store->m_tsf1ts3_id);
      bitArray[e_tsf1ts3_rt    ] = &(store->m_tsf1ts3_rt);
      bitArray[e_tsf1ts3_lr    ] = &(store->m_tsf1ts3_lr);
      bitArray[e_tsf1ts3_pr    ] = &(store->m_tsf1ts3_pr);
      bitArray[e_tsf1ts4_id    ] = &(store->m_tsf1ts4_id);
      bitArray[e_tsf1ts4_rt    ] = &(store->m_tsf1ts4_rt);
      bitArray[e_tsf1ts4_lr    ] = &(store->m_tsf1ts4_lr);
      bitArray[e_tsf1ts4_pr    ] = &(store->m_tsf1ts4_pr);
      bitArray[e_tsf1ts5_id    ] = &(store->m_tsf1ts5_id);
      bitArray[e_tsf1ts5_rt    ] = &(store->m_tsf1ts5_rt);
      bitArray[e_tsf1ts5_lr    ] = &(store->m_tsf1ts5_lr);
      bitArray[e_tsf1ts5_pr    ] = &(store->m_tsf1ts5_pr);
      bitArray[e_tsf1ts6_id    ] = &(store->m_tsf1ts6_id);
      bitArray[e_tsf1ts6_rt    ] = &(store->m_tsf1ts6_rt);
      bitArray[e_tsf1ts6_lr    ] = &(store->m_tsf1ts6_lr);
      bitArray[e_tsf1ts6_pr    ] = &(store->m_tsf1ts6_pr);
      bitArray[e_tsf1ts7_id    ] = &(store->m_tsf1ts7_id);
      bitArray[e_tsf1ts7_rt    ] = &(store->m_tsf1ts7_rt);
      bitArray[e_tsf1ts7_lr    ] = &(store->m_tsf1ts7_lr);
      bitArray[e_tsf1ts7_pr    ] = &(store->m_tsf1ts7_pr);
      bitArray[e_tsf1ts8_id    ] = &(store->m_tsf1ts8_id);
      bitArray[e_tsf1ts8_rt    ] = &(store->m_tsf1ts8_rt);
      bitArray[e_tsf1ts8_lr    ] = &(store->m_tsf1ts8_lr);
      bitArray[e_tsf1ts8_pr    ] = &(store->m_tsf1ts8_pr);
      bitArray[e_tsf1ts9_id    ] = &(store->m_tsf1ts9_id);
      bitArray[e_tsf1ts9_rt    ] = &(store->m_tsf1ts9_rt);
      bitArray[e_tsf1ts9_lr    ] = &(store->m_tsf1ts9_lr);
      bitArray[e_tsf1ts9_pr    ] = &(store->m_tsf1ts9_pr);
      bitArray[e_tsf3_cc    ] = &(store->m_tsf3_cc);
      bitArray[e_tsf3ts0_id    ] = &(store->m_tsf3ts0_id);
      bitArray[e_tsf3ts0_rt    ] = &(store->m_tsf3ts0_rt);
      bitArray[e_tsf3ts0_lr    ] = &(store->m_tsf3ts0_lr);
      bitArray[e_tsf3ts0_pr    ] = &(store->m_tsf3ts0_pr);
      bitArray[e_tsf3ts1_id    ] = &(store->m_tsf3ts1_id);
      bitArray[e_tsf3ts1_rt    ] = &(store->m_tsf3ts1_rt);
      bitArray[e_tsf3ts1_lr    ] = &(store->m_tsf3ts1_lr);
      bitArray[e_tsf3ts1_pr    ] = &(store->m_tsf3ts1_pr);
      bitArray[e_tsf3ts2_id    ] = &(store->m_tsf3ts2_id);
      bitArray[e_tsf3ts2_rt    ] = &(store->m_tsf3ts2_rt);
      bitArray[e_tsf3ts2_lr    ] = &(store->m_tsf3ts2_lr);
      bitArray[e_tsf3ts2_pr    ] = &(store->m_tsf3ts2_pr);
      bitArray[e_tsf3ts3_id    ] = &(store->m_tsf3ts3_id);
      bitArray[e_tsf3ts3_rt    ] = &(store->m_tsf3ts3_rt);
      bitArray[e_tsf3ts3_lr    ] = &(store->m_tsf3ts3_lr);
      bitArray[e_tsf3ts3_pr    ] = &(store->m_tsf3ts3_pr);
      bitArray[e_tsf3ts4_id    ] = &(store->m_tsf3ts4_id);
      bitArray[e_tsf3ts4_rt    ] = &(store->m_tsf3ts4_rt);
      bitArray[e_tsf3ts4_lr    ] = &(store->m_tsf3ts4_lr);
      bitArray[e_tsf3ts4_pr    ] = &(store->m_tsf3ts4_pr);
      bitArray[e_tsf3ts5_id    ] = &(store->m_tsf3ts5_id);
      bitArray[e_tsf3ts5_rt    ] = &(store->m_tsf3ts5_rt);
      bitArray[e_tsf3ts5_lr    ] = &(store->m_tsf3ts5_lr);
      bitArray[e_tsf3ts5_pr    ] = &(store->m_tsf3ts5_pr);
      bitArray[e_tsf3ts6_id    ] = &(store->m_tsf3ts6_id);
      bitArray[e_tsf3ts6_rt    ] = &(store->m_tsf3ts6_rt);
      bitArray[e_tsf3ts6_lr    ] = &(store->m_tsf3ts6_lr);
      bitArray[e_tsf3ts6_pr    ] = &(store->m_tsf3ts6_pr);
      bitArray[e_tsf3ts7_id    ] = &(store->m_tsf3ts7_id);
      bitArray[e_tsf3ts7_rt    ] = &(store->m_tsf3ts7_rt);
      bitArray[e_tsf3ts7_lr    ] = &(store->m_tsf3ts7_lr);
      bitArray[e_tsf3ts7_pr    ] = &(store->m_tsf3ts7_pr);
      bitArray[e_tsf3ts8_id    ] = &(store->m_tsf3ts8_id);
      bitArray[e_tsf3ts8_rt    ] = &(store->m_tsf3ts8_rt);
      bitArray[e_tsf3ts8_lr    ] = &(store->m_tsf3ts8_lr);
      bitArray[e_tsf3ts8_pr    ] = &(store->m_tsf3ts8_pr);
      bitArray[e_tsf3ts9_id    ] = &(store->m_tsf3ts9_id);
      bitArray[e_tsf3ts9_rt    ] = &(store->m_tsf3ts9_rt);
      bitArray[e_tsf3ts9_lr    ] = &(store->m_tsf3ts9_lr);
      bitArray[e_tsf3ts9_pr    ] = &(store->m_tsf3ts9_pr);
      bitArray[e_tsf5_cc    ] = &(store->m_tsf5_cc);
      bitArray[e_tsf5ts0_id    ] = &(store->m_tsf5ts0_id);
      bitArray[e_tsf5ts0_rt    ] = &(store->m_tsf5ts0_rt);
      bitArray[e_tsf5ts0_lr    ] = &(store->m_tsf5ts0_lr);
      bitArray[e_tsf5ts0_pr    ] = &(store->m_tsf5ts0_pr);
      bitArray[e_tsf5ts1_id    ] = &(store->m_tsf5ts1_id);
      bitArray[e_tsf5ts1_rt    ] = &(store->m_tsf5ts1_rt);
      bitArray[e_tsf5ts1_lr    ] = &(store->m_tsf5ts1_lr);
      bitArray[e_tsf5ts1_pr    ] = &(store->m_tsf5ts1_pr);
      bitArray[e_tsf5ts2_id    ] = &(store->m_tsf5ts2_id);
      bitArray[e_tsf5ts2_rt    ] = &(store->m_tsf5ts2_rt);
      bitArray[e_tsf5ts2_lr    ] = &(store->m_tsf5ts2_lr);
      bitArray[e_tsf5ts2_pr    ] = &(store->m_tsf5ts2_pr);
      bitArray[e_tsf5ts3_id    ] = &(store->m_tsf5ts3_id);
      bitArray[e_tsf5ts3_rt    ] = &(store->m_tsf5ts3_rt);
      bitArray[e_tsf5ts3_lr    ] = &(store->m_tsf5ts3_lr);
      bitArray[e_tsf5ts3_pr    ] = &(store->m_tsf5ts3_pr);
      bitArray[e_tsf5ts4_id    ] = &(store->m_tsf5ts4_id);
      bitArray[e_tsf5ts4_rt    ] = &(store->m_tsf5ts4_rt);
      bitArray[e_tsf5ts4_lr    ] = &(store->m_tsf5ts4_lr);
      bitArray[e_tsf5ts4_pr    ] = &(store->m_tsf5ts4_pr);
      bitArray[e_tsf5ts5_id    ] = &(store->m_tsf5ts5_id);
      bitArray[e_tsf5ts5_rt    ] = &(store->m_tsf5ts5_rt);
      bitArray[e_tsf5ts5_lr    ] = &(store->m_tsf5ts5_lr);
      bitArray[e_tsf5ts5_pr    ] = &(store->m_tsf5ts5_pr);
      bitArray[e_tsf5ts6_id    ] = &(store->m_tsf5ts6_id);
      bitArray[e_tsf5ts6_rt    ] = &(store->m_tsf5ts6_rt);
      bitArray[e_tsf5ts6_lr    ] = &(store->m_tsf5ts6_lr);
      bitArray[e_tsf5ts6_pr    ] = &(store->m_tsf5ts6_pr);
      bitArray[e_tsf5ts7_id    ] = &(store->m_tsf5ts7_id);
      bitArray[e_tsf5ts7_rt    ] = &(store->m_tsf5ts7_rt);
      bitArray[e_tsf5ts7_lr    ] = &(store->m_tsf5ts7_lr);
      bitArray[e_tsf5ts7_pr    ] = &(store->m_tsf5ts7_pr);
      bitArray[e_tsf5ts8_id    ] = &(store->m_tsf5ts8_id);
      bitArray[e_tsf5ts8_rt    ] = &(store->m_tsf5ts8_rt);
      bitArray[e_tsf5ts8_lr    ] = &(store->m_tsf5ts8_lr);
      bitArray[e_tsf5ts8_pr    ] = &(store->m_tsf5ts8_pr);
      bitArray[e_tsf5ts9_id    ] = &(store->m_tsf5ts9_id);
      bitArray[e_tsf5ts9_rt    ] = &(store->m_tsf5ts9_rt);
      bitArray[e_tsf5ts9_lr    ] = &(store->m_tsf5ts9_lr);
      bitArray[e_tsf5ts9_pr    ] = &(store->m_tsf5ts9_pr);
      bitArray[e_tsf7_cc    ] = &(store->m_tsf7_cc);
      bitArray[e_tsf7ts0_id    ] = &(store->m_tsf7ts0_id);
      bitArray[e_tsf7ts0_rt    ] = &(store->m_tsf7ts0_rt);
      bitArray[e_tsf7ts0_lr    ] = &(store->m_tsf7ts0_lr);
      bitArray[e_tsf7ts0_pr    ] = &(store->m_tsf7ts0_pr);
      bitArray[e_tsf7ts1_id    ] = &(store->m_tsf7ts1_id);
      bitArray[e_tsf7ts1_rt    ] = &(store->m_tsf7ts1_rt);
      bitArray[e_tsf7ts1_lr    ] = &(store->m_tsf7ts1_lr);
      bitArray[e_tsf7ts1_pr    ] = &(store->m_tsf7ts1_pr);
      bitArray[e_tsf7ts2_id    ] = &(store->m_tsf7ts2_id);
      bitArray[e_tsf7ts2_rt    ] = &(store->m_tsf7ts2_rt);
      bitArray[e_tsf7ts2_lr    ] = &(store->m_tsf7ts2_lr);
      bitArray[e_tsf7ts2_pr    ] = &(store->m_tsf7ts2_pr);
      bitArray[e_tsf7ts3_id    ] = &(store->m_tsf7ts3_id);
      bitArray[e_tsf7ts3_rt    ] = &(store->m_tsf7ts3_rt);
      bitArray[e_tsf7ts3_lr    ] = &(store->m_tsf7ts3_lr);
      bitArray[e_tsf7ts3_pr    ] = &(store->m_tsf7ts3_pr);
      bitArray[e_tsf7ts4_id    ] = &(store->m_tsf7ts4_id);
      bitArray[e_tsf7ts4_rt    ] = &(store->m_tsf7ts4_rt);
      bitArray[e_tsf7ts4_lr    ] = &(store->m_tsf7ts4_lr);
      bitArray[e_tsf7ts4_pr    ] = &(store->m_tsf7ts4_pr);
      bitArray[e_tsf7ts5_id    ] = &(store->m_tsf7ts5_id);
      bitArray[e_tsf7ts5_rt    ] = &(store->m_tsf7ts5_rt);
      bitArray[e_tsf7ts5_lr    ] = &(store->m_tsf7ts5_lr);
      bitArray[e_tsf7ts5_pr    ] = &(store->m_tsf7ts5_pr);
      bitArray[e_tsf7ts6_id    ] = &(store->m_tsf7ts6_id);
      bitArray[e_tsf7ts6_rt    ] = &(store->m_tsf7ts6_rt);
      bitArray[e_tsf7ts6_lr    ] = &(store->m_tsf7ts6_lr);
      bitArray[e_tsf7ts6_pr    ] = &(store->m_tsf7ts6_pr);
      bitArray[e_tsf7ts7_id    ] = &(store->m_tsf7ts7_id);
      bitArray[e_tsf7ts7_rt    ] = &(store->m_tsf7ts7_rt);
      bitArray[e_tsf7ts7_lr    ] = &(store->m_tsf7ts7_lr);
      bitArray[e_tsf7ts7_pr    ] = &(store->m_tsf7ts7_pr);
      bitArray[e_tsf7ts8_id    ] = &(store->m_tsf7ts8_id);
      bitArray[e_tsf7ts8_rt    ] = &(store->m_tsf7ts8_rt);
      bitArray[e_tsf7ts8_lr    ] = &(store->m_tsf7ts8_lr);
      bitArray[e_tsf7ts8_pr    ] = &(store->m_tsf7ts8_pr);
      bitArray[e_tsf7ts9_id    ] = &(store->m_tsf7ts9_id);
      bitArray[e_tsf7ts9_rt    ] = &(store->m_tsf7ts9_rt);
      bitArray[e_tsf7ts9_lr    ] = &(store->m_tsf7ts9_lr);
      bitArray[e_tsf7ts9_pr    ] = &(store->m_tsf7ts9_pr);
      bitArray[e_t2d_cc    ] = &(store->m_t2d_cc);
      bitArray[e_t2d_fnf    ] = &(store->m_t2d_fnf);
      bitArray[e_t2d0_charge    ] = &(store->m_t2d0_charge);
      bitArray[e_t2d0_rho_s    ] = &(store->m_t2d0_rho_s);
      bitArray[e_t2d0_phi    ] = &(store->m_t2d0_phi);
      bitArray[e_t2d0ts0_id    ] = &(store->m_t2d0ts0_id);
      bitArray[e_t2d0ts0_rt    ] = &(store->m_t2d0ts0_rt);
      bitArray[e_t2d0ts0_lr    ] = &(store->m_t2d0ts0_lr);
      bitArray[e_t2d0ts0_pr    ] = &(store->m_t2d0ts0_pr);
      bitArray[e_t2d0ts2_id    ] = &(store->m_t2d0ts2_id);
      bitArray[e_t2d0ts2_rt    ] = &(store->m_t2d0ts2_rt);
      bitArray[e_t2d0ts2_lr    ] = &(store->m_t2d0ts2_lr);
      bitArray[e_t2d0ts2_pr    ] = &(store->m_t2d0ts2_pr);
      bitArray[e_t2d0ts4_id    ] = &(store->m_t2d0ts4_id);
      bitArray[e_t2d0ts4_rt    ] = &(store->m_t2d0ts4_rt);
      bitArray[e_t2d0ts4_lr    ] = &(store->m_t2d0ts4_lr);
      bitArray[e_t2d0ts4_pr    ] = &(store->m_t2d0ts4_pr);
      bitArray[e_t2d0ts6_id    ] = &(store->m_t2d0ts6_id);
      bitArray[e_t2d0ts6_rt    ] = &(store->m_t2d0ts6_rt);
      bitArray[e_t2d0ts6_lr    ] = &(store->m_t2d0ts6_lr);
      bitArray[e_t2d0ts6_pr    ] = &(store->m_t2d0ts6_pr);
      bitArray[e_t2d0ts8_id    ] = &(store->m_t2d0ts8_id);
      bitArray[e_t2d0ts8_rt    ] = &(store->m_t2d0ts8_rt);
      bitArray[e_t2d0ts8_lr    ] = &(store->m_t2d0ts8_lr);
      bitArray[e_t2d0ts8_pr    ] = &(store->m_t2d0ts8_pr);
      bitArray[e_t2d1_charge    ] = &(store->m_t2d1_charge);
      bitArray[e_t2d1_rho_s    ] = &(store->m_t2d1_rho_s);
      bitArray[e_t2d1_phi    ] = &(store->m_t2d1_phi);
      bitArray[e_t2d1ts0_id    ] = &(store->m_t2d1ts0_id);
      bitArray[e_t2d1ts0_rt    ] = &(store->m_t2d1ts0_rt);
      bitArray[e_t2d1ts0_lr    ] = &(store->m_t2d1ts0_lr);
      bitArray[e_t2d1ts0_pr    ] = &(store->m_t2d1ts0_pr);
      bitArray[e_t2d1ts2_id    ] = &(store->m_t2d1ts2_id);
      bitArray[e_t2d1ts2_rt    ] = &(store->m_t2d1ts2_rt);
      bitArray[e_t2d1ts2_lr    ] = &(store->m_t2d1ts2_lr);
      bitArray[e_t2d1ts2_pr    ] = &(store->m_t2d1ts2_pr);
      bitArray[e_t2d1ts4_id    ] = &(store->m_t2d1ts4_id);
      bitArray[e_t2d1ts4_rt    ] = &(store->m_t2d1ts4_rt);
      bitArray[e_t2d1ts4_lr    ] = &(store->m_t2d1ts4_lr);
      bitArray[e_t2d1ts4_pr    ] = &(store->m_t2d1ts4_pr);
      bitArray[e_t2d1ts6_id    ] = &(store->m_t2d1ts6_id);
      bitArray[e_t2d1ts6_rt    ] = &(store->m_t2d1ts6_rt);
      bitArray[e_t2d1ts6_lr    ] = &(store->m_t2d1ts6_lr);
      bitArray[e_t2d1ts6_pr    ] = &(store->m_t2d1ts6_pr);
      bitArray[e_t2d1ts8_id    ] = &(store->m_t2d1ts8_id);
      bitArray[e_t2d1ts8_rt    ] = &(store->m_t2d1ts8_rt);
      bitArray[e_t2d1ts8_lr    ] = &(store->m_t2d1ts8_lr);
      bitArray[e_t2d1ts8_pr    ] = &(store->m_t2d1ts8_pr);
      bitArray[e_t2d2_charge    ] = &(store->m_t2d2_charge);
      bitArray[e_t2d2_rho_s    ] = &(store->m_t2d2_rho_s);
      bitArray[e_t2d2_phi    ] = &(store->m_t2d2_phi);
      bitArray[e_t2d2ts0_id    ] = &(store->m_t2d2ts0_id);
      bitArray[e_t2d2ts0_rt    ] = &(store->m_t2d2ts0_rt);
      bitArray[e_t2d2ts0_lr    ] = &(store->m_t2d2ts0_lr);
      bitArray[e_t2d2ts0_pr    ] = &(store->m_t2d2ts0_pr);
      bitArray[e_t2d2ts2_id    ] = &(store->m_t2d2ts2_id);
      bitArray[e_t2d2ts2_rt    ] = &(store->m_t2d2ts2_rt);
      bitArray[e_t2d2ts2_lr    ] = &(store->m_t2d2ts2_lr);
      bitArray[e_t2d2ts2_pr    ] = &(store->m_t2d2ts2_pr);
      bitArray[e_t2d2ts4_id    ] = &(store->m_t2d2ts4_id);
      bitArray[e_t2d2ts4_rt    ] = &(store->m_t2d2ts4_rt);
      bitArray[e_t2d2ts4_lr    ] = &(store->m_t2d2ts4_lr);
      bitArray[e_t2d2ts4_pr    ] = &(store->m_t2d2ts4_pr);
      bitArray[e_t2d2ts6_id    ] = &(store->m_t2d2ts6_id);
      bitArray[e_t2d2ts6_rt    ] = &(store->m_t2d2ts6_rt);
      bitArray[e_t2d2ts6_lr    ] = &(store->m_t2d2ts6_lr);
      bitArray[e_t2d2ts6_pr    ] = &(store->m_t2d2ts6_pr);
      bitArray[e_t2d2ts8_id    ] = &(store->m_t2d2ts8_id);
      bitArray[e_t2d2ts8_rt    ] = &(store->m_t2d2ts8_rt);
      bitArray[e_t2d2ts8_lr    ] = &(store->m_t2d2ts8_lr);
      bitArray[e_t2d2ts8_pr    ] = &(store->m_t2d2ts8_pr);
      bitArray[e_t2d3_charge    ] = &(store->m_t2d3_charge);
      bitArray[e_t2d3_rho_s    ] = &(store->m_t2d3_rho_s);
      bitArray[e_t2d3_phi    ] = &(store->m_t2d3_phi);
      bitArray[e_t2d3ts0_id    ] = &(store->m_t2d3ts0_id);
      bitArray[e_t2d3ts0_rt    ] = &(store->m_t2d3ts0_rt);
      bitArray[e_t2d3ts0_lr    ] = &(store->m_t2d3ts0_lr);
      bitArray[e_t2d3ts0_pr    ] = &(store->m_t2d3ts0_pr);
      bitArray[e_t2d3ts2_id    ] = &(store->m_t2d3ts2_id);
      bitArray[e_t2d3ts2_rt    ] = &(store->m_t2d3ts2_rt);
      bitArray[e_t2d3ts2_lr    ] = &(store->m_t2d3ts2_lr);
      bitArray[e_t2d3ts2_pr    ] = &(store->m_t2d3ts2_pr);
      bitArray[e_t2d3ts4_id    ] = &(store->m_t2d3ts4_id);
      bitArray[e_t2d3ts4_rt    ] = &(store->m_t2d3ts4_rt);
      bitArray[e_t2d3ts4_lr    ] = &(store->m_t2d3ts4_lr);
      bitArray[e_t2d3ts4_pr    ] = &(store->m_t2d3ts4_pr);
      bitArray[e_t2d3ts6_id    ] = &(store->m_t2d3ts6_id);
      bitArray[e_t2d3ts6_rt    ] = &(store->m_t2d3ts6_rt);
      bitArray[e_t2d3ts6_lr    ] = &(store->m_t2d3ts6_lr);
      bitArray[e_t2d3ts6_pr    ] = &(store->m_t2d3ts6_pr);
      bitArray[e_t2d3ts8_id    ] = &(store->m_t2d3ts8_id);
      bitArray[e_evt     ] = &(store->m_evt);
      bitArray[e_clk     ] = &(store->m_clk);
      bitArray[e_firmid  ] = &(store->m_firmid);
      bitArray[e_firmver ] = &(store->m_firmver);
    }

    /*! A module of TRGCDCT3DUnpacker */
    class TRGCDCT3DUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGCDCT3DUnpackerModule();

      /** Destructor */
      virtual ~TRGCDCT3DUnpackerModule();

      /** Initilizes TRGCDCT3DUnpackerUnpackerModule.*/
      virtual void initialize();

      /** Called event by event.*/
      virtual void event();

      /** Called when processing ended.*/
      virtual void terminate();

      /** Called when new run started.*/
      virtual void beginRun();

      /** Called when run ended*/
      virtual void endRun();

      /** returns version of TRGCDCT3DUnpackerModule.*/
      std::string version() const;

      /** Unpacker main function.*/
      virtual void fillTreeTRGCDCT3DUnpacker(int* buf, int evt);

    private:

      /** Output datastore for unpacker **/
      StoreArray<TRGCDCT3DUnpackerStore> m_store;

    };
  }
} // end namespace Belle2
#endif // TRGCDCT3DUNPACKER_H
