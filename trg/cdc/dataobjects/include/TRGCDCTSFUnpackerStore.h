/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGCDCTSFUnpackerStore.h
// Section  : TRG CDCTSF
// Owner    :
// Email    :
//-----------------------------------------------------------
// Description : A class to represent TRG CDCTSF.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGCDCTSFUNPACKERSTORE_H
#define TRGCDCTSFUNPACKERSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGCDCTSFUnpackerStore : public TObject {
  public:

    //! tsf leafs
    int m_c127;
    //! tsf leafs
//  int m_tshitmap;


    //! tsf leafs
    int m_validtracker;
    //! tsf leafs
//  int m_trackerout;
    //! m_trackercc leafs
    int m_trackercc;
    //! m_trackerhit0id leafs
    int m_trackerhit0id;
    //! m_trackerhit0rt leafs
    int m_trackerhit0rt;
    //! m_trackerhit0v leafs
    int m_trackerhit0v;
    //! m_trackerhit1id leafs
    int m_trackerhit1id;
    //! m_trackerhit1rt leafs
    int m_trackerhit1rt;
    //! m_trackerhit1v leafs
    int m_trackerhit1v;
    //! m_trackerhit2id leafs
    int m_trackerhit2id;
    //! m_trackercc leafs
    int m_trackerhit2rt;
    //! m_trackerhit2v leafs
    int m_trackerhit2v;
    //! m_trackerhit3id leafs
    int m_trackerhit3id;
    //! m_trackerhit3rt leafs
    int m_trackerhit3rt;
    //! m_trackerhit3v leafs
    int m_trackerhit3v;
    //! m_trackerhit4id leafs
    int m_trackerhit4id;
    //! m_trackerhit4rt leafs
    int m_trackerhit4rt;
    //! m_trackerhit4v leafs
    int m_trackerhit4v;
    //! m_trackerhit5id leafs
    int m_trackerhit5id;
    //! m_trackerhit5rt leafs
    int m_trackerhit5rt;
    //! m_trackerhit5v leafs
    int m_trackerhit5v;
    //! m_trackerhit6id leafs
    int m_trackerhit6id;
    //! m_trackerhit6rt leafs
    int m_trackerhit6rt;
    //! m_trackerhit6v leafs
    int m_trackerhit6v;
    //! m_trackerhit7id leafs
    int m_trackerhit7id;
    //! m_trackerhit7rt leafs
    int m_trackerhit7rt;
    //! m_trackerhit7v leafs
    int m_trackerhit7v;
    //! m_trackerhit8id leafs
    int m_trackerhit8id;
    //! m_trackerhit8rt leafs
    int m_trackerhit8rt;
    //! m_trackerhiy8v leafs
    int m_trackerhit8v;
    //! m_trackerhit9id leafs
    int m_trackerhit9id;
    //! m_trackerhit9rt leafs
    int m_trackerhit9rt;
    //! m_trackerhit9v leafs
    int m_trackerhit9v;
    //! m_trackerhit10id leafs
    int m_trackerhit10id;
    //! m_trackerhit10rt leafs
    int m_trackerhit10rt;
    //! m_trackerhit10v leafs
    int m_trackerhit10v;
    //! m_trackerhit11id leafs
    int m_trackerhit11id;
    //! m_trackerhit11rt leafs
    int m_trackerhit11rt;
    //! m_trackerhit11v leafs
    int m_trackerhit11v;
    //! m_trackerhit12id leafs
    int m_trackerhit12id;
    //! m_trackerhit12rt leafs
    int m_trackerhit12rt;
    //! m_trackerhit12v leafs
    int m_trackerhit12v;
    //! m_trackerhit13id leafs
    int m_trackerhit13id;
    //! m_trackerhit13rt leafs
    int m_trackerhit13rt;
    //! m_trackerhit13v leafs
    int m_trackerhit13v;
    //! m_trackerhit14id leafs
    int m_trackerhit14id;
    //! m_trackerhit14rt leafs
    int m_trackerhit14rt;
    //! m_trackerhit14v leafs
    int m_trackerhit14v;


    //! m_validetf leafs
    int m_validetf;
    //! m_etfcc leafs
    int m_etfcc;
    //! m_netfhit leafs
    int m_netfhit;
    //! m_etfhit0 leafs
    int m_etfhit0;
    //! m_etfhit1 leafs
    int m_etfhit1;
    //! m_etfhit2 leafs
    int m_etfhit2;
    //! m_etfhit3 leafs
    int m_etfhit3;
    //! m_etfhit4 leafs
    int m_etfhit4;
    //! m_etfhit5 leafs
    int m_etfhit5;
    //! m_etfhit6 leafs
    int m_etfhit6;
    //! m_etfhit7 leafs
    int m_etfhit7;
    //! m_etfhit8 leafs
    int m_etfhit8;
    //! m_etfhit9 leafs
    int m_etfhit9;

    //! m_mgr0 leafs
    int m_mgr0;
    //! m_mgr0cc leafs
    int m_mgr0cc;
    //! m_mgr1 leafs
    int m_mgr1;
    //! m_mgr1cc leafs
    int m_mgr1cc;
    //! m_mgr2 leafs
    int m_mgr2;
    //! m_mgr2cc leafs
    int m_mgr2cc;
    //! m_mgr3 leafs
    int m_mgr3;
    //! m_mgr3cc leafs
    int m_mgr3cc;
    //! m_mgr4 leafs
    int m_mgr4;
    //! m_mgr4cc leafs
    int m_mgr4cc;
    //! m_mgr5 leafs
    int m_mgr5;
    //! m_mgr5cc leafs
    int m_mgr5cc;

    //! m_mgr0ft0 leafs
    int m_mgr0ft0;
    //! m_mgr0ft1 leafs
    int m_mgr0ft1;
    //! m_mgr0ft2 leafs
    int m_mgr0ft2;
    //! m_mgr0ft3 leafs
    int m_mgr0ft3;
    //! m_mgr0ft4 leafs
    int m_mgr0ft4;
    //! m_mgr0ft5 leafs
    int m_mgr0ft5;
    //! m_mgr0ft6 leafs
    int m_mgr0ft6;
    //! m_mgr0ft7 leafs
    int m_mgr0ft7;
    //! m_mgr0ft8 leafs
    int m_mgr0ft8;
    //! m_mgr0ft9 leafs
    int m_mgr0ft9;
    //! m_mgr0ft10 leafs
    int m_mgr0ft10;
    //! m_mgr0ft11 leafs
    int m_mgr0ft11;
    //! m_mgr0ft12 leafs
    int m_mgr0ft12;
    //! m_mgr0ft13 leafs
    int m_mgr0ft13;
    //! m_mgr0ft14 leafs
    int m_mgr0ft14;
    //! m_mgr0ft15 leafs
    int m_mgr0ft15;


    //! m_mgr1ft0 leafs
    int m_mgr1ft0;
    //! m_mgr1ft1 leafs
    int m_mgr1ft1;
    //! m_mgr1ft2 leafs
    int m_mgr1ft2;
    //! m_mgr1ft3 leafs
    int m_mgr1ft3;
    //! m_mgr1ft4 leafs
    int m_mgr1ft4;
    //! m_mgr1ft5 leafs
    int m_mgr1ft5;
    //! m_mgr1ft6 leafs
    int m_mgr1ft6;
    //! m_mgr1ft7 leafs
    int m_mgr1ft7;
    //! m_mgr1ft8 leafs
    int m_mgr1ft8;
    //! m_mgr1ft9 leafs
    int m_mgr1ft9;
    //! m_mgr1ft10 leafs
    int m_mgr1ft10;
    //! m_mgr1ft11 leafs
    int m_mgr1ft11;
    //! m_mgr1ft12 leafs
    int m_mgr1ft12;
    //! m_mgr1ft13 leafs
    int m_mgr1ft13;
    //! m_mgr1ft14 leafs
    int m_mgr1ft14;
    //! m_mgr1ft15 leafs
    int m_mgr1ft15;

    //! m_mgr2ft0 leafs
    int m_mgr2ft0;
    //! m_mgr2ft1 leafs
    int m_mgr2ft1;
    //! m_mgr2ft2 leafs
    int m_mgr2ft2;
    //! m_mgr2ft3 leafs
    int m_mgr2ft3;
    //! m_mgr2ft4 leafs
    int m_mgr2ft4;
    //! m_mgr2ft5 leafs
    int m_mgr2ft5;
    //! m_mgr2ft6 leafs
    int m_mgr2ft6;
    //! m_mgr2ft7 leafs
    int m_mgr2ft7;
    //! m_mgr2ft8 leafs
    int m_mgr2ft8;
    //! m_mgr2ft9 leafs
    int m_mgr2ft9;
    //! m_mgr2ft10 leafs
    int m_mgr2ft10;
    //! m_mgr2ft11 leafs
    int m_mgr2ft11;
    //! m_mgr2ft12 leafs
    int m_mgr2ft12;
    //! m_mgr2ft13 leafs
    int m_mgr2ft13;
    //! m_mgr2ft14 leafs
    int m_mgr2ft14;
    //! m_mgr2ft15 leafs
    int m_mgr2ft15;

    //! m_mgr3ft0 leafs
    int m_mgr3ft0;
    //! m_mgr3ft1 leafs
    int m_mgr3ft1;
    //! m_mgr3ft2 leafs
    int m_mgr3ft2;
    //! m_mgr3ft3 leafs
    int m_mgr3ft3;
    //! m_mgr3ft4 leafs
    int m_mgr3ft4;
    //! m_mgr3ft5 leafs
    int m_mgr3ft5;
    //! m_mgr3ft6 leafs
    int m_mgr3ft6;
    //! m_mgr3ft7 leafs
    int m_mgr3ft7;
    //! m_mgr3ft8 leafs
    int m_mgr3ft8;
    //! m_mgr3ft9 leafs
    int m_mgr3ft9;
    //! m_mgr3ft10 leafs
    int m_mgr3ft10;
    //! m_mgr3ft11 leafs
    int m_mgr3ft11;
    //! m_mgr3ft12 leafs
    int m_mgr3ft12;
    //! m_mgr3ft13 leafs
    int m_mgr3ft13;
    //! m_mgr3ft14 leafs
    int m_mgr3ft14;
    //! m_mgr3ft15 leafs
    int m_mgr3ft15;

    //! m_mgr4ft0 leafs
    int m_mgr4ft0;
    //! m_mgr4ft1 leafs
    int m_mgr4ft1;
    //! m_mgr4ft2 leafs
    int m_mgr4ft2;
    //! m_mgr4ft3 leafs
    int m_mgr4ft3;
    //! m_mgr4ft4 leafs
    int m_mgr4ft4;
    //! m_mgr4ft5 leafs
    int m_mgr4ft5;
    //! m_mgr4ft6 leafs
    int m_mgr4ft6;
    //! m_mgr4ft7 leafs
    int m_mgr4ft7;
    //! m_mgr4ft8 leafs
    int m_mgr4ft8;
    //! m_mgr4ft9 leafs
    int m_mgr4ft9;
    //! m_mgr4ft10 leafs
    int m_mgr4ft10;
    //! m_mgr4ft11 leafs
    int m_mgr4ft11;
    //! m_mgr4ft12 leafs
    int m_mgr4ft12;
    //! m_mgr4ft13 leafs
    int m_mgr4ft13;
    //! m_mgr4ft14 leafs
    int m_mgr4ft14;
    //! m_mgr4ft15 leafs
    int m_mgr4ft15;

    //! m_mgr0pt0 leafs
    int m_mgr0pt0;
    //! m_mgr0pt1 leafs
    int m_mgr0pt1;
    //! m_mgr0pt2 leafs
    int m_mgr0pt2;
    //! m_mgr0pt3 leafs
    int m_mgr0pt3;
    //! m_mgr0pt4 leafs
    int m_mgr0pt4;
    //! m_mgr0pt5 leafs
    int m_mgr0pt5;
    //! m_mgr0pt6 leafs
    int m_mgr0pt6;
    //! m_mgr0pt7 leafs
    int m_mgr0pt7;
    //! m_mgr0pt8 leafs
    int m_mgr0pt8;
    //! m_mgr0pt9 leafs
    int m_mgr0pt9;
    //! m_mgr0pt10 leafs
    int m_mgr0pt10;
    //! m_mgr0pt11 leafs
    int m_mgr0pt11;
    //! m_mgr0pt12 leafs
    int m_mgr0pt12;
    //! m_mgr0pt13 leafs
    int m_mgr0pt13;
    //! m_mgr0pt14 leafs
    int m_mgr0pt14;
    //! m_mgr0pt15 leafs
    int m_mgr0pt15;

    //! m_mgr1pt0 leafs
    int m_mgr1pt0;
    //! m_mgr1pt1 leafs
    int m_mgr1pt1;
    //! m_mgr1pt2 leafs
    int m_mgr1pt2;
    //! m_mgr1pt3 leafs
    int m_mgr1pt3;
    //! m_mgr1pt4 leafs
    int m_mgr1pt4;
    //! m_mgr1pt5 leafs
    int m_mgr1pt5;
    //! m_mgr1pt6 leafs
    int m_mgr1pt6;
    //! m_mgr1pt7 leafs
    int m_mgr1pt7;
    //! m_mgr1pt8 leafs
    int m_mgr1pt8;
    //! m_mgr1pt9 leafs
    int m_mgr1pt9;
    //! m_mgr1pt10 leafs
    int m_mgr1pt10;
    //! m_mgr1pt11 leafs
    int m_mgr1pt11;
    //! m_mgr1pt12 leafs
    int m_mgr1pt12;
    //! m_mgr1pt13 leafs
    int m_mgr1pt13;
    //! m_mgr1pt14 leafs
    int m_mgr1pt14;
    //! m_mgr1pt15 leafs
    int m_mgr1pt15;

    //! m_mgr2pt0 leafs
    int m_mgr2pt0;
    //! m_mgr2pt1 leafs
    int m_mgr2pt1;
    //! m_mgr2pt2 leafs
    int m_mgr2pt2;
    //! m_mgr2pt3 leafs
    int m_mgr2pt3;
    //! m_mgr2pt4 leafs
    int m_mgr2pt4;
    //! m_mgr2pt5 leafs
    int m_mgr2pt5;
    //! m_mgr2pt6 leafs
    int m_mgr2pt6;
    //! m_mgr2pt7 leafs
    int m_mgr2pt7;
    //! m_mgr2pt8 leafs
    int m_mgr2pt8;
    //! m_mgr2pt9 leafs
    int m_mgr2pt9;
    //! m_mgr2pt10 leafs
    int m_mgr2pt10;
    //! m_mgr2pt11 leafs
    int m_mgr2pt11;
    //! m_mgr2pt12 leafs
    int m_mgr2pt12;
    //! m_mgr2pt13 leafs
    int m_mgr2pt13;
    //! m_mgr2pt14 leafs
    int m_mgr2pt14;
    //! m_mgr2pt15 leafs
    int m_mgr2pt15;

    //! m_mgr3pt0 leafs
    int m_mgr3pt0;
    //! m_mgr3pt1 leafs
    int m_mgr3pt1;
    //! m_mgr3pt2 leafs
    int m_mgr3pt2;
    //! m_mgr3pt3 leafs
    int m_mgr3pt3;
    //! m_mgr3pt4 leafs
    int m_mgr3pt4;
    //! m_mgr3pt5 leafs
    int m_mgr3pt5;
    //! m_mgr3pt6 leafs
    int m_mgr3pt6;
    //! m_mgr3pt7 leafs
    int m_mgr3pt7;
    //! m_mgr3pt8 leafs
    int m_mgr3pt8;
    //! m_mgr3pt9 leafs
    int m_mgr3pt9;
    //! m_mgr3pt10 leafs
    int m_mgr3pt10;
    //! m_mgr3pt11 leafs
    int m_mgr3pt11;
    //! m_mgr3pt12 leafs
    int m_mgr3pt12;
    //! m_mgr3pt13 leafs
    int m_mgr3pt13;
    //! m_mgr3pt14 leafs
    int m_mgr3pt14;
    //! m_mgr3pt15 leafs
    int m_mgr3pt15;

    //! m_mgr4pt0 leafs
    int m_mgr4pt0;
    //! m_mgr4pt1 leafs
    int m_mgr4pt1;
    //! m_mgr4pt2 leafs
    int m_mgr4pt2;
    //! m_mgr4pt3 leafs
    int m_mgr4pt3;
    //! m_mgr4pt4 leafs
    int m_mgr4pt4;
    //! m_mgr4pt5 leafs
    int m_mgr4pt5;
    //! m_mgr4pt6 leafs
    int m_mgr4pt6;
    //! m_mgr4pt7 leafs
    int m_mgr4pt7;
    //! m_mgr4pt8 leafs
    int m_mgr4pt8;
    //! m_mgr4pt9 leafs
    int m_mgr4pt9;
    //! m_mgr4pt10 leafs
    int m_mgr4pt10;
    //! m_mgr4pt11 leafs
    int m_mgr4pt11;
    //! m_mgr4pt12 leafs
    int m_mgr4pt12;
    //! m_mgr4pt13 leafs
    int m_mgr4pt13;
    //! m_mgr4pt14 leafs
    int m_mgr4pt14;
    //! m_mgr4pt15 leafs
    int m_mgr4pt15;

    //! m_mgr0l0hit leafs
    int m_mgr0l0hit;
    //! m_mgr0l1hit leafs
    int m_mgr0l1hit;
    //! m_mgr0l2hit leafs
    int m_mgr0l2hit;
    //! m_mgr0l3hit leafs
    int m_mgr0l3hit;
    //! m_mgr0l4hit leafs
    int m_mgr0l4hit;
    //! m_mgr1l0hit leafs
    int m_mgr1l0hit;
    //! m_mgr1l1hit leafs
    int m_mgr1l1hit;
    //! m_mgr1l2hit leafs
    int m_mgr1l2hit;
    //! m_mgr1l3hit leafs
    int m_mgr1l3hit;
    //! m_mgr1l4hit leafs
    int m_mgr1l4hit;
    //! m_mgr2l0hit leafs
    int m_mgr2l0hit;
    //! m_mgr2l1hit leafs
    int m_mgr2l1hit;
    //! m_mgr2l2hit leafs
    int m_mgr2l2hit;
    //! m_mgr2l3hit leafs
    int m_mgr2l3hit;
    //! m_mgr2l4hit leafs
    int m_mgr2l4hit;
    //! m_mgr3l0hit leafs
    int m_mgr3l0hit;
    //! m_mgr3l1hit leafs
    int m_mgr3l1hit;
    //! m_mgr3l2hit leafs
    int m_mgr3l2hit;
    //! m_mgr3l3hit leafs
    int m_mgr3l3hit;
    //! m_mgr3l4hit leafs
    int m_mgr3l4hit;
    //! m_mgr4l0hit leafs
    int m_mgr4l0hit;
    //! m_mgr4l1hit leafs
    int m_mgr4l1hit;
    //! m_mgr4l2hit leafs
    int m_mgr4l2hit;
    //! m_mgr4l3hit leafs
    int m_mgr4l3hit;
    //! m_mgr4l4hit leafs
    int m_mgr4l4hit;

    //! m_mgr0l0 leafs
    int m_mgr0l0;
    //! m_mgr0l1 leafs
    int m_mgr0l1;
    //! m_mgr0l2 leafs
    int m_mgr0l2;
    //! m_mgr0l3 leafs
    int m_mgr0l3;
    //! m_mgr0l4 leafs
    int m_mgr0l4;
    //! m_mgr0l5 leafs
    int m_mgr0l5;
    //! m_mgr0l6 leafs
    int m_mgr0l6;
    //! m_mgr0l7 leafs
    int m_mgr0l7;

    //! m_mgr1l0 leafs
    int m_mgr1l0;
    //! m_mgr1l1 leafs
    int m_mgr1l1;
    //! m_mgr1l2 leafs
    int m_mgr1l2;
    //! m_mgr1l3 leafs
    int m_mgr1l3;
    //! m_mgr1l4 leafs
    int m_mgr1l4;
    //! m_mgr1l5 leafs
    int m_mgr1l5;
    //! m_mgr1l6 leafs
    int m_mgr1l6;
    //! m_mgr1l7 leafs
    int m_mgr1l7;

    //! m_mgr2l0 leafs
    int m_mgr2l0;
    //! m_mgr2l1 leafs
    int m_mgr2l1;
    //! m_mgr2l2 leafs
    int m_mgr2l2;
    //! m_mgr2l3 leafs
    int m_mgr2l3;
    //! m_mgr2l4 leafs
    int m_mgr2l4;
    //! m_mgr2l5 leafs
    int m_mgr2l5;
    //! m_mgr2l6 leafs
    int m_mgr2l6;
    //! m_mgr2l7 leafs
    int m_mgr2l7;

    //! m_mgr3l0 leafs
    int m_mgr3l0;
    //! m_mgr3l1 leafs
    int m_mgr3l1;
    //! m_mgr3l2 leafs
    int m_mgr3l2;
    //! m_mgr3l3 leafs
    int m_mgr3l3;
    //! m_mgr3l4 leafs
    int m_mgr3l4;
    //! m_mgr3l5 leafs
    int m_mgr3l5;
    //! m_mgr3l6 leafs
    int m_mgr3l6;
    //! m_mgr3l7 leafs
    int m_mgr3l7;

    //! m_mgr4l0 leafs
    int m_mgr4l0;
    //! m_mgr4l1 leafs
    int m_mgr4l1;
    //! m_mgr4l2 leafs
    int m_mgr4l2;
    //! m_mgr4l3 leafs
    int m_mgr4l3;
    //! m_mgr4l4 leafs
    int m_mgr4l4;
    //! m_mgr4l5 leafs
    int m_mgr4l5;
    //! m_mgr4l6 leafs
    int m_mgr4l6;
    //! m_mgr4l7 leafs
    int m_mgr4l7;

    //! m_firmid leafs
    int m_firmid;
    //! m_firmver leafs
    int m_firmver;
    //! m_evt leafs
    int m_evt;
    //! m_clk leafs
    int m_clk;
    //! m_N2DTS leafs to indicate 10 or 15 TS version
    int m_N2DTS;

    /** the class title */
    ClassDef(TRGCDCTSFUnpackerStore, 1);

  };

} // end namespace Belle2

#endif

