/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
//+1015 for TSF1
      1764   , 0, //trackervalid
      1805 - 42, 8, //trackercc
      1796 - 42, 7, //trackerhit0id
      1788 - 42, 8, //trackerhit0rt
      1779 - 42, 3, //trackerhit0v
      1775 - 42, 7, //trackerhit1id
      1767 - 42, 8, //trackerhit1rt
      1758 - 42, 3, //trackerhit1v
      1754 - 42, 7, //trackerhit2id
      1746 - 42, 8, //trackerhit2rt
      1737 - 42, 3, //trackerhit2v
      1733 - 42, 7, //trackerhit3id
      1725 - 42, 8, //trackerhit3rt
      1716 - 42, 3, //trackerhit3v
      1712 - 42, 7, //trackerhit4id
      1704 - 42, 8, //trackerhit4rt
      1695 - 42, 3, //trackerhit4v
      1691 - 42, 7, //trackerhit5id
      1683 - 42, 8, //trackerhit5rt
      1674 - 42, 3, //trackerhit5v
      1670 - 42, 7, //trackerhit6id
      1662 - 42, 8, //trackerhit6rt
      1653 - 42, 3, //trackerhit6v
      1649 - 42, 7, //trackerhit7id
      1641 - 42, 8, //trackerhit7rt
      1632 - 42, 3, //trackerhit7v
      1628 - 42, 7, //trackerhit8id
      1620 - 42, 8, //trackerhit8rt
      1611 - 42, 3, //trackerhit8v
      1607 - 42, 7, //trackerhit9id
      1599 - 42, 8, //trackerhit9rt
      1590 - 42, 3, //trackerhit9v

//+1014 for TSF1
      1873, 0,  //validetf
      1872, 8,  //etfcc
      1863, 8,  //#ofetf
      1854, 8, //etfhit0cc
      1845, 8, //etfhit1cc
      1836, 8, //etfhit2cc
      1827, 8, //etfhit3cc
      1818, 8, //etfhit4cc
      1809, 8, //etfhit5cc
      1800, 8, //etfhit6cc
      1791, 8, //etfhit7cc
      1782, 8, //etfhit8cc
      1773, 8, //etfhit9cc

      255, 255, //mgr0
      244,   8, //mgr0cc
      511, 255, //mgr1
      500,   8, //mgr1cc
      767, 255, //mgr2
      756,   8, //mgr2cc
      1023, 255, //mgr3
      1012,  8, //mgr3cc
      1279, 255, //mgr4
      1268,  8, //mgr4cc
      1535, 255, //mgr5
      1524,  8, //mgr5cc

      147,    3, //mgr0ft0
      151,    3, //mgr0ft1
      155,    3, //mgr0ft2
      159,    3, //mgr0ft3
      163,    3, //mgr0ft4
      167,    3, //mgr0ft5
      171,    3, //mgr0ft6
      175,    3, //mgr0ft7
      179,    3, //mgr0ft8
      183,    3, //mgr0ft9
      187,    3, //mgr0ft10
      191,    3, //mgr0ft11
      195,    3, //mgr0ft12
      199,    3, //mgr0ft13
      203,    3, //mgr0ft14
      207,    3, //mgr0ft15

      147 + 256,    3, //mgr1ft0
      151 + 256,    3, //mgr1ft1
      155 + 256,    3, //mgr1ft2
      159 + 256,    3, //mgr1ft3
      163 + 256,    3, //mgr1ft4
      167 + 256,    3, //mgr1ft5
      171 + 256,    3, //mgr1ft6
      175 + 256,    3, //mgr1ft7
      179 + 256,    3, //mgr1ft8
      183 + 256,    3, //mgr1ft9
      187 + 256,    3, //mgr1ft10
      191 + 256,    3, //mgr1ft11
      195 + 256,    3, //mgr1ft12
      199 + 256,    3, //mgr1ft13
      203 + 256,    3, //mgr1ft14
      207 + 256,    3, //mgr1ft15
      147 + 512,    3, //mgr2ft0
      151 + 512,    3, //mgr2ft1
      155 + 512,    3, //mgr2ft2
      159 + 512,    3, //mgr2ft3
      163 + 512,    3, //mgr2ft4
      167 + 512,    3, //mgr2ft5
      171 + 512,    3, //mgr2ft6
      175 + 512,    3, //mgr2ft7
      179 + 512,    3, //mgr2ft8
      183 + 512,    3, //mgr2ft9
      187 + 512,    3, //mgr2ft10
      191 + 512,    3, //mgr2ft11
      195 + 512,    3, //mgr2ft12
      199 + 512,    3, //mgr2ft13
      203 + 512,    3, //mgr2ft14
      207 + 512,    3, //mgr2ft15
      147 + 768,    3, //mgr3ft0
      151 + 768,    3, //mgr3ft1
      155 + 768,    3, //mgr3ft2
      159 + 768,    3, //mgr3ft3
      163 + 768,    3, //mgr3ft4
      167 + 768,    3, //mgr3ft5
      171 + 768,    3, //mgr3ft6
      175 + 768,    3, //mgr3ft7
      179 + 768,    3, //mgr3ft8
      183 + 768,    3, //mgr3ft9
      187 + 768,    3, //mgr3ft10
      191 + 768,    3, //mgr3ft11
      195 + 768,    3, //mgr3ft12
      199 + 768,    3, //mgr3ft13
      203 + 768,    3, //mgr3ft14
      207 + 768,    3, //mgr3ft15

      147 + 1024,    3, //mgr4ft0
      151 + 1024,    3, //mgr4ft1
      155 + 1024,    3, //mgr4ft2
      159 + 1024,    3, //mgr4ft3
      163 + 1024,    3, //mgr4ft4
      167 + 1024,    3, //mgr4ft5
      171 + 1024,    3, //mgr4ft6
      175 + 1024,    3, //mgr4ft7
      179 + 1024,    3, //mgr4ft8
      183 + 1024,    3, //mgr4ft9
      187 + 1024,    3, //mgr4ft10
      191 + 1024,    3, //mgr4ft11
      195 + 1024,    3, //mgr4ft12
      199 + 1024,    3, //mgr4ft13
      203 + 1024,    3, //mgr4ft14
      207 + 1024,    3, //mgr4ft15

      147 - 64,    3, //mgr0pt0
      151 - 64,    3, //mgr0pt1
      155 - 64,    3, //mgr0pt2
      159 - 64,    3, //mgr0pt3
      163 - 64,    3, //mgr0pt4
      167 - 64,    3, //mgr0pt5
      171 - 64,    3, //mgr0pt6
      175 - 64,    3, //mgr0pt7
      179 - 64,    3, //mgr0pt8
      183 - 64,    3, //mgr0pt9
      187 - 64,    3, //mgr0pt10
      191 - 64,    3, //mgr0pt11
      195 - 64,    3, //mgr0pt12
      199 - 64,    3, //mgr0pt13
      203 - 64,    3, //mgr0pt14
      207 - 64,    3, //mgr0pt15

      147 + 256 - 64,    3, //mgr1ft0
      151 + 256 - 64,    3, //mgr1ft1
      155 + 256 - 64,    3, //mgr1ft2
      159 + 256 - 64,    3, //mgr1ft3
      163 + 256 - 64,    3, //mgr1ft4
      167 + 256 - 64,    3, //mgr1ft5
      171 + 256 - 64,    3, //mgr1ft6
      175 + 256 - 64,    3, //mgr1ft7
      179 + 256 - 64,    3, //mgr1ft8
      183 + 256 - 64,    3, //mgr1ft9
      187 + 256 - 64,    3, //mgr1ft10
      191 + 256 - 64,    3, //mgr1ft11
      195 + 256 - 64,    3, //mgr1ft12
      199 + 256 - 64,    3, //mgr1ft13
      203 + 256 - 64,    3, //mgr1ft14
      207 + 256 - 64,    3, //mgr1ft15
      147 + 512 - 64,    3, //mgr2ft0
      151 + 512 - 64,    3, //mgr2ft1
      155 + 512 - 64,    3, //mgr2ft2
      159 + 512 - 64,    3, //mgr2ft3
      163 + 512 - 64,    3, //mgr2ft4
      167 + 512 - 64,    3, //mgr2ft5
      171 + 512 - 64,    3, //mgr2ft6
      175 + 512 - 64,    3, //mgr2ft7
      179 + 512 - 64,    3, //mgr2ft8
      183 + 512 - 64,    3, //mgr2ft9
      187 + 512 - 64,    3, //mgr2ft10
      191 + 512 - 64,    3, //mgr2ft11
      195 + 512 - 64,    3, //mgr2ft12
      199 + 512 - 64,    3, //mgr2ft13
      203 + 512 - 64,    3, //mgr2ft14
      207 + 512 - 64,    3, //mgr2ft15
      147 + 768 - 64,    3, //mgr3ft0
      151 + 768 - 64,    3, //mgr3ft1
      155 + 768 - 64,    3, //mgr3ft2
      159 + 768 - 64,    3, //mgr3ft3
      163 + 768 - 64,    3, //mgr3ft4
      167 + 768 - 64,    3, //mgr3ft5
      171 + 768 - 64,    3, //mgr3ft6
      175 + 768 - 64,    3, //mgr3ft7
      179 + 768 - 64,    3, //mgr3ft8
      183 + 768 - 64,    3, //mgr3ft9
      187 + 768 - 64,    3, //mgr3ft10
      191 + 768 - 64,    3, //mgr3ft11
      195 + 768 - 64,    3, //mgr3ft12
      199 + 768 - 64,    3, //mgr3ft13
      203 + 768 - 64,    3, //mgr3ft14
      207 + 768 - 64,    3, //mgr3ft15
      147 + 1024 - 64,    3, //mgr4ft0
      151 + 1024 - 64,    3, //mgr4ft1
      155 + 1024 - 64,    3, //mgr4ft2
      159 + 1024 - 64,    3, //mgr4ft3
      163 + 1024 - 64,    3, //mgr4ft4
      167 + 1024 - 64,    3, //mgr4ft5
      171 + 1024 - 64,    3, //mgr4ft6
      175 + 1024 - 64,    3, //mgr4ft7
      179 + 1024 - 64,    3, //mgr4ft8
      183 + 1024 - 64,    3, //mgr4ft9
      187 + 1024 - 64,    3, //mgr4ft10
      191 + 1024 - 64,    3, //mgr4ft11
      195 + 1024 - 64,    3, //mgr4ft12
      199 + 1024 - 64,    3, //mgr4ft13
      203 + 1024 - 64,    3, //mgr4ft14
      207 + 1024 - 64,    3, //mgr4ft15


      15, 15, //mgr0l0hit
      31, 15, //mgr0l1hit
//    2031, 15, //mgr0l0hit
//    2047, 15, //mgr0l1hit
      47, 15, //mgr0l2hit
      63, 15, //mgr0l3hit
      79, 15, //mgr0l4hit
      15 + 256, 15, //mgr1l0hit
      31 + 256, 15, //mgr1l1hit
      47 + 256, 15, //mgr1l2hit
      63 + 256, 15, //mgr1l3hit
      79 + 256, 15, //mgr1l4hit
      15 + 512, 15, //mgr2l0hit
      31 + 512, 15, //mgr2l1hit
      47 + 512, 15, //mgr2l2hit
      63 + 512, 15, //mgr2l3hit
      79 + 512, 15, //mgr2l4hit
      15 + 768, 15, //mgr3l0hit
      31 + 768, 15, //mgr3l1hit
      47 + 768, 15, //mgr3l2hit
      63 + 768, 15, //mgr3l3hit
      79 + 768, 15, //mgr3l4hit
      15 + 1024,  15, //mgr4l0hit
      31 + 1024,  15, //mgr4l1hit
      47 + 1024,  15, //mgr4l2hit
      63 + 1024,  15, //mgr4l3hit
      79 + 1024,  15, //mgr4l4hit


      31 ,  31,
      63 ,  31,
      95 ,  31,
      127,  31,
      159,  31,
      191,  31,
      223,  31,
      255,  31,

      31 + 256, 31,
      63 + 256, 31,
      95 + 256, 31,
      127 + 256,  31,
      159 + 256,  31,
      191 + 256,  31,
      223 + 256,  31,
      255 + 256,  31,

      31 + 512, 31,
      63 + 512, 31,
      95 + 512, 31,
      127 + 512,  31,
      159 + 512,  31,
      191 + 512,  31,
      223 + 512,  31,
      255 + 512,  31,

      31 + 768, 31,
      63 + 768, 31,
      95 + 768, 31,
      127 + 768,  31,
      159 + 768,  31,
      191 + 768,  31,
      223 + 768,  31,
      255 + 768,  31,

      31 + 1024,  31,
      63 + 1024,  31,
      95 + 1024,  31,
      127 + 1024, 31,
      159 + 1024, 31,
      191 + 1024, 31,
      223 + 1024, 31,
      255 + 1024, 31


    };


    /** bus bit map. (a downto a-b) **/
    const int BitMap_4k[nLeafs][2] = {
      2031, 15, // c127 (2031 downto 2016)
//    2015, 159, //tshitmap
//+1015 for TSF1
      1764 + 1015   , 0, //trackervalid
      1805 + 1015 - 42, 8, //trackercc
      1796 + 1015 - 42, 7, //trackerhit0id
      1788 + 1015 - 42, 8, //trackerhit0rt
      1779 + 1015 - 42, 3, //trackerhit0v
      1775 + 1015 - 42, 7, //trackerhit1id
      1767 + 1015 - 42, 8, //trackerhit1rt
      1758 + 1015 - 42, 3, //trackerhit1v
      1754 + 1015 - 42, 7, //trackerhit2id
      1746 + 1015 - 42, 8, //trackerhit2rt
      1737 + 1015 - 42, 3, //trackerhit2v
      1733 + 1015 - 42, 7, //trackerhit3id
      1725 + 1015 - 42, 8, //trackerhit3rt
      1716 + 1015 - 42, 3, //trackerhit3v
      1712 + 1015 - 42, 7, //trackerhit4id
      1704 + 1015 - 42, 8, //trackerhit4rt
      1695 + 1015 - 42, 3, //trackerhit4v
      1691 + 1015 - 42, 7, //trackerhit5id
      1683 + 1015 - 42, 8, //trackerhit5rt
      1674 + 1015 - 42, 3, //trackerhit5v
      1670 + 1015 - 42, 7, //trackerhit6id
      1662 + 1015 - 42, 8, //trackerhit6rt
      1653 + 1015 - 42, 3, //trackerhit6v
      1649 + 1015 - 42, 7, //trackerhit7id
      1641 + 1015 - 42, 8, //trackerhit7rt
      1632 + 1015 - 42, 3, //trackerhit7v
      1628 + 1015 - 42, 7, //trackerhit8id
      1620 + 1015 - 42, 8, //trackerhit8rt
      1611 + 1015 - 42, 3, //trackerhit8v
      1607 + 1015 - 42, 7, //trackerhit9id
      1599 + 1015 - 42, 8, //trackerhit9rt
      1590 + 1015 - 42, 3, //trackerhit9v

//+1014 for TSF1
      1873 + 1014, 0, //validetf
      1872 + 1014, 8, //etfcc
      1863 + 1014, 8, //#ofetf
      1854 + 1014, 8, //etfhit0cc
      1845 + 1014, 8, //etfhit1cc
      1836 + 1014, 8, //etfhit2cc
      1827 + 1014, 8, //etfhit3cc
      1818 + 1014, 8, //etfhit4cc
      1809 + 1014, 8, //etfhit5cc
      1800 + 1014, 8, //etfhit6cc
      1791 + 1014, 8, //etfhit7cc
      1782 + 1014, 8, //etfhit8cc
      1773 + 1014, 8, //etfhit9cc

      255, 255, //mgr0
      244,   8, //mgr0cc
      511, 255, //mgr1
      500,   8, //mgr1cc
      767, 255, //mgr2
      756,   8, //mgr2cc
      1023, 255, //mgr3
      1012,  8, //mgr3cc
      1279, 255, //mgr4
      1268,  8, //mgr4cc
      1535, 255, //mgr5
      1524,  8, //mgr5cc

      147,    3, //mgr0ft0
      151,    3, //mgr0ft1
      155,    3, //mgr0ft2
      159,    3, //mgr0ft3
      163,    3, //mgr0ft4
      167,    3, //mgr0ft5
      171,    3, //mgr0ft6
      175,    3, //mgr0ft7
      179,    3, //mgr0ft8
      183,    3, //mgr0ft9
      187,    3, //mgr0ft10
      191,    3, //mgr0ft11
      195,    3, //mgr0ft12
      199,    3, //mgr0ft13
      203,    3, //mgr0ft14
      207,    3, //mgr0ft15

      147 + 256,    3, //mgr1ft0
      151 + 256,    3, //mgr1ft1
      155 + 256,    3, //mgr1ft2
      159 + 256,    3, //mgr1ft3
      163 + 256,    3, //mgr1ft4
      167 + 256,    3, //mgr1ft5
      171 + 256,    3, //mgr1ft6
      175 + 256,    3, //mgr1ft7
      179 + 256,    3, //mgr1ft8
      183 + 256,    3, //mgr1ft9
      187 + 256,    3, //mgr1ft10
      191 + 256,    3, //mgr1ft11
      195 + 256,    3, //mgr1ft12
      199 + 256,    3, //mgr1ft13
      203 + 256,    3, //mgr1ft14
      207 + 256,    3, //mgr1ft15
      147 + 512,    3, //mgr2ft0
      151 + 512,    3, //mgr2ft1
      155 + 512,    3, //mgr2ft2
      159 + 512,    3, //mgr2ft3
      163 + 512,    3, //mgr2ft4
      167 + 512,    3, //mgr2ft5
      171 + 512,    3, //mgr2ft6
      175 + 512,    3, //mgr2ft7
      179 + 512,    3, //mgr2ft8
      183 + 512,    3, //mgr2ft9
      187 + 512,    3, //mgr2ft10
      191 + 512,    3, //mgr2ft11
      195 + 512,    3, //mgr2ft12
      199 + 512,    3, //mgr2ft13
      203 + 512,    3, //mgr2ft14
      207 + 512,    3, //mgr2ft15
      147 + 768,    3, //mgr3ft0
      151 + 768,    3, //mgr3ft1
      155 + 768,    3, //mgr3ft2
      159 + 768,    3, //mgr3ft3
      163 + 768,    3, //mgr3ft4
      167 + 768,    3, //mgr3ft5
      171 + 768,    3, //mgr3ft6
      175 + 768,    3, //mgr3ft7
      179 + 768,    3, //mgr3ft8
      183 + 768,    3, //mgr3ft9
      187 + 768,    3, //mgr3ft10
      191 + 768,    3, //mgr3ft11
      195 + 768,    3, //mgr3ft12
      199 + 768,    3, //mgr3ft13
      203 + 768,    3, //mgr3ft14
      207 + 768,    3, //mgr3ft15

      147 + 1024,    3, //mgr4ft0
      151 + 1024,    3, //mgr4ft1
      155 + 1024,    3, //mgr4ft2
      159 + 1024,    3, //mgr4ft3
      163 + 1024,    3, //mgr4ft4
      167 + 1024,    3, //mgr4ft5
      171 + 1024,    3, //mgr4ft6
      175 + 1024,    3, //mgr4ft7
      179 + 1024,    3, //mgr4ft8
      183 + 1024,    3, //mgr4ft9
      187 + 1024,    3, //mgr4ft10
      191 + 1024,    3, //mgr4ft11
      195 + 1024,    3, //mgr4ft12
      199 + 1024,    3, //mgr4ft13
      203 + 1024,    3, //mgr4ft14
      207 + 1024,    3, //mgr4ft15

      147 - 64,    3, //mgr0pt0
      151 - 64,    3, //mgr0pt1
      155 - 64,    3, //mgr0pt2
      159 - 64,    3, //mgr0pt3
      163 - 64,    3, //mgr0pt4
      167 - 64,    3, //mgr0pt5
      171 - 64,    3, //mgr0pt6
      175 - 64,    3, //mgr0pt7
      179 - 64,    3, //mgr0pt8
      183 - 64,    3, //mgr0pt9
      187 - 64,    3, //mgr0pt10
      191 - 64,    3, //mgr0pt11
      195 - 64,    3, //mgr0pt12
      199 - 64,    3, //mgr0pt13
      203 - 64,    3, //mgr0pt14
      207 - 64,    3, //mgr0pt15

      147 + 256 - 64,    3, //mgr1ft0
      151 + 256 - 64,    3, //mgr1ft1
      155 + 256 - 64,    3, //mgr1ft2
      159 + 256 - 64,    3, //mgr1ft3
      163 + 256 - 64,    3, //mgr1ft4
      167 + 256 - 64,    3, //mgr1ft5
      171 + 256 - 64,    3, //mgr1ft6
      175 + 256 - 64,    3, //mgr1ft7
      179 + 256 - 64,    3, //mgr1ft8
      183 + 256 - 64,    3, //mgr1ft9
      187 + 256 - 64,    3, //mgr1ft10
      191 + 256 - 64,    3, //mgr1ft11
      195 + 256 - 64,    3, //mgr1ft12
      199 + 256 - 64,    3, //mgr1ft13
      203 + 256 - 64,    3, //mgr1ft14
      207 + 256 - 64,    3, //mgr1ft15
      147 + 512 - 64,    3, //mgr2ft0
      151 + 512 - 64,    3, //mgr2ft1
      155 + 512 - 64,    3, //mgr2ft2
      159 + 512 - 64,    3, //mgr2ft3
      163 + 512 - 64,    3, //mgr2ft4
      167 + 512 - 64,    3, //mgr2ft5
      171 + 512 - 64,    3, //mgr2ft6
      175 + 512 - 64,    3, //mgr2ft7
      179 + 512 - 64,    3, //mgr2ft8
      183 + 512 - 64,    3, //mgr2ft9
      187 + 512 - 64,    3, //mgr2ft10
      191 + 512 - 64,    3, //mgr2ft11
      195 + 512 - 64,    3, //mgr2ft12
      199 + 512 - 64,    3, //mgr2ft13
      203 + 512 - 64,    3, //mgr2ft14
      207 + 512 - 64,    3, //mgr2ft15
      147 + 768 - 64,    3, //mgr3ft0
      151 + 768 - 64,    3, //mgr3ft1
      155 + 768 - 64,    3, //mgr3ft2
      159 + 768 - 64,    3, //mgr3ft3
      163 + 768 - 64,    3, //mgr3ft4
      167 + 768 - 64,    3, //mgr3ft5
      171 + 768 - 64,    3, //mgr3ft6
      175 + 768 - 64,    3, //mgr3ft7
      179 + 768 - 64,    3, //mgr3ft8
      183 + 768 - 64,    3, //mgr3ft9
      187 + 768 - 64,    3, //mgr3ft10
      191 + 768 - 64,    3, //mgr3ft11
      195 + 768 - 64,    3, //mgr3ft12
      199 + 768 - 64,    3, //mgr3ft13
      203 + 768 - 64,    3, //mgr3ft14
      207 + 768 - 64,    3, //mgr3ft15
      147 + 1024 - 64,    3, //mgr4ft0
      151 + 1024 - 64,    3, //mgr4ft1
      155 + 1024 - 64,    3, //mgr4ft2
      159 + 1024 - 64,    3, //mgr4ft3
      163 + 1024 - 64,    3, //mgr4ft4
      167 + 1024 - 64,    3, //mgr4ft5
      171 + 1024 - 64,    3, //mgr4ft6
      175 + 1024 - 64,    3, //mgr4ft7
      179 + 1024 - 64,    3, //mgr4ft8
      183 + 1024 - 64,    3, //mgr4ft9
      187 + 1024 - 64,    3, //mgr4ft10
      191 + 1024 - 64,    3, //mgr4ft11
      195 + 1024 - 64,    3, //mgr4ft12
      199 + 1024 - 64,    3, //mgr4ft13
      203 + 1024 - 64,    3, //mgr4ft14
      207 + 1024 - 64,    3, //mgr4ft15


      15, 15, //mgr0l0hit
      31, 15, //mgr0l1hit
//    2031, 15, //mgr0l0hit
//    2047, 15, //mgr0l1hit
      47, 15, //mgr0l2hit
      63, 15, //mgr0l3hit
      79, 15, //mgr0l4hit
      15 + 256, 15, //mgr1l0hit
      31 + 256, 15, //mgr1l1hit
      47 + 256, 15, //mgr1l2hit
      63 + 256, 15, //mgr1l3hit
      79 + 256, 15, //mgr1l4hit
      15 + 512, 15, //mgr2l0hit
      31 + 512, 15, //mgr2l1hit
      47 + 512, 15, //mgr2l2hit
      63 + 512, 15, //mgr2l3hit
      79 + 512, 15, //mgr2l4hit
      15 + 768, 15, //mgr3l0hit
      31 + 768, 15, //mgr3l1hit
      47 + 768, 15, //mgr3l2hit
      63 + 768, 15, //mgr3l3hit
      79 + 768, 15, //mgr3l4hit
      15 + 1024,  15, //mgr4l0hit
      31 + 1024,  15, //mgr4l1hit
      47 + 1024,  15, //mgr4l2hit
      63 + 1024,  15, //mgr4l3hit
      79 + 1024,  15, //mgr4l4hit


      31 ,  31,
      63 ,  31,
      95 ,  31,
      127,  31,
      159,  31,
      191,  31,
      223,  31,
      255,  31,

      31 + 256, 31,
      63 + 256, 31,
      95 + 256, 31,
      127 + 256,  31,
      159 + 256,  31,
      191 + 256,  31,
      223 + 256,  31,
      255 + 256,  31,

      31 + 512, 31,
      63 + 512, 31,
      95 + 512, 31,
      127 + 512,  31,
      159 + 512,  31,
      191 + 512,  31,
      223 + 512,  31,
      255 + 512,  31,

      31 + 768, 31,
      63 + 768, 31,
      95 + 768, 31,
      127 + 768,  31,
      159 + 768,  31,
      191 + 768,  31,
      223 + 768,  31,
      255 + 768,  31,

      31 + 1024,  31,
      63 + 1024,  31,
      95 + 1024,  31,
      127 + 1024, 31,
      159 + 1024, 31,
      191 + 1024, 31,
      223 + 1024, 31,
      255 + 1024, 31


    };

    /** bus bit map. (a downto a-b) for 4k, 15 TS version**/
    const int BitMap_4k15ts[nLeafs_4k15ts][2] = {
//if original bit position > 2560, shift to MSB by 105 bit
      2031, 15, // c127 (2031 downto 2016)
//    2015, 159, //tshitmap
//+1015 for TSF1
      1764 + 1015 + 105  , 0, //trackervalid
      1805 + 1015 - 42 + 105, 8, //trackercc
      1796 + 1015 - 42 + 105, 7, //trackerhit0id
      1788 + 1015 - 42 + 105, 8, //trackerhit0rt
      1779 + 1015 - 42 + 105, 3, //trackerhit0v
      1775 + 1015 - 42 + 105, 7, //trackerhit1id
      1767 + 1015 - 42 + 105, 8, //trackerhit1rt
      1758 + 1015 - 42 + 105, 3, //trackerhit1v
      1754 + 1015 - 42 + 105, 7, //trackerhit2id
      1746 + 1015 - 42 + 105, 8, //trackerhit2rt
      1737 + 1015 - 42 + 105, 3, //trackerhit2v
      1733 + 1015 - 42 + 105, 7, //trackerhit3id
      1725 + 1015 - 42 + 105, 8, //trackerhit3rt
      1716 + 1015 - 42 + 105, 3, //trackerhit3v
      1712 + 1015 - 42 + 105, 7, //trackerhit4id
      1704 + 1015 - 42 + 105, 8, //trackerhit4rt
      1695 + 1015 - 42 + 105, 3, //trackerhit4v
      1691 + 1015 - 42 + 105, 7, //trackerhit5id
      1683 + 1015 - 42 + 105, 8, //trackerhit5rt
      1674 + 1015 - 42 + 105, 3, //trackerhit5v
      1670 + 1015 - 42 + 105, 7, //trackerhit6id
      1662 + 1015 - 42 + 105, 8, //trackerhit6rt
      1653 + 1015 - 42 + 105, 3, //trackerhit6v
      1649 + 1015 - 42 + 105, 7, //trackerhit7id
      1641 + 1015 - 42 + 105, 8, //trackerhit7rt
      1632 + 1015 - 42 + 105, 3, //trackerhit7v
      1628 + 1015 - 42 + 105, 7, //trackerhit8id
      1620 + 1015 - 42 + 105, 8, //trackerhit8rt
      1611 + 1015 - 42 + 105, 3, //trackerhit8v
      1607 + 1015 - 42 + 105, 7, //trackerhit9id
      1599 + 1015 - 42 + 105, 8, //trackerhit9rt
      1590 + 1015 - 42 + 105, 3, //trackerhit9v
//additional part for 15 TS version
      2664, 7, //trackerhit10id
      2656, 8, //trackerhit10rt
      2647, 3, //trackerhit10v
      2643, 7, //trackerhit11id
      2635, 8, //trackerhit11rt
      2626, 3, //trackerhit11v
      2622, 7, //trackerhit12id
      2614, 8, //trackerhit12rt
      2605, 3, //trackerhit12v
      2601, 7, //trackerhit13id
      2593, 8, //trackerhit13rt
      2584, 3, //trackerhit13v
      2580, 7, //trackerhit14id
      2572, 8, //trackerhit14rt
      2563, 3, //trackerhit14v
//+1014 for TSF1
      1873 + 1014 + 105, 0, //validetf
      1872 + 1014 + 105, 8, //etfcc
      1863 + 1014 + 105, 8, //#ofetf
      1854 + 1014 + 105, 8, //etfhit0cc
      1845 + 1014 + 105, 8, //etfhit1cc
      1836 + 1014 + 105, 8, //etfhit2cc
      1827 + 1014 + 105, 8, //etfhit3cc
      1818 + 1014 + 105, 8, //etfhit4cc
      1809 + 1014 + 105, 8, //etfhit5cc
      1800 + 1014 + 105, 8, //etfhit6cc
      1791 + 1014 + 105, 8, //etfhit7cc
      1782 + 1014 + 105, 8, //etfhit8cc
      1773 + 1014 + 105, 8, //etfhit9cc

      255, 255, //mgr0
      244,   8, //mgr0cc
      511, 255, //mgr1
      500,   8, //mgr1cc
      767, 255, //mgr2
      756,   8, //mgr2cc
      1023, 255, //mgr3
      1012,  8, //mgr3cc
      1279, 255, //mgr4
      1268,  8, //mgr4cc
      1535, 255, //mgr5
      1524,  8, //mgr5cc

      147,    3, //mgr0ft0
      151,    3, //mgr0ft1
      155,    3, //mgr0ft2
      159,    3, //mgr0ft3
      163,    3, //mgr0ft4
      167,    3, //mgr0ft5
      171,    3, //mgr0ft6
      175,    3, //mgr0ft7
      179,    3, //mgr0ft8
      183,    3, //mgr0ft9
      187,    3, //mgr0ft10
      191,    3, //mgr0ft11
      195,    3, //mgr0ft12
      199,    3, //mgr0ft13
      203,    3, //mgr0ft14
      207,    3, //mgr0ft15

      147 + 256,    3, //mgr1ft0
      151 + 256,    3, //mgr1ft1
      155 + 256,    3, //mgr1ft2
      159 + 256,    3, //mgr1ft3
      163 + 256,    3, //mgr1ft4
      167 + 256,    3, //mgr1ft5
      171 + 256,    3, //mgr1ft6
      175 + 256,    3, //mgr1ft7
      179 + 256,    3, //mgr1ft8
      183 + 256,    3, //mgr1ft9
      187 + 256,    3, //mgr1ft10
      191 + 256,    3, //mgr1ft11
      195 + 256,    3, //mgr1ft12
      199 + 256,    3, //mgr1ft13
      203 + 256,    3, //mgr1ft14
      207 + 256,    3, //mgr1ft15
      147 + 512,    3, //mgr2ft0
      151 + 512,    3, //mgr2ft1
      155 + 512,    3, //mgr2ft2
      159 + 512,    3, //mgr2ft3
      163 + 512,    3, //mgr2ft4
      167 + 512,    3, //mgr2ft5
      171 + 512,    3, //mgr2ft6
      175 + 512,    3, //mgr2ft7
      179 + 512,    3, //mgr2ft8
      183 + 512,    3, //mgr2ft9
      187 + 512,    3, //mgr2ft10
      191 + 512,    3, //mgr2ft11
      195 + 512,    3, //mgr2ft12
      199 + 512,    3, //mgr2ft13
      203 + 512,    3, //mgr2ft14
      207 + 512,    3, //mgr2ft15
      147 + 768,    3, //mgr3ft0
      151 + 768,    3, //mgr3ft1
      155 + 768,    3, //mgr3ft2
      159 + 768,    3, //mgr3ft3
      163 + 768,    3, //mgr3ft4
      167 + 768,    3, //mgr3ft5
      171 + 768,    3, //mgr3ft6
      175 + 768,    3, //mgr3ft7
      179 + 768,    3, //mgr3ft8
      183 + 768,    3, //mgr3ft9
      187 + 768,    3, //mgr3ft10
      191 + 768,    3, //mgr3ft11
      195 + 768,    3, //mgr3ft12
      199 + 768,    3, //mgr3ft13
      203 + 768,    3, //mgr3ft14
      207 + 768,    3, //mgr3ft15

      147 + 1024,    3, //mgr4ft0
      151 + 1024,    3, //mgr4ft1
      155 + 1024,    3, //mgr4ft2
      159 + 1024,    3, //mgr4ft3
      163 + 1024,    3, //mgr4ft4
      167 + 1024,    3, //mgr4ft5
      171 + 1024,    3, //mgr4ft6
      175 + 1024,    3, //mgr4ft7
      179 + 1024,    3, //mgr4ft8
      183 + 1024,    3, //mgr4ft9
      187 + 1024,    3, //mgr4ft10
      191 + 1024,    3, //mgr4ft11
      195 + 1024,    3, //mgr4ft12
      199 + 1024,    3, //mgr4ft13
      203 + 1024,    3, //mgr4ft14
      207 + 1024,    3, //mgr4ft15

      147 - 64,    3, //mgr0pt0
      151 - 64,    3, //mgr0pt1
      155 - 64,    3, //mgr0pt2
      159 - 64,    3, //mgr0pt3
      163 - 64,    3, //mgr0pt4
      167 - 64,    3, //mgr0pt5
      171 - 64,    3, //mgr0pt6
      175 - 64,    3, //mgr0pt7
      179 - 64,    3, //mgr0pt8
      183 - 64,    3, //mgr0pt9
      187 - 64,    3, //mgr0pt10
      191 - 64,    3, //mgr0pt11
      195 - 64,    3, //mgr0pt12
      199 - 64,    3, //mgr0pt13
      203 - 64,    3, //mgr0pt14
      207 - 64,    3, //mgr0pt15

      147 + 256 - 64,    3, //mgr1ft0
      151 + 256 - 64,    3, //mgr1ft1
      155 + 256 - 64,    3, //mgr1ft2
      159 + 256 - 64,    3, //mgr1ft3
      163 + 256 - 64,    3, //mgr1ft4
      167 + 256 - 64,    3, //mgr1ft5
      171 + 256 - 64,    3, //mgr1ft6
      175 + 256 - 64,    3, //mgr1ft7
      179 + 256 - 64,    3, //mgr1ft8
      183 + 256 - 64,    3, //mgr1ft9
      187 + 256 - 64,    3, //mgr1ft10
      191 + 256 - 64,    3, //mgr1ft11
      195 + 256 - 64,    3, //mgr1ft12
      199 + 256 - 64,    3, //mgr1ft13
      203 + 256 - 64,    3, //mgr1ft14
      207 + 256 - 64,    3, //mgr1ft15
      147 + 512 - 64,    3, //mgr2ft0
      151 + 512 - 64,    3, //mgr2ft1
      155 + 512 - 64,    3, //mgr2ft2
      159 + 512 - 64,    3, //mgr2ft3
      163 + 512 - 64,    3, //mgr2ft4
      167 + 512 - 64,    3, //mgr2ft5
      171 + 512 - 64,    3, //mgr2ft6
      175 + 512 - 64,    3, //mgr2ft7
      179 + 512 - 64,    3, //mgr2ft8
      183 + 512 - 64,    3, //mgr2ft9
      187 + 512 - 64,    3, //mgr2ft10
      191 + 512 - 64,    3, //mgr2ft11
      195 + 512 - 64,    3, //mgr2ft12
      199 + 512 - 64,    3, //mgr2ft13
      203 + 512 - 64,    3, //mgr2ft14
      207 + 512 - 64,    3, //mgr2ft15
      147 + 768 - 64,    3, //mgr3ft0
      151 + 768 - 64,    3, //mgr3ft1
      155 + 768 - 64,    3, //mgr3ft2
      159 + 768 - 64,    3, //mgr3ft3
      163 + 768 - 64,    3, //mgr3ft4
      167 + 768 - 64,    3, //mgr3ft5
      171 + 768 - 64,    3, //mgr3ft6
      175 + 768 - 64,    3, //mgr3ft7
      179 + 768 - 64,    3, //mgr3ft8
      183 + 768 - 64,    3, //mgr3ft9
      187 + 768 - 64,    3, //mgr3ft10
      191 + 768 - 64,    3, //mgr3ft11
      195 + 768 - 64,    3, //mgr3ft12
      199 + 768 - 64,    3, //mgr3ft13
      203 + 768 - 64,    3, //mgr3ft14
      207 + 768 - 64,    3, //mgr3ft15
      147 + 1024 - 64,    3, //mgr4ft0
      151 + 1024 - 64,    3, //mgr4ft1
      155 + 1024 - 64,    3, //mgr4ft2
      159 + 1024 - 64,    3, //mgr4ft3
      163 + 1024 - 64,    3, //mgr4ft4
      167 + 1024 - 64,    3, //mgr4ft5
      171 + 1024 - 64,    3, //mgr4ft6
      175 + 1024 - 64,    3, //mgr4ft7
      179 + 1024 - 64,    3, //mgr4ft8
      183 + 1024 - 64,    3, //mgr4ft9
      187 + 1024 - 64,    3, //mgr4ft10
      191 + 1024 - 64,    3, //mgr4ft11
      195 + 1024 - 64,    3, //mgr4ft12
      199 + 1024 - 64,    3, //mgr4ft13
      203 + 1024 - 64,    3, //mgr4ft14
      207 + 1024 - 64,    3, //mgr4ft15


      15, 15, //mgr0l0hit
      31, 15, //mgr0l1hit
//    2031, 15, //mgr0l0hit
//    2047, 15, //mgr0l1hit
      47, 15, //mgr0l2hit
      63, 15, //mgr0l3hit
      79, 15, //mgr0l4hit
      15 + 256, 15, //mgr1l0hit
      31 + 256, 15, //mgr1l1hit
      47 + 256, 15, //mgr1l2hit
      63 + 256, 15, //mgr1l3hit
      79 + 256, 15, //mgr1l4hit
      15 + 512, 15, //mgr2l0hit
      31 + 512, 15, //mgr2l1hit
      47 + 512, 15, //mgr2l2hit
      63 + 512, 15, //mgr2l3hit
      79 + 512, 15, //mgr2l4hit
      15 + 768, 15, //mgr3l0hit
      31 + 768, 15, //mgr3l1hit
      47 + 768, 15, //mgr3l2hit
      63 + 768, 15, //mgr3l3hit
      79 + 768, 15, //mgr3l4hit
      15 + 1024,  15, //mgr4l0hit
      31 + 1024,  15, //mgr4l1hit
      47 + 1024,  15, //mgr4l2hit
      63 + 1024,  15, //mgr4l3hit
      79 + 1024,  15, //mgr4l4hit


      31 ,  31,
      63 ,  31,
      95 ,  31,
      127,  31,
      159,  31,
      191,  31,
      223,  31,
      255,  31,

      31 + 256, 31,
      63 + 256, 31,
      95 + 256, 31,
      127 + 256,  31,
      159 + 256,  31,
      191 + 256,  31,
      223 + 256,  31,
      255 + 256,  31,

      31 + 512, 31,
      63 + 512, 31,
      95 + 512, 31,
      127 + 512,  31,
      159 + 512,  31,
      191 + 512,  31,
      223 + 512,  31,
      255 + 512,  31,

      31 + 768, 31,
      63 + 768, 31,
      95 + 768, 31,
      127 + 768,  31,
      159 + 768,  31,
      191 + 768,  31,
      223 + 768,  31,
      255 + 768,  31,

      31 + 1024,  31,
      63 + 1024,  31,
      95 + 1024,  31,
      127 + 1024, 31,
      159 + 1024, 31,
      191 + 1024, 31,
      223 + 1024, 31,
      255 + 1024, 31


    };
    /// set the pointer array of each leaf
    void
    setLeafPointersArray(TRGCDCTSFUnpackerStore* store, int** bitArray)
    {
      bitArray[e_c127    ] = &(store->m_c127);
//      bitArray[e_tshitmap] = &(store->m_tshitmap);
      bitArray[e_validtracker] = &(store->m_validtracker);
      bitArray[e_trackercc] = &(store->m_trackercc);
      bitArray[e_trackerhit0id] = &(store->m_trackerhit0id);
      bitArray[e_trackerhit0rt] = &(store->m_trackerhit0rt);
      bitArray[e_trackerhit0v] = &(store->m_trackerhit0v);
      bitArray[e_trackerhit1id] = &(store->m_trackerhit1id);
      bitArray[e_trackerhit1rt] = &(store->m_trackerhit1rt);
      bitArray[e_trackerhit1v] = &(store->m_trackerhit1v);
      bitArray[e_trackerhit2id] = &(store->m_trackerhit2id);
      bitArray[e_trackerhit2rt] = &(store->m_trackerhit2rt);
      bitArray[e_trackerhit2v] = &(store->m_trackerhit2v);
      bitArray[e_trackerhit3id] = &(store->m_trackerhit3id);
      bitArray[e_trackerhit3rt] = &(store->m_trackerhit3rt);
      bitArray[e_trackerhit3v] = &(store->m_trackerhit3v);
      bitArray[e_trackerhit4id] = &(store->m_trackerhit4id);
      bitArray[e_trackerhit4rt] = &(store->m_trackerhit4rt);
      bitArray[e_trackerhit4v] = &(store->m_trackerhit4v);
      bitArray[e_trackerhit5id] = &(store->m_trackerhit5id);
      bitArray[e_trackerhit5rt] = &(store->m_trackerhit5rt);
      bitArray[e_trackerhit5v] = &(store->m_trackerhit5v);
      bitArray[e_trackerhit6id] = &(store->m_trackerhit6id);
      bitArray[e_trackerhit6rt] = &(store->m_trackerhit6rt);
      bitArray[e_trackerhit6v] = &(store->m_trackerhit6v);
      bitArray[e_trackerhit7id] = &(store->m_trackerhit7id);
      bitArray[e_trackerhit7rt] = &(store->m_trackerhit7rt);
      bitArray[e_trackerhit7v] = &(store->m_trackerhit7v);
      bitArray[e_trackerhit8id] = &(store->m_trackerhit8id);
      bitArray[e_trackerhit8rt] = &(store->m_trackerhit8rt);
      bitArray[e_trackerhit8v] = &(store->m_trackerhit8v);
      bitArray[e_trackerhit9id] = &(store->m_trackerhit9id);
      bitArray[e_trackerhit9rt] = &(store->m_trackerhit9rt);
      bitArray[e_trackerhit9v] = &(store->m_trackerhit9v);
      bitArray[e_validetf] = & (store->m_validetf);
      bitArray[e_etfcc] = &(store->m_etfcc);
      bitArray[e_netfhit] = &(store->m_netfhit);
      bitArray[e_etfhit0] = &(store->m_etfhit0);
      bitArray[e_etfhit1] = &(store->m_etfhit1);
      bitArray[e_etfhit2] = &(store->m_etfhit2);
      bitArray[e_etfhit3] = &(store->m_etfhit3);
      bitArray[e_etfhit4] = &(store->m_etfhit4);
      bitArray[e_etfhit5] = &(store->m_etfhit5);
      bitArray[e_etfhit6] = &(store->m_etfhit6);
      bitArray[e_etfhit7] = &(store->m_etfhit7);
      bitArray[e_etfhit8] = &(store->m_etfhit8);
      bitArray[e_etfhit9] = &(store->m_etfhit9);
      bitArray[e_mgr0] = &(store->m_mgr0);
      bitArray[e_mgr0cc] = &(store->m_mgr0cc);
      bitArray[e_mgr1] = &(store->m_mgr1);
      bitArray[e_mgr1cc] = &(store->m_mgr1cc);
      bitArray[e_mgr2] = &(store->m_mgr2);
      bitArray[e_mgr2cc] = &(store->m_mgr2cc);
      bitArray[e_mgr3] = &(store->m_mgr3);
      bitArray[e_mgr3cc] = &(store->m_mgr3cc);
      bitArray[e_mgr4] = &(store->m_mgr4);
      bitArray[e_mgr4cc] = &(store->m_mgr4cc);
      bitArray[e_mgr5] = &(store->m_mgr5);
      bitArray[e_mgr5cc] = &(store->m_mgr5cc);
      bitArray[e_mgr0ft0]  = &(store->m_mgr0ft0);
      bitArray[e_mgr0ft1]  = &(store->m_mgr0ft1);
      bitArray[e_mgr0ft2]  = &(store->m_mgr0ft2);
      bitArray[e_mgr0ft3]  = &(store->m_mgr0ft3);
      bitArray[e_mgr0ft4]  = &(store->m_mgr0ft4);
      bitArray[e_mgr0ft5]  = &(store->m_mgr0ft5);
      bitArray[e_mgr0ft6]  = &(store->m_mgr0ft6);
      bitArray[e_mgr0ft7]  = &(store->m_mgr0ft7);
      bitArray[e_mgr0ft8]  = &(store->m_mgr0ft8);
      bitArray[e_mgr0ft9]  = &(store->m_mgr0ft9);
      bitArray[e_mgr0ft10] = &(store->m_mgr0ft10);
      bitArray[e_mgr0ft11] = &(store->m_mgr0ft11);
      bitArray[e_mgr0ft12] = &(store->m_mgr0ft12);
      bitArray[e_mgr0ft13] = &(store->m_mgr0ft13);
      bitArray[e_mgr0ft14] = &(store->m_mgr0ft14);
      bitArray[e_mgr0ft15] = &(store->m_mgr0ft15);
      bitArray[e_mgr1ft0]  = &(store->m_mgr1ft0);
      bitArray[e_mgr1ft1]  = &(store->m_mgr1ft1);
      bitArray[e_mgr1ft2]  = &(store->m_mgr1ft2);
      bitArray[e_mgr1ft3]  = &(store->m_mgr1ft3);
      bitArray[e_mgr1ft4]  = &(store->m_mgr1ft4);
      bitArray[e_mgr1ft5]  = &(store->m_mgr1ft5);
      bitArray[e_mgr1ft6]  = &(store->m_mgr1ft6);
      bitArray[e_mgr1ft7]  = &(store->m_mgr1ft7);
      bitArray[e_mgr1ft8]  = &(store->m_mgr1ft8);
      bitArray[e_mgr1ft9]  = &(store->m_mgr1ft9);
      bitArray[e_mgr1ft10] = &(store->m_mgr1ft10);
      bitArray[e_mgr1ft11] = &(store->m_mgr1ft11);
      bitArray[e_mgr1ft12] = &(store->m_mgr1ft12);
      bitArray[e_mgr1ft13] = &(store->m_mgr1ft13);
      bitArray[e_mgr1ft14] = &(store->m_mgr1ft14);
      bitArray[e_mgr1ft15] = &(store->m_mgr1ft15);
      bitArray[e_mgr2ft0]  = &(store->m_mgr2ft0);
      bitArray[e_mgr2ft1]  = &(store->m_mgr2ft1);
      bitArray[e_mgr2ft2]  = &(store->m_mgr2ft2);
      bitArray[e_mgr2ft3]  = &(store->m_mgr2ft3);
      bitArray[e_mgr2ft4]  = &(store->m_mgr2ft4);
      bitArray[e_mgr2ft5]  = &(store->m_mgr2ft5);
      bitArray[e_mgr2ft6]  = &(store->m_mgr2ft6);
      bitArray[e_mgr2ft7]  = &(store->m_mgr2ft7);
      bitArray[e_mgr2ft8]  = &(store->m_mgr2ft8);
      bitArray[e_mgr2ft9]  = &(store->m_mgr2ft9);
      bitArray[e_mgr2ft10] = &(store->m_mgr2ft10);
      bitArray[e_mgr2ft11] = &(store->m_mgr2ft11);
      bitArray[e_mgr2ft12] = &(store->m_mgr2ft12);
      bitArray[e_mgr2ft13] = &(store->m_mgr2ft13);
      bitArray[e_mgr2ft14] = &(store->m_mgr2ft14);
      bitArray[e_mgr2ft15] = &(store->m_mgr2ft15);
      bitArray[e_mgr3ft0]  = &(store->m_mgr3ft0);
      bitArray[e_mgr3ft1]  = &(store->m_mgr3ft1);
      bitArray[e_mgr3ft2]  = &(store->m_mgr3ft2);
      bitArray[e_mgr3ft3]  = &(store->m_mgr3ft3);
      bitArray[e_mgr3ft4]  = &(store->m_mgr3ft4);
      bitArray[e_mgr3ft5]  = &(store->m_mgr3ft5);
      bitArray[e_mgr3ft6]  = &(store->m_mgr3ft6);
      bitArray[e_mgr3ft7]  = &(store->m_mgr3ft7);
      bitArray[e_mgr3ft8]  = &(store->m_mgr3ft8);
      bitArray[e_mgr3ft9]  = &(store->m_mgr3ft9);
      bitArray[e_mgr3ft10] = &(store->m_mgr3ft10);
      bitArray[e_mgr3ft11] = &(store->m_mgr3ft11);
      bitArray[e_mgr3ft12] = &(store->m_mgr3ft12);
      bitArray[e_mgr3ft13] = &(store->m_mgr3ft13);
      bitArray[e_mgr3ft14] = &(store->m_mgr3ft14);
      bitArray[e_mgr3ft15] = &(store->m_mgr3ft15);
      bitArray[e_mgr4ft0]  = &(store->m_mgr4ft0);
      bitArray[e_mgr4ft1]  = &(store->m_mgr4ft1);
      bitArray[e_mgr4ft2]  = &(store->m_mgr4ft2);
      bitArray[e_mgr4ft3]  = &(store->m_mgr4ft3);
      bitArray[e_mgr4ft4]  = &(store->m_mgr4ft4);
      bitArray[e_mgr4ft5]  = &(store->m_mgr4ft5);
      bitArray[e_mgr4ft6]  = &(store->m_mgr4ft6);
      bitArray[e_mgr4ft7]  = &(store->m_mgr4ft7);
      bitArray[e_mgr4ft8]  = &(store->m_mgr4ft8);
      bitArray[e_mgr4ft9]  = &(store->m_mgr4ft9);
      bitArray[e_mgr4ft10] = &(store->m_mgr4ft10);
      bitArray[e_mgr4ft11] = &(store->m_mgr4ft11);
      bitArray[e_mgr4ft12] = &(store->m_mgr4ft12);
      bitArray[e_mgr4ft13] = &(store->m_mgr4ft13);
      bitArray[e_mgr4ft14] = &(store->m_mgr4ft14);
      bitArray[e_mgr4ft15] = &(store->m_mgr4ft15);
      bitArray[e_mgr0pt0]  = &(store->m_mgr0pt0);
      bitArray[e_mgr0pt1]  = &(store->m_mgr0pt1);
      bitArray[e_mgr0pt2]  = &(store->m_mgr0pt2);
      bitArray[e_mgr0pt3]  = &(store->m_mgr0pt3);
      bitArray[e_mgr0pt4]  = &(store->m_mgr0pt4);
      bitArray[e_mgr0pt5]  = &(store->m_mgr0pt5);
      bitArray[e_mgr0pt6]  = &(store->m_mgr0pt6);
      bitArray[e_mgr0pt7]  = &(store->m_mgr0pt7);
      bitArray[e_mgr0pt8]  = &(store->m_mgr0pt8);
      bitArray[e_mgr0pt9]  = &(store->m_mgr0pt9);
      bitArray[e_mgr0pt10] = &(store->m_mgr0pt10);
      bitArray[e_mgr0pt11] = &(store->m_mgr0pt11);
      bitArray[e_mgr0pt12] = &(store->m_mgr0pt12);
      bitArray[e_mgr0pt13] = &(store->m_mgr0pt13);
      bitArray[e_mgr0pt14] = &(store->m_mgr0pt14);
      bitArray[e_mgr0pt15] = &(store->m_mgr0pt15);
      bitArray[e_mgr1pt0]  = &(store->m_mgr1pt0);
      bitArray[e_mgr1pt1]  = &(store->m_mgr1pt1);
      bitArray[e_mgr1pt2]  = &(store->m_mgr1pt2);
      bitArray[e_mgr1pt3]  = &(store->m_mgr1pt3);
      bitArray[e_mgr1pt4]  = &(store->m_mgr1pt4);
      bitArray[e_mgr1pt5]  = &(store->m_mgr1pt5);
      bitArray[e_mgr1pt6]  = &(store->m_mgr1pt6);
      bitArray[e_mgr1pt7]  = &(store->m_mgr1pt7);
      bitArray[e_mgr1pt8]  = &(store->m_mgr1pt8);
      bitArray[e_mgr1pt9]  = &(store->m_mgr1pt9);
      bitArray[e_mgr1pt10] = &(store->m_mgr1pt10);
      bitArray[e_mgr1pt11] = &(store->m_mgr1pt11);
      bitArray[e_mgr1pt12] = &(store->m_mgr1pt12);
      bitArray[e_mgr1pt13] = &(store->m_mgr1pt13);
      bitArray[e_mgr1pt14] = &(store->m_mgr1pt14);
      bitArray[e_mgr1pt15] = &(store->m_mgr1pt15);
      bitArray[e_mgr2pt0]  = &(store->m_mgr2pt0);
      bitArray[e_mgr2pt1]  = &(store->m_mgr2pt1);
      bitArray[e_mgr2pt2]  = &(store->m_mgr2pt2);
      bitArray[e_mgr2pt3]  = &(store->m_mgr2pt3);
      bitArray[e_mgr2pt4]  = &(store->m_mgr2pt4);
      bitArray[e_mgr2pt5]  = &(store->m_mgr2pt5);
      bitArray[e_mgr2pt6]  = &(store->m_mgr2pt6);
      bitArray[e_mgr2pt7]  = &(store->m_mgr2pt7);
      bitArray[e_mgr2pt8]  = &(store->m_mgr2pt8);
      bitArray[e_mgr2pt9]  = &(store->m_mgr2pt9);
      bitArray[e_mgr2pt10] = &(store->m_mgr2pt10);
      bitArray[e_mgr2pt11] = &(store->m_mgr2pt11);
      bitArray[e_mgr2pt12] = &(store->m_mgr2pt12);
      bitArray[e_mgr2pt13] = &(store->m_mgr2pt13);
      bitArray[e_mgr2pt14] = &(store->m_mgr2pt14);
      bitArray[e_mgr2pt15] = &(store->m_mgr2pt15);
      bitArray[e_mgr3pt0]  = &(store->m_mgr3pt0);
      bitArray[e_mgr3pt1]  = &(store->m_mgr3pt1);
      bitArray[e_mgr3pt2]  = &(store->m_mgr3pt2);
      bitArray[e_mgr3pt3]  = &(store->m_mgr3pt3);
      bitArray[e_mgr3pt4]  = &(store->m_mgr3pt4);
      bitArray[e_mgr3pt5]  = &(store->m_mgr3pt5);
      bitArray[e_mgr3pt6]  = &(store->m_mgr3pt6);
      bitArray[e_mgr3pt7]  = &(store->m_mgr3pt7);
      bitArray[e_mgr3pt8]  = &(store->m_mgr3pt8);
      bitArray[e_mgr3pt9]  = &(store->m_mgr3pt9);
      bitArray[e_mgr3pt10] = &(store->m_mgr3pt10);
      bitArray[e_mgr3pt11] = &(store->m_mgr3pt11);
      bitArray[e_mgr3pt12] = &(store->m_mgr3pt12);
      bitArray[e_mgr3pt13] = &(store->m_mgr3pt13);
      bitArray[e_mgr3pt14] = &(store->m_mgr3pt14);
      bitArray[e_mgr3pt15] = &(store->m_mgr3pt15);
      bitArray[e_mgr4pt0]  = &(store->m_mgr4pt0);
      bitArray[e_mgr4pt1]  = &(store->m_mgr4pt1);
      bitArray[e_mgr4pt2]  = &(store->m_mgr4pt2);
      bitArray[e_mgr4pt3]  = &(store->m_mgr4pt3);
      bitArray[e_mgr4pt4]  = &(store->m_mgr4pt4);
      bitArray[e_mgr4pt5]  = &(store->m_mgr4pt5);
      bitArray[e_mgr4pt6]  = &(store->m_mgr4pt6);
      bitArray[e_mgr4pt7]  = &(store->m_mgr4pt7);
      bitArray[e_mgr4pt8]  = &(store->m_mgr4pt8);
      bitArray[e_mgr4pt9]  = &(store->m_mgr4pt9);
      bitArray[e_mgr4pt10] = &(store->m_mgr4pt10);
      bitArray[e_mgr4pt11] = &(store->m_mgr4pt11);
      bitArray[e_mgr4pt12] = &(store->m_mgr4pt12);
      bitArray[e_mgr4pt13] = &(store->m_mgr4pt13);
      bitArray[e_mgr4pt14] = &(store->m_mgr4pt14);
      bitArray[e_mgr4pt15] = &(store->m_mgr4pt15);
      bitArray[e_mgr0l0hit] = &(store->m_mgr0l0hit);
      bitArray[e_mgr0l1hit] = &(store->m_mgr0l1hit);
      bitArray[e_mgr0l2hit] = &(store->m_mgr0l2hit);
      bitArray[e_mgr0l3hit] = &(store->m_mgr0l3hit);
      bitArray[e_mgr0l4hit] = &(store->m_mgr0l4hit);
      bitArray[e_mgr1l0hit] = &(store->m_mgr1l0hit);
      bitArray[e_mgr1l1hit] = &(store->m_mgr1l1hit);
      bitArray[e_mgr1l2hit] = &(store->m_mgr1l2hit);
      bitArray[e_mgr1l3hit] = &(store->m_mgr1l3hit);
      bitArray[e_mgr1l4hit] = &(store->m_mgr1l4hit);
      bitArray[e_mgr2l0hit] = &(store->m_mgr2l0hit);
      bitArray[e_mgr2l1hit] = &(store->m_mgr2l1hit);
      bitArray[e_mgr2l2hit] = &(store->m_mgr2l2hit);
      bitArray[e_mgr2l3hit] = &(store->m_mgr2l3hit);
      bitArray[e_mgr2l4hit] = &(store->m_mgr2l4hit);
      bitArray[e_mgr3l0hit] = &(store->m_mgr3l0hit);
      bitArray[e_mgr3l1hit] = &(store->m_mgr3l1hit);
      bitArray[e_mgr3l2hit] = &(store->m_mgr3l2hit);
      bitArray[e_mgr3l3hit] = &(store->m_mgr3l3hit);
      bitArray[e_mgr3l4hit] = &(store->m_mgr3l4hit);
      bitArray[e_mgr4l0hit] = &(store->m_mgr4l0hit);
      bitArray[e_mgr4l1hit] = &(store->m_mgr4l1hit);
      bitArray[e_mgr4l2hit] = &(store->m_mgr4l2hit);
      bitArray[e_mgr4l3hit] = &(store->m_mgr4l3hit);
      bitArray[e_mgr4l4hit] = &(store->m_mgr4l4hit);
      bitArray[e_mgr0l0] = &(store->m_mgr0l0);
      bitArray[e_mgr0l1] = &(store->m_mgr0l1);
      bitArray[e_mgr0l2] = &(store->m_mgr0l2);
      bitArray[e_mgr0l3] = &(store->m_mgr0l3);
      bitArray[e_mgr0l4] = &(store->m_mgr0l4);
      bitArray[e_mgr0l5] = &(store->m_mgr0l5);
      bitArray[e_mgr0l6] = &(store->m_mgr0l6);
      bitArray[e_mgr0l7] = &(store->m_mgr0l7);
      bitArray[e_mgr1l0] = &(store->m_mgr1l0);
      bitArray[e_mgr1l1] = &(store->m_mgr1l1);
      bitArray[e_mgr1l2] = &(store->m_mgr1l2);
      bitArray[e_mgr1l3] = &(store->m_mgr1l3);
      bitArray[e_mgr1l4] = &(store->m_mgr1l4);
      bitArray[e_mgr1l5] = &(store->m_mgr1l5);
      bitArray[e_mgr1l6] = &(store->m_mgr1l6);
      bitArray[e_mgr1l7] = &(store->m_mgr1l7);
      bitArray[e_mgr2l0] = &(store->m_mgr2l0);
      bitArray[e_mgr2l1] = &(store->m_mgr2l1);
      bitArray[e_mgr2l2] = &(store->m_mgr2l2);
      bitArray[e_mgr2l3] = &(store->m_mgr2l3);
      bitArray[e_mgr2l4] = &(store->m_mgr2l4);
      bitArray[e_mgr2l5] = &(store->m_mgr2l5);
      bitArray[e_mgr2l6] = &(store->m_mgr2l6);
      bitArray[e_mgr2l7] = &(store->m_mgr2l7);
      bitArray[e_mgr3l0] = &(store->m_mgr3l0);
      bitArray[e_mgr3l1] = &(store->m_mgr3l1);
      bitArray[e_mgr3l2] = &(store->m_mgr3l2);
      bitArray[e_mgr3l3] = &(store->m_mgr3l3);
      bitArray[e_mgr3l4] = &(store->m_mgr3l4);
      bitArray[e_mgr3l5] = &(store->m_mgr3l5);
      bitArray[e_mgr3l6] = &(store->m_mgr3l6);
      bitArray[e_mgr3l7] = &(store->m_mgr3l7);
      bitArray[e_mgr4l0] = &(store->m_mgr4l0);
      bitArray[e_mgr4l1] = &(store->m_mgr4l1);
      bitArray[e_mgr4l2] = &(store->m_mgr4l2);
      bitArray[e_mgr4l3] = &(store->m_mgr4l3);
      bitArray[e_mgr4l4] = &(store->m_mgr4l4);
      bitArray[e_mgr4l5] = &(store->m_mgr4l5);
      bitArray[e_mgr4l6] = &(store->m_mgr4l6);
      bitArray[e_mgr4l7] = &(store->m_mgr4l7);
      bitArray[e_evt] = &(store->m_evt);
      bitArray[e_clk] = &(store->m_clk);
      bitArray[e_firmid  ] = &(store->m_firmid);
      bitArray[e_firmver ] = &(store->m_firmver);
    }

    /// set the pointer array of each leaf
    void
    setLeafPointersArray_4k15ts(TRGCDCTSFUnpackerStore* store, int** bitArray)
    {
      bitArray[e1_c127    ] = &(store->m_c127);
//      bitArray[e1_tshitmap] = &(store->m_tshitmap);
      bitArray[e1_validtracker] = &(store->m_validtracker);
      bitArray[e1_trackercc] = &(store->m_trackercc);
      bitArray[e1_trackerhit0id] = &(store->m_trackerhit0id);
      bitArray[e1_trackerhit0rt] = &(store->m_trackerhit0rt);
      bitArray[e1_trackerhit0v] = &(store->m_trackerhit0v);
      bitArray[e1_trackerhit1id] = &(store->m_trackerhit1id);
      bitArray[e1_trackerhit1rt] = &(store->m_trackerhit1rt);
      bitArray[e1_trackerhit1v] = &(store->m_trackerhit1v);
      bitArray[e1_trackerhit2id] = &(store->m_trackerhit2id);
      bitArray[e1_trackerhit2rt] = &(store->m_trackerhit2rt);
      bitArray[e1_trackerhit2v] = &(store->m_trackerhit2v);
      bitArray[e1_trackerhit3id] = &(store->m_trackerhit3id);
      bitArray[e1_trackerhit3rt] = &(store->m_trackerhit3rt);
      bitArray[e1_trackerhit3v] = &(store->m_trackerhit3v);
      bitArray[e1_trackerhit4id] = &(store->m_trackerhit4id);
      bitArray[e1_trackerhit4rt] = &(store->m_trackerhit4rt);
      bitArray[e1_trackerhit4v] = &(store->m_trackerhit4v);
      bitArray[e1_trackerhit5id] = &(store->m_trackerhit5id);
      bitArray[e1_trackerhit5rt] = &(store->m_trackerhit5rt);
      bitArray[e1_trackerhit5v] = &(store->m_trackerhit5v);
      bitArray[e1_trackerhit6id] = &(store->m_trackerhit6id);
      bitArray[e1_trackerhit6rt] = &(store->m_trackerhit6rt);
      bitArray[e1_trackerhit6v] = &(store->m_trackerhit6v);
      bitArray[e1_trackerhit7id] = &(store->m_trackerhit7id);
      bitArray[e1_trackerhit7rt] = &(store->m_trackerhit7rt);
      bitArray[e1_trackerhit7v] = &(store->m_trackerhit7v);
      bitArray[e1_trackerhit8id] = &(store->m_trackerhit8id);
      bitArray[e1_trackerhit8rt] = &(store->m_trackerhit8rt);
      bitArray[e1_trackerhit8v] = &(store->m_trackerhit8v);
      bitArray[e1_trackerhit9id] = &(store->m_trackerhit9id);
      bitArray[e1_trackerhit9rt] = &(store->m_trackerhit9rt);
      bitArray[e1_trackerhit9v] = &(store->m_trackerhit9v);
      bitArray[e1_trackerhit10id] = &(store->m_trackerhit10id);
      bitArray[e1_trackerhit10rt] = &(store->m_trackerhit10rt);
      bitArray[e1_trackerhit10v] = &(store->m_trackerhit10v);
      bitArray[e1_trackerhit11id] = &(store->m_trackerhit11id);
      bitArray[e1_trackerhit11rt] = &(store->m_trackerhit11rt);
      bitArray[e1_trackerhit11v] = &(store->m_trackerhit11v);
      bitArray[e1_trackerhit12id] = &(store->m_trackerhit12id);
      bitArray[e1_trackerhit12rt] = &(store->m_trackerhit12rt);
      bitArray[e1_trackerhit12v] = &(store->m_trackerhit12v);
      bitArray[e1_trackerhit13id] = &(store->m_trackerhit13id);
      bitArray[e1_trackerhit13rt] = &(store->m_trackerhit13rt);
      bitArray[e1_trackerhit13v] = &(store->m_trackerhit13v);
      bitArray[e1_trackerhit14id] = &(store->m_trackerhit14id);
      bitArray[e1_trackerhit14rt] = &(store->m_trackerhit14rt);
      bitArray[e1_trackerhit14v] = &(store->m_trackerhit14v);
      bitArray[e1_validetf] = & (store->m_validetf);
      bitArray[e1_etfcc] = &(store->m_etfcc);
      bitArray[e1_netfhit] = &(store->m_netfhit);
      bitArray[e1_etfhit0] = &(store->m_etfhit0);
      bitArray[e1_etfhit1] = &(store->m_etfhit1);
      bitArray[e1_etfhit2] = &(store->m_etfhit2);
      bitArray[e1_etfhit3] = &(store->m_etfhit3);
      bitArray[e1_etfhit4] = &(store->m_etfhit4);
      bitArray[e1_etfhit5] = &(store->m_etfhit5);
      bitArray[e1_etfhit6] = &(store->m_etfhit6);
      bitArray[e1_etfhit7] = &(store->m_etfhit7);
      bitArray[e1_etfhit8] = &(store->m_etfhit8);
      bitArray[e1_etfhit9] = &(store->m_etfhit9);
      bitArray[e1_mgr0] = &(store->m_mgr0);
      bitArray[e1_mgr0cc] = &(store->m_mgr0cc);
      bitArray[e1_mgr1] = &(store->m_mgr1);
      bitArray[e1_mgr1cc] = &(store->m_mgr1cc);
      bitArray[e1_mgr2] = &(store->m_mgr2);
      bitArray[e1_mgr2cc] = &(store->m_mgr2cc);
      bitArray[e1_mgr3] = &(store->m_mgr3);
      bitArray[e1_mgr3cc] = &(store->m_mgr3cc);
      bitArray[e1_mgr4] = &(store->m_mgr4);
      bitArray[e1_mgr4cc] = &(store->m_mgr4cc);
      bitArray[e1_mgr5] = &(store->m_mgr5);
      bitArray[e1_mgr5cc] = &(store->m_mgr5cc);
      bitArray[e1_mgr0ft0]  = &(store->m_mgr0ft0);
      bitArray[e1_mgr0ft1]  = &(store->m_mgr0ft1);
      bitArray[e1_mgr0ft2]  = &(store->m_mgr0ft2);
      bitArray[e1_mgr0ft3]  = &(store->m_mgr0ft3);
      bitArray[e1_mgr0ft4]  = &(store->m_mgr0ft4);
      bitArray[e1_mgr0ft5]  = &(store->m_mgr0ft5);
      bitArray[e1_mgr0ft6]  = &(store->m_mgr0ft6);
      bitArray[e1_mgr0ft7]  = &(store->m_mgr0ft7);
      bitArray[e1_mgr0ft8]  = &(store->m_mgr0ft8);
      bitArray[e1_mgr0ft9]  = &(store->m_mgr0ft9);
      bitArray[e1_mgr0ft10] = &(store->m_mgr0ft10);
      bitArray[e1_mgr0ft11] = &(store->m_mgr0ft11);
      bitArray[e1_mgr0ft12] = &(store->m_mgr0ft12);
      bitArray[e1_mgr0ft13] = &(store->m_mgr0ft13);
      bitArray[e1_mgr0ft14] = &(store->m_mgr0ft14);
      bitArray[e1_mgr0ft15] = &(store->m_mgr0ft15);
      bitArray[e1_mgr1ft0]  = &(store->m_mgr1ft0);
      bitArray[e1_mgr1ft1]  = &(store->m_mgr1ft1);
      bitArray[e1_mgr1ft2]  = &(store->m_mgr1ft2);
      bitArray[e1_mgr1ft3]  = &(store->m_mgr1ft3);
      bitArray[e1_mgr1ft4]  = &(store->m_mgr1ft4);
      bitArray[e1_mgr1ft5]  = &(store->m_mgr1ft5);
      bitArray[e1_mgr1ft6]  = &(store->m_mgr1ft6);
      bitArray[e1_mgr1ft7]  = &(store->m_mgr1ft7);
      bitArray[e1_mgr1ft8]  = &(store->m_mgr1ft8);
      bitArray[e1_mgr1ft9]  = &(store->m_mgr1ft9);
      bitArray[e1_mgr1ft10] = &(store->m_mgr1ft10);
      bitArray[e1_mgr1ft11] = &(store->m_mgr1ft11);
      bitArray[e1_mgr1ft12] = &(store->m_mgr1ft12);
      bitArray[e1_mgr1ft13] = &(store->m_mgr1ft13);
      bitArray[e1_mgr1ft14] = &(store->m_mgr1ft14);
      bitArray[e1_mgr1ft15] = &(store->m_mgr1ft15);
      bitArray[e1_mgr2ft0]  = &(store->m_mgr2ft0);
      bitArray[e1_mgr2ft1]  = &(store->m_mgr2ft1);
      bitArray[e1_mgr2ft2]  = &(store->m_mgr2ft2);
      bitArray[e1_mgr2ft3]  = &(store->m_mgr2ft3);
      bitArray[e1_mgr2ft4]  = &(store->m_mgr2ft4);
      bitArray[e1_mgr2ft5]  = &(store->m_mgr2ft5);
      bitArray[e1_mgr2ft6]  = &(store->m_mgr2ft6);
      bitArray[e1_mgr2ft7]  = &(store->m_mgr2ft7);
      bitArray[e1_mgr2ft8]  = &(store->m_mgr2ft8);
      bitArray[e1_mgr2ft9]  = &(store->m_mgr2ft9);
      bitArray[e1_mgr2ft10] = &(store->m_mgr2ft10);
      bitArray[e1_mgr2ft11] = &(store->m_mgr2ft11);
      bitArray[e1_mgr2ft12] = &(store->m_mgr2ft12);
      bitArray[e1_mgr2ft13] = &(store->m_mgr2ft13);
      bitArray[e1_mgr2ft14] = &(store->m_mgr2ft14);
      bitArray[e1_mgr2ft15] = &(store->m_mgr2ft15);
      bitArray[e1_mgr3ft0]  = &(store->m_mgr3ft0);
      bitArray[e1_mgr3ft1]  = &(store->m_mgr3ft1);
      bitArray[e1_mgr3ft2]  = &(store->m_mgr3ft2);
      bitArray[e1_mgr3ft3]  = &(store->m_mgr3ft3);
      bitArray[e1_mgr3ft4]  = &(store->m_mgr3ft4);
      bitArray[e1_mgr3ft5]  = &(store->m_mgr3ft5);
      bitArray[e1_mgr3ft6]  = &(store->m_mgr3ft6);
      bitArray[e1_mgr3ft7]  = &(store->m_mgr3ft7);
      bitArray[e1_mgr3ft8]  = &(store->m_mgr3ft8);
      bitArray[e1_mgr3ft9]  = &(store->m_mgr3ft9);
      bitArray[e1_mgr3ft10] = &(store->m_mgr3ft10);
      bitArray[e1_mgr3ft11] = &(store->m_mgr3ft11);
      bitArray[e1_mgr3ft12] = &(store->m_mgr3ft12);
      bitArray[e1_mgr3ft13] = &(store->m_mgr3ft13);
      bitArray[e1_mgr3ft14] = &(store->m_mgr3ft14);
      bitArray[e1_mgr3ft15] = &(store->m_mgr3ft15);
      bitArray[e1_mgr4ft0]  = &(store->m_mgr4ft0);
      bitArray[e1_mgr4ft1]  = &(store->m_mgr4ft1);
      bitArray[e1_mgr4ft2]  = &(store->m_mgr4ft2);
      bitArray[e1_mgr4ft3]  = &(store->m_mgr4ft3);
      bitArray[e1_mgr4ft4]  = &(store->m_mgr4ft4);
      bitArray[e1_mgr4ft5]  = &(store->m_mgr4ft5);
      bitArray[e1_mgr4ft6]  = &(store->m_mgr4ft6);
      bitArray[e1_mgr4ft7]  = &(store->m_mgr4ft7);
      bitArray[e1_mgr4ft8]  = &(store->m_mgr4ft8);
      bitArray[e1_mgr4ft9]  = &(store->m_mgr4ft9);
      bitArray[e1_mgr4ft10] = &(store->m_mgr4ft10);
      bitArray[e1_mgr4ft11] = &(store->m_mgr4ft11);
      bitArray[e1_mgr4ft12] = &(store->m_mgr4ft12);
      bitArray[e1_mgr4ft13] = &(store->m_mgr4ft13);
      bitArray[e1_mgr4ft14] = &(store->m_mgr4ft14);
      bitArray[e1_mgr4ft15] = &(store->m_mgr4ft15);
      bitArray[e1_mgr0pt0]  = &(store->m_mgr0pt0);
      bitArray[e1_mgr0pt1]  = &(store->m_mgr0pt1);
      bitArray[e1_mgr0pt2]  = &(store->m_mgr0pt2);
      bitArray[e1_mgr0pt3]  = &(store->m_mgr0pt3);
      bitArray[e1_mgr0pt4]  = &(store->m_mgr0pt4);
      bitArray[e1_mgr0pt5]  = &(store->m_mgr0pt5);
      bitArray[e1_mgr0pt6]  = &(store->m_mgr0pt6);
      bitArray[e1_mgr0pt7]  = &(store->m_mgr0pt7);
      bitArray[e1_mgr0pt8]  = &(store->m_mgr0pt8);
      bitArray[e1_mgr0pt9]  = &(store->m_mgr0pt9);
      bitArray[e1_mgr0pt10] = &(store->m_mgr0pt10);
      bitArray[e1_mgr0pt11] = &(store->m_mgr0pt11);
      bitArray[e1_mgr0pt12] = &(store->m_mgr0pt12);
      bitArray[e1_mgr0pt13] = &(store->m_mgr0pt13);
      bitArray[e1_mgr0pt14] = &(store->m_mgr0pt14);
      bitArray[e1_mgr0pt15] = &(store->m_mgr0pt15);
      bitArray[e1_mgr1pt0]  = &(store->m_mgr1pt0);
      bitArray[e1_mgr1pt1]  = &(store->m_mgr1pt1);
      bitArray[e1_mgr1pt2]  = &(store->m_mgr1pt2);
      bitArray[e1_mgr1pt3]  = &(store->m_mgr1pt3);
      bitArray[e1_mgr1pt4]  = &(store->m_mgr1pt4);
      bitArray[e1_mgr1pt5]  = &(store->m_mgr1pt5);
      bitArray[e1_mgr1pt6]  = &(store->m_mgr1pt6);
      bitArray[e1_mgr1pt7]  = &(store->m_mgr1pt7);
      bitArray[e1_mgr1pt8]  = &(store->m_mgr1pt8);
      bitArray[e1_mgr1pt9]  = &(store->m_mgr1pt9);
      bitArray[e1_mgr1pt10] = &(store->m_mgr1pt10);
      bitArray[e1_mgr1pt11] = &(store->m_mgr1pt11);
      bitArray[e1_mgr1pt12] = &(store->m_mgr1pt12);
      bitArray[e1_mgr1pt13] = &(store->m_mgr1pt13);
      bitArray[e1_mgr1pt14] = &(store->m_mgr1pt14);
      bitArray[e1_mgr1pt15] = &(store->m_mgr1pt15);
      bitArray[e1_mgr2pt0]  = &(store->m_mgr2pt0);
      bitArray[e1_mgr2pt1]  = &(store->m_mgr2pt1);
      bitArray[e1_mgr2pt2]  = &(store->m_mgr2pt2);
      bitArray[e1_mgr2pt3]  = &(store->m_mgr2pt3);
      bitArray[e1_mgr2pt4]  = &(store->m_mgr2pt4);
      bitArray[e1_mgr2pt5]  = &(store->m_mgr2pt5);
      bitArray[e1_mgr2pt6]  = &(store->m_mgr2pt6);
      bitArray[e1_mgr2pt7]  = &(store->m_mgr2pt7);
      bitArray[e1_mgr2pt8]  = &(store->m_mgr2pt8);
      bitArray[e1_mgr2pt9]  = &(store->m_mgr2pt9);
      bitArray[e1_mgr2pt10] = &(store->m_mgr2pt10);
      bitArray[e1_mgr2pt11] = &(store->m_mgr2pt11);
      bitArray[e1_mgr2pt12] = &(store->m_mgr2pt12);
      bitArray[e1_mgr2pt13] = &(store->m_mgr2pt13);
      bitArray[e1_mgr2pt14] = &(store->m_mgr2pt14);
      bitArray[e1_mgr2pt15] = &(store->m_mgr2pt15);
      bitArray[e1_mgr3pt0]  = &(store->m_mgr3pt0);
      bitArray[e1_mgr3pt1]  = &(store->m_mgr3pt1);
      bitArray[e1_mgr3pt2]  = &(store->m_mgr3pt2);
      bitArray[e1_mgr3pt3]  = &(store->m_mgr3pt3);
      bitArray[e1_mgr3pt4]  = &(store->m_mgr3pt4);
      bitArray[e1_mgr3pt5]  = &(store->m_mgr3pt5);
      bitArray[e1_mgr3pt6]  = &(store->m_mgr3pt6);
      bitArray[e1_mgr3pt7]  = &(store->m_mgr3pt7);
      bitArray[e1_mgr3pt8]  = &(store->m_mgr3pt8);
      bitArray[e1_mgr3pt9]  = &(store->m_mgr3pt9);
      bitArray[e1_mgr3pt10] = &(store->m_mgr3pt10);
      bitArray[e1_mgr3pt11] = &(store->m_mgr3pt11);
      bitArray[e1_mgr3pt12] = &(store->m_mgr3pt12);
      bitArray[e1_mgr3pt13] = &(store->m_mgr3pt13);
      bitArray[e1_mgr3pt14] = &(store->m_mgr3pt14);
      bitArray[e1_mgr3pt15] = &(store->m_mgr3pt15);
      bitArray[e1_mgr4pt0]  = &(store->m_mgr4pt0);
      bitArray[e1_mgr4pt1]  = &(store->m_mgr4pt1);
      bitArray[e1_mgr4pt2]  = &(store->m_mgr4pt2);
      bitArray[e1_mgr4pt3]  = &(store->m_mgr4pt3);
      bitArray[e1_mgr4pt4]  = &(store->m_mgr4pt4);
      bitArray[e1_mgr4pt5]  = &(store->m_mgr4pt5);
      bitArray[e1_mgr4pt6]  = &(store->m_mgr4pt6);
      bitArray[e1_mgr4pt7]  = &(store->m_mgr4pt7);
      bitArray[e1_mgr4pt8]  = &(store->m_mgr4pt8);
      bitArray[e1_mgr4pt9]  = &(store->m_mgr4pt9);
      bitArray[e1_mgr4pt10] = &(store->m_mgr4pt10);
      bitArray[e1_mgr4pt11] = &(store->m_mgr4pt11);
      bitArray[e1_mgr4pt12] = &(store->m_mgr4pt12);
      bitArray[e1_mgr4pt13] = &(store->m_mgr4pt13);
      bitArray[e1_mgr4pt14] = &(store->m_mgr4pt14);
      bitArray[e1_mgr4pt15] = &(store->m_mgr4pt15);
      bitArray[e1_mgr0l0hit] = &(store->m_mgr0l0hit);
      bitArray[e1_mgr0l1hit] = &(store->m_mgr0l1hit);
      bitArray[e1_mgr0l2hit] = &(store->m_mgr0l2hit);
      bitArray[e1_mgr0l3hit] = &(store->m_mgr0l3hit);
      bitArray[e1_mgr0l4hit] = &(store->m_mgr0l4hit);
      bitArray[e1_mgr1l0hit] = &(store->m_mgr1l0hit);
      bitArray[e1_mgr1l1hit] = &(store->m_mgr1l1hit);
      bitArray[e1_mgr1l2hit] = &(store->m_mgr1l2hit);
      bitArray[e1_mgr1l3hit] = &(store->m_mgr1l3hit);
      bitArray[e1_mgr1l4hit] = &(store->m_mgr1l4hit);
      bitArray[e1_mgr2l0hit] = &(store->m_mgr2l0hit);
      bitArray[e1_mgr2l1hit] = &(store->m_mgr2l1hit);
      bitArray[e1_mgr2l2hit] = &(store->m_mgr2l2hit);
      bitArray[e1_mgr2l3hit] = &(store->m_mgr2l3hit);
      bitArray[e1_mgr2l4hit] = &(store->m_mgr2l4hit);
      bitArray[e1_mgr3l0hit] = &(store->m_mgr3l0hit);
      bitArray[e1_mgr3l1hit] = &(store->m_mgr3l1hit);
      bitArray[e1_mgr3l2hit] = &(store->m_mgr3l2hit);
      bitArray[e1_mgr3l3hit] = &(store->m_mgr3l3hit);
      bitArray[e1_mgr3l4hit] = &(store->m_mgr3l4hit);
      bitArray[e1_mgr4l0hit] = &(store->m_mgr4l0hit);
      bitArray[e1_mgr4l1hit] = &(store->m_mgr4l1hit);
      bitArray[e1_mgr4l2hit] = &(store->m_mgr4l2hit);
      bitArray[e1_mgr4l3hit] = &(store->m_mgr4l3hit);
      bitArray[e1_mgr4l4hit] = &(store->m_mgr4l4hit);
      bitArray[e1_mgr0l0] = &(store->m_mgr0l0);
      bitArray[e1_mgr0l1] = &(store->m_mgr0l1);
      bitArray[e1_mgr0l2] = &(store->m_mgr0l2);
      bitArray[e1_mgr0l3] = &(store->m_mgr0l3);
      bitArray[e1_mgr0l4] = &(store->m_mgr0l4);
      bitArray[e1_mgr0l5] = &(store->m_mgr0l5);
      bitArray[e1_mgr0l6] = &(store->m_mgr0l6);
      bitArray[e1_mgr0l7] = &(store->m_mgr0l7);
      bitArray[e1_mgr1l0] = &(store->m_mgr1l0);
      bitArray[e1_mgr1l1] = &(store->m_mgr1l1);
      bitArray[e1_mgr1l2] = &(store->m_mgr1l2);
      bitArray[e1_mgr1l3] = &(store->m_mgr1l3);
      bitArray[e1_mgr1l4] = &(store->m_mgr1l4);
      bitArray[e1_mgr1l5] = &(store->m_mgr1l5);
      bitArray[e1_mgr1l6] = &(store->m_mgr1l6);
      bitArray[e1_mgr1l7] = &(store->m_mgr1l7);
      bitArray[e1_mgr2l0] = &(store->m_mgr2l0);
      bitArray[e1_mgr2l1] = &(store->m_mgr2l1);
      bitArray[e1_mgr2l2] = &(store->m_mgr2l2);
      bitArray[e1_mgr2l3] = &(store->m_mgr2l3);
      bitArray[e1_mgr2l4] = &(store->m_mgr2l4);
      bitArray[e1_mgr2l5] = &(store->m_mgr2l5);
      bitArray[e1_mgr2l6] = &(store->m_mgr2l6);
      bitArray[e1_mgr2l7] = &(store->m_mgr2l7);
      bitArray[e1_mgr3l0] = &(store->m_mgr3l0);
      bitArray[e1_mgr3l1] = &(store->m_mgr3l1);
      bitArray[e1_mgr3l2] = &(store->m_mgr3l2);
      bitArray[e1_mgr3l3] = &(store->m_mgr3l3);
      bitArray[e1_mgr3l4] = &(store->m_mgr3l4);
      bitArray[e1_mgr3l5] = &(store->m_mgr3l5);
      bitArray[e1_mgr3l6] = &(store->m_mgr3l6);
      bitArray[e1_mgr3l7] = &(store->m_mgr3l7);
      bitArray[e1_mgr4l0] = &(store->m_mgr4l0);
      bitArray[e1_mgr4l1] = &(store->m_mgr4l1);
      bitArray[e1_mgr4l2] = &(store->m_mgr4l2);
      bitArray[e1_mgr4l3] = &(store->m_mgr4l3);
      bitArray[e1_mgr4l4] = &(store->m_mgr4l4);
      bitArray[e1_mgr4l5] = &(store->m_mgr4l5);
      bitArray[e1_mgr4l6] = &(store->m_mgr4l6);
      bitArray[e1_mgr4l7] = &(store->m_mgr4l7);
      bitArray[e1_evt] = &(store->m_evt);
      bitArray[e1_clk] = &(store->m_clk);
      bitArray[e1_firmid  ] = &(store->m_firmid);
      bitArray[e1_firmver ] = &(store->m_firmver);
    }


    /*! A module of TRGCDCTSF Unpacker */
    class TRGCDCTSFUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGCDCTSFUnpackerModule();

      /** Destructor */
      virtual ~TRGCDCTSFUnpackerModule();

      /** Initilizes TRGCDCTSFUnpackerModuel.*/
      virtual void initialize() override;

      /** Called event by event.*/
      virtual void event() override;

      /** Called when processing ended.*/
      virtual void terminate() override;

      /** Called when new run started.*/
      virtual void beginRun() override;

      /** Called when run ended*/
      virtual void endRun() override;

      /** returns version of TRGCDCTSFUnpackerModule.*/
      std::string version() const;

      /** Unpacker main function.*/
      virtual void fillTreeCDCTSF(int* buf, int evt);

      /** Unpacker main function for 4k, 15 TS version.*/
      virtual void fillTreeCDCTSF_4k15ts(int* buf, int evt);

    private:

      //! StoreArray of TRGCDCTSFUnpackerStore
      StoreArray<TRGCDCTSFUnpackerStore> storeAry;

      //! bitmap
      //int m_BitMap[nLeafs][2];
      //! number of bit
      int m_nBits;
      //! number of word
      int m_nword;

      //! address of copper module
      unsigned int m_copper_address;
      //! address of copper module
      unsigned int m_copper_ab;

      //! TSF module number
      int m_TSFMOD;

      //! exp number
      unsigned _exp;
      //! run number
      unsigned _run;

    };
  }
} // end namespace Belle2
