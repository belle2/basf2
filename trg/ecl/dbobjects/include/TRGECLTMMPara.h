//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLTMMPara.h
// Section  : TRG ECL
// Owner    : HanEol Cho/InSu Lee/Yuuji Unno
// Email    : hecho@hep.hanyang.ac.kr / islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLTMMPARA_H
#define TRGECLTMMPARA_H

#include <TObject.h>

namespace Belle2 {

  //! Raw TC result nefor digitizing
  class TRGECLTMMPara : public TObject {
  public:
    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLTMMPara() : m_FPGAversion(1)
    {

    }
    //! Construction
    explicit TRGECLTMMPara(int FPGAversion):
      m_FPGAversion(FPGAversion)

    {
    }

    //! Set FPGAversion
    void setFPGAversion(int FPGAversion) { m_FPGAversion = FPGAversion; }

    //! Get FPGAversion
    int getFPGAversion() const
    { return m_FPGAversion ; }


    //  private :

    //! FPGAversion
    int m_FPGAversion;


    //! the class title
    ClassDef(TRGECLTMMPara, 1); /*< the class title */
  };
} //! end namespace Belle2

#endif

