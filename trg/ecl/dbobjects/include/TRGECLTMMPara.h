/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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

