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
    /**m_etf_threhold leaf*/
    int m_etf_threshold;
    /**m_etf_threhold2 leaf*/
    int m_etf_threshold2;
    /**m_etf_holdtime leaf*/
    int m_etf_holdtime;
    /**m_etf_firmvers leaf*/
    int m_etf_firmvers;
    /**m_etf_cc leaf*/
    int m_etf_cc;
    /**m_etf_output_gdl leaf*/
    int m_etf_output_gdl;
    /**m_etf_valid_gdl leaf*/
    int m_etf_valid_gdl;
    /**m_etf_output leaf*/
    int m_etf_output;
    /**m_etf_valid leaf*/
    int m_etf_valid;

    /**m_etf_tsf8_cc leaf*/
    int m_etf_tsf8_cc;
    /**m_etf_tsf8_nhit leaf*/
    int m_etf_tsf8_nhit;
    /**m_etf_tsf8_ft9 leaf*/
    int m_etf_tsf8_ft9;
    /**m_etf_tsf8_ft8 leaf*/
    int m_etf_tsf8_ft8;
    /**m_etf_tsf8_ft7 leaf*/
    int m_etf_tsf8_ft7;
    /**m_etf_tsf8_ft6 leaf*/
    int m_etf_tsf8_ft6;
    /**m_etf_tsf8_ft5 leaf*/
    int m_etf_tsf8_ft5;
    /**m_etf_tsf8_ft4 leaf*/
    int m_etf_tsf8_ft4;
    /**m_etf_tsf8_ft3 leaf*/
    int m_etf_tsf8_ft3;
    /**m_etf_tsf8_ft2 leaf*/
    int m_etf_tsf8_ft2;
    /**m_etf_tsf8_ft1 leaf*/
    int m_etf_tsf8_ft1;
    /**m_etf_tsf8_ft0 leaf*/
    int m_etf_tsf8_ft0;

    /**m_etf_tsf7_cc leaf*/
    int m_etf_tsf7_cc;
    /**m_etf_tsf7_nhit leaf*/
    int m_etf_tsf7_nhit;
    /**m_etf_tsf7_ft9 leaf*/
    int m_etf_tsf7_ft9;
    /**m_etf_tsf7_ft8 leaf*/
    int m_etf_tsf7_ft8;
    /**m_etf_tsf7_ft7 leaf*/
    int m_etf_tsf7_ft7;
    /**m_etf_tsf7_ft6 leaf*/
    int m_etf_tsf7_ft6;
    /**m_etf_tsf7_ft5 leaf*/
    int m_etf_tsf7_ft5;
    /**m_etf_tsf7_ft4 leaf*/
    int m_etf_tsf7_ft4;
    /**m_etf_tsf7_ft3 leaf*/
    int m_etf_tsf7_ft3;
    /**m_etf_tsf7_ft2 leaf*/
    int m_etf_tsf7_ft2;
    /**m_etf_tsf7_ft1 leaf*/
    int m_etf_tsf7_ft1;
    /**m_etf_tsf7_ft0 leaf*/
    int m_etf_tsf7_ft0;

    /**m_etf_tsf6_cc leaf*/
    int m_etf_tsf6_cc;
    /**m_etf_tsf6_nhit leaf*/
    int m_etf_tsf6_nhit;
    /**m_etf_tsf6_ft9 leaf*/
    int m_etf_tsf6_ft9;
    /**m_etf_tsf6_ft8 leaf*/
    int m_etf_tsf6_ft8;
    /**m_etf_tsf6_ft7 leaf*/
    int m_etf_tsf6_ft7;
    /**m_etf_tsf6_ft6 leaf*/
    int m_etf_tsf6_ft6;
    /**m_etf_tsf6_ft5 leaf*/
    int m_etf_tsf6_ft5;
    /**m_etf_tsf6_ft4 leaf*/
    int m_etf_tsf6_ft4;
    /**m_etf_tsf6_ft3 leaf*/
    int m_etf_tsf6_ft3;
    /**m_etf_tsf6_ft2 leaf*/
    int m_etf_tsf6_ft2;
    /**m_etf_tsf6_ft1 leaf*/
    int m_etf_tsf6_ft1;
    /**m_etf_tsf6_ft0 leaf*/
    int m_etf_tsf6_ft0;

    /**m_etf_tsf5_cc leaf*/
    int m_etf_tsf5_cc;
    /**m_etf_tsf5_nhit leaf*/
    int m_etf_tsf5_nhit;
    /**m_etf_tsf5_ft9 leaf*/
    int m_etf_tsf5_ft9;
    /**m_etf_tsf5_ft8 leaf*/
    int m_etf_tsf5_ft8;
    /**m_etf_tsf5_ft7 leaf*/
    int m_etf_tsf5_ft7;
    /**m_etf_tsf5_ft6 leaf*/
    int m_etf_tsf5_ft6;
    /**m_etf_tsf5_ft5 leaf*/
    int m_etf_tsf5_ft5;
    /**m_etf_tsf5_ft4 leaf*/
    int m_etf_tsf5_ft4;
    /**m_etf_tsf5_ft3 leaf*/
    int m_etf_tsf5_ft3;
    /**m_etf_tsf5_ft2 leaf*/
    int m_etf_tsf5_ft2;
    /**m_etf_tsf5_ft1 leaf*/
    int m_etf_tsf5_ft1;
    /**m_etf_tsf5_ft0 leaf*/
    int m_etf_tsf5_ft0;

    /**m_etf_tsf4_cc leaf*/
    int m_etf_tsf4_cc;
    /**m_etf_tsfc_nhit leaf*/
    int m_etf_tsf4_nhit;
    /**m_etf_tsf4_ft9 leaf*/
    int m_etf_tsf4_ft9;
    /**m_etf_tsf4_ft8 leaf*/
    int m_etf_tsf4_ft8;
    /**m_etf_tsf4_ft7 leaf*/
    int m_etf_tsf4_ft7;
    /**m_etf_tsf4_ft6 leaf*/
    int m_etf_tsf4_ft6;
    /**m_etf_tsf4_ft5 leaf*/
    int m_etf_tsf4_ft5;
    /**m_etf_tsf4_ft4 leaf*/
    int m_etf_tsf4_ft4;
    /**m_etf_tsf4_ft3 leaf*/
    int m_etf_tsf4_ft3;
    /**m_etf_tsf4_ft2 leaf*/
    int m_etf_tsf4_ft2;
    /**m_etf_tsf4_ft1 leaf*/
    int m_etf_tsf4_ft1;
    /**m_etf_tsf4_ft0 leaf*/
    int m_etf_tsf4_ft0;

    /**m_etf_tsf3_cc leaf*/
    int m_etf_tsf3_cc;
    /**m_etf_tsf3_nhit leaf*/
    int m_etf_tsf3_nhit;
    /**m_etf_tsf3_ft9 leaf*/
    int m_etf_tsf3_ft9;
    /**m_etf_tsf3_ft8 leaf*/
    int m_etf_tsf3_ft8;
    /**m_etf_tsf3_ft7 leaf*/
    int m_etf_tsf3_ft7;
    /**m_etf_tsf3_ft6 leaf*/
    int m_etf_tsf3_ft6;
    /**m_etf_tsf3_ft5 leaf*/
    int m_etf_tsf3_ft5;
    /**m_etf_tsf3_ft4 leaf*/
    int m_etf_tsf3_ft4;
    /**m_etf_tsf3_ft3 leaf*/
    int m_etf_tsf3_ft3;
    /**m_etf_tsf3_ft2 leaf*/
    int m_etf_tsf3_ft2;
    /**m_etf_tsf3_ft1 leaf*/
    int m_etf_tsf3_ft1;
    /**m_etf_tsf3_ft0 leaf*/
    int m_etf_tsf3_ft0;

    /**m_etf_tsf2_cc leaf*/
    int m_etf_tsf2_cc;
    /**m_etf_tsf2_nhit leaf*/
    int m_etf_tsf2_nhit;
    /**m_etf_tsf2_ft9 leaf*/
    int m_etf_tsf2_ft9;
    /**m_etf_tsf2_ft8 leaf*/
    int m_etf_tsf2_ft8;
    /**m_etf_tsf2_ft7 leaf*/
    int m_etf_tsf2_ft7;
    /**m_etf_tsf2_ft6 leaf*/
    int m_etf_tsf2_ft6;
    /**m_etf_tsf2_ft5 leaf*/
    int m_etf_tsf2_ft5;
    /**m_etf_tsf2_ft4 leaf*/
    int m_etf_tsf2_ft4;
    /**m_etf_tsf2_ft3 leaf*/
    int m_etf_tsf2_ft3;
    /**m_etf_tsf2_ft2 leaf*/
    int m_etf_tsf2_ft2;
    /**m_etf_tsf2_ft1 leaf*/
    int m_etf_tsf2_ft1;
    /**m_etf_tsf2_ft0 leaf*/
    int m_etf_tsf2_ft0;

    /**m_etf_tsf1_cc leaf*/
    int m_etf_tsf1_cc;
    /**m_etf_tsf1_nhit leaf*/
    int m_etf_tsf1_nhit;
    /**m_etf_tsf1_ft9 leaf*/
    int m_etf_tsf1_ft9;
    /**m_etf_tsf1_ft8 leaf*/
    int m_etf_tsf1_ft8;
    /**m_etf_tsf1_ft7 leaf*/
    int m_etf_tsf1_ft7;
    /**m_etf_tsf1_ft6 leaf*/
    int m_etf_tsf1_ft6;
    /**m_etf_tsf1_ft5 leaf*/
    int m_etf_tsf1_ft5;
    /**m_etf_tsf1_ft4 leaf*/
    int m_etf_tsf1_ft4;
    /**m_etf_tsf1_ft3 leaf*/
    int m_etf_tsf1_ft3;
    /**m_etf_tsf1_ft2 leaf*/
    int m_etf_tsf1_ft2;
    /**m_etf_tsf1_ft1 leaf*/
    int m_etf_tsf1_ft1;
    /**m_etf_tsf1_ft0 leaf*/
    int m_etf_tsf1_ft0;

    /**m_etf_tsf0_cc leaf*/
    int m_etf_tsf0_cc;
    /**m_etf_tsf0_nhit leaf*/
    int m_etf_tsf0_nhit;
    /**m_etf_tsf0_ft9 leaf*/
    int m_etf_tsf0_ft9;
    /**m_etf_tsf0_ft8 leaf*/
    int m_etf_tsf0_ft8;
    /**m_etf_tsf0_ft7 leaf*/
    int m_etf_tsf0_ft7;
    /**m_etf_tsf0_ft6 leaf*/
    int m_etf_tsf0_ft6;
    /**m_etf_tsf0_ft5 leaf*/
    int m_etf_tsf0_ft5;
    /**m_etf_tsf0_ft4 leaf*/
    int m_etf_tsf0_ft4;
    /**m_etf_tsf0_ft3 leaf*/
    int m_etf_tsf0_ft3;
    /**m_etf_tsf0_ft2 leaf*/
    int m_etf_tsf0_ft2;
    /**m_etf_tsf0_ft1 leaf*/
    int m_etf_tsf0_ft1;
    /**m_etf_tsf0_ft0 leaf*/
    int m_etf_tsf0_ft0;

    /**m_evt leaf*/
    int m_evt;
    /**m_clk leaf*/
    int m_clk;
    /**m_firmid leaf*/
    int m_firmid;
    /**m_firmvers leaf*/
    int m_firmvers;

    /** the class title*/
    ClassDef(TRGCDCETFUnpackerStore, 2);

  };

} // end namespace Belle2

#endif
