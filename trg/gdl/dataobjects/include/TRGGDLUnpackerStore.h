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


    //! ..
    int m_etffmv;

    //! ..
    int m_l1rvc;

    //! ..
    int m_timtype;

    //! ..
    int m_etyp;

    //! ..
    int m_final;

    //! ..
    int m_gdll1;

    //! ..
    int m_etfout;

    //! ..
    int m_etfvd;

    //! ..
    int m_toprvc;

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
    int m_rvcout;

    //! ..
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

    //! ..
    int m_drvc;

    //! ..
    int m_dtoprvc;

    //! ..
    int m_declrvc;

    //! ..
    int m_dcdcrvc;

    //! ..
    int m_psn1;

    //! ..
    int m_etfth2;

    //! ..
    int m_etfhdt;

    //! ..
    int m_etfth;

    //! ..
    int m_psn0;

    //! ..
    int m_ftd1;

    //! ..
    int m_etfvdrvc;

    //! ..
    int m_ftd0;

    //! ..
    int m_itd2;

    //! ..
    int m_itd1;

    //! ..
    int m_itd0;

    //! ..
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

