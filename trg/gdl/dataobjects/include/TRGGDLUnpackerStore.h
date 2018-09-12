//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGDLUnpackerStore.h
// Section  : TRG GDL
// Owner    :
// Email    :
//-----------------------------------------------------------
// Description : A class to represent TRG GDL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGGDLUNPACKERSTORE_H
#define TRGGDLUNPACKERSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGGDLUnpackerStore : public TObject {
  public:


    //! ETF FW version
    int m_etffmv;

    //! revoclk at L1
    int m_l1rvc;

    //! Timing Type
    int m_timtype;

    //! Event Type
    int m_etyp;

    //! TMDL out signal
    int m_final;

    //! gdlL1 signal
    int m_gdll1;

    //! ETF t0 value
    int m_etfout;

    //! ETF valid signal
    int m_etfvd;

    //! revoclk of TOP
    int m_toprvc;

    //! TOP valid signal
    int m_topvd;

    //! TOP t0 value
    int m_topt0;

    //! MSB 7bit of ECL t0 value
    int m_eclmsb7;

    //! LSB 7bit of ECL t0 value
    int m_ecllsb7;

    //! cdc_timing signal
    int m_cdc_timing;

    //! t0 value at TMDL output
    int m_rvcout;

    //! timing signal at TMDL output
    int m_rvcout0;

    //! ..
    int m_comrvc;

    //! ..
    int m_etnrvc;

    //! ..
    int m_nim0rvc;

    //! ..
    int m_etm0rvc;

    //! ..
    int m_rvc;

    //! num of revoclk between TMDL and L1
    int m_drvc;

    //! num of revoclk between TMDL and L1 when top_timing
    int m_dtoprvc;

    //! num of revoclk between TMDL and L1 when ecl_timing
    int m_declrvc;

    //! num of revoclk between TMDL and L1 when cdc_timing
    int m_dcdcrvc;

    //! 2nd word PSNM bits
    int m_psn1;

    //! ETF threshold 2
    int m_etfth2;

    //! ETF hold time
    int m_etfhdt;

    //! ETF threshold
    int m_etfth;

    //! PSNM signal 1st word
    int m_psn0;

    //! 2nd word of FTDL bits
    int m_ftd1;

    //! ..
    int m_etfvdrvc;

    //! 1st word of FTDL bits
    int m_ftd0;

    //! 3rd word of ITD bits
    int m_itd2;

    //! 2nd word of ITD bits
    int m_itd1;

    //! 1st word of ITD bits
    int m_itd0;

    //! 3rd word of INP bits
    int m_inp2;

    //! ..
    int m_inp1;

    //! ..
    int m_inp0;

    //! ..
    int m_evt;

    //! ..
    int m_clk;

    //! ..
    int m_coml1;

    //! ..
    int m_firmid;

    //! ..
    int m_firmver;

    //! ..
    int m_b2ldly;

    //! ..
    int m_maxrvc;

    /** the class title */
    ClassDef(TRGGDLUnpackerStore, 1);

  };

} // end namespace Belle2

#endif

