//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGRLPHOTON.h
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@post.kek.jp
//-----------------------------------------------------------
// Description : storeArray for TRG GRL CDC-KLM matching.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGGRLPHOTON_H
#define TRGGRLPHOTON_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  class TRGGRLPHOTON : public RelationsObject {
  public:

    TRGGRLPHOTON(): m_phi_d(-999.), m_e(-1.0) {}
    ~TRGGRLPHOTON() {}

    /**Get phi_d*/
    double get_phi_d() const {return m_phi_d;}

    /** set the phi_d*/
    void set_phi_d(double phi_d) { m_phi_d = phi_d; }

    /**Get energy*/
    double get_e() const {return m_e;}

    /** set energy*/
    void set_e(double e) { m_e = e; }

  private:

    /**Digitized phi angle of the cluster*/
    double m_phi_d;

    /**Energy of the cluster*/
    double m_e;

    /**! The Class title*/
    ClassDef(TRGGRLPHOTON, 2); /*< the class title */

  };

} // end namespace Belle2

#endif
