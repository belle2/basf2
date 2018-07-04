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

    //! timestamp
    int m_rvc;

    //! Timing Type of TMDL output
    int m_timtype;

    //! Event Type of TMDL output
    int m_etyp;

    //! trigger type in TMDL
    int m_tttmdl;

    //! timing source pipe in TMDL
    int m_timingp;

    //! t0 value of TMDL output
    int m_rvcout;

    //! revoclk when GDL receives top_timing
    int m_toprvc;

    //! revoclk when GDL receives ecl_timing
    int m_eclrvc;

    //! timestamp when GDL receives cdc_timing
    int m_cdcrvc;

    //! t0 value from TOP
    int m_toptiming;

    //! t0 value from ECL
    int m_ecltiming;

    //! t0 value from CDC
    int m_cdctiming;

    //! timestamp when GDL receives nimin0 signal
    int m_nim0rvc;

    //! relative timestamp between TMDL out and L1 when top_timing
    int m_dtoprvc;

    //! relative timestamp between TMDL out and L1 when ecl_timing
    int m_declrvc;

    //! relative timestamp between TMDL out and L1 when cdc_timing
    int m_dcdcrvc;

    //! 3rd word of PSNM bits
    int m_psn2;

    //! 2nd word of PSNM bits
    int m_psn1;

    //! 1st word of PSNM bits
    int m_psn0;

    //! 2nd word of TOP geometry signal
    int m_topslot1;

    //! 1st word of TOP geometry signal
    int m_topslot0;

    //! number of top slot used to generate top_timing
    int m_ntopslot;

    //! 3rd word of FTDL bits
    int m_ftd2;

    //! 2nd word of FTDL bits
    int m_ftd1;

    //! 1st word of FTDL bits
    int m_ftd0;

    //! 3rd word of ITD bits
    int m_itd2;

    //! 2nd word of ITD bits
    int m_itd1;

    //! 1st word of ITD bits
    int m_itd0;

    //! 3rd word of Input bits
    int m_inp2;

    //! 2nd word of Input bits
    int m_inp1;

    //! 1st word of Input bits
    int m_inp0;

    //! event number
    int m_evt;

    //! sequencial value in time window
    int m_clk;

    //! Firmware ID
    int m_firmid;

    //! Firmware Version
    int m_firmver;

    //! timestamp of TMDL final signal
    int m_finalrvc;

    //! relative timestamp between TMDL and L1
    int m_drvc;

    //! timestamp when GDL generates L1
    int m_gdll1rvc;

    //! timestamp when GDL receives L1
    int m_coml1rvc;

    //! delay for belle2link
    int m_b2ldly;

    //! Accepted timestamp range
    int m_maxrvc;

    //! ..
    int m_etffmv;

    //! ..
    int m_l1rvc;

    //! ..
    int m_final;

    //! ..
    int m_gdll1;

    //! ..
    int m_etfout;

    //! ..
    int m_etfvd;

    //! ..
    int m_topvd;

    //! ..
    int m_topt0;

    //! ..
    int m_eclmsb7;

    //! ..
    int m_ecllsb7;

    //! ..
    int m_cdc_timing;

    //! ..
    int m_rvcout0;

    //! ..
    int m_comrvc;

    //! ..
    int m_etnrvc;

    //! ..
    int m_etm0rvc;

    //! ..
    int m_etfth2;

    //! ..
    int m_etfhdt;

    //! ..
    int m_etfth;

    //! ..
    int m_etfvdrvc;

    //! ..
    int m_coml1;

    /** the class title */
    ClassDef(TRGGDLUnpackerStore, 1);

  };

} // end namespace Belle2
#endif
