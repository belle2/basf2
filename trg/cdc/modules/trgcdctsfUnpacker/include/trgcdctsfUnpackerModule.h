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
// Filename : trgcdctsfUnpackerModule.h
// Section  : TRG TRG
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRG TRG Unpacker Module
//---------------------------------------------------------------
// 1.00 : 2017/07/28 : First version
//---------------------------------------------------------------

#ifndef TRGCDCTSFUNPACKER_H
#define TRGCDCTSFUNPACKER_H

#include <string>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/cdc/dataobjects/TRGCDCTSFUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  namespace TRGCDCTSF {

    /** num of leafs in data_b2l **/
    const int nLeafs = 283;
    /** num of leafs in data_b2l, for 4k and 15 TS version **/
    const int nLeafs_4k15ts = 283 + 3 * 5;
//    const int nLeafs = 86;
    /** num of leafs for others **/
    const int nLeafsExtra = 4;
    /** num of clk time window **/
    const int nClks = 48;
    /** num of b2l bits for 2K case **/
    const int nBits_2k = 2048;
    /** num of b2l bits for 4K case **/
    const int nBits_4k = 4096;
    /** num of word for 2K case **/
    const int nword_2k = 0xC03;
    /** num of word for 4K case **/
    const int nword_4k = 6147;

    /** leaf names **/
    const char* LeafNames[nLeafs + nLeafsExtra] = {
      "c127",
      "validtracker", "trackercc",
      "trackerhit0id", "trackerhit0rt", "trackerhit0v",
      "trackerhit1id", "trackerhit1rt", "trackerhit1v",
      "trackerhit2id", "trackerhit2rt", "trackerhit2v",
      "trackerhit3id", "trackerhit3rt", "trackerhit3v",
      "trackerhit4id", "trackerhit4rt", "trackerhit4v",
      "trackerhit5id", "trackerhit5rt", "trackerhit5v",
      "trackerhit6id", "trackerhit6rt", "trackerhit6v",
      "trackerhit7id", "trackerhit7rt", "trackerhit7v",
      "trackerhit8id", "trackerhit8rt", "trackerhit8v",
      "trackerhit9id", "trackerhit9rt", "trackerhit9v",
      "validetf", "etfcc", "netfhit"
      "etfhit0", "etfhit1", "etfhit2", "etfhit3", "etfhit4",
      "etfhit5", "etfhit6", "etfhit7", "etfhit8", "etfhit9",
      "mgr0", "mgr0cc",
      "mgr1", "mgr1cc",
      "mgr2", "mgr2cc",
      "mgr3", "mgr3cc",
      "mgr4", "mgr4cc",
      "mgr5", "mgr5cc",
      "mgr0ft0", "mgr0ft1", "mgr0ft2", "mgr0ft3", "mgr0ft4", "mgr0ft5", "mgr0ft6", "mgr0ft7",
      "mgr0ft8", "mgr0ft9", "mgr0ft10", "mgr0ft11", "mgr0ft12", "mgr0ft13", "mgr0ft14", "mgr0ft15",
      "mgr1ft0", "mgr1ft1", "mgr1ft2", "mgr1ft3", "mgr1ft4", "mgr1ft5", "mgr1ft6", "mgr1ft7",
      "mgr1ft8", "mgr1ft9", "mgr1ft10", "mgr1ft11", "mgr1ft12", "mgr1ft13", "mgr1ft14", "mgr1ft15",
      "mgr2ft0", "mgr2ft1", "mgr2ft2", "mgr2ft3", "mgr2ft4", "mgr2ft5", "mgr2ft6", "mgr2ft7",
      "mgr2ft8", "mgr2ft9", "mgr2ft10", "mgr2ft11", "mgr2ft12", "mgr2ft13", "mgr2ft14", "mgr2ft15",
      "mgr3ft0", "mgr3ft1", "mgr3ft2", "mgr3ft3", "mgr3ft4", "mgr3ft5", "mgr3ft6", "mgr3ft7",
      "mgr3ft8", "mgr3ft9", "mgr3ft10", "mgr3ft11", "mgr3ft12", "mgr3ft13", "mgr3ft14", "mgr3ft15",
      "mgr4ft0", "mgr4ft1", "mgr4ft2", "mgr4ft3", "mgr4ft4", "mgr4ft5", "mgr4ft6", "mgr4ft7",
      "mgr4ft8", "mgr4ft9", "mgr4ft10", "mgr4ft11", "mgr4ft12", "mgr4ft13", "mgr4ft14", "mgr4ft15",
      "mgr0pt0", "mgr0pt1", "mgr0pt2", "mgr0pt3", "mgr0pt4", "mgr0pt5", "mgr0pt6", "mgr0pt7",
      "mgr0pt8", "mgr0pt9", "mgr0pt10", "mgr0pt11", "mgr0pt12", "mgr0pt13", "mgr0pt14", "mgr0pt15",
      "mgr1pt0", "mgr1pt1", "mgr1pt2", "mgr1pt3", "mgr1pt4", "mgr1pt5", "mgr1pt6", "mgr1pt7",
      "mgr1pt8", "mgr1pt9", "mgr1pt10", "mgr1pt11", "mgr1pt12", "mgr1pt13", "mgr1pt14", "mgr1pt15",
      "mgr2pt0", "mgr2pt1", "mgr2pt2", "mgr2pt3", "mgr2pt4", "mgr2pt5", "mgr2pt6", "mgr2pt7",
      "mgr2pt8", "mgr2pt9", "mgr2pt10", "mgr2pt11", "mgr2pt12", "mgr2pt13", "mgr2pt14", "mgr2pt15",
      "mgr3pt0", "mgr3pt1", "mgr3pt2", "mgr3pt3", "mgr3pt4", "mgr3pt5", "mgr3pt6", "mgr3pt7",
      "mgr3pt8", "mgr3pt9", "mgr3pt10", "mgr3pt11", "mgr3pt12", "mgr3pt13", "mgr3pt14", "mgr3pt15",
      "mgr4pt0", "mgr4pt1", "mgr4pt2", "mgr4pt3", "mgr4pt4", "mgr4pt5", "mgr4pt6", "mgr4pt7",
      "mgr4pt8", "mgr4pt9", "mgr4pt10", "mgr4pt11", "mgr4pt12", "mgr4pt13", "mgr4pt14", "mgr4pt15",
      "mgr0l0hit", "mgr0l1hit", "mgr0l2hit", "mgr0l3hit", "mgr0l4hit",
      "mgr1l0hit", "mgr1l1hit", "mgr1l2hit", "mgr1l3hit", "mgr1l4hit",
      "mgr2l0hit", "mgr2l1hit", "mgr2l2hit", "mgr2l3hit", "mgr2l4hit",
      "mgr3l0hit", "mgr3l1hit", "mgr3l2hit", "mgr3l3hit", "mgr3l4hit",
      "mgr4l0hit", "mgr4l1hit", "mgr4l2hit", "mgr4l3hit", "mgr4l4hit",
      "mgr0l0" , "mgr0l1", "mgr0l2", "mgr0l3", "mgr0l4", "mgr0l5", "mgr0l6", "mgr0l7",
      "mgr1l0" , "mgr1l1", "mgr1l2", "mgr1l3", "mgr1l4", "mgr1l5", "mgr1l6", "mgr1l7",
      "mgr2l0" , "mgr2l1", "mgr2l2", "mgr2l3", "mgr2l4", "mgr2l5", "mgr2l6", "mgr2l7",
      "mgr3l0" , "mgr3l1", "mgr3l2", "mgr3l3", "mgr3l4", "mgr3l5", "mgr3l6", "mgr3l7",
      "mgr4l0" , "mgr4l1", "mgr4l2", "mgr4l3", "mgr4l4", "mgr4l5", "mgr4l6", "mgr4l7",
      "firmid", "firmver", "evt", "clk"
    };

    /** leaf names, for 4k, 15 TS version **/
    const char* LeafNames_4k15ts[nLeafs_4k15ts + nLeafsExtra] = {
      "c127",
      "validtracker", "trackercc",
      "trackerhit0id", "trackerhit0rt", "trackerhit0v",
      "trackerhit1id", "trackerhit1rt", "trackerhit1v",
      "trackerhit2id", "trackerhit2rt", "trackerhit2v",
      "trackerhit3id", "trackerhit3rt", "trackerhit3v",
      "trackerhit4id", "trackerhit4rt", "trackerhit4v",
      "trackerhit5id", "trackerhit5rt", "trackerhit5v",
      "trackerhit6id", "trackerhit6rt", "trackerhit6v",
      "trackerhit7id", "trackerhit7rt", "trackerhit7v",
      "trackerhit8id", "trackerhit8rt", "trackerhit8v",
      "trackerhit9id", "trackerhit9rt", "trackerhit9v",
      "trackerhit10id", "trackerhit10rt", "trackerhit10v",
      "trackerhit11id", "trackerhit11rt", "trackerhit11v",
      "trackerhit12id", "trackerhit12rt", "trackerhit12v",
      "trackerhit13id", "trackerhit13rt", "trackerhit13v",
      "trackerhit14id", "trackerhit14rt", "trackerhit14v",
      "validetf", "etfcc", "netfhit"
      "etfhit0", "etfhit1", "etfhit2", "etfhit3", "etfhit4",
      "etfhit5", "etfhit6", "etfhit7", "etfhit8", "etfhit9",
      "mgr0", "mgr0cc",
      "mgr1", "mgr1cc",
      "mgr2", "mgr2cc",
      "mgr3", "mgr3cc",
      "mgr4", "mgr4cc",
      "mgr5", "mgr5cc",
      "mgr0ft0", "mgr0ft1", "mgr0ft2", "mgr0ft3", "mgr0ft4", "mgr0ft5", "mgr0ft6", "mgr0ft7",
      "mgr0ft8", "mgr0ft9", "mgr0ft10", "mgr0ft11", "mgr0ft12", "mgr0ft13", "mgr0ft14", "mgr0ft15",
      "mgr1ft0", "mgr1ft1", "mgr1ft2", "mgr1ft3", "mgr1ft4", "mgr1ft5", "mgr1ft6", "mgr1ft7",
      "mgr1ft8", "mgr1ft9", "mgr1ft10", "mgr1ft11", "mgr1ft12", "mgr1ft13", "mgr1ft14", "mgr1ft15",
      "mgr2ft0", "mgr2ft1", "mgr2ft2", "mgr2ft3", "mgr2ft4", "mgr2ft5", "mgr2ft6", "mgr2ft7",
      "mgr2ft8", "mgr2ft9", "mgr2ft10", "mgr2ft11", "mgr2ft12", "mgr2ft13", "mgr2ft14", "mgr2ft15",
      "mgr3ft0", "mgr3ft1", "mgr3ft2", "mgr3ft3", "mgr3ft4", "mgr3ft5", "mgr3ft6", "mgr3ft7",
      "mgr3ft8", "mgr3ft9", "mgr3ft10", "mgr3ft11", "mgr3ft12", "mgr3ft13", "mgr3ft14", "mgr3ft15",
      "mgr4ft0", "mgr4ft1", "mgr4ft2", "mgr4ft3", "mgr4ft4", "mgr4ft5", "mgr4ft6", "mgr4ft7",
      "mgr4ft8", "mgr4ft9", "mgr4ft10", "mgr4ft11", "mgr4ft12", "mgr4ft13", "mgr4ft14", "mgr4ft15",
      "mgr0pt0", "mgr0pt1", "mgr0pt2", "mgr0pt3", "mgr0pt4", "mgr0pt5", "mgr0pt6", "mgr0pt7",
      "mgr0pt8", "mgr0pt9", "mgr0pt10", "mgr0pt11", "mgr0pt12", "mgr0pt13", "mgr0pt14", "mgr0pt15",
      "mgr1pt0", "mgr1pt1", "mgr1pt2", "mgr1pt3", "mgr1pt4", "mgr1pt5", "mgr1pt6", "mgr1pt7",
      "mgr1pt8", "mgr1pt9", "mgr1pt10", "mgr1pt11", "mgr1pt12", "mgr1pt13", "mgr1pt14", "mgr1pt15",
      "mgr2pt0", "mgr2pt1", "mgr2pt2", "mgr2pt3", "mgr2pt4", "mgr2pt5", "mgr2pt6", "mgr2pt7",
      "mgr2pt8", "mgr2pt9", "mgr2pt10", "mgr2pt11", "mgr2pt12", "mgr2pt13", "mgr2pt14", "mgr2pt15",
      "mgr3pt0", "mgr3pt1", "mgr3pt2", "mgr3pt3", "mgr3pt4", "mgr3pt5", "mgr3pt6", "mgr3pt7",
      "mgr3pt8", "mgr3pt9", "mgr3pt10", "mgr3pt11", "mgr3pt12", "mgr3pt13", "mgr3pt14", "mgr3pt15",
      "mgr4pt0", "mgr4pt1", "mgr4pt2", "mgr4pt3", "mgr4pt4", "mgr4pt5", "mgr4pt6", "mgr4pt7",
      "mgr4pt8", "mgr4pt9", "mgr4pt10", "mgr4pt11", "mgr4pt12", "mgr4pt13", "mgr4pt14", "mgr4pt15",
      "mgr0l0hit", "mgr0l1hit", "mgr0l2hit", "mgr0l3hit", "mgr0l4hit",
      "mgr1l0hit", "mgr1l1hit", "mgr1l2hit", "mgr1l3hit", "mgr1l4hit",
      "mgr2l0hit", "mgr2l1hit", "mgr2l2hit", "mgr2l3hit", "mgr2l4hit",
      "mgr3l0hit", "mgr3l1hit", "mgr3l2hit", "mgr3l3hit", "mgr3l4hit",
      "mgr4l0hit", "mgr4l1hit", "mgr4l2hit", "mgr4l3hit", "mgr4l4hit",
      "mgr0l0" , "mgr0l1", "mgr0l2", "mgr0l3", "mgr0l4", "mgr0l5", "mgr0l6", "mgr0l7",
      "mgr1l0" , "mgr1l1", "mgr1l2", "mgr1l3", "mgr1l4", "mgr1l5", "mgr1l6", "mgr1l7",
      "mgr2l0" , "mgr2l1", "mgr2l2", "mgr2l3", "mgr2l4", "mgr2l5", "mgr2l6", "mgr2l7",
      "mgr3l0" , "mgr3l1", "mgr3l2", "mgr3l3", "mgr3l4", "mgr3l5", "mgr3l6", "mgr3l7",
      "mgr4l0" , "mgr4l1", "mgr4l2", "mgr4l3", "mgr4l4", "mgr4l5", "mgr4l6", "mgr4l7",
      "firmid", "firmver", "evt", "clk"
    };

    /** enum of leafs **/
    enum EBits {
      e_c127,
      e_validtracker, e_trackercc,
      e_trackerhit0id, e_trackerhit0rt, e_trackerhit0v,
      e_trackerhit1id, e_trackerhit1rt, e_trackerhit1v,
      e_trackerhit2id, e_trackerhit2rt, e_trackerhit2v,
      e_trackerhit3id, e_trackerhit3rt, e_trackerhit3v,
      e_trackerhit4id, e_trackerhit4rt, e_trackerhit4v,
      e_trackerhit5id, e_trackerhit5rt, e_trackerhit5v,
      e_trackerhit6id, e_trackerhit6rt, e_trackerhit6v,
      e_trackerhit7id, e_trackerhit7rt, e_trackerhit7v,
      e_trackerhit8id, e_trackerhit8rt, e_trackerhit8v,
      e_trackerhit9id, e_trackerhit9rt, e_trackerhit9v,
      e_validetf, e_etfcc, e_netfhit,
      e_etfhit0, e_etfhit1, e_etfhit2, e_etfhit3, e_etfhit4,
      e_etfhit5, e_etfhit6, e_etfhit7, e_etfhit8, e_etfhit9,
      e_mgr0, e_mgr0cc,
      e_mgr1, e_mgr1cc,
      e_mgr2, e_mgr2cc,
      e_mgr3, e_mgr3cc,
      e_mgr4, e_mgr4cc,
      e_mgr5, e_mgr5cc,
      e_mgr0ft0, e_mgr0ft1, e_mgr0ft2,  e_mgr0ft3,  e_mgr0ft4,  e_mgr0ft5,  e_mgr0ft6,  e_mgr0ft7,
      e_mgr0ft8, e_mgr0ft9, e_mgr0ft10, e_mgr0ft11, e_mgr0ft12, e_mgr0ft13, e_mgr0ft14, e_mgr0ft15,
      e_mgr1ft0, e_mgr1ft1, e_mgr1ft2,  e_mgr1ft3,  e_mgr1ft4,  e_mgr1ft5,  e_mgr1ft6,  e_mgr1ft7,
      e_mgr1ft8, e_mgr1ft9, e_mgr1ft10, e_mgr1ft11, e_mgr1ft12, e_mgr1ft13, e_mgr1ft14, e_mgr1ft15,
      e_mgr2ft0, e_mgr2ft1, e_mgr2ft2,  e_mgr2ft3,  e_mgr2ft4,  e_mgr2ft5,  e_mgr2ft6,  e_mgr2ft7,
      e_mgr2ft8, e_mgr2ft9, e_mgr2ft10, e_mgr2ft11, e_mgr2ft12, e_mgr2ft13, e_mgr2ft14, e_mgr2ft15,
      e_mgr3ft0, e_mgr3ft1, e_mgr3ft2,  e_mgr3ft3,  e_mgr3ft4,  e_mgr3ft5,  e_mgr3ft6,  e_mgr3ft7,
      e_mgr3ft8, e_mgr3ft9, e_mgr3ft10, e_mgr3ft11, e_mgr3ft12, e_mgr3ft13, e_mgr3ft14, e_mgr3ft15,
      e_mgr4ft0, e_mgr4ft1, e_mgr4ft2,  e_mgr4ft3,  e_mgr4ft4,  e_mgr4ft5,  e_mgr4ft6,  e_mgr4ft7,
      e_mgr4ft8, e_mgr4ft9, e_mgr4ft10, e_mgr4ft11, e_mgr4ft12, e_mgr4ft13, e_mgr4ft14, e_mgr4ft15,
      e_mgr0pt0, e_mgr0pt1, e_mgr0pt2,  e_mgr0pt3,  e_mgr0pt4,  e_mgr0pt5,  e_mgr0pt6,  e_mgr0pt7,
      e_mgr0pt8, e_mgr0pt9, e_mgr0pt10, e_mgr0pt11, e_mgr0pt12, e_mgr0pt13, e_mgr0pt14, e_mgr0pt15,
      e_mgr1pt0, e_mgr1pt1, e_mgr1pt2,  e_mgr1pt3,  e_mgr1pt4,  e_mgr1pt5,  e_mgr1pt6,  e_mgr1pt7,
      e_mgr1pt8, e_mgr1pt9, e_mgr1pt10, e_mgr1pt11, e_mgr1pt12, e_mgr1pt13, e_mgr1pt14, e_mgr1pt15,
      e_mgr2pt0, e_mgr2pt1, e_mgr2pt2,  e_mgr2pt3,  e_mgr2pt4,  e_mgr2pt5,  e_mgr2pt6,  e_mgr2pt7,
      e_mgr2pt8, e_mgr2pt9, e_mgr2pt10, e_mgr2pt11, e_mgr2pt12, e_mgr2pt13, e_mgr2pt14, e_mgr2pt15,
      e_mgr3pt0, e_mgr3pt1, e_mgr3pt2,  e_mgr3pt3,  e_mgr3pt4,  e_mgr3pt5,  e_mgr3pt6,  e_mgr3pt7,
      e_mgr3pt8, e_mgr3pt9, e_mgr3pt10, e_mgr3pt11, e_mgr3pt12, e_mgr3pt13, e_mgr3pt14, e_mgr3pt15,
      e_mgr4pt0, e_mgr4pt1, e_mgr4pt2,  e_mgr4pt3,  e_mgr4pt4,  e_mgr4pt5,  e_mgr4pt6,  e_mgr4pt7,
      e_mgr4pt8, e_mgr4pt9, e_mgr4pt10, e_mgr4pt11, e_mgr4pt12, e_mgr4pt13, e_mgr4pt14, e_mgr4pt15,
      e_mgr0l0hit, e_mgr0l1hit, e_mgr0l2hit, e_mgr0l3hit, e_mgr0l4hit,
      e_mgr1l0hit, e_mgr1l1hit, e_mgr1l2hit, e_mgr1l3hit, e_mgr1l4hit,
      e_mgr2l0hit, e_mgr2l1hit, e_mgr2l2hit, e_mgr2l3hit, e_mgr2l4hit,
      e_mgr3l0hit, e_mgr3l1hit, e_mgr3l2hit, e_mgr3l3hit, e_mgr3l4hit,
      e_mgr4l0hit, e_mgr4l1hit, e_mgr4l2hit, e_mgr4l3hit, e_mgr4l4hit,
      e_mgr0l0, e_mgr0l1, e_mgr0l2, e_mgr0l3, e_mgr0l4, e_mgr0l5, e_mgr0l6, e_mgr0l7,
      e_mgr1l0, e_mgr1l1, e_mgr1l2, e_mgr1l3, e_mgr1l4, e_mgr1l5, e_mgr1l6, e_mgr1l7,
      e_mgr2l0, e_mgr2l1, e_mgr2l2, e_mgr2l3, e_mgr2l4, e_mgr2l5, e_mgr2l6, e_mgr2l7,
      e_mgr3l0, e_mgr3l1, e_mgr3l2, e_mgr3l3, e_mgr3l4, e_mgr3l5, e_mgr3l6, e_mgr3l7,
      e_mgr4l0, e_mgr4l1, e_mgr4l2, e_mgr4l3, e_mgr4l4, e_mgr4l5, e_mgr4l6, e_mgr4l7,
      e_firmid, e_firmver, e_evt, e_clk
    };

    /** enum of leafs, for 4k, 15 TS versoin **/
    enum EBits_4k15ts {
      e1_c127,
      e1_validtracker, e1_trackercc,
      e1_trackerhit0id, e1_trackerhit0rt, e1_trackerhit0v,
      e1_trackerhit1id, e1_trackerhit1rt, e1_trackerhit1v,
      e1_trackerhit2id, e1_trackerhit2rt, e1_trackerhit2v,
      e1_trackerhit3id, e1_trackerhit3rt, e1_trackerhit3v,
      e1_trackerhit4id, e1_trackerhit4rt, e1_trackerhit4v,
      e1_trackerhit5id, e1_trackerhit5rt, e1_trackerhit5v,
      e1_trackerhit6id, e1_trackerhit6rt, e1_trackerhit6v,
      e1_trackerhit7id, e1_trackerhit7rt, e1_trackerhit7v,
      e1_trackerhit8id, e1_trackerhit8rt, e1_trackerhit8v,
      e1_trackerhit9id, e1_trackerhit9rt, e1_trackerhit9v,
      e1_trackerhit10id, e1_trackerhit10rt, e1_trackerhit10v,
      e1_trackerhit11id, e1_trackerhit11rt, e1_trackerhit11v,
      e1_trackerhit12id, e1_trackerhit12rt, e1_trackerhit12v,
      e1_trackerhit13id, e1_trackerhit13rt, e1_trackerhit13v,
      e1_trackerhit14id, e1_trackerhit14rt, e1_trackerhit14v,
      e1_validetf, e1_etfcc, e1_netfhit,
      e1_etfhit0, e1_etfhit1, e1_etfhit2, e1_etfhit3, e1_etfhit4,
      e1_etfhit5, e1_etfhit6, e1_etfhit7, e1_etfhit8, e1_etfhit9,
      e1_mgr0, e1_mgr0cc,
      e1_mgr1, e1_mgr1cc,
      e1_mgr2, e1_mgr2cc,
      e1_mgr3, e1_mgr3cc,
      e1_mgr4, e1_mgr4cc,
      e1_mgr5, e1_mgr5cc,
      e1_mgr0ft0, e1_mgr0ft1, e1_mgr0ft2,  e1_mgr0ft3,  e1_mgr0ft4,  e1_mgr0ft5,  e1_mgr0ft6,  e1_mgr0ft7,
      e1_mgr0ft8, e1_mgr0ft9, e1_mgr0ft10, e1_mgr0ft11, e1_mgr0ft12, e1_mgr0ft13, e1_mgr0ft14, e1_mgr0ft15,
      e1_mgr1ft0, e1_mgr1ft1, e1_mgr1ft2,  e1_mgr1ft3,  e1_mgr1ft4,  e1_mgr1ft5,  e1_mgr1ft6,  e1_mgr1ft7,
      e1_mgr1ft8, e1_mgr1ft9, e1_mgr1ft10, e1_mgr1ft11, e1_mgr1ft12, e1_mgr1ft13, e1_mgr1ft14, e1_mgr1ft15,
      e1_mgr2ft0, e1_mgr2ft1, e1_mgr2ft2,  e1_mgr2ft3,  e1_mgr2ft4,  e1_mgr2ft5,  e1_mgr2ft6,  e1_mgr2ft7,
      e1_mgr2ft8, e1_mgr2ft9, e1_mgr2ft10, e1_mgr2ft11, e1_mgr2ft12, e1_mgr2ft13, e1_mgr2ft14, e1_mgr2ft15,
      e1_mgr3ft0, e1_mgr3ft1, e1_mgr3ft2,  e1_mgr3ft3,  e1_mgr3ft4,  e1_mgr3ft5,  e1_mgr3ft6,  e1_mgr3ft7,
      e1_mgr3ft8, e1_mgr3ft9, e1_mgr3ft10, e1_mgr3ft11, e1_mgr3ft12, e1_mgr3ft13, e1_mgr3ft14, e1_mgr3ft15,
      e1_mgr4ft0, e1_mgr4ft1, e1_mgr4ft2,  e1_mgr4ft3,  e1_mgr4ft4,  e1_mgr4ft5,  e1_mgr4ft6,  e1_mgr4ft7,
      e1_mgr4ft8, e1_mgr4ft9, e1_mgr4ft10, e1_mgr4ft11, e1_mgr4ft12, e1_mgr4ft13, e1_mgr4ft14, e1_mgr4ft15,
      e1_mgr0pt0, e1_mgr0pt1, e1_mgr0pt2,  e1_mgr0pt3,  e1_mgr0pt4,  e1_mgr0pt5,  e1_mgr0pt6,  e1_mgr0pt7,
      e1_mgr0pt8, e1_mgr0pt9, e1_mgr0pt10, e1_mgr0pt11, e1_mgr0pt12, e1_mgr0pt13, e1_mgr0pt14, e1_mgr0pt15,
      e1_mgr1pt0, e1_mgr1pt1, e1_mgr1pt2,  e1_mgr1pt3,  e1_mgr1pt4,  e1_mgr1pt5,  e1_mgr1pt6,  e1_mgr1pt7,
      e1_mgr1pt8, e1_mgr1pt9, e1_mgr1pt10, e1_mgr1pt11, e1_mgr1pt12, e1_mgr1pt13, e1_mgr1pt14, e1_mgr1pt15,
      e1_mgr2pt0, e1_mgr2pt1, e1_mgr2pt2,  e1_mgr2pt3,  e1_mgr2pt4,  e1_mgr2pt5,  e1_mgr2pt6,  e1_mgr2pt7,
      e1_mgr2pt8, e1_mgr2pt9, e1_mgr2pt10, e1_mgr2pt11, e1_mgr2pt12, e1_mgr2pt13, e1_mgr2pt14, e1_mgr2pt15,
      e1_mgr3pt0, e1_mgr3pt1, e1_mgr3pt2,  e1_mgr3pt3,  e1_mgr3pt4,  e1_mgr3pt5,  e1_mgr3pt6,  e1_mgr3pt7,
      e1_mgr3pt8, e1_mgr3pt9, e1_mgr3pt10, e1_mgr3pt11, e1_mgr3pt12, e1_mgr3pt13, e1_mgr3pt14, e1_mgr3pt15,
      e1_mgr4pt0, e1_mgr4pt1, e1_mgr4pt2,  e1_mgr4pt3,  e1_mgr4pt4,  e1_mgr4pt5,  e1_mgr4pt6,  e1_mgr4pt7,
      e1_mgr4pt8, e1_mgr4pt9, e1_mgr4pt10, e1_mgr4pt11, e1_mgr4pt12, e1_mgr4pt13, e1_mgr4pt14, e1_mgr4pt15,
      e1_mgr0l0hit, e1_mgr0l1hit, e1_mgr0l2hit, e1_mgr0l3hit, e1_mgr0l4hit,
      e1_mgr1l0hit, e1_mgr1l1hit, e1_mgr1l2hit, e1_mgr1l3hit, e1_mgr1l4hit,
      e1_mgr2l0hit, e1_mgr2l1hit, e1_mgr2l2hit, e1_mgr2l3hit, e1_mgr2l4hit,
      e1_mgr3l0hit, e1_mgr3l1hit, e1_mgr3l2hit, e1_mgr3l3hit, e1_mgr3l4hit,
      e1_mgr4l0hit, e1_mgr4l1hit, e1_mgr4l2hit, e1_mgr4l3hit, e1_mgr4l4hit,
      e1_mgr0l0, e1_mgr0l1, e1_mgr0l2, e1_mgr0l3, e1_mgr0l4, e1_mgr0l5, e1_mgr0l6, e1_mgr0l7,
      e1_mgr1l0, e1_mgr1l1, e1_mgr1l2, e1_mgr1l3, e1_mgr1l4, e1_mgr1l5, e1_mgr1l6, e1_mgr1l7,
      e1_mgr2l0, e1_mgr2l1, e1_mgr2l2, e1_mgr2l3, e1_mgr2l4, e1_mgr2l5, e1_mgr2l6, e1_mgr2l7,
      e1_mgr3l0, e1_mgr3l1, e1_mgr3l2, e1_mgr3l3, e1_mgr3l4, e1_mgr3l5, e1_mgr3l6, e1_mgr3l7,
      e1_mgr4l0, e1_mgr4l1, e1_mgr4l2, e1_mgr4l3, e1_mgr4l4, e1_mgr4l5, e1_mgr4l6, e1_mgr4l7,
      e1_firmid, e1_firmver, e1_evt, e1_clk
    };

    /** bus bit map. (a downto a-b) **/
    const int BitMap_2k[nLeafs][2] = {
      2031, 15, // c127 (2031 downto 2016)
//    2015, 159, //tshitmap
