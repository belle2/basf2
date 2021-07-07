/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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

#include <string>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/cdc/dataobjects/TRGCDCT3DUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

namespace Belle2 {

  namespace TRGCDCT3DUNPACKERSPACE {

    /** num of leafs in data_b2l, 2k **/
    const int nLeafs = 313;
    /** num of leafs in data_b2l, for 2D fitter **/
    const int nLeafs_2dfitter = 141;
    /** num of leafs in data_b2l, 2.6k, 15 TS version **/
    const int nLeafs_2624 = 313 + 4 * 5 * 4;
    /** num of leafs for others **/
    const int nLeafsExtra = 4;
    /** num of clk time window **/
    const int nClks = 48 ;
    /** num of b2l bits, 2k **/
    const int nBits_2k = 2048;
    /** num of b2l bits, 2.6k, 15 TS version **/
    const int nBits_2624 = 2624;
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
      "t2d3ts8_rt",
      "t2d3ts8_lr",
      "t2d3ts8_pr",
      "etf_thresh",
      "etf_cc",
      "etf_t0",
      "etf_valid",
      "firmid", "firmver", "evt", "clk"
    };
    /** leaf names for 2624 bits, 15 TS version **/
    const char* LeafNames_2624[nLeafs_2624 + nLeafsExtra] = {
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
      "tsf1ts10_id",
      "tsf1ts10_rt",
      "tsf1ts10_lr",
      "tsf1ts10_pr",
      "tsf1ts11_id",
      "tsf1ts11_rt",
      "tsf1ts11_lr",
      "tsf1ts11_pr",
      "tsf1ts12_id",
      "tsf1ts12_rt",
      "tsf1ts12_lr",
      "tsf1ts12_pr",
      "tsf1ts13_id",
      "tsf1ts13_rt",
      "tsf1ts13_lr",
      "tsf1ts13_pr",
      "tsf1ts14_id",
      "tsf1ts14_rt",
      "tsf1ts14_lr",
      "tsf1ts14_pr",
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
      "tsf3ts10_id",
      "tsf3ts10_rt",
      "tsf3ts10_lr",
      "tsf3ts10_pr",
      "tsf3ts11_id",
      "tsf3ts11_rt",
      "tsf3ts11_lr",
      "tsf3ts11_pr",
      "tsf3ts12_id",
      "tsf3ts12_rt",
      "tsf3ts12_lr",
      "tsf3ts12_pr",
      "tsf3ts13_id",
      "tsf3ts13_rt",
      "tsf3ts13_lr",
      "tsf3ts13_pr",
      "tsf3ts14_id",
      "tsf3ts14_rt",
      "tsf3ts14_lr",
      "tsf3ts14_pr",
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
      "tsf5ts10_id",
      "tsf5ts10_rt",
      "tsf5ts10_lr",
      "tsf5ts10_pr",
      "tsf5ts11_id",
      "tsf5ts11_rt",
      "tsf5ts11_lr",
      "tsf5ts11_pr",
      "tsf5ts12_id",
      "tsf5ts12_rt",
      "tsf5ts12_lr",
      "tsf5ts12_pr",
      "tsf5ts13_id",
      "tsf5ts13_rt",
      "tsf5ts13_lr",
      "tsf5ts13_pr",
      "tsf5ts14_id",
      "tsf5ts14_rt",
      "tsf5ts14_lr",
      "tsf5ts14_pr",
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
      "tsf7ts10_id",
      "tsf7ts10_rt",
      "tsf7ts10_lr",
      "tsf7ts10_pr",
      "tsf7ts11_id",
      "tsf7ts11_rt",
      "tsf7ts11_lr",
      "tsf7ts11_pr",
      "tsf7ts12_id",
      "tsf7ts12_rt",
      "tsf7ts12_lr",
      "tsf7ts12_pr",
      "tsf7ts13_id",
      "tsf7ts13_rt",
      "tsf7ts13_lr",
      "tsf7ts13_pr",
      "tsf7ts14_id",
      "tsf7ts14_rt",
      "tsf7ts14_lr",
      "tsf7ts14_pr",
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
      "t2d3ts8_rt",
      "t2d3ts8_lr",
      "t2d3ts8_pr",
      "etf_thresh",
      "etf_cc",
      "etf_t0",
      "etf_valid",
      "firmid", "firmver", "evt", "clk"
    };
    /** leaf names for 2D fitter**/
    const char* LeafNames_2dfitter[nLeafs_2dfitter + nLeafsExtra] = {
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
      "t2dfittertrk0_rho",
      "t2dfittertrk0_phi0",
      "t2dfittertrk1_rho",
      "t2dfittertrk1_phi0",
      "t2dfittertrk2_rho",
      "t2dfittertrk2_phi0",
      "t2dfittertrk3_rho",
      "t2dfittertrk3_phi0",
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
      "t2d3ts8_rt",
      "t2d3ts8_lr",
      "t2d3ts8_pr",
      "etf_thresh",
      "etf_cc",
      "etf_t0",
      "etf_valid",
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
      e_t2d3ts8_rt,
      e_t2d3ts8_lr,
      e_t2d3ts8_pr,
      e_etf_thresh,
      e_etf_cc,
      e_etf_t0,
      e_etf_valid,
      e_firmid, e_firmver, e_evt, e_clk
    };

    /** enum of leafs for 2624 bits, 15 TS version **/
    enum EBits_2624 {
      e1_t3d_2doldtrk,
      e1_t3dtrk0_evtTime_delay,
      e1_t3dtrk0_evtTimeValid_delay,
      e1_t3dtrk0_charge,
      e1_t3dtrk0_rho,
      e1_t3dtrk0_phi0,
      e1_t3dtrk0ts0_id,
      e1_t3dtrk0ts0_lr,
      e1_t3dtrk0ts0_rt,
      e1_t3dtrk0ts1_id,
      e1_t3dtrk0ts1_lr,
      e1_t3dtrk0ts1_rt,
      e1_t3dtrk0ts2_id,
      e1_t3dtrk0ts2_lr,
      e1_t3dtrk0ts2_rt,
      e1_t3dtrk0ts3_id,
      e1_t3dtrk0ts3_lr,
      e1_t3dtrk0ts3_rt,
      e1_t3dtrk1_evtTime_delay,
      e1_t3dtrk1_evtTimeValid_delay,
      e1_t3dtrk1_charge,
      e1_t3dtrk1_rho,
      e1_t3dtrk1_phi0,
      e1_t3dtrk1ts0_id,
      e1_t3dtrk1ts0_lr,
      e1_t3dtrk1ts0_rt,
      e1_t3dtrk1ts1_id,
      e1_t3dtrk1ts1_lr,
      e1_t3dtrk1ts1_rt,
      e1_t3dtrk1ts2_id,
      e1_t3dtrk1ts2_lr,
      e1_t3dtrk1ts2_rt,
      e1_t3dtrk1ts3_id,
      e1_t3dtrk1ts3_lr,
      e1_t3dtrk1ts3_rt,
      e1_t3d_phase,
      e1_t3d_validTS,
      e1_t3d_2dfnf,
      e1_t3dtrk0_z0_s,
      e1_t3dtrk0_cot_s,
      e1_t3dtrk0_zchisq,
      e1_t3dtrk1_z0_s,
      e1_t3dtrk1_cot_s,
      e1_t3dtrk1_zchisq,
      e1_t3dtrk2_z0_s,
      e1_t3dtrk2_cot_s,
      e1_t3dtrk2_zchisq,
      e1_t3dtrk3_z0_s,
      e1_t3dtrk3_cot_s,
      e1_t3dtrk3_zchisq,
      e1_t3dtrk_rd_req,
      e1_tsf1_cc,
      e1_tsf1ts0_id,
      e1_tsf1ts0_rt,
      e1_tsf1ts0_lr,
      e1_tsf1ts0_pr,
      e1_tsf1ts1_id,
      e1_tsf1ts1_rt,
      e1_tsf1ts1_lr,
      e1_tsf1ts1_pr,
      e1_tsf1ts2_id,
      e1_tsf1ts2_rt,
      e1_tsf1ts2_lr,
      e1_tsf1ts2_pr,
      e1_tsf1ts3_id,
      e1_tsf1ts3_rt,
      e1_tsf1ts3_lr,
      e1_tsf1ts3_pr,
      e1_tsf1ts4_id,
      e1_tsf1ts4_rt,
      e1_tsf1ts4_lr,
      e1_tsf1ts4_pr,
      e1_tsf1ts5_id,
      e1_tsf1ts5_rt,
      e1_tsf1ts5_lr,
      e1_tsf1ts5_pr,
      e1_tsf1ts6_id,
      e1_tsf1ts6_rt,
      e1_tsf1ts6_lr,
      e1_tsf1ts6_pr,
      e1_tsf1ts7_id,
      e1_tsf1ts7_rt,
      e1_tsf1ts7_lr,
      e1_tsf1ts7_pr,
      e1_tsf1ts8_id,
      e1_tsf1ts8_rt,
      e1_tsf1ts8_lr,
      e1_tsf1ts8_pr,
      e1_tsf1ts9_id,
      e1_tsf1ts9_rt,
      e1_tsf1ts9_lr,
      e1_tsf1ts9_pr,
      e1_tsf1ts10_id,
      e1_tsf1ts10_rt,
      e1_tsf1ts10_lr,
      e1_tsf1ts10_pr,
      e1_tsf1ts11_id,
      e1_tsf1ts11_rt,
      e1_tsf1ts11_lr,
      e1_tsf1ts11_pr,
      e1_tsf1ts12_id,
      e1_tsf1ts12_rt,
      e1_tsf1ts12_lr,
      e1_tsf1ts12_pr,
      e1_tsf1ts13_id,
      e1_tsf1ts13_rt,
      e1_tsf1ts13_lr,
      e1_tsf1ts13_pr,
      e1_tsf1ts14_id,
      e1_tsf1ts14_rt,
      e1_tsf1ts14_lr,
      e1_tsf1ts14_pr,
      e1_tsf3_cc,
      e1_tsf3ts0_id,
      e1_tsf3ts0_rt,
      e1_tsf3ts0_lr,
      e1_tsf3ts0_pr,
      e1_tsf3ts1_id,
      e1_tsf3ts1_rt,
      e1_tsf3ts1_lr,
      e1_tsf3ts1_pr,
      e1_tsf3ts2_id,
      e1_tsf3ts2_rt,
      e1_tsf3ts2_lr,
      e1_tsf3ts2_pr,
      e1_tsf3ts3_id,
      e1_tsf3ts3_rt,
      e1_tsf3ts3_lr,
      e1_tsf3ts3_pr,
      e1_tsf3ts4_id,
      e1_tsf3ts4_rt,
      e1_tsf3ts4_lr,
      e1_tsf3ts4_pr,
      e1_tsf3ts5_id,
      e1_tsf3ts5_rt,
      e1_tsf3ts5_lr,
      e1_tsf3ts5_pr,
      e1_tsf3ts6_id,
      e1_tsf3ts6_rt,
      e1_tsf3ts6_lr,
      e1_tsf3ts6_pr,
      e1_tsf3ts7_id,
      e1_tsf3ts7_rt,
      e1_tsf3ts7_lr,
      e1_tsf3ts7_pr,
      e1_tsf3ts8_id,
      e1_tsf3ts8_rt,
      e1_tsf3ts8_lr,
      e1_tsf3ts8_pr,
      e1_tsf3ts9_id,
      e1_tsf3ts9_rt,
      e1_tsf3ts9_lr,
      e1_tsf3ts9_pr,
      e1_tsf3ts10_id,
      e1_tsf3ts10_rt,
      e1_tsf3ts10_lr,
      e1_tsf3ts10_pr,
      e1_tsf3ts11_id,
      e1_tsf3ts11_rt,
      e1_tsf3ts11_lr,
      e1_tsf3ts11_pr,
      e1_tsf3ts12_id,
      e1_tsf3ts12_rt,
      e1_tsf3ts12_lr,
      e1_tsf3ts12_pr,
      e1_tsf3ts13_id,
      e1_tsf3ts13_rt,
      e1_tsf3ts13_lr,
      e1_tsf3ts13_pr,
      e1_tsf3ts14_id,
      e1_tsf3ts14_rt,
      e1_tsf3ts14_lr,
      e1_tsf3ts14_pr,
      e1_tsf5_cc,
      e1_tsf5ts0_id,
      e1_tsf5ts0_rt,
      e1_tsf5ts0_lr,
      e1_tsf5ts0_pr,
      e1_tsf5ts1_id,
      e1_tsf5ts1_rt,
      e1_tsf5ts1_lr,
      e1_tsf5ts1_pr,
      e1_tsf5ts2_id,
      e1_tsf5ts2_rt,
      e1_tsf5ts2_lr,
      e1_tsf5ts2_pr,
      e1_tsf5ts3_id,
      e1_tsf5ts3_rt,
      e1_tsf5ts3_lr,
      e1_tsf5ts3_pr,
      e1_tsf5ts4_id,
      e1_tsf5ts4_rt,
      e1_tsf5ts4_lr,
      e1_tsf5ts4_pr,
      e1_tsf5ts5_id,
      e1_tsf5ts5_rt,
      e1_tsf5ts5_lr,
      e1_tsf5ts5_pr,
      e1_tsf5ts6_id,
      e1_tsf5ts6_rt,
      e1_tsf5ts6_lr,
      e1_tsf5ts6_pr,
      e1_tsf5ts7_id,
      e1_tsf5ts7_rt,
      e1_tsf5ts7_lr,
      e1_tsf5ts7_pr,
      e1_tsf5ts8_id,
      e1_tsf5ts8_rt,
      e1_tsf5ts8_lr,
      e1_tsf5ts8_pr,
      e1_tsf5ts9_id,
      e1_tsf5ts9_rt,
      e1_tsf5ts9_lr,
      e1_tsf5ts9_pr,
      e1_tsf5ts10_id,
      e1_tsf5ts10_rt,
      e1_tsf5ts10_lr,
      e1_tsf5ts10_pr,
      e1_tsf5ts11_id,
      e1_tsf5ts11_rt,
      e1_tsf5ts11_lr,
      e1_tsf5ts11_pr,
      e1_tsf5ts12_id,
      e1_tsf5ts12_rt,
      e1_tsf5ts12_lr,
      e1_tsf5ts12_pr,
      e1_tsf5ts13_id,
      e1_tsf5ts13_rt,
      e1_tsf5ts13_lr,
      e1_tsf5ts13_pr,
      e1_tsf5ts14_id,
      e1_tsf5ts14_rt,
      e1_tsf5ts14_lr,
      e1_tsf5ts14_pr,
      e1_tsf7_cc,
      e1_tsf7ts0_id,
      e1_tsf7ts0_rt,
      e1_tsf7ts0_lr,
      e1_tsf7ts0_pr,
      e1_tsf7ts1_id,
      e1_tsf7ts1_rt,
      e1_tsf7ts1_lr,
      e1_tsf7ts1_pr,
      e1_tsf7ts2_id,
      e1_tsf7ts2_rt,
      e1_tsf7ts2_lr,
      e1_tsf7ts2_pr,
      e1_tsf7ts3_id,
      e1_tsf7ts3_rt,
      e1_tsf7ts3_lr,
      e1_tsf7ts3_pr,
      e1_tsf7ts4_id,
      e1_tsf7ts4_rt,
      e1_tsf7ts4_lr,
      e1_tsf7ts4_pr,
      e1_tsf7ts5_id,
      e1_tsf7ts5_rt,
      e1_tsf7ts5_lr,
      e1_tsf7ts5_pr,
      e1_tsf7ts6_id,
      e1_tsf7ts6_rt,
      e1_tsf7ts6_lr,
      e1_tsf7ts6_pr,
      e1_tsf7ts7_id,
      e1_tsf7ts7_rt,
      e1_tsf7ts7_lr,
      e1_tsf7ts7_pr,
      e1_tsf7ts8_id,
      e1_tsf7ts8_rt,
      e1_tsf7ts8_lr,
      e1_tsf7ts8_pr,
      e1_tsf7ts9_id,
      e1_tsf7ts9_rt,
      e1_tsf7ts9_lr,
      e1_tsf7ts9_pr,
      e1_tsf7ts10_id,
      e1_tsf7ts10_rt,
      e1_tsf7ts10_lr,
      e1_tsf7ts10_pr,
      e1_tsf7ts11_id,
      e1_tsf7ts11_rt,
      e1_tsf7ts11_lr,
      e1_tsf7ts11_pr,
      e1_tsf7ts12_id,
      e1_tsf7ts12_rt,
      e1_tsf7ts12_lr,
      e1_tsf7ts12_pr,
      e1_tsf7ts13_id,
      e1_tsf7ts13_rt,
      e1_tsf7ts13_lr,
      e1_tsf7ts13_pr,
      e1_tsf7ts14_id,
      e1_tsf7ts14_rt,
      e1_tsf7ts14_lr,
      e1_tsf7ts14_pr,
      e1_t2d_cc,
      e1_t2d_fnf,
      e1_t2d0_charge,
      e1_t2d0_rho_s,
      e1_t2d0_phi,
      e1_t2d0ts0_id,
      e1_t2d0ts0_rt,
      e1_t2d0ts0_lr,
      e1_t2d0ts0_pr,
      e1_t2d0ts2_id,
      e1_t2d0ts2_rt,
      e1_t2d0ts2_lr,
      e1_t2d0ts2_pr,
      e1_t2d0ts4_id,
      e1_t2d0ts4_rt,
      e1_t2d0ts4_lr,
      e1_t2d0ts4_pr,
      e1_t2d0ts6_id,
      e1_t2d0ts6_rt,
      e1_t2d0ts6_lr,
      e1_t2d0ts6_pr,
      e1_t2d0ts8_id,
      e1_t2d0ts8_rt,
      e1_t2d0ts8_lr,
      e1_t2d0ts8_pr,
      e1_t2d1_charge,
      e1_t2d1_rho_s,
      e1_t2d1_phi,
      e1_t2d1ts0_id,
      e1_t2d1ts0_rt,
      e1_t2d1ts0_lr,
      e1_t2d1ts0_pr,
      e1_t2d1ts2_id,
      e1_t2d1ts2_rt,
      e1_t2d1ts2_lr,
      e1_t2d1ts2_pr,
      e1_t2d1ts4_id,
      e1_t2d1ts4_rt,
      e1_t2d1ts4_lr,
      e1_t2d1ts4_pr,
      e1_t2d1ts6_id,
      e1_t2d1ts6_rt,
      e1_t2d1ts6_lr,
      e1_t2d1ts6_pr,
      e1_t2d1ts8_id,
      e1_t2d1ts8_rt,
      e1_t2d1ts8_lr,
      e1_t2d1ts8_pr,
      e1_t2d2_charge,
      e1_t2d2_rho_s,
      e1_t2d2_phi,
      e1_t2d2ts0_id,
      e1_t2d2ts0_rt,
      e1_t2d2ts0_lr,
      e1_t2d2ts0_pr,
      e1_t2d2ts2_id,
      e1_t2d2ts2_rt,
      e1_t2d2ts2_lr,
      e1_t2d2ts2_pr,
      e1_t2d2ts4_id,
      e1_t2d2ts4_rt,
      e1_t2d2ts4_lr,
      e1_t2d2ts4_pr,
      e1_t2d2ts6_id,
      e1_t2d2ts6_rt,
      e1_t2d2ts6_lr,
      e1_t2d2ts6_pr,
      e1_t2d2ts8_id,
      e1_t2d2ts8_rt,
      e1_t2d2ts8_lr,
      e1_t2d2ts8_pr,
      e1_t2d3_charge,
      e1_t2d3_rho_s,
      e1_t2d3_phi,
      e1_t2d3ts0_id,
      e1_t2d3ts0_rt,
      e1_t2d3ts0_lr,
      e1_t2d3ts0_pr,
      e1_t2d3ts2_id,
      e1_t2d3ts2_rt,
      e1_t2d3ts2_lr,
      e1_t2d3ts2_pr,
      e1_t2d3ts4_id,
      e1_t2d3ts4_rt,
      e1_t2d3ts4_lr,
      e1_t2d3ts4_pr,
      e1_t2d3ts6_id,
      e1_t2d3ts6_rt,
      e1_t2d3ts6_lr,
      e1_t2d3ts6_pr,
      e1_t2d3ts8_id,
      e1_t2d3ts8_rt,
      e1_t2d3ts8_lr,
      e1_t2d3ts8_pr,
      e1_etf_thresh,
      e1_etf_cc,
      e1_etf_t0,
      e1_etf_valid,
      e1_firmid, e1_firmver, e1_evt, e1_clk
    };

    /** enum of leafs for 2D fitter **/
    enum EBits_2dfitter {
      e2_t3d_2doldtrk,
      e2_t3dtrk0_evtTime_delay,
      e2_t3dtrk0_evtTimeValid_delay,
      e2_t3dtrk0_charge,
      e2_t3dtrk0_rho,
      e2_t3dtrk0_phi0,
      e2_t3dtrk0ts0_id,
      e2_t3dtrk0ts0_lr,
      e2_t3dtrk0ts0_rt,
      e2_t3dtrk0ts1_id,
      e2_t3dtrk0ts1_lr,
      e2_t3dtrk0ts1_rt,
      e2_t3dtrk0ts2_id,
      e2_t3dtrk0ts2_lr,
      e2_t3dtrk0ts2_rt,
      e2_t3dtrk0ts3_id,
      e2_t3dtrk0ts3_lr,
      e2_t3dtrk0ts3_rt,
      e2_t3dtrk1_evtTime_delay,
      e2_t3dtrk1_evtTimeValid_delay,
      e2_t3dtrk1_charge,
      e2_t3dtrk1_rho,
      e2_t3dtrk1_phi0,
      e2_t3dtrk1ts0_id,
      e2_t3dtrk1ts0_lr,
      e2_t3dtrk1ts0_rt,
      e2_t3dtrk1ts1_id,
      e2_t3dtrk1ts1_lr,
      e2_t3dtrk1ts1_rt,
      e2_t3dtrk1ts2_id,
      e2_t3dtrk1ts2_lr,
      e2_t3dtrk1ts2_rt,
      e2_t3dtrk1ts3_id,
      e2_t3dtrk1ts3_lr,
      e2_t3dtrk1ts3_rt,
      e2_t2dfittertrk0_rho,
      e2_t2dfittertrk0_phi0,
      e2_t2dfittertrk1_rho,
      e2_t2dfittertrk1_phi0,
      e2_t2dfittertrk2_rho,
      e2_t2dfittertrk2_phi0,
      e2_t2dfittertrk3_rho,
      e2_t2dfittertrk3_phi0,
      e2_t2d_cc,
      e2_t2d_fnf,
      e2_t2d0_charge,
      e2_t2d0_rho_s,
      e2_t2d0_phi,
      e2_t2d0ts0_id,
      e2_t2d0ts0_rt,
      e2_t2d0ts0_lr,
      e2_t2d0ts0_pr,
      e2_t2d0ts2_id,
      e2_t2d0ts2_rt,
      e2_t2d0ts2_lr,
      e2_t2d0ts2_pr,
      e2_t2d0ts4_id,
      e2_t2d0ts4_rt,
      e2_t2d0ts4_lr,
      e2_t2d0ts4_pr,
      e2_t2d0ts6_id,
      e2_t2d0ts6_rt,
      e2_t2d0ts6_lr,
      e2_t2d0ts6_pr,
      e2_t2d0ts8_id,
      e2_t2d0ts8_rt,
      e2_t2d0ts8_lr,
      e2_t2d0ts8_pr,
      e2_t2d1_charge,
      e2_t2d1_rho_s,
      e2_t2d1_phi,
      e2_t2d1ts0_id,
      e2_t2d1ts0_rt,
      e2_t2d1ts0_lr,
      e2_t2d1ts0_pr,
      e2_t2d1ts2_id,
      e2_t2d1ts2_rt,
      e2_t2d1ts2_lr,
      e2_t2d1ts2_pr,
      e2_t2d1ts4_id,
      e2_t2d1ts4_rt,
      e2_t2d1ts4_lr,
      e2_t2d1ts4_pr,
      e2_t2d1ts6_id,
      e2_t2d1ts6_rt,
      e2_t2d1ts6_lr,
      e2_t2d1ts6_pr,
      e2_t2d1ts8_id,
      e2_t2d1ts8_rt,
      e2_t2d1ts8_lr,
      e2_t2d1ts8_pr,
      e2_t2d2_charge,
      e2_t2d2_rho_s,
      e2_t2d2_phi,
      e2_t2d2ts0_id,
      e2_t2d2ts0_rt,
      e2_t2d2ts0_lr,
      e2_t2d2ts0_pr,
      e2_t2d2ts2_id,
      e2_t2d2ts2_rt,
      e2_t2d2ts2_lr,
      e2_t2d2ts2_pr,
      e2_t2d2ts4_id,
      e2_t2d2ts4_rt,
      e2_t2d2ts4_lr,
      e2_t2d2ts4_pr,
      e2_t2d2ts6_id,
      e2_t2d2ts6_rt,
      e2_t2d2ts6_lr,
      e2_t2d2ts6_pr,
      e2_t2d2ts8_id,
      e2_t2d2ts8_rt,
      e2_t2d2ts8_lr,
      e2_t2d2ts8_pr,
      e2_t2d3_charge,
      e2_t2d3_rho_s,
      e2_t2d3_phi,
      e2_t2d3ts0_id,
      e2_t2d3ts0_rt,
      e2_t2d3ts0_lr,
      e2_t2d3ts0_pr,
      e2_t2d3ts2_id,
      e2_t2d3ts2_rt,
      e2_t2d3ts2_lr,
      e2_t2d3ts2_pr,
      e2_t2d3ts4_id,
      e2_t2d3ts4_rt,
      e2_t2d3ts4_lr,
      e2_t2d3ts4_pr,
      e2_t2d3ts6_id,
      e2_t2d3ts6_rt,
      e2_t2d3ts6_lr,
      e2_t2d3ts6_pr,
      e2_t2d3ts8_id,
      e2_t2d3ts8_rt,
      e2_t2d3ts8_lr,
      e2_t2d3ts8_pr,
      e2_etf_thresh,
      e2_etf_cc,
      e2_etf_t0,
      e2_etf_valid,
      e2_firmid, e2_firmver, e2_evt, e2_clk
    };


    /** bus bit map. (a downto a-b) **/
    const int BitMap[nLeafs][2] = {
      /** the numbers in the () are still old, add 32 of them to get the correct ones **/
      1746, 5, // t3d_2doldtrk (1714 downto 1709)
      1740, 8, // t3dtrk0_evtTime_delay (1708 downto 1700)
      1731, 0, // t3dtrk0_evtTimeValid_delay (1699 downto 1699)
      1730, 0, // t3dtrk0_charge (1698 downto 1698)
      1729, 10, // t3dtrk0_rho (1697 downto 1687)
      1718, 12, // t3dtrk0_phi0 (1686 downto 1674)
      1705, 7, // t3dtrk0ts0_id (1673 downto 1666)
      1697, 1, // t3dtrk0ts0_lr (1665 downto 1664)
      1695, 8, // t3dtrk0ts0_rt (1663 downto 1655)
      1686, 7, // t3dtrk0ts1_id (1654 downto 1647)
      1678, 1, // t3dtrk0ts1_lr (1646 downto 1645)
      1676, 8, // t3dtrk0ts1_rt (1644 downto 1636)
      1667, 8, // t3dtrk0ts2_id (1635 downto 1627)
      1658, 1, // t3dtrk0ts2_lr (1626 downto 1625)
      1656, 8, // t3dtrk0ts2_rt (1624 downto 1616)
      1647, 8, // t3dtrk0ts3_id (1615 downto 1607)
      1638, 1, // t3dtrk0ts3_lr (1606 downto 1605)
      1636, 8, // t3dtrk0ts3_rt (1604 downto 1596)
      1627, 8, // t3dtrk1_evtTime_delay (1595 downto 1587)
      1618, 0, // t3dtrk1_evtTimeValid_delay (1586 downto 1586)
      1617, 0, // t3dtrk1_charge (1585 downto 1585)
      1616, 10, // t3dtrk1_rho (1584 downto 1574)
      1605, 12, // t3dtrk1_phi0 (1573 downto 1561)
      1592, 7, // t3dtrk1ts0_id (1560 downto 1553)
      1584, 1, // t3dtrk1ts0_lr (1552 downto 1551)
      1582, 8, // t3dtrk1ts0_rt (1550 downto 1542)
      1573, 7, // t3dtrk1ts1_id (1541 downto 1534)
      1565, 1, // t3dtrk1ts1_lr (1533 downto 1532)
      1563, 8, // t3dtrk1ts1_rt (1531 downto 1523)
      1554, 8, // t3dtrk1ts2_id (1522 downto 1514)
      1545, 1, // t3dtrk1ts2_lr (1513 downto 1512)
      1543, 8, // t3dtrk1ts2_rt (1511 downto 1503)
      1534, 8, // t3dtrk1ts3_id (1502 downto 1494)
      1525, 1, // t3dtrk1ts3_lr (1493 downto 1492)
      1523, 8, // t3dtrk1ts3_rt (1491 downto 1483)
      1514, 1, // t3d_phase (1482 downto 1481)
      1512, 3, // t3d_validTS (1480 downto 1477)
      1508, 5, // t3d_2dfnf (1476 downto 1471)
      1502, 10, // t3dtrk0_z0_s (1470 downto 1460)
      1491, 10, // t3dtrk0_cot_s (1459 downto 1449)
      1480, 3, // t3dtrk0_zchisq (1448 downto 1445)
      1476, 10, // t3dtrk1_z0_s (1444 downto 1434)
      1465, 10, // t3dtrk1_cot_s (1433 downto 1423)
      1454, 3, // t3dtrk1_zchisq (1422 downto 1419)
      1450, 10, // t3dtrk2_z0_s (1418 downto 1408)
      1439, 10, // t3dtrk2_cot_s (1407 downto 1397)
      1428, 3, // t3dtrk2_zchisq (1396 downto 1393)
      1424, 10, // t3dtrk3_z0_s (1392 downto 1382)
      1413, 10, // t3dtrk3_cot_s (1381 downto 1371)
      1402, 3, // t3dtrk3_zchisq (1370 downto 1367)
      1398, 0, // t3dtrk_rd_req (1366 downto 1366)
      1397, 8, // tsf1_cc (1365 downto 1357)
      1388, 7, // tsf1ts0_id (1356 downto 1349)
      1380, 8, // tsf1ts0_rt (1348 downto 1340)
      1371, 1, // tsf1ts0_lr (1339 downto 1338)
      1369, 1, // tsf1ts0_pr (1337 downto 1336)
      1367, 7, // tsf1ts1_id (1335 downto 1328)
      1359, 8, // tsf1ts1_rt (1327 downto 1319)
      1350, 1, // tsf1ts1_lr (1318 downto 1317)
      1348, 1, // tsf1ts1_pr (1316 downto 1315)
      1346, 7, // tsf1ts2_id (1314 downto 1307)
      1338, 8, // tsf1ts2_rt (1306 downto 1298)
      1329, 1, // tsf1ts2_lr (1297 downto 1296)
      1327, 1, // tsf1ts2_pr (1295 downto 1294)
      1325, 7, // tsf1ts3_id (1293 downto 1286)
      1317, 8, // tsf1ts3_rt (1285 downto 1277)
      1308, 1, // tsf1ts3_lr (1276 downto 1275)
      1306, 1, // tsf1ts3_pr (1274 downto 1273)
      1304, 7, // tsf1ts4_id (1272 downto 1265)
      1296, 8, // tsf1ts4_rt (1264 downto 1256)
      1287, 1, // tsf1ts4_lr (1255 downto 1254)
      1285, 1, // tsf1ts4_pr (1253 downto 1252)
      1283, 7, // tsf1ts5_id (1251 downto 1244)
      1275, 8, // tsf1ts5_rt (1243 downto 1235)
      1266, 1, // tsf1ts5_lr (1234 downto 1233)
      1264, 1, // tsf1ts5_pr (1232 downto 1231)
      1262, 7, // tsf1ts6_id (1230 downto 1223)
      1254, 8, // tsf1ts6_rt (1222 downto 1214)
      1245, 1, // tsf1ts6_lr (1213 downto 1212)
      1243, 1, // tsf1ts6_pr (1211 downto 1210)
      1241, 7, // tsf1ts7_id (1209 downto 1202)
      1233, 8, // tsf1ts7_rt (1201 downto 1193)
      1224, 1, // tsf1ts7_lr (1192 downto 1191)
      1222, 1, // tsf1ts7_pr (1190 downto 1189)
      1220, 7, // tsf1ts8_id (1188 downto 1181)
      1212, 8, // tsf1ts8_rt (1180 downto 1172)
      1203, 1, // tsf1ts8_lr (1171 downto 1170)
      1201, 1, // tsf1ts8_pr (1169 downto 1168)
      1199, 7, // tsf1ts9_id (1167 downto 1160)
      1191, 8, // tsf1ts9_rt (1159 downto 1151)
      1182, 1, // tsf1ts9_lr (1150 downto 1149)
      1180, 1, // tsf1ts9_pr (1148 downto 1147)
      1178, 8, // tsf3_cc (1146 downto 1138)
      1169, 7, // tsf3ts0_id (1137 downto 1130)
      1161, 8, // tsf3ts0_rt (1129 downto 1121)
      1152, 1, // tsf3ts0_lr (1120 downto 1119)
      1150, 1, // tsf3ts0_pr (1118 downto 1117)
      1148, 7, // tsf3ts1_id (1116 downto 1109)
      1140, 8, // tsf3ts1_rt (1108 downto 1100)
      1131, 1, // tsf3ts1_lr (1099 downto 1098)
      1129, 1, // tsf3ts1_pr (1097 downto 1096)
      1127, 7, // tsf3ts2_id (1095 downto 1088)
      1119, 8, // tsf3ts2_rt (1087 downto 1079)
      1110, 1, // tsf3ts2_lr (1078 downto 1077)
      1108, 1, // tsf3ts2_pr (1076 downto 1075)
      1106, 7, // tsf3ts3_id (1074 downto 1067)
      1098, 8, // tsf3ts3_rt (1066 downto 1058)
      1089, 1, // tsf3ts3_lr (1057 downto 1056)
      1087, 1, // tsf3ts3_pr (1055 downto 1054)
      1085, 7, // tsf3ts4_id (1053 downto 1046)
      1077, 8, // tsf3ts4_rt (1045 downto 1037)
      1068, 1, // tsf3ts4_lr (1036 downto 1035)
      1066, 1, // tsf3ts4_pr (1034 downto 1033)
      1064, 7, // tsf3ts5_id (1032 downto 1025)
      1056, 8, // tsf3ts5_rt (1024 downto 1016)
      1047, 1, // tsf3ts5_lr (1015 downto 1014)
      1045, 1, // tsf3ts5_pr (1013 downto 1012)
      1043, 7, // tsf3ts6_id (1011 downto 1004)
      1035, 8, // tsf3ts6_rt (1003 downto 995)
      1026, 1, // tsf3ts6_lr (994 downto 993)
      1024, 1, // tsf3ts6_pr (992 downto 991)
      1022, 7, // tsf3ts7_id (990 downto 983)
      1014, 8, // tsf3ts7_rt (982 downto 974)
      1005, 1, // tsf3ts7_lr (973 downto 972)
      1003, 1, // tsf3ts7_pr (971 downto 970)
      1001, 7, // tsf3ts8_id (969 downto 962)
      993, 8, // tsf3ts8_rt (961 downto 953)
      984, 1, // tsf3ts8_lr (952 downto 951)
      982, 1, // tsf3ts8_pr (950 downto 949)
      980, 7, // tsf3ts9_id (948 downto 941)
      972, 8, // tsf3ts9_rt (940 downto 932)
      963, 1, // tsf3ts9_lr (931 downto 930)
      961, 1, // tsf3ts9_pr (929 downto 928)
      959, 8, // tsf5_cc (927 downto 919)
      950, 7, // tsf5ts0_id (918 downto 911)
      942, 8, // tsf5ts0_rt (910 downto 902)
      933, 1, // tsf5ts0_lr (901 downto 900)
      931, 1, // tsf5ts0_pr (899 downto 898)
      929, 7, // tsf5ts1_id (897 downto 890)
      921, 8, // tsf5ts1_rt (889 downto 881)
      912, 1, // tsf5ts1_lr (880 downto 879)
      910, 1, // tsf5ts1_pr (878 downto 877)
      908, 7, // tsf5ts2_id (876 downto 869)
      900, 8, // tsf5ts2_rt (868 downto 860)
      891, 1, // tsf5ts2_lr (859 downto 858)
      889, 1, // tsf5ts2_pr (857 downto 856)
      887, 7, // tsf5ts3_id (855 downto 848)
      879, 8, // tsf5ts3_rt (847 downto 839)
      870, 1, // tsf5ts3_lr (838 downto 837)
      868, 1, // tsf5ts3_pr (836 downto 835)
      866, 7, // tsf5ts4_id (834 downto 827)
      858, 8, // tsf5ts4_rt (826 downto 818)
      849, 1, // tsf5ts4_lr (817 downto 816)
      847, 1, // tsf5ts4_pr (815 downto 814)
      845, 7, // tsf5ts5_id (813 downto 806)
      837, 8, // tsf5ts5_rt (805 downto 797)
      828, 1, // tsf5ts5_lr (796 downto 795)
      826, 1, // tsf5ts5_pr (794 downto 793)
      824, 7, // tsf5ts6_id (792 downto 785)
      816, 8, // tsf5ts6_rt (784 downto 776)
      807, 1, // tsf5ts6_lr (775 downto 774)
      805, 1, // tsf5ts6_pr (773 downto 772)
      803, 7, // tsf5ts7_id (771 downto 764)
      795, 8, // tsf5ts7_rt (763 downto 755)
      786, 1, // tsf5ts7_lr (754 downto 753)
      784, 1, // tsf5ts7_pr (752 downto 751)
      782, 7, // tsf5ts8_id (750 downto 743)
      774, 8, // tsf5ts8_rt (742 downto 734)
      765, 1, // tsf5ts8_lr (733 downto 732)
      763, 1, // tsf5ts8_pr (731 downto 730)
      761, 7, // tsf5ts9_id (729 downto 722)
      753, 8, // tsf5ts9_rt (721 downto 713)
      744, 1, // tsf5ts9_lr (712 downto 711)
      742, 1, // tsf5ts9_pr (710 downto 709)
      740, 8, // tsf7_cc (708 downto 700)
      731, 7, // tsf7ts0_id (699 downto 692)
      723, 8, // tsf7ts0_rt (691 downto 683)
      714, 1, // tsf7ts0_lr (682 downto 681)
      712, 1, // tsf7ts0_pr (680 downto 679)
      710, 7, // tsf7ts1_id (678 downto 671)
      702, 8, // tsf7ts1_rt (670 downto 662)
      693, 1, // tsf7ts1_lr (661 downto 660)
      691, 1, // tsf7ts1_pr (659 downto 658)
      689, 7, // tsf7ts2_id (657 downto 650)
      681, 8, // tsf7ts2_rt (649 downto 641)
      672, 1, // tsf7ts2_lr (640 downto 639)
      670, 1, // tsf7ts2_pr (638 downto 637)
      668, 7, // tsf7ts3_id (636 downto 629)
      660, 8, // tsf7ts3_rt (628 downto 620)
      651, 1, // tsf7ts3_lr (619 downto 618)
      649, 1, // tsf7ts3_pr (617 downto 616)
      647, 7, // tsf7ts4_id (615 downto 608)
      639, 8, // tsf7ts4_rt (607 downto 599)
      630, 1, // tsf7ts4_lr (598 downto 597)
      628, 1, // tsf7ts4_pr (596 downto 595)
      626, 7, // tsf7ts5_id (594 downto 587)
      618, 8, // tsf7ts5_rt (586 downto 578)
      609, 1, // tsf7ts5_lr (577 downto 576)
      607, 1, // tsf7ts5_pr (575 downto 574)
      605, 7, // tsf7ts6_id (573 downto 566)
      597, 8, // tsf7ts6_rt (565 downto 557)
      588, 1, // tsf7ts6_lr (556 downto 555)
      586, 1, // tsf7ts6_pr (554 downto 553)
      584, 7, // tsf7ts7_id (552 downto 545)
      576, 8, // tsf7ts7_rt (544 downto 536)
      567, 1, // tsf7ts7_lr (535 downto 534)
      565, 1, // tsf7ts7_pr (533 downto 532)
      563, 7, // tsf7ts8_id (531 downto 524)
      555, 8, // tsf7ts8_rt (523 downto 515)
      546, 1, // tsf7ts8_lr (514 downto 513)
      544, 1, // tsf7ts8_pr (512 downto 511)
      542, 7, // tsf7ts9_id (510 downto 503)
      534, 8, // tsf7ts9_rt (502 downto 494)
      525, 1, // tsf7ts9_lr (493 downto 492)
      523, 1, // tsf7ts9_pr (491 downto 490)
      521, 8, // t2d_cc (489 downto 481)
      512, 5, // t2d_fnf (480 downto 475)
      506, 1, // t2d0_charge (474 downto 473)
      504, 6, // t2d0_rho_s (472 downto 466)
      497, 6, // t2d0_phi (465 downto 459)
      490, 7, // t2d0ts0_id (458 downto 451)
      482, 8, // t2d0ts0_rt (450 downto 442)
      473, 1, // t2d0ts0_lr (441 downto 440)
      471, 1, // t2d0ts0_pr (439 downto 438)
      469, 7, // t2d0ts2_id (437 downto 430)
      461, 8, // t2d0ts2_rt (429 downto 421)
      452, 1, // t2d0ts2_lr (420 downto 419)
      450, 1, // t2d0ts2_pr (418 downto 417)
      448, 7, // t2d0ts4_id (416 downto 409)
      440, 8, // t2d0ts4_rt (408 downto 400)
      431, 1, // t2d0ts4_lr (399 downto 398)
      429, 1, // t2d0ts4_pr (397 downto 396)
      427, 7, // t2d0ts6_id (395 downto 388)
      419, 8, // t2d0ts6_rt (387 downto 379)
      410, 1, // t2d0ts6_lr (378 downto 377)
      408, 1, // t2d0ts6_pr (376 downto 375)
      406, 7, // t2d0ts8_id (374 downto 367)
      398, 8, // t2d0ts8_rt (366 downto 358)
      389, 1, // t2d0ts8_lr (357 downto 356)
      387, 1, // t2d0ts8_pr (355 downto 354)
      385, 1, // t2d1_charge (353 downto 352)
      383, 6, // t2d1_rho_s (351 downto 345)
      376, 6, // t2d1_phi (344 downto 338)
      369, 7, // t2d1ts0_id (337 downto 330)
      361, 8, // t2d1ts0_rt (329 downto 321)
      352, 1, // t2d1ts0_lr (320 downto 319)
      350, 1, // t2d1ts0_pr (318 downto 317)
      348, 7, // t2d1ts2_id (316 downto 309)
      340, 8, // t2d1ts2_rt (308 downto 300)
      331, 1, // t2d1ts2_lr (299 downto 298)
      329, 1, // t2d1ts2_pr (297 downto 296)
      327, 7, // t2d1ts4_id (295 downto 288)
      319, 8, // t2d1ts4_rt (287 downto 279)
      310, 1, // t2d1ts4_lr (278 downto 277)
      308, 1, // t2d1ts4_pr (276 downto 275)
      306, 7, // t2d1ts6_id (274 downto 267)
      298, 8, // t2d1ts6_rt (266 downto 258)
      289, 1, // t2d1ts6_lr (257 downto 256)
      287, 1, // t2d1ts6_pr (255 downto 254)
      285, 7, // t2d1ts8_id (253 downto 246)
      277, 8, // t2d1ts8_rt (245 downto 237)
      268, 1, // t2d1ts8_lr (236 downto 235)
      266, 1, // t2d1ts8_pr (234 downto 233)
      264, 1, // t2d2_charge (232 downto 231)
      262, 6, // t2d2_rho_s (230 downto 224)
      255, 6, // t2d2_phi (223 downto 217)
      248, 7, // t2d2ts0_id (216 downto 209)
      240, 8, // t2d2ts0_rt (208 downto 200)
      231, 1, // t2d2ts0_lr (199 downto 198)
      229, 1, // t2d2ts0_pr (197 downto 196)
      227, 7, // t2d2ts2_id (195 downto 188)
      219, 8, // t2d2ts2_rt (187 downto 179)
      210, 1, // t2d2ts2_lr (178 downto 177)
      208, 1, // t2d2ts2_pr (176 downto 175)
      206, 7, // t2d2ts4_id (174 downto 167)
      198, 8, // t2d2ts4_rt (166 downto 158)
      189, 1, // t2d2ts4_lr (157 downto 156)
      187, 1, // t2d2ts4_pr (155 downto 154)
      185, 7, // t2d2ts6_id (153 downto 146)
      177, 8, // t2d2ts6_rt (145 downto 137)
      168, 1, // t2d2ts6_lr (136 downto 135)
      166, 1, // t2d2ts6_pr (134 downto 133)
      164, 7, // t2d2ts8_id (132 downto 125)
      156, 8, // t2d2ts8_rt (124 downto 116)
      147, 1, // t2d2ts8_lr (115 downto 114)
      145, 1, // t2d2ts8_pr (113 downto 112)
      143, 1, // t2d3_charge (111 downto 110)
      141, 6, // t2d3_rho_s (109 downto 103)
      134, 6, // t2d3_phi (102 downto 96)
      127, 7, // t2d3ts0_id (95 downto 88)
      119, 8, // t2d3ts0_rt (87 downto 79)
      110, 1, // t2d3ts0_lr (78 downto 77)
      108, 1, // t2d3ts0_pr (76 downto 75)
      106, 7, // t2d3ts2_id (74 downto 67)
      98, 8, // t2d3ts2_rt (66 downto 58)
      89, 1, // t2d3ts2_lr (57 downto 56)
      87, 1, // t2d3ts2_pr (55 downto 54)
      85, 7, // t2d3ts4_id (53 downto 46)
      77, 8, // t2d3ts4_rt (45 downto 37)
      68, 1, // t2d3ts4_lr (36 downto 35)
      66, 1, // t2d3ts4_pr (34 downto 33)
      64, 7, // t2d3ts6_id (32 downto 25)
      56, 8, // t2d3ts6_rt (24 downto 16)
      47, 1, // t2d3ts6_lr (15 downto 14)
      45, 1, // t2d3ts6_pr (13 downto 12)
      43, 7, // t2d3ts8_id (11 downto 4)
      35, 8, // t2d3ts8_rt
      26, 1, // t2d3ts8_lr
      24, 1, // t2d3ts8_pr
      22, 3, // etf_thresh
      18, 8, // etf_cc
      9, 8, // etf_t0
      0, 0  // etf_valid
    };

    /** bus bit map. 2624 bits, 15 TS version (a downto a-b) **/
    const int BitMap_2624[nLeafs_2624][2] = {
      /** the numbers in the () are still old, add 32 of them to get the correct ones **/
      1746 + 420, 5, // t3d_2doldtrk (1714 downto 1709)
      1740 + 420, 8, // t3dtrk0_evtTime_delay (1708 downto 1700)
      1731 + 420, 0, // t3dtrk0_evtTimeValid_delay (1699 downto 1699)
      1730 + 420, 0, // t3dtrk0_charge (1698 downto 1698)
      1729 + 420, 10, // t3dtrk0_rho (1697 downto 1687)
      1718 + 420, 12, // t3dtrk0_phi0 (1686 downto 1674)
      1705 + 420, 7, // t3dtrk0ts0_id (1673 downto 1666)
      1697 + 420, 1, // t3dtrk0ts0_lr (1665 downto 1664)
      1695 + 420, 8, // t3dtrk0ts0_rt (1663 downto 1655)
      1686 + 420, 7, // t3dtrk0ts1_id (1654 downto 1647)
      1678 + 420, 1, // t3dtrk0ts1_lr (1646 downto 1645)
      1676 + 420, 8, // t3dtrk0ts1_rt (1644 downto 1636)
      1667 + 420, 8, // t3dtrk0ts2_id (1635 downto 1627)
      1658 + 420, 1, // t3dtrk0ts2_lr (1626 downto 1625)
      1656 + 420, 8, // t3dtrk0ts2_rt (1624 downto 1616)
      1647 + 420, 8, // t3dtrk0ts3_id (1615 downto 1607)
      1638 + 420, 1, // t3dtrk0ts3_lr (1606 downto 1605)
      1636 + 420, 8, // t3dtrk0ts3_rt (1604 downto 1596)
      1627 + 420, 8, // t3dtrk1_evtTime_delay (1595 downto 1587)
      1618 + 420, 0, // t3dtrk1_evtTimeValid_delay (1586 downto 1586)
      1617 + 420, 0, // t3dtrk1_charge (1585 downto 1585)
      1616 + 420, 10, // t3dtrk1_rho (1584 downto 1574)
      1605 + 420, 12, // t3dtrk1_phi0 (1573 downto 1561)
      1592 + 420, 7, // t3dtrk1ts0_id (1560 downto 1553)
      1584 + 420, 1, // t3dtrk1ts0_lr (1552 downto 1551)
      1582 + 420, 8, // t3dtrk1ts0_rt (1550 downto 1542)
      1573 + 420, 7, // t3dtrk1ts1_id (1541 downto 1534)
      1565 + 420, 1, // t3dtrk1ts1_lr (1533 downto 1532)
      1563 + 420, 8, // t3dtrk1ts1_rt (1531 downto 1523)
      1554 + 420, 8, // t3dtrk1ts2_id (1522 downto 1514)
      1545 + 420, 1, // t3dtrk1ts2_lr (1513 downto 1512)
      1543 + 420, 8, // t3dtrk1ts2_rt (1511 downto 1503)
      1534 + 420, 8, // t3dtrk1ts3_id (1502 downto 1494)
      1525 + 420, 1, // t3dtrk1ts3_lr (1493 downto 1492)
      1523 + 420, 8, // t3dtrk1ts3_rt (1491 downto 1483)
      1514 + 420, 1, // t3d_phase (1482 downto 1481)
      1512 + 420, 3, // t3d_validTS (1480 downto 1477)
      1508 + 420, 5, // t3d_2dfnf (1476 downto 1471)
      1502 + 420, 10, // t3dtrk0_z0_s (1470 downto 1460)
      1491 + 420, 10, // t3dtrk0_cot_s (1459 downto 1449)
      1480 + 420, 3, // t3dtrk0_zchisq (1448 downto 1445)
      1476 + 420, 10, // t3dtrk1_z0_s (1444 downto 1434)
      1465 + 420, 10, // t3dtrk1_cot_s (1433 downto 1423)
      1454 + 420, 3, // t3dtrk1_zchisq (1422 downto 1419)
      1450 + 420, 10, // t3dtrk2_z0_s (1418 downto 1408)
      1439 + 420, 10, // t3dtrk2_cot_s (1407 downto 1397)
      1428 + 420, 3, // t3dtrk2_zchisq (1396 downto 1393)
      1424 + 420, 10, // t3dtrk3_z0_s (1392 downto 1382)
      1413 + 420, 10, // t3dtrk3_cot_s (1381 downto 1371)
      1402 + 420, 3, // t3dtrk3_zchisq (1370 downto 1367)
      1398 + 420, 0, // t3dtrk_rd_req (1366 downto 1366)
      1397 + 420, 8, // tsf1_cc (1365 downto 1357)
      1388 + 420, 7, // tsf1ts0_id (1356 downto 1349)
      1380 + 420, 8, // tsf1ts0_rt (1348 downto 1340)
      1371 + 420, 1, // tsf1ts0_lr (1339 downto 1338)
      1369 + 420, 1, // tsf1ts0_pr (1337 downto 1336)
      1367 + 420, 7, // tsf1ts1_id (1335 downto 1328)
      1359 + 420, 8, // tsf1ts1_rt (1327 downto 1319)
      1350 + 420, 1, // tsf1ts1_lr (1318 downto 1317)
      1348 + 420, 1, // tsf1ts1_pr (1316 downto 1315)
      1346 + 420, 7, // tsf1ts2_id (1314 downto 1307)
      1338 + 420, 8, // tsf1ts2_rt (1306 downto 1298)
      1329 + 420, 1, // tsf1ts2_lr (1297 downto 1296)
      1327 + 420, 1, // tsf1ts2_pr (1295 downto 1294)
      1325 + 420, 7, // tsf1ts3_id (1293 downto 1286)
      1317 + 420, 8, // tsf1ts3_rt (1285 downto 1277)
      1308 + 420, 1, // tsf1ts3_lr (1276 downto 1275)
      1306 + 420, 1, // tsf1ts3_pr (1274 downto 1273)
      1304 + 420, 7, // tsf1ts4_id (1272 downto 1265)
      1296 + 420, 8, // tsf1ts4_rt (1264 downto 1256)
      1287 + 420, 1, // tsf1ts4_lr (1255 downto 1254)
      1285 + 420, 1, // tsf1ts4_pr (1253 downto 1252)
      1283 + 420, 7, // tsf1ts5_id (1251 downto 1244)
      1275 + 420, 8, // tsf1ts5_rt (1243 downto 1235)
      1266 + 420, 1, // tsf1ts5_lr (1234 downto 1233)
      1264 + 420, 1, // tsf1ts5_pr (1232 downto 1231)
      1262 + 420, 7, // tsf1ts6_id (1230 downto 1223)
      1254 + 420, 8, // tsf1ts6_rt (1222 downto 1214)
      1245 + 420, 1, // tsf1ts6_lr (1213 downto 1212)
      1243 + 420, 1, // tsf1ts6_pr (1211 downto 1210)
      1241 + 420, 7, // tsf1ts7_id (1209 downto 1202)
      1233 + 420, 8, // tsf1ts7_rt (1201 downto 1193)
      1224 + 420, 1, // tsf1ts7_lr (1192 downto 1191)
      1222 + 420, 1, // tsf1ts7_pr (1190 downto 1189)
      1220 + 420, 7, // tsf1ts8_id (1188 downto 1181)
      1212 + 420, 8, // tsf1ts8_rt (1180 downto 1172)
      1203 + 420, 1, // tsf1ts8_lr (1171 downto 1170)
      1201 + 420, 1, // tsf1ts8_pr (1169 downto 1168)
      1199 + 420, 7, // tsf1ts9_id (1167 downto 1160)
      1191 + 420, 8, // tsf1ts9_rt (1159 downto 1151)
      1182 + 420, 1, // tsf1ts9_lr (1150 downto 1149)
      1180 + 420, 1, // tsf1ts9_pr (1148 downto 1147)
      1598, 7, // tsf1ts10_id (1167 downto 1160)
      1590, 8, // tsf1ts10_rt (1159 downto 1151)
      1581, 1, // tsf1ts10_lr (1150 downto 1149)
      1579, 1, // tsf1ts10_pr (1148 downto 1147)
      1577, 7, // tsf1ts11_id (1167 downto 1160)
      1569, 8, // tsf1ts11_rt (1159 downto 1151)
      1560, 1, // tsf1ts11_lr (1150 downto 1149)
      1558, 1, // tsf1ts11_pr (1148 downto 1147)
      1556, 7, // tsf1ts12_id (1167 downto 1160)
      1548, 8, // tsf1ts12_rt (1159 downto 1151)
      1539, 1, // tsf1ts12_lr (1150 downto 1149)
      1537, 1, // tsf1ts12_pr (1148 downto 1147)
      1535, 7, // tsf1ts13_id (1167 downto 1160)
      1527, 8, // tsf1ts13_rt (1159 downto 1151)
      1518, 1, // tsf1ts13_lr (1150 downto 1149)
      1516, 1, // tsf1ts13_pr (1148 downto 1147)
      1514, 7, // tsf1ts14_id (1167 downto 1160)
      1506, 8, // tsf1ts14_rt (1159 downto 1151)
      1497, 1, // tsf1ts14_lr (1150 downto 1149)
      1495, 1, // tsf1ts14_pr (1148 downto 1147)
      1178 + 315, 8, // tsf3_cc (1146 downto 1138)
      1169 + 315, 7, // tsf3ts0_id (1137 downto 1130)
      1161 + 315, 8, // tsf3ts0_rt (1129 downto 1121)
      1152 + 315, 1, // tsf3ts0_lr (1120 downto 1119)
      1150 + 315, 1, // tsf3ts0_pr (1118 downto 1117)
      1148 + 315, 7, // tsf3ts1_id (1116 downto 1109)
      1140 + 315, 8, // tsf3ts1_rt (1108 downto 1100)
      1131 + 315, 1, // tsf3ts1_lr (1099 downto 1098)
      1129 + 315, 1, // tsf3ts1_pr (1097 downto 1096)
      1127 + 315, 7, // tsf3ts2_id (1095 downto 1088)
      1119 + 315, 8, // tsf3ts2_rt (1087 downto 1079)
      1110 + 315, 1, // tsf3ts2_lr (1078 downto 1077)
      1108 + 315, 1, // tsf3ts2_pr (1076 downto 1075)
      1106 + 315, 7, // tsf3ts3_id (1074 downto 1067)
      1098 + 315, 8, // tsf3ts3_rt (1066 downto 1058)
      1089 + 315, 1, // tsf3ts3_lr (1057 downto 1056)
      1087 + 315, 1, // tsf3ts3_pr (1055 downto 1054)
      1085 + 315, 7, // tsf3ts4_id (1053 downto 1046)
      1077 + 315, 8, // tsf3ts4_rt (1045 downto 1037)
      1068 + 315, 1, // tsf3ts4_lr (1036 downto 1035)
      1066 + 315, 1, // tsf3ts4_pr (1034 downto 1033)
      1064 + 315, 7, // tsf3ts5_id (1032 downto 1025)
      1056 + 315, 8, // tsf3ts5_rt (1024 downto 1016)
      1047 + 315, 1, // tsf3ts5_lr (1015 downto 1014)
      1045 + 315, 1, // tsf3ts5_pr (1013 downto 1012)
      1043 + 315, 7, // tsf3ts6_id (1011 downto 1004)
      1035 + 315, 8, // tsf3ts6_rt (1003 downto 995)
      1026 + 315, 1, // tsf3ts6_lr (994 downto 993)
      1024 + 315, 1, // tsf3ts6_pr (992 downto 991)
      1022 + 315, 7, // tsf3ts7_id (990 downto 983)
      1014 + 315, 8, // tsf3ts7_rt (982 downto 974)
      1005 + 315, 1, // tsf3ts7_lr (973 downto 972)
      1003 + 315, 1, // tsf3ts7_pr (971 downto 970)
      1001 + 315, 7, // tsf3ts8_id (969 downto 962)
      993 + 315, 8, // tsf3ts8_rt (961 downto 953)
      984 + 315, 1, // tsf3ts8_lr (952 downto 951)
      982 + 315, 1, // tsf3ts8_pr (950 downto 949)
      980 + 315, 7, // tsf3ts9_id (948 downto 941)
      972 + 315, 8, // tsf3ts9_rt (940 downto 932)
      963 + 315, 1, // tsf3ts9_lr (931 downto 930)
      961 + 315, 1, // tsf3ts9_pr (929 downto 928)
      1274, 7, // tsf3ts10_id (1167 downto 1160)
      1266, 8, // tsf3ts10_rt (1159 downto 1151)
      1257, 1, // tsf3ts10_lr (1150 downto 1149)
      1255, 1, // tsf3ts10_pr (1148 downto 1147)
      1253, 7, // tsf3ts11_id (1167 downto 1160)
      1245, 8, // tsf3ts11_rt (1159 downto 1151)
      1236, 1, // tsf3ts11_lr (1150 downto 1149)
      1234, 1, // tsf3ts11_pr (1148 downto 1147)
      1232, 7, // tsf3ts12_id (1167 downto 1160)
      1224, 8, // tsf3ts12_rt (1159 downto 1151)
      1215, 1, // tsf3ts12_lr (1150 downto 1149)
      1213, 1, // tsf3ts12_pr (1148 downto 1147)
      1211, 7, // tsf3ts13_id (1167 downto 1160)
      1203, 8, // tsf3ts13_rt (1159 downto 1151)
      1194, 1, // tsf3ts13_lr (1150 downto 1149)
      1192, 1, // tsf3ts13_pr (1148 downto 1147)
      1190, 7, // tsf3ts14_id (1167 downto 1160)
      1182, 8, // tsf3ts14_rt (1159 downto 1151)
      1173, 1, // tsf3ts14_lr (1150 downto 1149)
      1171, 1, // tsf3ts14_pr (1148 downto 1147)
      959 + 210, 8, // tsf5_cc (927 downto 919)
      950 + 210, 7, // tsf5ts0_id (918 downto 911)
      942 + 210, 8, // tsf5ts0_rt (910 downto 902)
      933 + 210, 1, // tsf5ts0_lr (901 downto 900)
      931 + 210, 1, // tsf5ts0_pr (899 downto 898)
      929 + 210, 7, // tsf5ts1_id (897 downto 890)
      921 + 210, 8, // tsf5ts1_rt (889 downto 881)
      912 + 210, 1, // tsf5ts1_lr (880 downto 879)
      910 + 210, 1, // tsf5ts1_pr (878 downto 877)
      908 + 210, 7, // tsf5ts2_id (876 downto 869)
      900 + 210, 8, // tsf5ts2_rt (868 downto 860)
      891 + 210, 1, // tsf5ts2_lr (859 downto 858)
      889 + 210, 1, // tsf5ts2_pr (857 downto 856)
      887 + 210, 7, // tsf5ts3_id (855 downto 848)
      879 + 210, 8, // tsf5ts3_rt (847 downto 839)
      870 + 210, 1, // tsf5ts3_lr (838 downto 837)
      868 + 210, 1, // tsf5ts3_pr (836 downto 835)
      866 + 210, 7, // tsf5ts4_id (834 downto 827)
      858 + 210, 8, // tsf5ts4_rt (826 downto 818)
      849 + 210, 1, // tsf5ts4_lr (817 downto 816)
      847 + 210, 1, // tsf5ts4_pr (815 downto 814)
      845 + 210, 7, // tsf5ts5_id (813 downto 806)
      837 + 210, 8, // tsf5ts5_rt (805 downto 797)
      828 + 210, 1, // tsf5ts5_lr (796 downto 795)
      826 + 210, 1, // tsf5ts5_pr (794 downto 793)
      824 + 210, 7, // tsf5ts6_id (792 downto 785)
      816 + 210, 8, // tsf5ts6_rt (784 downto 776)
      807 + 210, 1, // tsf5ts6_lr (775 downto 774)
      805 + 210, 1, // tsf5ts6_pr (773 downto 772)
      803 + 210, 7, // tsf5ts7_id (771 downto 764)
      795 + 210, 8, // tsf5ts7_rt (763 downto 755)
      786 + 210, 1, // tsf5ts7_lr (754 downto 753)
      784 + 210, 1, // tsf5ts7_pr (752 downto 751)
      782 + 210, 7, // tsf5ts8_id (750 downto 743)
      774 + 210, 8, // tsf5ts8_rt (742 downto 734)
      765 + 210, 1, // tsf5ts8_lr (733 downto 732)
      763 + 210, 1, // tsf5ts8_pr (731 downto 730)
      761 + 210, 7, // tsf5ts9_id (729 downto 722)
      753 + 210, 8, // tsf5ts9_rt (721 downto 713)
      744 + 210, 1, // tsf5ts9_lr (712 downto 711)
      742 + 210, 1, // tsf5ts9_pr (710 downto 709)
      950, 7, // tsf5ts10_id (1167 downto 1160)
      942, 8, // tsf5ts10_rt (1159 downto 1151)
      933, 1, // tsf5ts10_lr (1150 downto 1149)
      931, 1, // tsf5ts10_pr (1148 downto 1147)
      929, 7, // tsf5ts11_id (1167 downto 1160)
      921, 8, // tsf5ts11_rt (1159 downto 1151)
      912, 1, // tsf5ts11_lr (1150 downto 1149)
      910, 1, // tsf5ts11_pr (1148 downto 1147)
      908, 7, // tsf5ts12_id (1167 downto 1160)
      900, 8, // tsf5ts12_rt (1159 downto 1151)
      891, 1, // tsf5ts12_lr (1150 downto 1149)
      889, 1, // tsf5ts12_pr (1148 downto 1147)
      887, 7, // tsf5ts13_id (1167 downto 1160)
      879, 8, // tsf5ts13_rt (1159 downto 1151)
      870, 1, // tsf5ts13_lr (1150 downto 1149)
      868, 1, // tsf5ts13_pr (1148 downto 1147)
      866, 7, // tsf5ts14_id (1167 downto 1160)
      858, 8, // tsf5ts14_rt (1159 downto 1151)
      849, 1, // tsf5ts14_lr (1150 downto 1149)
      847, 1, // tsf5ts14_pr (1148 downto 1147)
      740 + 105, 8, // tsf7_cc (708 downto 700)
      731 + 105, 7, // tsf7ts0_id (699 downto 692)
      723 + 105, 8, // tsf7ts0_rt (691 downto 683)
      714 + 105, 1, // tsf7ts0_lr (682 downto 681)
      712 + 105, 1, // tsf7ts0_pr (680 downto 679)
      710 + 105, 7, // tsf7ts1_id (678 downto 671)
      702 + 105, 8, // tsf7ts1_rt (670 downto 662)
      693 + 105, 1, // tsf7ts1_lr (661 downto 660)
      691 + 105, 1, // tsf7ts1_pr (659 downto 658)
      689 + 105, 7, // tsf7ts2_id (657 downto 650)
      681 + 105, 8, // tsf7ts2_rt (649 downto 641)
      672 + 105, 1, // tsf7ts2_lr (640 downto 639)
      670 + 105, 1, // tsf7ts2_pr (638 downto 637)
      668 + 105, 7, // tsf7ts3_id (636 downto 629)
      660 + 105, 8, // tsf7ts3_rt (628 downto 620)
      651 + 105, 1, // tsf7ts3_lr (619 downto 618)
      649 + 105, 1, // tsf7ts3_pr (617 downto 616)
      647 + 105, 7, // tsf7ts4_id (615 downto 608)
      639 + 105, 8, // tsf7ts4_rt (607 downto 599)
      630 + 105, 1, // tsf7ts4_lr (598 downto 597)
      628 + 105, 1, // tsf7ts4_pr (596 downto 595)
      626 + 105, 7, // tsf7ts5_id (594 downto 587)
      618 + 105, 8, // tsf7ts5_rt (586 downto 578)
      609 + 105, 1, // tsf7ts5_lr (577 downto 576)
      607 + 105, 1, // tsf7ts5_pr (575 downto 574)
      605 + 105, 7, // tsf7ts6_id (573 downto 566)
      597 + 105, 8, // tsf7ts6_rt (565 downto 557)
      588 + 105, 1, // tsf7ts6_lr (556 downto 555)
      586 + 105, 1, // tsf7ts6_pr (554 downto 553)
      584 + 105, 7, // tsf7ts7_id (552 downto 545)
      576 + 105, 8, // tsf7ts7_rt (544 downto 536)
      567 + 105, 1, // tsf7ts7_lr (535 downto 534)
      565 + 105, 1, // tsf7ts7_pr (533 downto 532)
      563 + 105, 7, // tsf7ts8_id (531 downto 524)
      555 + 105, 8, // tsf7ts8_rt (523 downto 515)
      546 + 105, 1, // tsf7ts8_lr (514 downto 513)
      544 + 105, 1, // tsf7ts8_pr (512 downto 511)
      542 + 105, 7, // tsf7ts9_id (510 downto 503)
      534 + 105, 8, // tsf7ts9_rt (502 downto 494)
      525 + 105, 1, // tsf7ts9_lr (493 downto 492)
      523 + 105, 1, // tsf7ts9_pr (491 downto 490)
      626, 7, // tsf7ts10_id (1167 downto 1160)
      618, 8, // tsf7ts10_rt (1159 downto 1151)
      609, 1, // tsf7ts10_lr (1150 downto 1149)
      607, 1, // tsf7ts10_pr (1148 downto 1147)
      605, 7, // tsf7ts11_id (1167 downto 1160)
      597, 8, // tsf7ts11_rt (1159 downto 1151)
      588, 1, // tsf7ts11_lr (1150 downto 1149)
      586, 1, // tsf7ts11_pr (1148 downto 1147)
      584, 7, // tsf7ts12_id (1167 downto 1160)
      576, 8, // tsf7ts12_rt (1159 downto 1151)
      567, 1, // tsf7ts12_lr (1150 downto 1149)
      565, 1, // tsf7ts12_pr (1148 downto 1147)
      563, 7, // tsf7ts13_id (1167 downto 1160)
      555, 8, // tsf7ts13_rt (1159 downto 1151)
      546, 1, // tsf7ts13_lr (1150 downto 1149)
      544, 1, // tsf7ts13_pr (1148 downto 1147)
      542, 7, // tsf7ts14_id (1167 downto 1160)
      534, 8, // tsf7ts14_rt (1159 downto 1151)
      525, 1, // tsf7ts14_lr (1150 downto 1149)
      523, 1, // tsf7ts14_pr (1148 downto 1147)
      521, 8, // t2d_cc (489 downto 481)
      512, 5, // t2d_fnf (480 downto 475)
      506, 1, // t2d0_charge (474 downto 473)
      504, 6, // t2d0_rho_s (472 downto 466)
      497, 6, // t2d0_phi (465 downto 459)
      490, 7, // t2d0ts0_id (458 downto 451)
      482, 8, // t2d0ts0_rt (450 downto 442)
      473, 1, // t2d0ts0_lr (441 downto 440)
      471, 1, // t2d0ts0_pr (439 downto 438)
      469, 7, // t2d0ts2_id (437 downto 430)
      461, 8, // t2d0ts2_rt (429 downto 421)
      452, 1, // t2d0ts2_lr (420 downto 419)
      450, 1, // t2d0ts2_pr (418 downto 417)
      448, 7, // t2d0ts4_id (416 downto 409)
      440, 8, // t2d0ts4_rt (408 downto 400)
      431, 1, // t2d0ts4_lr (399 downto 398)
      429, 1, // t2d0ts4_pr (397 downto 396)
      427, 7, // t2d0ts6_id (395 downto 388)
      419, 8, // t2d0ts6_rt (387 downto 379)
      410, 1, // t2d0ts6_lr (378 downto 377)
      408, 1, // t2d0ts6_pr (376 downto 375)
      406, 7, // t2d0ts8_id (374 downto 367)
      398, 8, // t2d0ts8_rt (366 downto 358)
      389, 1, // t2d0ts8_lr (357 downto 356)
      387, 1, // t2d0ts8_pr (355 downto 354)
      385, 1, // t2d1_charge (353 downto 352)
      383, 6, // t2d1_rho_s (351 downto 345)
      376, 6, // t2d1_phi (344 downto 338)
      369, 7, // t2d1ts0_id (337 downto 330)
      361, 8, // t2d1ts0_rt (329 downto 321)
      352, 1, // t2d1ts0_lr (320 downto 319)
      350, 1, // t2d1ts0_pr (318 downto 317)
      348, 7, // t2d1ts2_id (316 downto 309)
      340, 8, // t2d1ts2_rt (308 downto 300)
      331, 1, // t2d1ts2_lr (299 downto 298)
      329, 1, // t2d1ts2_pr (297 downto 296)
      327, 7, // t2d1ts4_id (295 downto 288)
      319, 8, // t2d1ts4_rt (287 downto 279)
      310, 1, // t2d1ts4_lr (278 downto 277)
      308, 1, // t2d1ts4_pr (276 downto 275)
      306, 7, // t2d1ts6_id (274 downto 267)
      298, 8, // t2d1ts6_rt (266 downto 258)
      289, 1, // t2d1ts6_lr (257 downto 256)
      287, 1, // t2d1ts6_pr (255 downto 254)
      285, 7, // t2d1ts8_id (253 downto 246)
      277, 8, // t2d1ts8_rt (245 downto 237)
      268, 1, // t2d1ts8_lr (236 downto 235)
      266, 1, // t2d1ts8_pr (234 downto 233)
      264, 1, // t2d2_charge (232 downto 231)
      262, 6, // t2d2_rho_s (230 downto 224)
      255, 6, // t2d2_phi (223 downto 217)
      248, 7, // t2d2ts0_id (216 downto 209)
      240, 8, // t2d2ts0_rt (208 downto 200)
      231, 1, // t2d2ts0_lr (199 downto 198)
      229, 1, // t2d2ts0_pr (197 downto 196)
      227, 7, // t2d2ts2_id (195 downto 188)
      219, 8, // t2d2ts2_rt (187 downto 179)
      210, 1, // t2d2ts2_lr (178 downto 177)
      208, 1, // t2d2ts2_pr (176 downto 175)
      206, 7, // t2d2ts4_id (174 downto 167)
      198, 8, // t2d2ts4_rt (166 downto 158)
      189, 1, // t2d2ts4_lr (157 downto 156)
      187, 1, // t2d2ts4_pr (155 downto 154)
      185, 7, // t2d2ts6_id (153 downto 146)
      177, 8, // t2d2ts6_rt (145 downto 137)
      168, 1, // t2d2ts6_lr (136 downto 135)
      166, 1, // t2d2ts6_pr (134 downto 133)
      164, 7, // t2d2ts8_id (132 downto 125)
      156, 8, // t2d2ts8_rt (124 downto 116)
      147, 1, // t2d2ts8_lr (115 downto 114)
      145, 1, // t2d2ts8_pr (113 downto 112)
      143, 1, // t2d3_charge (111 downto 110)
      141, 6, // t2d3_rho_s (109 downto 103)
      134, 6, // t2d3_phi (102 downto 96)
      127, 7, // t2d3ts0_id (95 downto 88)
      119, 8, // t2d3ts0_rt (87 downto 79)
      110, 1, // t2d3ts0_lr (78 downto 77)
      108, 1, // t2d3ts0_pr (76 downto 75)
      106, 7, // t2d3ts2_id (74 downto 67)
      98, 8, // t2d3ts2_rt (66 downto 58)
      89, 1, // t2d3ts2_lr (57 downto 56)
      87, 1, // t2d3ts2_pr (55 downto 54)
      85, 7, // t2d3ts4_id (53 downto 46)
      77, 8, // t2d3ts4_rt (45 downto 37)
      68, 1, // t2d3ts4_lr (36 downto 35)
      66, 1, // t2d3ts4_pr (34 downto 33)
      64, 7, // t2d3ts6_id (32 downto 25)
      56, 8, // t2d3ts6_rt (24 downto 16)
      47, 1, // t2d3ts6_lr (15 downto 14)
      45, 1, // t2d3ts6_pr (13 downto 12)
      43, 7, // t2d3ts8_id (11 downto 4)
      35, 8, // t2d3ts8_rt
      26, 1, // t2d3ts8_lr
      24, 1, // t2d3ts8_pr
      22, 3, // etf_thresh
      18, 8, // etf_cc
      9, 8, // etf_t0
      0, 0  // etf_valid
    };

    /** bus bit map. for 2D fitter (a downto a-b) **/
    const int BitMap_2dfitter[nLeafs_2dfitter][2] = {
      /** the numbers in the () are still old, add 32 of them to get the correct ones **/
      1746, 5, // t3d_2doldtrk (1714 downto 1709)
      1740, 8, // t3dtrk0_evtTime_delay (1708 downto 1700)
      1731, 0, // t3dtrk0_evtTimeValid_delay (1699 downto 1699)
      1730, 0, // t3dtrk0_charge (1698 downto 1698)
      1729, 10, // t3dtrk0_rho (1697 downto 1687)
      1718, 12, // t3dtrk0_phi0 (1686 downto 1674)
      1705, 7, // t3dtrk0ts0_id (1673 downto 1666)
      1697, 1, // t3dtrk0ts0_lr (1665 downto 1664)
      1695, 8, // t3dtrk0ts0_rt (1663 downto 1655)
      1686, 7, // t3dtrk0ts1_id (1654 downto 1647)
      1678, 1, // t3dtrk0ts1_lr (1646 downto 1645)
      1676, 8, // t3dtrk0ts1_rt (1644 downto 1636)
      1667, 8, // t3dtrk0ts2_id (1635 downto 1627)
      1658, 1, // t3dtrk0ts2_lr (1626 downto 1625)
      1656, 8, // t3dtrk0ts2_rt (1624 downto 1616)
      1647, 8, // t3dtrk0ts3_id (1615 downto 1607)
      1638, 1, // t3dtrk0ts3_lr (1606 downto 1605)
      1636, 8, // t3dtrk0ts3_rt (1604 downto 1596)
      1627, 8, // t3dtrk1_evtTime_delay (1595 downto 1587)
      1618, 0, // t3dtrk1_evtTimeValid_delay (1586 downto 1586)
      1617, 0, // t3dtrk1_charge (1585 downto 1585)
      1616, 10, // t3dtrk1_rho (1584 downto 1574)
      1605, 12, // t3dtrk1_phi0 (1573 downto 1561)
      1592, 7, // t3dtrk1ts0_id (1560 downto 1553)
      1584, 1, // t3dtrk1ts0_lr (1552 downto 1551)
      1582, 8, // t3dtrk1ts0_rt (1550 downto 1542)
      1573, 7, // t3dtrk1ts1_id (1541 downto 1534)
      1565, 1, // t3dtrk1ts1_lr (1533 downto 1532)
      1563, 8, // t3dtrk1ts1_rt (1531 downto 1523)
      1554, 8, // t3dtrk1ts2_id (1522 downto 1514)
      1545, 1, // t3dtrk1ts2_lr (1513 downto 1512)
      1543, 8, // t3dtrk1ts2_rt (1511 downto 1503)
      1534, 8, // t3dtrk1ts3_id (1502 downto 1494)
      1525, 1, // t3dtrk1ts3_lr (1493 downto 1492)
      1523, 8, // t3dtrk1ts3_rt (1491 downto 1483)
      1514, 28, // t2dfittertrk0_rho
      1485, 17, // t2dfittertrk0_phi0
      1467, 28, // t2dfittertrk1_rho
      1438, 17, // t2dfittertrk1_phi0
      1420, 28, // t2dfittertrk2_rho
      1391, 17, // t2dfittertrk2_phi0
      1373, 28, // t2dfittertrk3_rho
      1344, 17, // t2dfittertrk3_phi0
      521, 8, // t2d_cc (489 downto 481)
      512, 5, // t2d_fnf (480 downto 475)
      506, 1, // t2d0_charge (474 downto 473)
      504, 6, // t2d0_rho_s (472 downto 466)
      497, 6, // t2d0_phi (465 downto 459)
      490, 7, // t2d0ts0_id (458 downto 451)
      482, 8, // t2d0ts0_rt (450 downto 442)
      473, 1, // t2d0ts0_lr (441 downto 440)
      471, 1, // t2d0ts0_pr (439 downto 438)
      469, 7, // t2d0ts2_id (437 downto 430)
      461, 8, // t2d0ts2_rt (429 downto 421)
      452, 1, // t2d0ts2_lr (420 downto 419)
      450, 1, // t2d0ts2_pr (418 downto 417)
      448, 7, // t2d0ts4_id (416 downto 409)
      440, 8, // t2d0ts4_rt (408 downto 400)
      431, 1, // t2d0ts4_lr (399 downto 398)
      429, 1, // t2d0ts4_pr (397 downto 396)
      427, 7, // t2d0ts6_id (395 downto 388)
      419, 8, // t2d0ts6_rt (387 downto 379)
      410, 1, // t2d0ts6_lr (378 downto 377)
      408, 1, // t2d0ts6_pr (376 downto 375)
      406, 7, // t2d0ts8_id (374 downto 367)
      398, 8, // t2d0ts8_rt (366 downto 358)
      389, 1, // t2d0ts8_lr (357 downto 356)
      387, 1, // t2d0ts8_pr (355 downto 354)
      385, 1, // t2d1_charge (353 downto 352)
      383, 6, // t2d1_rho_s (351 downto 345)
      376, 6, // t2d1_phi (344 downto 338)
      369, 7, // t2d1ts0_id (337 downto 330)
      361, 8, // t2d1ts0_rt (329 downto 321)
      352, 1, // t2d1ts0_lr (320 downto 319)
      350, 1, // t2d1ts0_pr (318 downto 317)
      348, 7, // t2d1ts2_id (316 downto 309)
      340, 8, // t2d1ts2_rt (308 downto 300)
      331, 1, // t2d1ts2_lr (299 downto 298)
      329, 1, // t2d1ts2_pr (297 downto 296)
      327, 7, // t2d1ts4_id (295 downto 288)
      319, 8, // t2d1ts4_rt (287 downto 279)
      310, 1, // t2d1ts4_lr (278 downto 277)
      308, 1, // t2d1ts4_pr (276 downto 275)
      306, 7, // t2d1ts6_id (274 downto 267)
      298, 8, // t2d1ts6_rt (266 downto 258)
      289, 1, // t2d1ts6_lr (257 downto 256)
      287, 1, // t2d1ts6_pr (255 downto 254)
      285, 7, // t2d1ts8_id (253 downto 246)
      277, 8, // t2d1ts8_rt (245 downto 237)
      268, 1, // t2d1ts8_lr (236 downto 235)
      266, 1, // t2d1ts8_pr (234 downto 233)
      264, 1, // t2d2_charge (232 downto 231)
      262, 6, // t2d2_rho_s (230 downto 224)
      255, 6, // t2d2_phi (223 downto 217)
      248, 7, // t2d2ts0_id (216 downto 209)
      240, 8, // t2d2ts0_rt (208 downto 200)
      231, 1, // t2d2ts0_lr (199 downto 198)
      229, 1, // t2d2ts0_pr (197 downto 196)
      227, 7, // t2d2ts2_id (195 downto 188)
      219, 8, // t2d2ts2_rt (187 downto 179)
      210, 1, // t2d2ts2_lr (178 downto 177)
      208, 1, // t2d2ts2_pr (176 downto 175)
      206, 7, // t2d2ts4_id (174 downto 167)
      198, 8, // t2d2ts4_rt (166 downto 158)
      189, 1, // t2d2ts4_lr (157 downto 156)
      187, 1, // t2d2ts4_pr (155 downto 154)
      185, 7, // t2d2ts6_id (153 downto 146)
      177, 8, // t2d2ts6_rt (145 downto 137)
      168, 1, // t2d2ts6_lr (136 downto 135)
      166, 1, // t2d2ts6_pr (134 downto 133)
      164, 7, // t2d2ts8_id (132 downto 125)
      156, 8, // t2d2ts8_rt (124 downto 116)
      147, 1, // t2d2ts8_lr (115 downto 114)
      145, 1, // t2d2ts8_pr (113 downto 112)
      143, 1, // t2d3_charge (111 downto 110)
      141, 6, // t2d3_rho_s (109 downto 103)
      134, 6, // t2d3_phi (102 downto 96)
      127, 7, // t2d3ts0_id (95 downto 88)
      119, 8, // t2d3ts0_rt (87 downto 79)
      110, 1, // t2d3ts0_lr (78 downto 77)
      108, 1, // t2d3ts0_pr (76 downto 75)
      106, 7, // t2d3ts2_id (74 downto 67)
      98, 8, // t2d3ts2_rt (66 downto 58)
      89, 1, // t2d3ts2_lr (57 downto 56)
      87, 1, // t2d3ts2_pr (55 downto 54)
      85, 7, // t2d3ts4_id (53 downto 46)
      77, 8, // t2d3ts4_rt (45 downto 37)
      68, 1, // t2d3ts4_lr (36 downto 35)
      66, 1, // t2d3ts4_pr (34 downto 33)
      64, 7, // t2d3ts6_id (32 downto 25)
      56, 8, // t2d3ts6_rt (24 downto 16)
      47, 1, // t2d3ts6_lr (15 downto 14)
      45, 1, // t2d3ts6_pr (13 downto 12)
      43, 7, // t2d3ts8_id (11 downto 4)
      35, 8, // t2d3ts8_rt
      26, 1, // t2d3ts8_lr
      24, 1, // t2d3ts8_pr
      22, 3, // etf_thresh
      18, 8, // etf_cc
      9, 8, // etf_t0
      0, 0  // etf_valid
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
      bitArray[e_t2d3ts8_rt    ] = &(store->m_t2d3ts6_rt);
      bitArray[e_t2d3ts8_lr    ] = &(store->m_t2d3ts6_lr);
      bitArray[e_t2d3ts8_pr    ] = &(store->m_t2d3ts6_pr);
      bitArray[e_etf_thresh    ] = &(store->m_etf_thresh);
      bitArray[e_etf_cc        ] = &(store->m_etf_cc);
      bitArray[e_etf_t0        ] = &(store->m_etf_t0);
      bitArray[e_etf_valid     ] = &(store->m_etf_valid);
      bitArray[e_evt     ] = &(store->m_evt);
      bitArray[e_clk     ] = &(store->m_clk);
      bitArray[e_firmid  ] = &(store->m_firmid);
      bitArray[e_firmver ] = &(store->m_firmver);
    }

    void
    setLeafPointersArray_2624(TRGCDCT3DUnpackerStore* store, int** bitArray)
    {
      bitArray[e1_t3d_2doldtrk    ] = &(store->m_t3d_2doldtrk);
      bitArray[e1_t3dtrk0_evtTime_delay    ] = &(store->m_t3dtrk0_evtTime_delay);
      bitArray[e1_t3dtrk0_evtTimeValid_delay    ] = &(store->m_t3dtrk0_evtTimeValid_delay);
      bitArray[e1_t3dtrk0_charge    ] = &(store->m_t3dtrk0_charge);
      bitArray[e1_t3dtrk0_rho    ] = &(store->m_t3dtrk0_rho);
      bitArray[e1_t3dtrk0_phi0    ] = &(store->m_t3dtrk0_phi0);
      bitArray[e1_t3dtrk0ts0_id    ] = &(store->m_t3dtrk0ts0_id);
      bitArray[e1_t3dtrk0ts0_lr    ] = &(store->m_t3dtrk0ts0_lr);
      bitArray[e1_t3dtrk0ts0_rt    ] = &(store->m_t3dtrk0ts0_rt);
      bitArray[e1_t3dtrk0ts1_id    ] = &(store->m_t3dtrk0ts1_id);
      bitArray[e1_t3dtrk0ts1_lr    ] = &(store->m_t3dtrk0ts1_lr);
      bitArray[e1_t3dtrk0ts1_rt    ] = &(store->m_t3dtrk0ts1_rt);
      bitArray[e1_t3dtrk0ts2_id    ] = &(store->m_t3dtrk0ts2_id);
      bitArray[e1_t3dtrk0ts2_lr    ] = &(store->m_t3dtrk0ts2_lr);
      bitArray[e1_t3dtrk0ts2_rt    ] = &(store->m_t3dtrk0ts2_rt);
      bitArray[e1_t3dtrk0ts3_id    ] = &(store->m_t3dtrk0ts3_id);
      bitArray[e1_t3dtrk0ts3_lr    ] = &(store->m_t3dtrk0ts3_lr);
      bitArray[e1_t3dtrk0ts3_rt    ] = &(store->m_t3dtrk0ts3_rt);
      bitArray[e1_t3dtrk1_evtTime_delay    ] = &(store->m_t3dtrk1_evtTime_delay);
      bitArray[e1_t3dtrk1_evtTimeValid_delay    ] = &(store->m_t3dtrk1_evtTimeValid_delay);
      bitArray[e1_t3dtrk1_charge    ] = &(store->m_t3dtrk1_charge);
      bitArray[e1_t3dtrk1_rho    ] = &(store->m_t3dtrk1_rho);
      bitArray[e1_t3dtrk1_phi0    ] = &(store->m_t3dtrk1_phi0);
      bitArray[e1_t3dtrk1ts0_id    ] = &(store->m_t3dtrk1ts0_id);
      bitArray[e1_t3dtrk1ts0_lr    ] = &(store->m_t3dtrk1ts0_lr);
      bitArray[e1_t3dtrk1ts0_rt    ] = &(store->m_t3dtrk1ts0_rt);
      bitArray[e1_t3dtrk1ts1_id    ] = &(store->m_t3dtrk1ts1_id);
      bitArray[e1_t3dtrk1ts1_lr    ] = &(store->m_t3dtrk1ts1_lr);
      bitArray[e1_t3dtrk1ts1_rt    ] = &(store->m_t3dtrk1ts1_rt);
      bitArray[e1_t3dtrk1ts2_id    ] = &(store->m_t3dtrk1ts2_id);
      bitArray[e1_t3dtrk1ts2_lr    ] = &(store->m_t3dtrk1ts2_lr);
      bitArray[e1_t3dtrk1ts2_rt    ] = &(store->m_t3dtrk1ts2_rt);
      bitArray[e1_t3dtrk1ts3_id    ] = &(store->m_t3dtrk1ts3_id);
      bitArray[e1_t3dtrk1ts3_lr    ] = &(store->m_t3dtrk1ts3_lr);
      bitArray[e1_t3dtrk1ts3_rt    ] = &(store->m_t3dtrk1ts3_rt);
      bitArray[e1_t3d_phase    ] = &(store->m_t3d_phase);
      bitArray[e1_t3d_validTS    ] = &(store->m_t3d_validTS);
      bitArray[e1_t3d_2dfnf    ] = &(store->m_t3d_2dfnf);
      bitArray[e1_t3dtrk0_z0_s    ] = &(store->m_t3dtrk0_z0_s);
      bitArray[e1_t3dtrk0_cot_s    ] = &(store->m_t3dtrk0_cot_s);
      bitArray[e1_t3dtrk0_zchisq    ] = &(store->m_t3dtrk0_zchisq);
      bitArray[e1_t3dtrk1_z0_s    ] = &(store->m_t3dtrk1_z0_s);
      bitArray[e1_t3dtrk1_cot_s    ] = &(store->m_t3dtrk1_cot_s);
      bitArray[e1_t3dtrk1_zchisq    ] = &(store->m_t3dtrk1_zchisq);
      bitArray[e1_t3dtrk2_z0_s    ] = &(store->m_t3dtrk2_z0_s);
      bitArray[e1_t3dtrk2_cot_s    ] = &(store->m_t3dtrk2_cot_s);
      bitArray[e1_t3dtrk2_zchisq    ] = &(store->m_t3dtrk2_zchisq);
      bitArray[e1_t3dtrk3_z0_s    ] = &(store->m_t3dtrk3_z0_s);
      bitArray[e1_t3dtrk3_cot_s    ] = &(store->m_t3dtrk3_cot_s);
      bitArray[e1_t3dtrk3_zchisq    ] = &(store->m_t3dtrk3_zchisq);
      bitArray[e1_t3dtrk_rd_req    ] = &(store->m_t3dtrk_rd_req);
      bitArray[e1_tsf1_cc    ] = &(store->m_tsf1_cc);
      bitArray[e1_tsf1ts0_id    ] = &(store->m_tsf1ts0_id);
      bitArray[e1_tsf1ts0_rt    ] = &(store->m_tsf1ts0_rt);
      bitArray[e1_tsf1ts0_lr    ] = &(store->m_tsf1ts0_lr);
      bitArray[e1_tsf1ts0_pr    ] = &(store->m_tsf1ts0_pr);
      bitArray[e1_tsf1ts1_id    ] = &(store->m_tsf1ts1_id);
      bitArray[e1_tsf1ts1_rt    ] = &(store->m_tsf1ts1_rt);
      bitArray[e1_tsf1ts1_lr    ] = &(store->m_tsf1ts1_lr);
      bitArray[e1_tsf1ts1_pr    ] = &(store->m_tsf1ts1_pr);
      bitArray[e1_tsf1ts2_id    ] = &(store->m_tsf1ts2_id);
      bitArray[e1_tsf1ts2_rt    ] = &(store->m_tsf1ts2_rt);
      bitArray[e1_tsf1ts2_lr    ] = &(store->m_tsf1ts2_lr);
      bitArray[e1_tsf1ts2_pr    ] = &(store->m_tsf1ts2_pr);
      bitArray[e1_tsf1ts3_id    ] = &(store->m_tsf1ts3_id);
      bitArray[e1_tsf1ts3_rt    ] = &(store->m_tsf1ts3_rt);
      bitArray[e1_tsf1ts3_lr    ] = &(store->m_tsf1ts3_lr);
      bitArray[e1_tsf1ts3_pr    ] = &(store->m_tsf1ts3_pr);
      bitArray[e1_tsf1ts4_id    ] = &(store->m_tsf1ts4_id);
      bitArray[e1_tsf1ts4_rt    ] = &(store->m_tsf1ts4_rt);
      bitArray[e1_tsf1ts4_lr    ] = &(store->m_tsf1ts4_lr);
      bitArray[e1_tsf1ts4_pr    ] = &(store->m_tsf1ts4_pr);
      bitArray[e1_tsf1ts5_id    ] = &(store->m_tsf1ts5_id);
      bitArray[e1_tsf1ts5_rt    ] = &(store->m_tsf1ts5_rt);
      bitArray[e1_tsf1ts5_lr    ] = &(store->m_tsf1ts5_lr);
      bitArray[e1_tsf1ts5_pr    ] = &(store->m_tsf1ts5_pr);
      bitArray[e1_tsf1ts6_id    ] = &(store->m_tsf1ts6_id);
      bitArray[e1_tsf1ts6_rt    ] = &(store->m_tsf1ts6_rt);
      bitArray[e1_tsf1ts6_lr    ] = &(store->m_tsf1ts6_lr);
      bitArray[e1_tsf1ts6_pr    ] = &(store->m_tsf1ts6_pr);
      bitArray[e1_tsf1ts7_id    ] = &(store->m_tsf1ts7_id);
      bitArray[e1_tsf1ts7_rt    ] = &(store->m_tsf1ts7_rt);
      bitArray[e1_tsf1ts7_lr    ] = &(store->m_tsf1ts7_lr);
      bitArray[e1_tsf1ts7_pr    ] = &(store->m_tsf1ts7_pr);
      bitArray[e1_tsf1ts8_id    ] = &(store->m_tsf1ts8_id);
      bitArray[e1_tsf1ts8_rt    ] = &(store->m_tsf1ts8_rt);
      bitArray[e1_tsf1ts8_lr    ] = &(store->m_tsf1ts8_lr);
      bitArray[e1_tsf1ts8_pr    ] = &(store->m_tsf1ts8_pr);
      bitArray[e1_tsf1ts9_id    ] = &(store->m_tsf1ts9_id);
      bitArray[e1_tsf1ts9_rt    ] = &(store->m_tsf1ts9_rt);
      bitArray[e1_tsf1ts9_lr    ] = &(store->m_tsf1ts9_lr);
      bitArray[e1_tsf1ts9_pr    ] = &(store->m_tsf1ts9_pr);
      bitArray[e1_tsf1ts10_id    ] = &(store->m_tsf1ts10_id);
      bitArray[e1_tsf1ts10_rt    ] = &(store->m_tsf1ts10_rt);
      bitArray[e1_tsf1ts10_lr    ] = &(store->m_tsf1ts10_lr);
      bitArray[e1_tsf1ts10_pr    ] = &(store->m_tsf1ts10_pr);
      bitArray[e1_tsf1ts11_id    ] = &(store->m_tsf1ts11_id);
      bitArray[e1_tsf1ts11_rt    ] = &(store->m_tsf1ts11_rt);
      bitArray[e1_tsf1ts11_lr    ] = &(store->m_tsf1ts11_lr);
      bitArray[e1_tsf1ts11_pr    ] = &(store->m_tsf1ts11_pr);
      bitArray[e1_tsf1ts12_id    ] = &(store->m_tsf1ts12_id);
      bitArray[e1_tsf1ts12_rt    ] = &(store->m_tsf1ts12_rt);
      bitArray[e1_tsf1ts12_lr    ] = &(store->m_tsf1ts12_lr);
      bitArray[e1_tsf1ts12_pr    ] = &(store->m_tsf1ts12_pr);
      bitArray[e1_tsf1ts13_id    ] = &(store->m_tsf1ts13_id);
      bitArray[e1_tsf1ts13_rt    ] = &(store->m_tsf1ts13_rt);
      bitArray[e1_tsf1ts13_lr    ] = &(store->m_tsf1ts13_lr);
      bitArray[e1_tsf1ts13_pr    ] = &(store->m_tsf1ts13_pr);
      bitArray[e1_tsf1ts14_id    ] = &(store->m_tsf1ts14_id);
      bitArray[e1_tsf1ts14_rt    ] = &(store->m_tsf1ts14_rt);
      bitArray[e1_tsf1ts14_lr    ] = &(store->m_tsf1ts14_lr);
      bitArray[e1_tsf1ts14_pr    ] = &(store->m_tsf1ts14_pr);
      bitArray[e1_tsf3_cc    ] = &(store->m_tsf3_cc);
      bitArray[e1_tsf3ts0_id    ] = &(store->m_tsf3ts0_id);
      bitArray[e1_tsf3ts0_rt    ] = &(store->m_tsf3ts0_rt);
      bitArray[e1_tsf3ts0_lr    ] = &(store->m_tsf3ts0_lr);
      bitArray[e1_tsf3ts0_pr    ] = &(store->m_tsf3ts0_pr);
      bitArray[e1_tsf3ts1_id    ] = &(store->m_tsf3ts1_id);
      bitArray[e1_tsf3ts1_rt    ] = &(store->m_tsf3ts1_rt);
      bitArray[e1_tsf3ts1_lr    ] = &(store->m_tsf3ts1_lr);
      bitArray[e1_tsf3ts1_pr    ] = &(store->m_tsf3ts1_pr);
      bitArray[e1_tsf3ts2_id    ] = &(store->m_tsf3ts2_id);
      bitArray[e1_tsf3ts2_rt    ] = &(store->m_tsf3ts2_rt);
      bitArray[e1_tsf3ts2_lr    ] = &(store->m_tsf3ts2_lr);
      bitArray[e1_tsf3ts2_pr    ] = &(store->m_tsf3ts2_pr);
      bitArray[e1_tsf3ts3_id    ] = &(store->m_tsf3ts3_id);
      bitArray[e1_tsf3ts3_rt    ] = &(store->m_tsf3ts3_rt);
      bitArray[e1_tsf3ts3_lr    ] = &(store->m_tsf3ts3_lr);
      bitArray[e1_tsf3ts3_pr    ] = &(store->m_tsf3ts3_pr);
      bitArray[e1_tsf3ts4_id    ] = &(store->m_tsf3ts4_id);
      bitArray[e1_tsf3ts4_rt    ] = &(store->m_tsf3ts4_rt);
      bitArray[e1_tsf3ts4_lr    ] = &(store->m_tsf3ts4_lr);
      bitArray[e1_tsf3ts4_pr    ] = &(store->m_tsf3ts4_pr);
      bitArray[e1_tsf3ts5_id    ] = &(store->m_tsf3ts5_id);
      bitArray[e1_tsf3ts5_rt    ] = &(store->m_tsf3ts5_rt);
      bitArray[e1_tsf3ts5_lr    ] = &(store->m_tsf3ts5_lr);
      bitArray[e1_tsf3ts5_pr    ] = &(store->m_tsf3ts5_pr);
      bitArray[e1_tsf3ts6_id    ] = &(store->m_tsf3ts6_id);
      bitArray[e1_tsf3ts6_rt    ] = &(store->m_tsf3ts6_rt);
      bitArray[e1_tsf3ts6_lr    ] = &(store->m_tsf3ts6_lr);
      bitArray[e1_tsf3ts6_pr    ] = &(store->m_tsf3ts6_pr);
      bitArray[e1_tsf3ts7_id    ] = &(store->m_tsf3ts7_id);
      bitArray[e1_tsf3ts7_rt    ] = &(store->m_tsf3ts7_rt);
      bitArray[e1_tsf3ts7_lr    ] = &(store->m_tsf3ts7_lr);
      bitArray[e1_tsf3ts7_pr    ] = &(store->m_tsf3ts7_pr);
      bitArray[e1_tsf3ts8_id    ] = &(store->m_tsf3ts8_id);
      bitArray[e1_tsf3ts8_rt    ] = &(store->m_tsf3ts8_rt);
      bitArray[e1_tsf3ts8_lr    ] = &(store->m_tsf3ts8_lr);
      bitArray[e1_tsf3ts8_pr    ] = &(store->m_tsf3ts8_pr);
      bitArray[e1_tsf3ts9_id    ] = &(store->m_tsf3ts9_id);
      bitArray[e1_tsf3ts9_rt    ] = &(store->m_tsf3ts9_rt);
      bitArray[e1_tsf3ts9_lr    ] = &(store->m_tsf3ts9_lr);
      bitArray[e1_tsf3ts9_pr    ] = &(store->m_tsf3ts9_pr);
      bitArray[e1_tsf3ts10_id    ] = &(store->m_tsf3ts10_id);
      bitArray[e1_tsf3ts10_rt    ] = &(store->m_tsf3ts10_rt);
      bitArray[e1_tsf3ts10_lr    ] = &(store->m_tsf3ts10_lr);
      bitArray[e1_tsf3ts10_pr    ] = &(store->m_tsf3ts10_pr);
      bitArray[e1_tsf3ts11_id    ] = &(store->m_tsf3ts11_id);
      bitArray[e1_tsf3ts11_rt    ] = &(store->m_tsf3ts11_rt);
      bitArray[e1_tsf3ts11_lr    ] = &(store->m_tsf3ts11_lr);
      bitArray[e1_tsf3ts11_pr    ] = &(store->m_tsf3ts11_pr);
      bitArray[e1_tsf3ts12_id    ] = &(store->m_tsf3ts12_id);
      bitArray[e1_tsf3ts12_rt    ] = &(store->m_tsf3ts12_rt);
      bitArray[e1_tsf3ts12_lr    ] = &(store->m_tsf3ts12_lr);
      bitArray[e1_tsf3ts12_pr    ] = &(store->m_tsf3ts12_pr);
      bitArray[e1_tsf3ts13_id    ] = &(store->m_tsf3ts13_id);
      bitArray[e1_tsf3ts13_rt    ] = &(store->m_tsf3ts13_rt);
      bitArray[e1_tsf3ts13_lr    ] = &(store->m_tsf3ts13_lr);
      bitArray[e1_tsf3ts13_pr    ] = &(store->m_tsf3ts13_pr);
      bitArray[e1_tsf3ts14_id    ] = &(store->m_tsf3ts14_id);
      bitArray[e1_tsf3ts14_rt    ] = &(store->m_tsf3ts14_rt);
      bitArray[e1_tsf3ts14_lr    ] = &(store->m_tsf3ts14_lr);
      bitArray[e1_tsf3ts14_pr    ] = &(store->m_tsf3ts14_pr);
      bitArray[e1_tsf5_cc    ] = &(store->m_tsf5_cc);
      bitArray[e1_tsf5ts0_id    ] = &(store->m_tsf5ts0_id);
      bitArray[e1_tsf5ts0_rt    ] = &(store->m_tsf5ts0_rt);
      bitArray[e1_tsf5ts0_lr    ] = &(store->m_tsf5ts0_lr);
      bitArray[e1_tsf5ts0_pr    ] = &(store->m_tsf5ts0_pr);
      bitArray[e1_tsf5ts1_id    ] = &(store->m_tsf5ts1_id);
      bitArray[e1_tsf5ts1_rt    ] = &(store->m_tsf5ts1_rt);
      bitArray[e1_tsf5ts1_lr    ] = &(store->m_tsf5ts1_lr);
      bitArray[e1_tsf5ts1_pr    ] = &(store->m_tsf5ts1_pr);
      bitArray[e1_tsf5ts2_id    ] = &(store->m_tsf5ts2_id);
      bitArray[e1_tsf5ts2_rt    ] = &(store->m_tsf5ts2_rt);
      bitArray[e1_tsf5ts2_lr    ] = &(store->m_tsf5ts2_lr);
      bitArray[e1_tsf5ts2_pr    ] = &(store->m_tsf5ts2_pr);
      bitArray[e1_tsf5ts3_id    ] = &(store->m_tsf5ts3_id);
      bitArray[e1_tsf5ts3_rt    ] = &(store->m_tsf5ts3_rt);
      bitArray[e1_tsf5ts3_lr    ] = &(store->m_tsf5ts3_lr);
      bitArray[e1_tsf5ts3_pr    ] = &(store->m_tsf5ts3_pr);
      bitArray[e1_tsf5ts4_id    ] = &(store->m_tsf5ts4_id);
      bitArray[e1_tsf5ts4_rt    ] = &(store->m_tsf5ts4_rt);
      bitArray[e1_tsf5ts4_lr    ] = &(store->m_tsf5ts4_lr);
      bitArray[e1_tsf5ts4_pr    ] = &(store->m_tsf5ts4_pr);
      bitArray[e1_tsf5ts5_id    ] = &(store->m_tsf5ts5_id);
      bitArray[e1_tsf5ts5_rt    ] = &(store->m_tsf5ts5_rt);
      bitArray[e1_tsf5ts5_lr    ] = &(store->m_tsf5ts5_lr);
      bitArray[e1_tsf5ts5_pr    ] = &(store->m_tsf5ts5_pr);
      bitArray[e1_tsf5ts6_id    ] = &(store->m_tsf5ts6_id);
      bitArray[e1_tsf5ts6_rt    ] = &(store->m_tsf5ts6_rt);
      bitArray[e1_tsf5ts6_lr    ] = &(store->m_tsf5ts6_lr);
      bitArray[e1_tsf5ts6_pr    ] = &(store->m_tsf5ts6_pr);
      bitArray[e1_tsf5ts7_id    ] = &(store->m_tsf5ts7_id);
      bitArray[e1_tsf5ts7_rt    ] = &(store->m_tsf5ts7_rt);
      bitArray[e1_tsf5ts7_lr    ] = &(store->m_tsf5ts7_lr);
      bitArray[e1_tsf5ts7_pr    ] = &(store->m_tsf5ts7_pr);
      bitArray[e1_tsf5ts8_id    ] = &(store->m_tsf5ts8_id);
      bitArray[e1_tsf5ts8_rt    ] = &(store->m_tsf5ts8_rt);
      bitArray[e1_tsf5ts8_lr    ] = &(store->m_tsf5ts8_lr);
      bitArray[e1_tsf5ts8_pr    ] = &(store->m_tsf5ts8_pr);
      bitArray[e1_tsf5ts9_id    ] = &(store->m_tsf5ts9_id);
      bitArray[e1_tsf5ts9_rt    ] = &(store->m_tsf5ts9_rt);
      bitArray[e1_tsf5ts9_lr    ] = &(store->m_tsf5ts9_lr);
      bitArray[e1_tsf5ts9_pr    ] = &(store->m_tsf5ts9_pr);
      bitArray[e1_tsf5ts10_id    ] = &(store->m_tsf5ts10_id);
      bitArray[e1_tsf5ts10_rt    ] = &(store->m_tsf5ts10_rt);
      bitArray[e1_tsf5ts10_lr    ] = &(store->m_tsf5ts10_lr);
      bitArray[e1_tsf5ts10_pr    ] = &(store->m_tsf5ts10_pr);
      bitArray[e1_tsf5ts11_id    ] = &(store->m_tsf5ts11_id);
      bitArray[e1_tsf5ts11_rt    ] = &(store->m_tsf5ts11_rt);
      bitArray[e1_tsf5ts11_lr    ] = &(store->m_tsf5ts11_lr);
      bitArray[e1_tsf5ts11_pr    ] = &(store->m_tsf5ts11_pr);
      bitArray[e1_tsf5ts12_id    ] = &(store->m_tsf5ts12_id);
      bitArray[e1_tsf5ts12_rt    ] = &(store->m_tsf5ts12_rt);
      bitArray[e1_tsf5ts12_lr    ] = &(store->m_tsf5ts12_lr);
      bitArray[e1_tsf5ts12_pr    ] = &(store->m_tsf5ts12_pr);
      bitArray[e1_tsf5ts13_id    ] = &(store->m_tsf5ts13_id);
      bitArray[e1_tsf5ts13_rt    ] = &(store->m_tsf5ts13_rt);
      bitArray[e1_tsf5ts13_lr    ] = &(store->m_tsf5ts13_lr);
      bitArray[e1_tsf5ts13_pr    ] = &(store->m_tsf5ts13_pr);
      bitArray[e1_tsf5ts14_id    ] = &(store->m_tsf5ts14_id);
      bitArray[e1_tsf5ts14_rt    ] = &(store->m_tsf5ts14_rt);
      bitArray[e1_tsf5ts14_lr    ] = &(store->m_tsf5ts14_lr);
      bitArray[e1_tsf5ts14_pr    ] = &(store->m_tsf5ts14_pr);
      bitArray[e1_tsf7_cc    ] = &(store->m_tsf7_cc);
      bitArray[e1_tsf7ts0_id    ] = &(store->m_tsf7ts0_id);
      bitArray[e1_tsf7ts0_rt    ] = &(store->m_tsf7ts0_rt);
      bitArray[e1_tsf7ts0_lr    ] = &(store->m_tsf7ts0_lr);
      bitArray[e1_tsf7ts0_pr    ] = &(store->m_tsf7ts0_pr);
      bitArray[e1_tsf7ts1_id    ] = &(store->m_tsf7ts1_id);
      bitArray[e1_tsf7ts1_rt    ] = &(store->m_tsf7ts1_rt);
      bitArray[e1_tsf7ts1_lr    ] = &(store->m_tsf7ts1_lr);
      bitArray[e1_tsf7ts1_pr    ] = &(store->m_tsf7ts1_pr);
      bitArray[e1_tsf7ts2_id    ] = &(store->m_tsf7ts2_id);
      bitArray[e1_tsf7ts2_rt    ] = &(store->m_tsf7ts2_rt);
      bitArray[e1_tsf7ts2_lr    ] = &(store->m_tsf7ts2_lr);
      bitArray[e1_tsf7ts2_pr    ] = &(store->m_tsf7ts2_pr);
      bitArray[e1_tsf7ts3_id    ] = &(store->m_tsf7ts3_id);
      bitArray[e1_tsf7ts3_rt    ] = &(store->m_tsf7ts3_rt);
      bitArray[e1_tsf7ts3_lr    ] = &(store->m_tsf7ts3_lr);
      bitArray[e1_tsf7ts3_pr    ] = &(store->m_tsf7ts3_pr);
      bitArray[e1_tsf7ts4_id    ] = &(store->m_tsf7ts4_id);
      bitArray[e1_tsf7ts4_rt    ] = &(store->m_tsf7ts4_rt);
      bitArray[e1_tsf7ts4_lr    ] = &(store->m_tsf7ts4_lr);
      bitArray[e1_tsf7ts4_pr    ] = &(store->m_tsf7ts4_pr);
      bitArray[e1_tsf7ts5_id    ] = &(store->m_tsf7ts5_id);
      bitArray[e1_tsf7ts5_rt    ] = &(store->m_tsf7ts5_rt);
      bitArray[e1_tsf7ts5_lr    ] = &(store->m_tsf7ts5_lr);
      bitArray[e1_tsf7ts5_pr    ] = &(store->m_tsf7ts5_pr);
      bitArray[e1_tsf7ts6_id    ] = &(store->m_tsf7ts6_id);
      bitArray[e1_tsf7ts6_rt    ] = &(store->m_tsf7ts6_rt);
      bitArray[e1_tsf7ts6_lr    ] = &(store->m_tsf7ts6_lr);
      bitArray[e1_tsf7ts6_pr    ] = &(store->m_tsf7ts6_pr);
      bitArray[e1_tsf7ts7_id    ] = &(store->m_tsf7ts7_id);
      bitArray[e1_tsf7ts7_rt    ] = &(store->m_tsf7ts7_rt);
      bitArray[e1_tsf7ts7_lr    ] = &(store->m_tsf7ts7_lr);
      bitArray[e1_tsf7ts7_pr    ] = &(store->m_tsf7ts7_pr);
      bitArray[e1_tsf7ts8_id    ] = &(store->m_tsf7ts8_id);
      bitArray[e1_tsf7ts8_rt    ] = &(store->m_tsf7ts8_rt);
      bitArray[e1_tsf7ts8_lr    ] = &(store->m_tsf7ts8_lr);
      bitArray[e1_tsf7ts8_pr    ] = &(store->m_tsf7ts8_pr);
      bitArray[e1_tsf7ts9_id    ] = &(store->m_tsf7ts9_id);
      bitArray[e1_tsf7ts9_rt    ] = &(store->m_tsf7ts9_rt);
      bitArray[e1_tsf7ts9_lr    ] = &(store->m_tsf7ts9_lr);
      bitArray[e1_tsf7ts9_pr    ] = &(store->m_tsf7ts9_pr);
      bitArray[e1_tsf7ts10_id    ] = &(store->m_tsf7ts10_id);
      bitArray[e1_tsf7ts10_rt    ] = &(store->m_tsf7ts10_rt);
      bitArray[e1_tsf7ts10_lr    ] = &(store->m_tsf7ts10_lr);
      bitArray[e1_tsf7ts10_pr    ] = &(store->m_tsf7ts10_pr);
      bitArray[e1_tsf7ts11_id    ] = &(store->m_tsf7ts11_id);
      bitArray[e1_tsf7ts11_rt    ] = &(store->m_tsf7ts11_rt);
      bitArray[e1_tsf7ts11_lr    ] = &(store->m_tsf7ts11_lr);
      bitArray[e1_tsf7ts11_pr    ] = &(store->m_tsf7ts11_pr);
      bitArray[e1_tsf7ts12_id    ] = &(store->m_tsf7ts12_id);
      bitArray[e1_tsf7ts12_rt    ] = &(store->m_tsf7ts12_rt);
      bitArray[e1_tsf7ts12_lr    ] = &(store->m_tsf7ts12_lr);
      bitArray[e1_tsf7ts12_pr    ] = &(store->m_tsf7ts12_pr);
      bitArray[e1_tsf7ts13_id    ] = &(store->m_tsf7ts13_id);
      bitArray[e1_tsf7ts13_rt    ] = &(store->m_tsf7ts13_rt);
      bitArray[e1_tsf7ts13_lr    ] = &(store->m_tsf7ts13_lr);
      bitArray[e1_tsf7ts13_pr    ] = &(store->m_tsf7ts13_pr);
      bitArray[e1_tsf7ts14_id    ] = &(store->m_tsf7ts14_id);
      bitArray[e1_tsf7ts14_rt    ] = &(store->m_tsf7ts14_rt);
      bitArray[e1_tsf7ts14_lr    ] = &(store->m_tsf7ts14_lr);
      bitArray[e1_tsf7ts14_pr    ] = &(store->m_tsf7ts14_pr);
      bitArray[e1_t2d_cc    ] = &(store->m_t2d_cc);
      bitArray[e1_t2d_fnf    ] = &(store->m_t2d_fnf);
      bitArray[e1_t2d0_charge    ] = &(store->m_t2d0_charge);
      bitArray[e1_t2d0_rho_s    ] = &(store->m_t2d0_rho_s);
      bitArray[e1_t2d0_phi    ] = &(store->m_t2d0_phi);
      bitArray[e1_t2d0ts0_id    ] = &(store->m_t2d0ts0_id);
      bitArray[e1_t2d0ts0_rt    ] = &(store->m_t2d0ts0_rt);
      bitArray[e1_t2d0ts0_lr    ] = &(store->m_t2d0ts0_lr);
      bitArray[e1_t2d0ts0_pr    ] = &(store->m_t2d0ts0_pr);
      bitArray[e1_t2d0ts2_id    ] = &(store->m_t2d0ts2_id);
      bitArray[e1_t2d0ts2_rt    ] = &(store->m_t2d0ts2_rt);
      bitArray[e1_t2d0ts2_lr    ] = &(store->m_t2d0ts2_lr);
      bitArray[e1_t2d0ts2_pr    ] = &(store->m_t2d0ts2_pr);
      bitArray[e1_t2d0ts4_id    ] = &(store->m_t2d0ts4_id);
      bitArray[e1_t2d0ts4_rt    ] = &(store->m_t2d0ts4_rt);
      bitArray[e1_t2d0ts4_lr    ] = &(store->m_t2d0ts4_lr);
      bitArray[e1_t2d0ts4_pr    ] = &(store->m_t2d0ts4_pr);
      bitArray[e1_t2d0ts6_id    ] = &(store->m_t2d0ts6_id);
      bitArray[e1_t2d0ts6_rt    ] = &(store->m_t2d0ts6_rt);
      bitArray[e1_t2d0ts6_lr    ] = &(store->m_t2d0ts6_lr);
      bitArray[e1_t2d0ts6_pr    ] = &(store->m_t2d0ts6_pr);
      bitArray[e1_t2d0ts8_id    ] = &(store->m_t2d0ts8_id);
      bitArray[e1_t2d0ts8_rt    ] = &(store->m_t2d0ts8_rt);
      bitArray[e1_t2d0ts8_lr    ] = &(store->m_t2d0ts8_lr);
      bitArray[e1_t2d0ts8_pr    ] = &(store->m_t2d0ts8_pr);
      bitArray[e1_t2d1_charge    ] = &(store->m_t2d1_charge);
      bitArray[e1_t2d1_rho_s    ] = &(store->m_t2d1_rho_s);
      bitArray[e1_t2d1_phi    ] = &(store->m_t2d1_phi);
      bitArray[e1_t2d1ts0_id    ] = &(store->m_t2d1ts0_id);
      bitArray[e1_t2d1ts0_rt    ] = &(store->m_t2d1ts0_rt);
      bitArray[e1_t2d1ts0_lr    ] = &(store->m_t2d1ts0_lr);
      bitArray[e1_t2d1ts0_pr    ] = &(store->m_t2d1ts0_pr);
      bitArray[e1_t2d1ts2_id    ] = &(store->m_t2d1ts2_id);
      bitArray[e1_t2d1ts2_rt    ] = &(store->m_t2d1ts2_rt);
      bitArray[e1_t2d1ts2_lr    ] = &(store->m_t2d1ts2_lr);
      bitArray[e1_t2d1ts2_pr    ] = &(store->m_t2d1ts2_pr);
      bitArray[e1_t2d1ts4_id    ] = &(store->m_t2d1ts4_id);
      bitArray[e1_t2d1ts4_rt    ] = &(store->m_t2d1ts4_rt);
      bitArray[e1_t2d1ts4_lr    ] = &(store->m_t2d1ts4_lr);
      bitArray[e1_t2d1ts4_pr    ] = &(store->m_t2d1ts4_pr);
      bitArray[e1_t2d1ts6_id    ] = &(store->m_t2d1ts6_id);
      bitArray[e1_t2d1ts6_rt    ] = &(store->m_t2d1ts6_rt);
      bitArray[e1_t2d1ts6_lr    ] = &(store->m_t2d1ts6_lr);
      bitArray[e1_t2d1ts6_pr    ] = &(store->m_t2d1ts6_pr);
      bitArray[e1_t2d1ts8_id    ] = &(store->m_t2d1ts8_id);
      bitArray[e1_t2d1ts8_rt    ] = &(store->m_t2d1ts8_rt);
      bitArray[e1_t2d1ts8_lr    ] = &(store->m_t2d1ts8_lr);
      bitArray[e1_t2d1ts8_pr    ] = &(store->m_t2d1ts8_pr);
      bitArray[e1_t2d2_charge    ] = &(store->m_t2d2_charge);
      bitArray[e1_t2d2_rho_s    ] = &(store->m_t2d2_rho_s);
      bitArray[e1_t2d2_phi    ] = &(store->m_t2d2_phi);
      bitArray[e1_t2d2ts0_id    ] = &(store->m_t2d2ts0_id);
      bitArray[e1_t2d2ts0_rt    ] = &(store->m_t2d2ts0_rt);
      bitArray[e1_t2d2ts0_lr    ] = &(store->m_t2d2ts0_lr);
      bitArray[e1_t2d2ts0_pr    ] = &(store->m_t2d2ts0_pr);
      bitArray[e1_t2d2ts2_id    ] = &(store->m_t2d2ts2_id);
      bitArray[e1_t2d2ts2_rt    ] = &(store->m_t2d2ts2_rt);
      bitArray[e1_t2d2ts2_lr    ] = &(store->m_t2d2ts2_lr);
      bitArray[e1_t2d2ts2_pr    ] = &(store->m_t2d2ts2_pr);
      bitArray[e1_t2d2ts4_id    ] = &(store->m_t2d2ts4_id);
      bitArray[e1_t2d2ts4_rt    ] = &(store->m_t2d2ts4_rt);
      bitArray[e1_t2d2ts4_lr    ] = &(store->m_t2d2ts4_lr);
      bitArray[e1_t2d2ts4_pr    ] = &(store->m_t2d2ts4_pr);
      bitArray[e1_t2d2ts6_id    ] = &(store->m_t2d2ts6_id);
      bitArray[e1_t2d2ts6_rt    ] = &(store->m_t2d2ts6_rt);
      bitArray[e1_t2d2ts6_lr    ] = &(store->m_t2d2ts6_lr);
      bitArray[e1_t2d2ts6_pr    ] = &(store->m_t2d2ts6_pr);
      bitArray[e1_t2d2ts8_id    ] = &(store->m_t2d2ts8_id);
      bitArray[e1_t2d2ts8_rt    ] = &(store->m_t2d2ts8_rt);
      bitArray[e1_t2d2ts8_lr    ] = &(store->m_t2d2ts8_lr);
      bitArray[e1_t2d2ts8_pr    ] = &(store->m_t2d2ts8_pr);
      bitArray[e1_t2d3_charge    ] = &(store->m_t2d3_charge);
      bitArray[e1_t2d3_rho_s    ] = &(store->m_t2d3_rho_s);
      bitArray[e1_t2d3_phi    ] = &(store->m_t2d3_phi);
      bitArray[e1_t2d3ts0_id    ] = &(store->m_t2d3ts0_id);
      bitArray[e1_t2d3ts0_rt    ] = &(store->m_t2d3ts0_rt);
      bitArray[e1_t2d3ts0_lr    ] = &(store->m_t2d3ts0_lr);
      bitArray[e1_t2d3ts0_pr    ] = &(store->m_t2d3ts0_pr);
      bitArray[e1_t2d3ts2_id    ] = &(store->m_t2d3ts2_id);
      bitArray[e1_t2d3ts2_rt    ] = &(store->m_t2d3ts2_rt);
      bitArray[e1_t2d3ts2_lr    ] = &(store->m_t2d3ts2_lr);
      bitArray[e1_t2d3ts2_pr    ] = &(store->m_t2d3ts2_pr);
      bitArray[e1_t2d3ts4_id    ] = &(store->m_t2d3ts4_id);
      bitArray[e1_t2d3ts4_rt    ] = &(store->m_t2d3ts4_rt);
      bitArray[e1_t2d3ts4_lr    ] = &(store->m_t2d3ts4_lr);
      bitArray[e1_t2d3ts4_pr    ] = &(store->m_t2d3ts4_pr);
      bitArray[e1_t2d3ts6_id    ] = &(store->m_t2d3ts6_id);
      bitArray[e1_t2d3ts6_rt    ] = &(store->m_t2d3ts6_rt);
      bitArray[e1_t2d3ts6_lr    ] = &(store->m_t2d3ts6_lr);
      bitArray[e1_t2d3ts6_pr    ] = &(store->m_t2d3ts6_pr);
      bitArray[e1_t2d3ts8_id    ] = &(store->m_t2d3ts8_id);
      bitArray[e1_t2d3ts8_rt    ] = &(store->m_t2d3ts6_rt);
      bitArray[e1_t2d3ts8_lr    ] = &(store->m_t2d3ts6_lr);
      bitArray[e1_t2d3ts8_pr    ] = &(store->m_t2d3ts6_pr);
      bitArray[e1_etf_thresh    ] = &(store->m_etf_thresh);
      bitArray[e1_etf_cc        ] = &(store->m_etf_cc);
      bitArray[e1_etf_t0        ] = &(store->m_etf_t0);
      bitArray[e1_etf_valid     ] = &(store->m_etf_valid);
      bitArray[e1_evt     ] = &(store->m_evt);
      bitArray[e1_clk     ] = &(store->m_clk);
      bitArray[e1_firmid  ] = &(store->m_firmid);
      bitArray[e1_firmver ] = &(store->m_firmver);
    }

    void
    setLeafPointersArray_2dfitter(TRGCDCT3DUnpackerStore* store, int** bitArray)
    {
      bitArray[e2_t3d_2doldtrk    ] = &(store->m_t3d_2doldtrk);
      bitArray[e2_t3dtrk0_evtTime_delay    ] = &(store->m_t3dtrk0_evtTime_delay);
      bitArray[e2_t3dtrk0_evtTimeValid_delay    ] = &(store->m_t3dtrk0_evtTimeValid_delay);
      bitArray[e2_t3dtrk0_charge    ] = &(store->m_t3dtrk0_charge);
      bitArray[e2_t3dtrk0_rho    ] = &(store->m_t3dtrk0_rho);
      bitArray[e2_t3dtrk0_phi0    ] = &(store->m_t3dtrk0_phi0);
      bitArray[e2_t3dtrk0ts0_id    ] = &(store->m_t3dtrk0ts0_id);
      bitArray[e2_t3dtrk0ts0_lr    ] = &(store->m_t3dtrk0ts0_lr);
      bitArray[e2_t3dtrk0ts0_rt    ] = &(store->m_t3dtrk0ts0_rt);
      bitArray[e2_t3dtrk0ts1_id    ] = &(store->m_t3dtrk0ts1_id);
      bitArray[e2_t3dtrk0ts1_lr    ] = &(store->m_t3dtrk0ts1_lr);
      bitArray[e2_t3dtrk0ts1_rt    ] = &(store->m_t3dtrk0ts1_rt);
      bitArray[e2_t3dtrk0ts2_id    ] = &(store->m_t3dtrk0ts2_id);
      bitArray[e2_t3dtrk0ts2_lr    ] = &(store->m_t3dtrk0ts2_lr);
      bitArray[e2_t3dtrk0ts2_rt    ] = &(store->m_t3dtrk0ts2_rt);
      bitArray[e2_t3dtrk0ts3_id    ] = &(store->m_t3dtrk0ts3_id);
      bitArray[e2_t3dtrk0ts3_lr    ] = &(store->m_t3dtrk0ts3_lr);
      bitArray[e2_t3dtrk0ts3_rt    ] = &(store->m_t3dtrk0ts3_rt);
      bitArray[e2_t3dtrk1_evtTime_delay    ] = &(store->m_t3dtrk1_evtTime_delay);
      bitArray[e2_t3dtrk1_evtTimeValid_delay    ] = &(store->m_t3dtrk1_evtTimeValid_delay);
      bitArray[e2_t3dtrk1_charge    ] = &(store->m_t3dtrk1_charge);
      bitArray[e2_t3dtrk1_rho    ] = &(store->m_t3dtrk1_rho);
      bitArray[e2_t3dtrk1_phi0    ] = &(store->m_t3dtrk1_phi0);
      bitArray[e2_t3dtrk1ts0_id    ] = &(store->m_t3dtrk1ts0_id);
      bitArray[e2_t3dtrk1ts0_lr    ] = &(store->m_t3dtrk1ts0_lr);
      bitArray[e2_t3dtrk1ts0_rt    ] = &(store->m_t3dtrk1ts0_rt);
      bitArray[e2_t3dtrk1ts1_id    ] = &(store->m_t3dtrk1ts1_id);
      bitArray[e2_t3dtrk1ts1_lr    ] = &(store->m_t3dtrk1ts1_lr);
      bitArray[e2_t3dtrk1ts1_rt    ] = &(store->m_t3dtrk1ts1_rt);
      bitArray[e2_t3dtrk1ts2_id    ] = &(store->m_t3dtrk1ts2_id);
      bitArray[e2_t3dtrk1ts2_lr    ] = &(store->m_t3dtrk1ts2_lr);
      bitArray[e2_t3dtrk1ts2_rt    ] = &(store->m_t3dtrk1ts2_rt);
      bitArray[e2_t3dtrk1ts3_id    ] = &(store->m_t3dtrk1ts3_id);
      bitArray[e2_t3dtrk1ts3_lr    ] = &(store->m_t3dtrk1ts3_lr);
      bitArray[e2_t3dtrk1ts3_rt    ] = &(store->m_t3dtrk1ts3_rt);
      bitArray[e2_t2dfittertrk0_rho    ] = &(store->m_t2dfittertrk0_rho);
      bitArray[e2_t2dfittertrk0_phi0    ] = &(store->m_t2dfittertrk0_phi0);
      bitArray[e2_t2dfittertrk1_rho    ] = &(store->m_t2dfittertrk1_rho);
      bitArray[e2_t2dfittertrk1_phi0    ] = &(store->m_t2dfittertrk1_phi0);
      bitArray[e2_t2dfittertrk2_rho    ] = &(store->m_t2dfittertrk2_rho);
      bitArray[e2_t2dfittertrk2_phi0    ] = &(store->m_t2dfittertrk2_phi0);
      bitArray[e2_t2dfittertrk3_rho    ] = &(store->m_t2dfittertrk3_rho);
      bitArray[e2_t2dfittertrk3_phi0    ] = &(store->m_t2dfittertrk3_phi0);
      bitArray[e2_t2d_cc    ] = &(store->m_t2d_cc);
      bitArray[e2_t2d_fnf    ] = &(store->m_t2d_fnf);
      bitArray[e2_t2d0_charge    ] = &(store->m_t2d0_charge);
      bitArray[e2_t2d0_rho_s    ] = &(store->m_t2d0_rho_s);
      bitArray[e2_t2d0_phi    ] = &(store->m_t2d0_phi);
      bitArray[e2_t2d0ts0_id    ] = &(store->m_t2d0ts0_id);
      bitArray[e2_t2d0ts0_rt    ] = &(store->m_t2d0ts0_rt);
      bitArray[e2_t2d0ts0_lr    ] = &(store->m_t2d0ts0_lr);
      bitArray[e2_t2d0ts0_pr    ] = &(store->m_t2d0ts0_pr);
      bitArray[e2_t2d0ts2_id    ] = &(store->m_t2d0ts2_id);
      bitArray[e2_t2d0ts2_rt    ] = &(store->m_t2d0ts2_rt);
      bitArray[e2_t2d0ts2_lr    ] = &(store->m_t2d0ts2_lr);
      bitArray[e2_t2d0ts2_pr    ] = &(store->m_t2d0ts2_pr);
      bitArray[e2_t2d0ts4_id    ] = &(store->m_t2d0ts4_id);
      bitArray[e2_t2d0ts4_rt    ] = &(store->m_t2d0ts4_rt);
      bitArray[e2_t2d0ts4_lr    ] = &(store->m_t2d0ts4_lr);
      bitArray[e2_t2d0ts4_pr    ] = &(store->m_t2d0ts4_pr);
      bitArray[e2_t2d0ts6_id    ] = &(store->m_t2d0ts6_id);
      bitArray[e2_t2d0ts6_rt    ] = &(store->m_t2d0ts6_rt);
      bitArray[e2_t2d0ts6_lr    ] = &(store->m_t2d0ts6_lr);
      bitArray[e2_t2d0ts6_pr    ] = &(store->m_t2d0ts6_pr);
      bitArray[e2_t2d0ts8_id    ] = &(store->m_t2d0ts8_id);
      bitArray[e2_t2d0ts8_rt    ] = &(store->m_t2d0ts8_rt);
      bitArray[e2_t2d0ts8_lr    ] = &(store->m_t2d0ts8_lr);
      bitArray[e2_t2d0ts8_pr    ] = &(store->m_t2d0ts8_pr);
      bitArray[e2_t2d1_charge    ] = &(store->m_t2d1_charge);
      bitArray[e2_t2d1_rho_s    ] = &(store->m_t2d1_rho_s);
      bitArray[e2_t2d1_phi    ] = &(store->m_t2d1_phi);
      bitArray[e2_t2d1ts0_id    ] = &(store->m_t2d1ts0_id);
      bitArray[e2_t2d1ts0_rt    ] = &(store->m_t2d1ts0_rt);
      bitArray[e2_t2d1ts0_lr    ] = &(store->m_t2d1ts0_lr);
      bitArray[e2_t2d1ts0_pr    ] = &(store->m_t2d1ts0_pr);
      bitArray[e2_t2d1ts2_id    ] = &(store->m_t2d1ts2_id);
      bitArray[e2_t2d1ts2_rt    ] = &(store->m_t2d1ts2_rt);
      bitArray[e2_t2d1ts2_lr    ] = &(store->m_t2d1ts2_lr);
      bitArray[e2_t2d1ts2_pr    ] = &(store->m_t2d1ts2_pr);
      bitArray[e2_t2d1ts4_id    ] = &(store->m_t2d1ts4_id);
      bitArray[e2_t2d1ts4_rt    ] = &(store->m_t2d1ts4_rt);
      bitArray[e2_t2d1ts4_lr    ] = &(store->m_t2d1ts4_lr);
      bitArray[e2_t2d1ts4_pr    ] = &(store->m_t2d1ts4_pr);
      bitArray[e2_t2d1ts6_id    ] = &(store->m_t2d1ts6_id);
      bitArray[e2_t2d1ts6_rt    ] = &(store->m_t2d1ts6_rt);
      bitArray[e2_t2d1ts6_lr    ] = &(store->m_t2d1ts6_lr);
      bitArray[e2_t2d1ts6_pr    ] = &(store->m_t2d1ts6_pr);
      bitArray[e2_t2d1ts8_id    ] = &(store->m_t2d1ts8_id);
      bitArray[e2_t2d1ts8_rt    ] = &(store->m_t2d1ts8_rt);
      bitArray[e2_t2d1ts8_lr    ] = &(store->m_t2d1ts8_lr);
      bitArray[e2_t2d1ts8_pr    ] = &(store->m_t2d1ts8_pr);
      bitArray[e2_t2d2_charge    ] = &(store->m_t2d2_charge);
      bitArray[e2_t2d2_rho_s    ] = &(store->m_t2d2_rho_s);
      bitArray[e2_t2d2_phi    ] = &(store->m_t2d2_phi);
      bitArray[e2_t2d2ts0_id    ] = &(store->m_t2d2ts0_id);
      bitArray[e2_t2d2ts0_rt    ] = &(store->m_t2d2ts0_rt);
      bitArray[e2_t2d2ts0_lr    ] = &(store->m_t2d2ts0_lr);
      bitArray[e2_t2d2ts0_pr    ] = &(store->m_t2d2ts0_pr);
      bitArray[e2_t2d2ts2_id    ] = &(store->m_t2d2ts2_id);
      bitArray[e2_t2d2ts2_rt    ] = &(store->m_t2d2ts2_rt);
      bitArray[e2_t2d2ts2_lr    ] = &(store->m_t2d2ts2_lr);
      bitArray[e2_t2d2ts2_pr    ] = &(store->m_t2d2ts2_pr);
      bitArray[e2_t2d2ts4_id    ] = &(store->m_t2d2ts4_id);
      bitArray[e2_t2d2ts4_rt    ] = &(store->m_t2d2ts4_rt);
      bitArray[e2_t2d2ts4_lr    ] = &(store->m_t2d2ts4_lr);
      bitArray[e2_t2d2ts4_pr    ] = &(store->m_t2d2ts4_pr);
      bitArray[e2_t2d2ts6_id    ] = &(store->m_t2d2ts6_id);
      bitArray[e2_t2d2ts6_rt    ] = &(store->m_t2d2ts6_rt);
      bitArray[e2_t2d2ts6_lr    ] = &(store->m_t2d2ts6_lr);
      bitArray[e2_t2d2ts6_pr    ] = &(store->m_t2d2ts6_pr);
      bitArray[e2_t2d2ts8_id    ] = &(store->m_t2d2ts8_id);
      bitArray[e2_t2d2ts8_rt    ] = &(store->m_t2d2ts8_rt);
      bitArray[e2_t2d2ts8_lr    ] = &(store->m_t2d2ts8_lr);
      bitArray[e2_t2d2ts8_pr    ] = &(store->m_t2d2ts8_pr);
      bitArray[e2_t2d3_charge    ] = &(store->m_t2d3_charge);
      bitArray[e2_t2d3_rho_s    ] = &(store->m_t2d3_rho_s);
      bitArray[e2_t2d3_phi    ] = &(store->m_t2d3_phi);
      bitArray[e2_t2d3ts0_id    ] = &(store->m_t2d3ts0_id);
      bitArray[e2_t2d3ts0_rt    ] = &(store->m_t2d3ts0_rt);
      bitArray[e2_t2d3ts0_lr    ] = &(store->m_t2d3ts0_lr);
      bitArray[e2_t2d3ts0_pr    ] = &(store->m_t2d3ts0_pr);
      bitArray[e2_t2d3ts2_id    ] = &(store->m_t2d3ts2_id);
      bitArray[e2_t2d3ts2_rt    ] = &(store->m_t2d3ts2_rt);
      bitArray[e2_t2d3ts2_lr    ] = &(store->m_t2d3ts2_lr);
      bitArray[e2_t2d3ts2_pr    ] = &(store->m_t2d3ts2_pr);
      bitArray[e2_t2d3ts4_id    ] = &(store->m_t2d3ts4_id);
      bitArray[e2_t2d3ts4_rt    ] = &(store->m_t2d3ts4_rt);
      bitArray[e2_t2d3ts4_lr    ] = &(store->m_t2d3ts4_lr);
      bitArray[e2_t2d3ts4_pr    ] = &(store->m_t2d3ts4_pr);
      bitArray[e2_t2d3ts6_id    ] = &(store->m_t2d3ts6_id);
      bitArray[e2_t2d3ts6_rt    ] = &(store->m_t2d3ts6_rt);
      bitArray[e2_t2d3ts6_lr    ] = &(store->m_t2d3ts6_lr);
      bitArray[e2_t2d3ts6_pr    ] = &(store->m_t2d3ts6_pr);
      bitArray[e2_t2d3ts8_id    ] = &(store->m_t2d3ts8_id);
      bitArray[e2_t2d3ts8_rt    ] = &(store->m_t2d3ts6_rt);
      bitArray[e2_t2d3ts8_lr    ] = &(store->m_t2d3ts6_lr);
      bitArray[e2_t2d3ts8_pr    ] = &(store->m_t2d3ts6_pr);
      bitArray[e2_etf_thresh    ] = &(store->m_etf_thresh);
      bitArray[e2_etf_cc        ] = &(store->m_etf_cc);
      bitArray[e2_etf_t0        ] = &(store->m_etf_t0);
      bitArray[e2_etf_valid     ] = &(store->m_etf_valid);
      bitArray[e2_evt     ] = &(store->m_evt);
      bitArray[e2_clk     ] = &(store->m_clk);
      bitArray[e2_firmid  ] = &(store->m_firmid);
      bitArray[e2_firmver ] = &(store->m_firmver);
    }

    /*! A module of TRGCDCT3DUnpacker */
    class TRGCDCT3DUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGCDCT3DUnpackerModule();

      /** Destructor */
      virtual ~TRGCDCT3DUnpackerModule();

      /** Initilizes TRGCDCT3DUnpackerUnpackerModule.*/
      virtual void initialize() override;

      /** Called event by event.*/
      virtual void event() override;

      /** Called when processing ended.*/
      virtual void terminate() override;

      /** Called when new run started.*/
      virtual void beginRun() override;

      /** Called when run ended*/
      virtual void endRun() override;

      /** returns version of TRGCDCT3DUnpackerModule.*/
      std::string version() const;

      /** Unpacker main function.*/
      virtual void fillTreeTRGCDCT3DUnpacker(int* buf, int evt);

      /** Unpacker main function.*/
      virtual void fillTreeTRGCDCT3DUnpacker_2dfitter(int* buf, int evt);

      /** Unpacker main function, 2.6k, 15 TS version*/
      virtual void fillTreeTRGCDCT3DUnpacker_2624(int* buf, int evt);

    private:

      /** Output datastore for unpacker **/
      StoreArray<TRGCDCT3DUnpackerStore> m_store;

      /// address of copper module
      unsigned int m_copper_address;
      /// address of copper module
      unsigned int m_copper_ab;
      /// Number of words im raw data, 2k
      int m_nword_2k = 3075;
      /// Number of words im raw data, 2.6k, 15 TS version
      int m_nword_2624 = 2624 * 48 / 32 + 3;

      /// T3D module number
      int m_T3DMOD;


    };
  }
} // end namespace Belle2
#endif // TRGCDCT3DUNPACKER_H
