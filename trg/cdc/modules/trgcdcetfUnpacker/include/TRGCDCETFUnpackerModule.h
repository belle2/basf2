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
// Filename : TRGCDCETFUnpackerModule.h
// Section  :
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TRGCDCETFUnpacker Module
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#ifndef TRGCDCETFUNPACKER_H
#define TRGCDCETFUNPACKER_H

#include <string>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/cdc/dataobjects/TRGCDCETFUnpackerStore.h"

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

namespace Belle2 {

  namespace TRGCDCETFUNPACKERSPACE {

    /** num of leafs in data_b2l **/
    const int nLeafs = 108;
    /** num of leafs for others **/
    const int nLeafsExtra = 4;
    /** num of clk time window **/
    const int nClks = 48 ;
    /** num of b2l bits **/
    const int nBits = 1024;
    /** leaf names **/
    const char* LeafNames[nLeafs + nLeafsExtra] = {
      "etf_threshold",
      "etf_threshold2",
      "etf_holdtime",
      "etf_firmvers",
      "etf_cc",
      "etf_output_gdl",
      "etf_valid_gdl",
      "etf_output",
      "etf_valid",

      "etf_tsf8_nhit",
      "etf_tsf8_ft9",
      "etf_tsf8_ft8",
      "etf_tsf8_ft7",
      "etf_tsf8_ft6",
      "etf_tsf8_ft5",
      "etf_tsf8_ft4",
      "etf_tsf8_ft3",
      "etf_tsf8_ft2",
      "etf_tsf8_ft1",
      "etf_tsf8_ft0",

      "etf_tsf7_nhit",
      "etf_tsf7_ft9",
      "etf_tsf7_ft8",
      "etf_tsf7_ft7",
      "etf_tsf7_ft6",
      "etf_tsf7_ft5",
      "etf_tsf7_ft4",
      "etf_tsf7_ft3",
      "etf_tsf7_ft2",
      "etf_tsf7_ft1",
      "etf_tsf7_ft0",

      "etf_tsf6_nhit",
      "etf_tsf6_ft9",
      "etf_tsf6_ft8",
      "etf_tsf6_ft7",
      "etf_tsf6_ft6",
      "etf_tsf6_ft5",
      "etf_tsf6_ft4",
      "etf_tsf6_ft3",
      "etf_tsf6_ft2",
      "etf_tsf6_ft1",
      "etf_tsf6_ft0",

      "etf_tsf5_nhit",
      "etf_tsf5_ft9",
      "etf_tsf5_ft8",
      "etf_tsf5_ft7",
      "etf_tsf5_ft6",
      "etf_tsf5_ft5",
      "etf_tsf5_ft4",
      "etf_tsf5_ft3",
      "etf_tsf5_ft2",
      "etf_tsf5_ft1",
      "etf_tsf5_ft0",

      "etf_tsf4_nhit",
      "etf_tsf4_ft9",
      "etf_tsf4_ft8",
      "etf_tsf4_ft7",
      "etf_tsf4_ft6",
      "etf_tsf4_ft5",
      "etf_tsf4_ft4",
      "etf_tsf4_ft3",
      "etf_tsf4_ft2",
      "etf_tsf4_ft1",
      "etf_tsf4_ft0",

      "etf_tsf3_nhit",
      "etf_tsf3_ft9",
      "etf_tsf3_ft8",
      "etf_tsf3_ft7",
      "etf_tsf3_ft6",
      "etf_tsf3_ft5",
      "etf_tsf3_ft4",
      "etf_tsf3_ft3",
      "etf_tsf3_ft2",
      "etf_tsf3_ft1",
      "etf_tsf3_ft0",

      "etf_tsf2_nhit",
      "etf_tsf2_ft9",
      "etf_tsf2_ft8",
      "etf_tsf2_ft7",
      "etf_tsf2_ft6",
      "etf_tsf2_ft5",
      "etf_tsf2_ft4",
      "etf_tsf2_ft3",
      "etf_tsf2_ft2",
      "etf_tsf2_ft1",
      "etf_tsf2_ft0",

      "etf_tsf1_nhit",
      "etf_tsf1_ft9",
      "etf_tsf1_ft8",
      "etf_tsf1_ft7",
      "etf_tsf1_ft6",
      "etf_tsf1_ft5",
      "etf_tsf1_ft4",
      "etf_tsf1_ft3",
      "etf_tsf1_ft2",
      "etf_tsf1_ft1",
      "etf_tsf1_ft0",

      "etf_tsf0_nhit",
      "etf_tsf0_ft9",
      "etf_tsf0_ft8",
      "etf_tsf0_ft7",
      "etf_tsf0_ft6",
      "etf_tsf0_ft5",
      "etf_tsf0_ft4",
      "etf_tsf0_ft3",
      "etf_tsf0_ft2",
      "etf_tsf0_ft1",
      "etf_tsf0_ft0",
      "evt", "clk", "firmid", "firmvers"

    };

    /** enum of leafs **/
    enum EBits {
      e_etf_threshold,
      e_etf_threshold2,
      e_etf_holdtime,
      e_etf_firmvers,
      e_etf_cc,
      e_etf_output_gdl,
      e_etf_valid_gdl,
      e_etf_output,
      e_etf_valid,

      e_etf_tsf8_nhit,
      e_etf_tsf8_ft9,
      e_etf_tsf8_ft8,
      e_etf_tsf8_ft7,
      e_etf_tsf8_ft6,
      e_etf_tsf8_ft5,
      e_etf_tsf8_ft4,
      e_etf_tsf8_ft3,
      e_etf_tsf8_ft2,
      e_etf_tsf8_ft1,
      e_etf_tsf8_ft0,

      e_etf_tsf7_nhit,
      e_etf_tsf7_ft9,
      e_etf_tsf7_ft8,
      e_etf_tsf7_ft7,
      e_etf_tsf7_ft6,
      e_etf_tsf7_ft5,
      e_etf_tsf7_ft4,
      e_etf_tsf7_ft3,
      e_etf_tsf7_ft2,
      e_etf_tsf7_ft1,
      e_etf_tsf7_ft0,

      e_etf_tsf6_nhit,
      e_etf_tsf6_ft9,
      e_etf_tsf6_ft8,
      e_etf_tsf6_ft7,
      e_etf_tsf6_ft6,
      e_etf_tsf6_ft5,
      e_etf_tsf6_ft4,
      e_etf_tsf6_ft3,
      e_etf_tsf6_ft2,
      e_etf_tsf6_ft1,
      e_etf_tsf6_ft0,

      e_etf_tsf5_nhit,
      e_etf_tsf5_ft9,
      e_etf_tsf5_ft8,
      e_etf_tsf5_ft7,
      e_etf_tsf5_ft6,
      e_etf_tsf5_ft5,
      e_etf_tsf5_ft4,
      e_etf_tsf5_ft3,
      e_etf_tsf5_ft2,
      e_etf_tsf5_ft1,
      e_etf_tsf5_ft0,

      e_etf_tsf4_nhit,
      e_etf_tsf4_ft9,
      e_etf_tsf4_ft8,
      e_etf_tsf4_ft7,
      e_etf_tsf4_ft6,
      e_etf_tsf4_ft5,
      e_etf_tsf4_ft4,
      e_etf_tsf4_ft3,
      e_etf_tsf4_ft2,
      e_etf_tsf4_ft1,
      e_etf_tsf4_ft0,

      e_etf_tsf3_nhit,
      e_etf_tsf3_ft9,
      e_etf_tsf3_ft8,
      e_etf_tsf3_ft7,
      e_etf_tsf3_ft6,
      e_etf_tsf3_ft5,
      e_etf_tsf3_ft4,
      e_etf_tsf3_ft3,
      e_etf_tsf3_ft2,
      e_etf_tsf3_ft1,
      e_etf_tsf3_ft0,

      e_etf_tsf2_nhit,
      e_etf_tsf2_ft9,
      e_etf_tsf2_ft8,
      e_etf_tsf2_ft7,
      e_etf_tsf2_ft6,
      e_etf_tsf2_ft5,
      e_etf_tsf2_ft4,
      e_etf_tsf2_ft3,
      e_etf_tsf2_ft2,
      e_etf_tsf2_ft1,
      e_etf_tsf2_ft0,

      e_etf_tsf1_nhit,
      e_etf_tsf1_ft9,
      e_etf_tsf1_ft8,
      e_etf_tsf1_ft7,
      e_etf_tsf1_ft6,
      e_etf_tsf1_ft5,
      e_etf_tsf1_ft4,
      e_etf_tsf1_ft3,
      e_etf_tsf1_ft2,
      e_etf_tsf1_ft1,
      e_etf_tsf1_ft0,

      e_etf_tsf0_nhit,
      e_etf_tsf0_ft9,
      e_etf_tsf0_ft8,
      e_etf_tsf0_ft7,
      e_etf_tsf0_ft6,
      e_etf_tsf0_ft5,
      e_etf_tsf0_ft4,
      e_etf_tsf0_ft3,
      e_etf_tsf0_ft2,
      e_etf_tsf0_ft1,
      e_etf_tsf0_ft0,

      e_evt, e_clk, e_firmid, e_firmvers
    };

    /** bus bit map. (a downto a-b) **/
    const int BitMap[nLeafs][2] = {
      971, 3,   //etf_threshold     (971 downto 968)
      967, 3,   //etf_threshold2    (967 downto 964)
      963, 7,   //etf_holdtime      (963 downto 956)
      955, 31,  //etf_firmvers      (955 downto 924)
      923, 8,   //etf_cc            (923 downto 915)
      914, 12,  //etf_output_gdl    (914 downto 902)
      901, 0,   //etf_valid_gdl     (901 downto 901)
      900, 8,   //etf_output        (900 downto 892)
      891, 0,   //etf_valid         (891 downto 891)

      890, 8,   //etf_tsf8_nhit      (890 downto 882)
      881, 8,   //etf_tsf8_ft9       (881 downto 873)
      872, 8,   //etf_tsf8_ft8       (872 downto 864)
      863, 8,   //etf_tsf8_ft7       (863 downto 855)
      854, 8,   //etf_tsf8_ft6       (854 downto 846)
      845, 8,   //etf_tsf8_ft5       (845 downto 837)
      836, 8,   //etf_tsf8_ft4       (836 downto 828)
      827, 8,   //etf_tsf8_ft3       (827 downto 819)
      818, 8,   //etf_tsf8_ft2       (818 downto 810)
      809, 8,   //etf_tsf8_ft1       (809 downto 801)
      800, 8,   //etf_tsf8_ft0       (800 downto 792)

      791, 8,   //etf_tsf7_nhit      (791 downto 783)
      782, 8,   //etf_tsf7_ft9       (782 downto 774)
      773, 8,   //etf_tsf7_ft8       (773 downto 765)
      764, 8,   //etf_tsf7_ft7       (764 downto 756)
      755, 8,   //etf_tsf7_ft6       (755 downto 747)
      746, 8,   //etf_tsf7_ft5       (746 downto 738)
      737, 8,   //etf_tsf7_ft4       (737 downto 729)
      728, 8,   //etf_tsf7_ft3       (728 downto 720)
      719, 8,   //etf_tsf7_ft2       (719 downto 711)
      710, 8,   //etf_tsf7_ft1       (710 downto 702)
      701, 8,   //etf_tsf7_ft0       (701 downto 693)

      692, 8,   //etf_tsf6_nhit      (692 downto 684)
      683, 8,   //etf_tsf6_ft9       (683 downto 675)
      674, 8,   //etf_tsf6_ft8       (674 downto 666)
      665, 8,   //etf_tsf6_ft7       (665 downto 657)
      656, 8,   //etf_tsf6_ft6       (656 downto 648)
      647, 8,   //etf_tsf6_ft5       (647 downto 639)
      638, 8,   //etf_tsf6_ft4       (638 downto 630)
      629, 8,   //etf_tsf6_ft3       (629 downto 621)
      620, 8,   //etf_tsf6_ft2       (620 downto 612)
      611, 8,   //etf_tsf6_ft1       (611 downto 603)
      602, 8,   //etf_tsf6_ft0       (602 downto 594)

      593, 8,   //etf_tsf5_nhit      (593 downto 585)
      584, 8,   //etf_tsf5_ft9       (584 downto 576)
      575, 8,   //etf_tsf5_ft8       (575 downto 567)
      566, 8,   //etf_tsf5_ft7       (566 downto 558)
      557, 8,   //etf_tsf5_ft6       (557 downto 549)
      548, 8,   //etf_tsf5_ft5       (548 downto 540)
      539, 8,   //etf_tsf5_ft4       (539 downto 531)
      530, 8,   //etf_tsf5_ft3       (530 downto 522)
      521, 8,   //etf_tsf5_ft2       (521 downto 513)
      512, 8,   //etf_tsf5_ft1       (512 downto 504)
      503, 8,   //etf_tsf5_ft0       (503 downto 495)

      494, 8,   //etf_tsf4_nhit      (494 downto 486)
      485, 8,   //etf_tsf4_ft9       (485 downto 477)
      476, 8,   //etf_tsf4_ft8       (476 downto 468)
      467, 8,   //etf_tsf4_ft7       (467 downto 459)
      458, 8,   //etf_tsf4_ft6       (458 downto 450)
      449, 8,   //etf_tsf4_ft5       (449 downto 441)
      440, 8,   //etf_tsf4_ft4       (440 downto 432)
      431, 8,   //etf_tsf4_ft3       (431 downto 423)
      422, 8,   //etf_tsf4_ft2       (422 downto 414)
      413, 8,   //etf_tsf4_ft1       (413 downto 405)
      404, 8,   //etf_tsf4_ft0       (404 downto 396)

      395, 8,   //etf_tsf3_nhit      (395 downto 387)
      386, 8,   //etf_tsf3_ft9       (386 downto 378)
      377, 8,   //etf_tsf3_ft8       (377 downto 369)
      368, 8,   //etf_tsf3_ft7       (368 downto 360)
      359, 8,   //etf_tsf3_ft6       (359 downto 351)
      350, 8,   //etf_tsf3_ft5       (350 downto 342)
      341, 8,   //etf_tsf3_ft4       (341 downto 333)
      332, 8,   //etf_tsf3_ft3       (332 downto 324)
      323, 8,   //etf_tsf3_ft2       (323 downto 315)
      314, 8,   //etf_tsf3_ft1       (314 downto 306)
      305, 8,   //etf_tsf3_ft0       (305 downto 297)

      296, 8,   //etf_tsf2_nhit      (296 downto 288)
      287, 8,   //etf_tsf2_ft9       (287 downto 279)
      278, 8,   //etf_tsf2_ft8       (278 downto 270)
      269, 8,   //etf_tsf2_ft7       (269 downto 261)
      260, 8,   //etf_tsf2_ft6       (260 downto 252)
      251, 8,   //etf_tsf2_ft5       (251 downto 243)
      242, 8,   //etf_tsf2_ft4       (242 downto 234)
      233, 8,   //etf_tsf2_ft3       (233 downto 225)
      224, 8,   //etf_tsf2_ft2       (224 downto 216)
      215, 8,   //etf_tsf2_ft1       (215 downto 207)
      206, 8,   //etf_tsf2_ft0       (206 downto 198)

      197, 8,   //etf_tsf1_nhit      (197 downto 189)
      188, 8,   //etf_tsf1_ft9       (188 downto 180)
      179, 8,   //etf_tsf1_ft8       (179 downto 171)
      170, 8,   //etf_tsf1_ft7       (170 downto 162)
      161, 8,   //etf_tsf1_ft6       (161 downto 153)
      152, 8,   //etf_tsf1_ft5       (152 downto 144)
      143, 8,   //etf_tsf1_ft4       (143 downto 135)
      134, 8,   //etf_tsf1_ft3       (134 downto 126)
      125, 8,   //etf_tsf1_ft2       (125 downto 117)
      116, 8,   //etf_tsf1_ft1       (116 downto 108)
      107, 8,   //etf_tsf1_ft0       (107 downto  99)

      98, 8,   //etf_tsf0_nhit      ( 98 downto  90)
      89, 8,   //etf_tsf0_ft9       ( 89 downto  81)
      80, 8,   //etf_tsf0_ft8       ( 80 downto  72)
      71, 8,   //etf_tsf0_ft7       ( 71 downto  63)
      62, 8,   //etf_tsf0_ft6       ( 62 downto  54)
      53, 8,   //etf_tsf0_ft5       ( 53 downto  45)
      44, 8,   //etf_tsf0_ft4       ( 44 downto  36)
      35, 8,   //etf_tsf0_ft3       ( 35 downto  27)
      26, 8,   //etf_tsf0_ft2       ( 26 downto  18)
      17, 8,   //etf_tsf0_ft1       ( 17 downto   9)
      8, 8    //etf_tsf0_ft0       (  8 downto   0)


      //971, 8,   //etf_tsf8_cc        (890 downto 882)
      //962, 8,   //etf_tsf8_nhit      (890 downto 882)
      //953, 8,   //etf_tsf8_ft0       (881 downto 873)
      //944, 8,   //etf_tsf8_ft1       (872 downto 864)
      //935, 8,   //etf_tsf8_ft2       (863 downto 855)
      //926, 8,   //etf_tsf8_ft3       (854 downto 846)
      //917, 8,   //etf_tsf8_ft4       (845 downto 837)
      //908, 8,   //etf_tsf8_ft5       (836 downto 828)
      //899, 8,   //etf_tsf8_ft6       (827 downto 819)
      //890, 8,   //etf_tsf8_ft7       (818 downto 810)
      //881, 8,   //etf_tsf8_ft8       (809 downto 801)
      //872, 8,   //etf_tsf8_ft9       (800 downto 792)
      //
      //863, 8,   //etf_tsf7_cc        (890 downto 882)
      //854, 8,   //etf_tsf7_nhit      (791 downto 783)
      //845, 8,   //etf_tsf7_ft0       (782 downto 774)
      //836, 8,   //etf_tsf7_ft1       (773 downto 765)
      //827, 8,   //etf_tsf7_ft2       (764 downto 756)
      //818, 8,   //etf_tsf7_ft3       (755 downto 747)
      //809, 8,   //etf_tsf7_ft4       (746 downto 738)
      //800, 8,   //etf_tsf7_ft5       (737 downto 729)
      //791, 8,   //etf_tsf7_ft6       (728 downto 720)
      //782, 8,   //etf_tsf7_ft7       (719 downto 711)
      //773, 8,   //etf_tsf7_ft8       (710 downto 702)
      //764, 8,   //etf_tsf7_ft9       (701 downto 693)
      //
      //755, 8,   //etf_tsf6_cc        (890 downto 882)
      //746, 8,   //etf_tsf6_nhit      (692 downto 684)
      //737, 8,   //etf_tsf6_ft0       (683 downto 675)
      //728, 8,   //etf_tsf6_ft1       (674 downto 666)
      //719, 8,   //etf_tsf6_ft2       (665 downto 657)
      //710, 8,   //etf_tsf6_ft3       (656 downto 648)
      //701, 8,   //etf_tsf6_ft4       (647 downto 639)
      //692, 8,   //etf_tsf6_ft5       (638 downto 630)
      //683, 8,   //etf_tsf6_ft6       (629 downto 621)
      //674, 8,   //etf_tsf6_ft7       (620 downto 612)
      //665, 8,   //etf_tsf6_ft8       (611 downto 603)
      //656, 8,   //etf_tsf6_ft9       (602 downto 594)
      //
      //647, 8,   //etf_tsf5_cc        (890 downto 882)
      //638, 8,   //etf_tsf5_nhit      (593 downto 585)
      //629, 8,   //etf_tsf5_ft0       (584 downto 576)
      //620, 8,   //etf_tsf5_ft1       (575 downto 567)
      //611, 8,   //etf_tsf5_ft2       (566 downto 558)
      //602, 8,   //etf_tsf5_ft3       (557 downto 549)
      //593, 8,   //etf_tsf5_ft4       (548 downto 540)
      //584, 8,   //etf_tsf5_ft5       (539 downto 531)
      //575, 8,   //etf_tsf5_ft6       (530 downto 522)
      //566, 8,   //etf_tsf5_ft7       (521 downto 513)
      //557, 8,   //etf_tsf5_ft8       (512 downto 504)
      //548, 8,   //etf_tsf5_ft9       (503 downto 495)
      //
      //539, 8,   //etf_tsf4_cc        (890 downto 882)
      //530, 8,   //etf_tsf4_nhit      (494 downto 486)
      //521, 8,   //etf_tsf4_ft0       (485 downto 477)
      //512, 8,   //etf_tsf4_ft1       (476 downto 468)
      //503, 8,   //etf_tsf4_ft2       (467 downto 459)
      //494, 8,   //etf_tsf4_ft3       (458 downto 450)
      //485, 8,   //etf_tsf4_ft4       (449 downto 441)
      //476, 8,   //etf_tsf4_ft5       (440 downto 432)
      //467, 8,   //etf_tsf4_ft6       (431 downto 423)
      //458, 8,   //etf_tsf4_ft7       (422 downto 414)
      //449, 8,   //etf_tsf4_ft8       (413 downto 405)
      //440, 8,   //etf_tsf4_ft9       (404 downto 396)
      //
      //431, 8,   //etf_tsf3_cc        (890 downto 882)
      //422, 8,   //etf_tsf3_nhit      (395 downto 387)
      //413, 8,   //etf_tsf3_ft0       (386 downto 378)
      //404, 8,   //etf_tsf3_ft1       (377 downto 369)
      //395, 8,   //etf_tsf3_ft2       (368 downto 360)
      //386, 8,   //etf_tsf3_ft3       (359 downto 351)
      //377, 8,   //etf_tsf3_ft4       (350 downto 342)
      //368, 8,   //etf_tsf3_ft5       (341 downto 333)
      //359, 8,   //etf_tsf3_ft6       (332 downto 324)
      //350, 8,   //etf_tsf3_ft7       (323 downto 315)
      //341, 8,   //etf_tsf3_ft8       (314 downto 306)
      //332, 8,   //etf_tsf3_ft9       (305 downto 297)
      //
      //323, 8,   //etf_tsf2_cc        (890 downto 882)
      //314, 8,   //etf_tsf2_nhit      (296 downto 288)
      //305, 8,   //etf_tsf2_ft0       (287 downto 279)
      //296, 8,   //etf_tsf2_ft1       (278 downto 270)
      //287, 8,   //etf_tsf2_ft2       (269 downto 261)
      //278, 8,   //etf_tsf2_ft3       (260 downto 252)
      //269, 8,   //etf_tsf2_ft4       (251 downto 243)
      //260, 8,   //etf_tsf2_ft5       (242 downto 234)
      //251, 8,   //etf_tsf2_ft6       (233 downto 225)
      //242, 8,   //etf_tsf2_ft7       (224 downto 216)
      //233, 8,   //etf_tsf2_ft8       (215 downto 207)
      //224, 8,   //etf_tsf2_ft9       (206 downto 198)
      //
      //215, 8,   //etf_tsf1_cc        (890 downto 882)
      //206, 8,   //etf_tsf1_nhit      (197 downto 189)
      //197, 8,   //etf_tsf1_ft0       (188 downto 180)
      //188, 8,   //etf_tsf1_ft1       (179 downto 171)
      //179, 8,   //etf_tsf1_ft2       (170 downto 162)
      //170, 8,   //etf_tsf1_ft3       (161 downto 153)
      //161, 8,   //etf_tsf1_ft4       (152 downto 144)
      //152, 8,   //etf_tsf1_ft5       (143 downto 135)
      //143, 8,   //etf_tsf1_ft6       (134 downto 126)
      //134, 8,   //etf_tsf1_ft7       (125 downto 117)
      //125, 8,   //etf_tsf1_ft8       (116 downto 108)
      //116, 8,   //etf_tsf1_ft9       (107 downto  99)
      //
      //107, 8,   //etf_tsf0_cc        (890 downto 882)
      // 98, 8,   //etf_tsf0_nhit      ( 98 downto  90)
      // 89, 8,   //etf_tsf0_ft0       ( 89 downto  81)
      // 80, 8,   //etf_tsf0_ft1       ( 80 downto  72)
      // 71, 8,   //etf_tsf0_ft2       ( 71 downto  63)
      // 62, 8,   //etf_tsf0_ft3       ( 62 downto  54)
      // 53, 8,   //etf_tsf0_ft4       ( 53 downto  45)
      // 44, 8,   //etf_tsf0_ft5       ( 44 downto  36)
      // 35, 8,   //etf_tsf0_ft6       ( 35 downto  27)
      // 26, 8,   //etf_tsf0_ft7       ( 26 downto  18)
      // 17, 8,   //etf_tsf0_ft8       ( 17 downto   9)
      //  8, 8    //etf_tsf0_ft9       (  8 downto   0)



    };

    void
    setLeafPointersArray(TRGCDCETFUnpackerStore* store, int** bitArray)
    {
      bitArray[e_etf_threshold]   = &(store->m_etf_threshold);
      bitArray[e_etf_threshold2]  = &(store->m_etf_threshold2);
      bitArray[e_etf_holdtime]    = &(store->m_etf_holdtime);
      bitArray[e_etf_firmvers]    = &(store->m_etf_firmvers);
      bitArray[e_etf_cc]          = &(store->m_etf_cc);
      bitArray[e_etf_output_gdl]  = &(store->m_etf_output_gdl);
      bitArray[e_etf_valid_gdl]   = &(store->m_etf_valid_gdl);
      bitArray[e_etf_output]      = &(store->m_etf_output);
      bitArray[e_etf_valid]       = &(store->m_etf_valid);

      bitArray[e_etf_tsf8_nhit]   = &(store->m_etf_tsf8_nhit);
      bitArray[e_etf_tsf8_ft9]    = &(store->m_etf_tsf8_ft9);
      bitArray[e_etf_tsf8_ft8]    = &(store->m_etf_tsf8_ft8);
      bitArray[e_etf_tsf8_ft7]    = &(store->m_etf_tsf8_ft7);
      bitArray[e_etf_tsf8_ft6]    = &(store->m_etf_tsf8_ft6);
      bitArray[e_etf_tsf8_ft5]    = &(store->m_etf_tsf8_ft5);
      bitArray[e_etf_tsf8_ft4]    = &(store->m_etf_tsf8_ft4);
      bitArray[e_etf_tsf8_ft3]    = &(store->m_etf_tsf8_ft3);
      bitArray[e_etf_tsf8_ft2]    = &(store->m_etf_tsf8_ft2);
      bitArray[e_etf_tsf8_ft1]    = &(store->m_etf_tsf8_ft1);
      bitArray[e_etf_tsf8_ft0]    = &(store->m_etf_tsf8_ft0);

      bitArray[e_etf_tsf7_nhit]   = &(store->m_etf_tsf7_nhit);
      bitArray[e_etf_tsf7_ft9]    = &(store->m_etf_tsf7_ft9);
      bitArray[e_etf_tsf7_ft8]    = &(store->m_etf_tsf7_ft8);
      bitArray[e_etf_tsf7_ft7]    = &(store->m_etf_tsf7_ft7);
      bitArray[e_etf_tsf7_ft6]    = &(store->m_etf_tsf7_ft6);
      bitArray[e_etf_tsf7_ft5]    = &(store->m_etf_tsf7_ft5);
      bitArray[e_etf_tsf7_ft4]    = &(store->m_etf_tsf7_ft4);
      bitArray[e_etf_tsf7_ft3]    = &(store->m_etf_tsf7_ft3);
      bitArray[e_etf_tsf7_ft2]    = &(store->m_etf_tsf7_ft2);
      bitArray[e_etf_tsf7_ft1]    = &(store->m_etf_tsf7_ft1);
      bitArray[e_etf_tsf7_ft0]    = &(store->m_etf_tsf7_ft0);

      bitArray[e_etf_tsf6_nhit]   = &(store->m_etf_tsf6_nhit);
      bitArray[e_etf_tsf6_ft9]    = &(store->m_etf_tsf6_ft9);
      bitArray[e_etf_tsf6_ft8]    = &(store->m_etf_tsf6_ft8);
      bitArray[e_etf_tsf6_ft7]    = &(store->m_etf_tsf6_ft7);
      bitArray[e_etf_tsf6_ft6]    = &(store->m_etf_tsf6_ft6);
      bitArray[e_etf_tsf6_ft5]    = &(store->m_etf_tsf6_ft5);
      bitArray[e_etf_tsf6_ft4]    = &(store->m_etf_tsf6_ft4);
      bitArray[e_etf_tsf6_ft3]    = &(store->m_etf_tsf6_ft3);
      bitArray[e_etf_tsf6_ft2]    = &(store->m_etf_tsf6_ft2);
      bitArray[e_etf_tsf6_ft1]    = &(store->m_etf_tsf6_ft1);
      bitArray[e_etf_tsf6_ft0]    = &(store->m_etf_tsf6_ft0);

      bitArray[e_etf_tsf5_nhit]   = &(store->m_etf_tsf5_nhit);
      bitArray[e_etf_tsf5_ft9]    = &(store->m_etf_tsf5_ft9);
      bitArray[e_etf_tsf5_ft8]    = &(store->m_etf_tsf5_ft8);
      bitArray[e_etf_tsf5_ft7]    = &(store->m_etf_tsf5_ft7);
      bitArray[e_etf_tsf5_ft6]    = &(store->m_etf_tsf5_ft6);
      bitArray[e_etf_tsf5_ft5]    = &(store->m_etf_tsf5_ft5);
      bitArray[e_etf_tsf5_ft4]    = &(store->m_etf_tsf5_ft4);
      bitArray[e_etf_tsf5_ft3]    = &(store->m_etf_tsf5_ft3);
      bitArray[e_etf_tsf5_ft2]    = &(store->m_etf_tsf5_ft2);
      bitArray[e_etf_tsf5_ft1]    = &(store->m_etf_tsf5_ft1);
      bitArray[e_etf_tsf5_ft0]    = &(store->m_etf_tsf5_ft0);

      bitArray[e_etf_tsf4_nhit]   = &(store->m_etf_tsf4_nhit);
      bitArray[e_etf_tsf4_ft9]    = &(store->m_etf_tsf4_ft9);
      bitArray[e_etf_tsf4_ft8]    = &(store->m_etf_tsf4_ft8);
      bitArray[e_etf_tsf4_ft7]    = &(store->m_etf_tsf4_ft7);
      bitArray[e_etf_tsf4_ft6]    = &(store->m_etf_tsf4_ft6);
      bitArray[e_etf_tsf4_ft5]    = &(store->m_etf_tsf4_ft5);
      bitArray[e_etf_tsf4_ft4]    = &(store->m_etf_tsf4_ft4);
      bitArray[e_etf_tsf4_ft3]    = &(store->m_etf_tsf4_ft3);
      bitArray[e_etf_tsf4_ft2]    = &(store->m_etf_tsf4_ft2);
      bitArray[e_etf_tsf4_ft1]    = &(store->m_etf_tsf4_ft1);
      bitArray[e_etf_tsf4_ft0]    = &(store->m_etf_tsf4_ft0);

      bitArray[e_etf_tsf3_nhit]   = &(store->m_etf_tsf3_nhit);
      bitArray[e_etf_tsf3_ft9]    = &(store->m_etf_tsf3_ft9);
      bitArray[e_etf_tsf3_ft8]    = &(store->m_etf_tsf3_ft8);
      bitArray[e_etf_tsf3_ft7]    = &(store->m_etf_tsf3_ft7);
      bitArray[e_etf_tsf3_ft6]    = &(store->m_etf_tsf3_ft6);
      bitArray[e_etf_tsf3_ft5]    = &(store->m_etf_tsf3_ft5);
      bitArray[e_etf_tsf3_ft4]    = &(store->m_etf_tsf3_ft4);
      bitArray[e_etf_tsf3_ft3]    = &(store->m_etf_tsf3_ft3);
      bitArray[e_etf_tsf3_ft2]    = &(store->m_etf_tsf3_ft2);
      bitArray[e_etf_tsf3_ft1]    = &(store->m_etf_tsf3_ft1);
      bitArray[e_etf_tsf3_ft0]    = &(store->m_etf_tsf3_ft0);

      bitArray[e_etf_tsf2_nhit]   = &(store->m_etf_tsf2_nhit);
      bitArray[e_etf_tsf2_ft9]    = &(store->m_etf_tsf2_ft9);
      bitArray[e_etf_tsf2_ft8]    = &(store->m_etf_tsf2_ft8);
      bitArray[e_etf_tsf2_ft7]    = &(store->m_etf_tsf2_ft7);
      bitArray[e_etf_tsf2_ft6]    = &(store->m_etf_tsf2_ft6);
      bitArray[e_etf_tsf2_ft5]    = &(store->m_etf_tsf2_ft5);
      bitArray[e_etf_tsf2_ft4]    = &(store->m_etf_tsf2_ft4);
      bitArray[e_etf_tsf2_ft3]    = &(store->m_etf_tsf2_ft3);
      bitArray[e_etf_tsf2_ft2]    = &(store->m_etf_tsf2_ft2);
      bitArray[e_etf_tsf2_ft1]    = &(store->m_etf_tsf2_ft1);
      bitArray[e_etf_tsf2_ft0]    = &(store->m_etf_tsf2_ft0);

      bitArray[e_etf_tsf1_nhit]   = &(store->m_etf_tsf1_nhit);
      bitArray[e_etf_tsf1_ft9]    = &(store->m_etf_tsf1_ft9);
      bitArray[e_etf_tsf1_ft8]    = &(store->m_etf_tsf1_ft8);
      bitArray[e_etf_tsf1_ft7]    = &(store->m_etf_tsf1_ft7);
      bitArray[e_etf_tsf1_ft6]    = &(store->m_etf_tsf1_ft6);
      bitArray[e_etf_tsf1_ft5]    = &(store->m_etf_tsf1_ft5);
      bitArray[e_etf_tsf1_ft4]    = &(store->m_etf_tsf1_ft4);
      bitArray[e_etf_tsf1_ft3]    = &(store->m_etf_tsf1_ft3);
      bitArray[e_etf_tsf1_ft2]    = &(store->m_etf_tsf1_ft2);
      bitArray[e_etf_tsf1_ft1]    = &(store->m_etf_tsf1_ft1);
      bitArray[e_etf_tsf1_ft0]    = &(store->m_etf_tsf1_ft0);

      bitArray[e_etf_tsf0_nhit]   = &(store->m_etf_tsf0_nhit);
      bitArray[e_etf_tsf0_ft9]    = &(store->m_etf_tsf0_ft9);
      bitArray[e_etf_tsf0_ft8]    = &(store->m_etf_tsf0_ft8);
      bitArray[e_etf_tsf0_ft7]    = &(store->m_etf_tsf0_ft7);
      bitArray[e_etf_tsf0_ft6]    = &(store->m_etf_tsf0_ft6);
      bitArray[e_etf_tsf0_ft5]    = &(store->m_etf_tsf0_ft5);
      bitArray[e_etf_tsf0_ft4]    = &(store->m_etf_tsf0_ft4);
      bitArray[e_etf_tsf0_ft3]    = &(store->m_etf_tsf0_ft3);
      bitArray[e_etf_tsf0_ft2]    = &(store->m_etf_tsf0_ft2);
      bitArray[e_etf_tsf0_ft1]    = &(store->m_etf_tsf0_ft1);
      bitArray[e_etf_tsf0_ft0]    = &(store->m_etf_tsf0_ft0);

      bitArray[e_evt]             = &(store->m_evt);
      bitArray[e_clk]             = &(store->m_clk);
      bitArray[e_firmid]          = &(store->m_firmid);
      bitArray[e_firmvers]        = &(store->m_firmvers);
    }

    /*! A module of TRGCDCETFUnpacker */
    class TRGCDCETFUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGCDCETFUnpackerModule();

      /** Destructor */
      virtual ~TRGCDCETFUnpackerModule();

      /** Initilizes TRGCDCETFUnpackerUnpackerModule.*/
      virtual void initialize() override;

      /** Called event by event.*/
      virtual void event() override;

      /** Called when processing ended.*/
      virtual void terminate() override;

      /** Called when new run started.*/
      virtual void beginRun() override;

      /** Called when run ended*/
      virtual void endRun() override;

      /** returns version of TRGCDCETFUnpackerModule.*/
      std::string version() const;

      /** Unpacker main function.*/
      virtual void fillTreeTRGCDCETFUnpacker(int* buf, int evt);

    private:

      /** Output datastore for unpacker **/
      StoreArray<TRGCDCETFUnpackerStore> m_store;

    };
  }
} // end namespace Belle2
#endif // TRGCDCETFUNPACKER_H
