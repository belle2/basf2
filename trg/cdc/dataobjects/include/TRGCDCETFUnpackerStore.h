//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGCDCETFUnpackerStore.h
// Section  :
// Owner    :
// Email    :
//-----------------------------------------------------------
// Description : A dataobject for TRGCDCETFUnpacker.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGCDCETFUNPACKERSTORE_H
#define TRGCDCETFUNPACKERSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGCDCETFUnpackerStore : public TObject {
  public:

    int m_etf_threshold;
    int m_etf_threshold2;
    int m_etf_holdtime;
    int m_etf_firmvers;
    int m_etf_cc;
    int m_etf_output_gdl;
    int m_etf_valid_gdl;
    int m_etf_output;
    int m_etf_valid;

    int m_etf_tsf8_cc;
    int m_etf_tsf8_nhit;
    int m_etf_tsf8_ft9;
    int m_etf_tsf8_ft8;
    int m_etf_tsf8_ft7;
    int m_etf_tsf8_ft6;
    int m_etf_tsf8_ft5;
    int m_etf_tsf8_ft4;
    int m_etf_tsf8_ft3;
    int m_etf_tsf8_ft2;
    int m_etf_tsf8_ft1;
    int m_etf_tsf8_ft0;

    int m_etf_tsf7_cc;
    int m_etf_tsf7_nhit;
    int m_etf_tsf7_ft9;
    int m_etf_tsf7_ft8;
    int m_etf_tsf7_ft7;
    int m_etf_tsf7_ft6;
    int m_etf_tsf7_ft5;
    int m_etf_tsf7_ft4;
    int m_etf_tsf7_ft3;
    int m_etf_tsf7_ft2;
    int m_etf_tsf7_ft1;
    int m_etf_tsf7_ft0;

    int m_etf_tsf6_cc;
    int m_etf_tsf6_nhit;
    int m_etf_tsf6_ft9;
    int m_etf_tsf6_ft8;
    int m_etf_tsf6_ft7;
    int m_etf_tsf6_ft6;
    int m_etf_tsf6_ft5;
    int m_etf_tsf6_ft4;
    int m_etf_tsf6_ft3;
    int m_etf_tsf6_ft2;
    int m_etf_tsf6_ft1;
    int m_etf_tsf6_ft0;

    int m_etf_tsf5_cc;
    int m_etf_tsf5_nhit;
    int m_etf_tsf5_ft9;
    int m_etf_tsf5_ft8;
    int m_etf_tsf5_ft7;
    int m_etf_tsf5_ft6;
    int m_etf_tsf5_ft5;
    int m_etf_tsf5_ft4;
    int m_etf_tsf5_ft3;
    int m_etf_tsf5_ft2;
    int m_etf_tsf5_ft1;
    int m_etf_tsf5_ft0;

    int m_etf_tsf4_cc;
    int m_etf_tsf4_nhit;
    int m_etf_tsf4_ft9;
    int m_etf_tsf4_ft8;
    int m_etf_tsf4_ft7;
    int m_etf_tsf4_ft6;
    int m_etf_tsf4_ft5;
    int m_etf_tsf4_ft4;
    int m_etf_tsf4_ft3;
    int m_etf_tsf4_ft2;
    int m_etf_tsf4_ft1;
    int m_etf_tsf4_ft0;

    int m_etf_tsf3_cc;
    int m_etf_tsf3_nhit;
    int m_etf_tsf3_ft9;
    int m_etf_tsf3_ft8;
    int m_etf_tsf3_ft7;
    int m_etf_tsf3_ft6;
    int m_etf_tsf3_ft5;
    int m_etf_tsf3_ft4;
    int m_etf_tsf3_ft3;
    int m_etf_tsf3_ft2;
    int m_etf_tsf3_ft1;
    int m_etf_tsf3_ft0;

    int m_etf_tsf2_cc;
    int m_etf_tsf2_nhit;
    int m_etf_tsf2_ft9;
    int m_etf_tsf2_ft8;
    int m_etf_tsf2_ft7;
    int m_etf_tsf2_ft6;
    int m_etf_tsf2_ft5;
    int m_etf_tsf2_ft4;
    int m_etf_tsf2_ft3;
    int m_etf_tsf2_ft2;
    int m_etf_tsf2_ft1;
    int m_etf_tsf2_ft0;

    int m_etf_tsf1_cc;
    int m_etf_tsf1_nhit;
    int m_etf_tsf1_ft9;
    int m_etf_tsf1_ft8;
    int m_etf_tsf1_ft7;
    int m_etf_tsf1_ft6;
    int m_etf_tsf1_ft5;
    int m_etf_tsf1_ft4;
    int m_etf_tsf1_ft3;
    int m_etf_tsf1_ft2;
    int m_etf_tsf1_ft1;
    int m_etf_tsf1_ft0;

    int m_etf_tsf0_cc;
    int m_etf_tsf0_nhit;
    int m_etf_tsf0_ft9;
    int m_etf_tsf0_ft8;
    int m_etf_tsf0_ft7;
    int m_etf_tsf0_ft6;
    int m_etf_tsf0_ft5;
    int m_etf_tsf0_ft4;
    int m_etf_tsf0_ft3;
    int m_etf_tsf0_ft2;
    int m_etf_tsf0_ft1;
    int m_etf_tsf0_ft0;

    int m_evt;
    int m_clk;
    int m_firmid;
    int m_firmvers;

    /** the class title */
    ClassDef(TRGCDCETFUnpackerStore, 2);

  };

} // end namespace Belle2

#endif
