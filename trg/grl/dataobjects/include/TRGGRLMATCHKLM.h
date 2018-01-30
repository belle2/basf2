//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGRLMATCHKLM.h
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@post.kek.jp
//-----------------------------------------------------------
// Description : storeArray for TRG GRL CDC-KLM matching.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGGRLMATCHKLM_H
#define TRGGRLMATCHKLM_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  class TRGGRLMATCHKLM : public RelationsObject {
  public:

    TRGGRLMATCHKLM(): m_dphi(-999.) {}
    ~TRGGRLMATCHKLM() {}

    /**Get dphi*/
    double get_dphi() const {return m_dphi;}

    /** set the dphi*/
    void set_dphi(double dphi) { m_dphi = dphi; }

  private:

    /**Phi angle difference between the 2D track's extrapolated hit on superconucting coil
    and the KLM track's sector*/
    double m_dphi;

    /**! The Class title*/
    ClassDef(TRGGRLMATCHKLM, 2); /*< the class title */

  };

} // end namespace Belle2

#endif
