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
    int m_trackercc;
    int m_trackerhit0id;
    int m_trackerhit0rt;
    int m_trackerhit0v;
    int m_trackerhit1id;
    int m_trackerhit1rt;
    int m_trackerhit1v;
    int m_trackerhit2id;
    int m_trackerhit2rt;
    int m_trackerhit2v;
    int m_trackerhit3id;
    int m_trackerhit3rt;
    int m_trackerhit3v;
    int m_trackerhit4id;
    int m_trackerhit4rt;
    int m_trackerhit4v;
    int m_trackerhit5id;
    int m_trackerhit5rt;
    int m_trackerhit5v;
    int m_trackerhit6id;
    int m_trackerhit6rt;
    int m_trackerhit6v;
    int m_trackerhit7id;
    int m_trackerhit7rt;
    int m_trackerhit7v;
    int m_trackerhit8id;
    int m_trackerhit8rt;
    int m_trackerhit8v;
    int m_trackerhit9id;
    int m_trackerhit9rt;
    int m_trackerhit9v;


    int m_validetf;
    int m_etfcc;
    int m_netfhit;
    int m_etfhit0;
    int m_etfhit1;
    int m_etfhit2;
    int m_etfhit3;
    int m_etfhit4;
    int m_etfhit5;
    int m_etfhit6;
    int m_etfhit7;
    int m_etfhit8;
    int m_etfhit9;

    //! tsf leafs
    int m_mgr0;
    int m_mgr0cc;
    int m_mgr1;
    int m_mgr1cc;
    int m_mgr2;
    int m_mgr2cc;
    int m_mgr3;
    int m_mgr3cc;
    int m_mgr4;
    int m_mgr4cc;
    int m_mgr5;
    int m_mgr5cc;

    //! tsf leafs
    int m_mgr0ft0;
    int m_mgr0ft1;
    int m_mgr0ft2;
    int m_mgr0ft3;
    int m_mgr0ft4;
    int m_mgr0ft5;
    int m_mgr0ft6;
    int m_mgr0ft7;
    int m_mgr0ft8;
    int m_mgr0ft9;
    int m_mgr0ft10;
    int m_mgr0ft11;
    int m_mgr0ft12;
    int m_mgr0ft13;
    int m_mgr0ft14;
    int m_mgr0ft15;


    int m_mgr1ft0;
    int m_mgr1ft1;
    int m_mgr1ft2;
    int m_mgr1ft3;
    int m_mgr1ft4;
    int m_mgr1ft5;
    int m_mgr1ft6;
    int m_mgr1ft7;
    int m_mgr1ft8;
    int m_mgr1ft9;
    int m_mgr1ft10;
    int m_mgr1ft11;
    int m_mgr1ft12;
    int m_mgr1ft13;
    int m_mgr1ft14;
    int m_mgr1ft15;

    int m_mgr2ft0;
    int m_mgr2ft1;
    int m_mgr2ft2;
    int m_mgr2ft3;
    int m_mgr2ft4;
    int m_mgr2ft5;
    int m_mgr2ft6;
    int m_mgr2ft7;
    int m_mgr2ft8;
    int m_mgr2ft9;
    int m_mgr2ft10;
    int m_mgr2ft11;
    int m_mgr2ft12;
    int m_mgr2ft13;
    int m_mgr2ft14;
    int m_mgr2ft15;

    int m_mgr3ft0;
    int m_mgr3ft1;
    int m_mgr3ft2;
    int m_mgr3ft3;
    int m_mgr3ft4;
    int m_mgr3ft5;
    int m_mgr3ft6;
    int m_mgr3ft7;
    int m_mgr3ft8;
    int m_mgr3ft9;
    int m_mgr3ft10;
    int m_mgr3ft11;
    int m_mgr3ft12;
    int m_mgr3ft13;
    int m_mgr3ft14;
    int m_mgr3ft15;

    int m_mgr4ft0;
    int m_mgr4ft1;
    int m_mgr4ft2;
    int m_mgr4ft3;
    int m_mgr4ft4;
    int m_mgr4ft5;
    int m_mgr4ft6;
    int m_mgr4ft7;
    int m_mgr4ft8;
    int m_mgr4ft9;
    int m_mgr4ft10;
    int m_mgr4ft11;
    int m_mgr4ft12;
    int m_mgr4ft13;
    int m_mgr4ft14;
    int m_mgr4ft15;

    //! tsf leafs
    int m_mgr0pt0;
    int m_mgr0pt1;
    int m_mgr0pt2;
    int m_mgr0pt3;
    int m_mgr0pt4;
    int m_mgr0pt5;
    int m_mgr0pt6;
    int m_mgr0pt7;
    int m_mgr0pt8;
    int m_mgr0pt9;
    int m_mgr0pt10;
    int m_mgr0pt11;
    int m_mgr0pt12;
    int m_mgr0pt13;
    int m_mgr0pt14;
    int m_mgr0pt15;

    int m_mgr1pt0;
    int m_mgr1pt1;
    int m_mgr1pt2;
    int m_mgr1pt3;
    int m_mgr1pt4;
    int m_mgr1pt5;
    int m_mgr1pt6;
    int m_mgr1pt7;
    int m_mgr1pt8;
    int m_mgr1pt9;
    int m_mgr1pt10;
    int m_mgr1pt11;
    int m_mgr1pt12;
    int m_mgr1pt13;
    int m_mgr1pt14;
    int m_mgr1pt15;

    int m_mgr2pt0;
    int m_mgr2pt1;
    int m_mgr2pt2;
    int m_mgr2pt3;
    int m_mgr2pt4;
    int m_mgr2pt5;
    int m_mgr2pt6;
    int m_mgr2pt7;
    int m_mgr2pt8;
    int m_mgr2pt9;
    int m_mgr2pt10;
    int m_mgr2pt11;
    int m_mgr2pt12;
    int m_mgr2pt13;
    int m_mgr2pt14;
    int m_mgr2pt15;

    int m_mgr3pt0;
    int m_mgr3pt1;
    int m_mgr3pt2;
    int m_mgr3pt3;
    int m_mgr3pt4;
    int m_mgr3pt5;
    int m_mgr3pt6;
    int m_mgr3pt7;
    int m_mgr3pt8;
    int m_mgr3pt9;
    int m_mgr3pt10;
    int m_mgr3pt11;
    int m_mgr3pt12;
    int m_mgr3pt13;
    int m_mgr3pt14;
    int m_mgr3pt15;

    int m_mgr4pt0;
    int m_mgr4pt1;
    int m_mgr4pt2;
    int m_mgr4pt3;
    int m_mgr4pt4;
    int m_mgr4pt5;
    int m_mgr4pt6;
    int m_mgr4pt7;
    int m_mgr4pt8;
    int m_mgr4pt9;
    int m_mgr4pt10;
    int m_mgr4pt11;
    int m_mgr4pt12;
    int m_mgr4pt13;
    int m_mgr4pt14;
    int m_mgr4pt15;

    int m_mgr0l0hit;
    int m_mgr0l1hit;
    int m_mgr0l2hit;
    int m_mgr0l3hit;
    int m_mgr0l4hit;
    int m_mgr1l0hit;
    int m_mgr1l1hit;
    int m_mgr1l2hit;
    int m_mgr1l3hit;
    int m_mgr1l4hit;
    int m_mgr2l0hit;
    int m_mgr2l1hit;
    int m_mgr2l2hit;
    int m_mgr2l3hit;
    int m_mgr2l4hit;
    int m_mgr3l0hit;
    int m_mgr3l1hit;
    int m_mgr3l2hit;
    int m_mgr3l3hit;
    int m_mgr3l4hit;
    int m_mgr4l0hit;
    int m_mgr4l1hit;
    int m_mgr4l2hit;
    int m_mgr4l3hit;
    int m_mgr4l4hit;

    int m_mgr0l0;
    int m_mgr0l1;
    int m_mgr0l2;
    int m_mgr0l3;
    int m_mgr0l4;
    int m_mgr0l5;
    int m_mgr0l6;
    int m_mgr0l7;

    int m_mgr1l0;
    int m_mgr1l1;
    int m_mgr1l2;
    int m_mgr1l3;
    int m_mgr1l4;
    int m_mgr1l5;
    int m_mgr1l6;
    int m_mgr1l7;

    int m_mgr2l0;
    int m_mgr2l1;
    int m_mgr2l2;
    int m_mgr2l3;
    int m_mgr2l4;
    int m_mgr2l5;
    int m_mgr2l6;
    int m_mgr2l7;

    int m_mgr3l0;
    int m_mgr3l1;
    int m_mgr3l2;
    int m_mgr3l3;
    int m_mgr3l4;
    int m_mgr3l5;
    int m_mgr3l6;
    int m_mgr3l7;

    int m_mgr4l0;
    int m_mgr4l1;
    int m_mgr4l2;
    int m_mgr4l3;
    int m_mgr4l4;
    int m_mgr4l5;
    int m_mgr4l6;
    int m_mgr4l7;

    //! tsf leafs
    int m_firmid;
    //! tsf leafs
    int m_firmver;
    //! tsf leafs
    int m_evt;
    //! tsf leafs
    int m_clk;

    /** the class title */
    ClassDef(TRGCDCTSFUnpackerStore, 1);

  };

} // end namespace Belle2

#endif

