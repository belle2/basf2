//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGRLMATCH.h
// Section  : TRG GRL
// Owner    : Ke LI / Junhao Yin
// Email    : like@ihep.ac.cn / yinjh@ihep.ac.cn
//-----------------------------------------------------------
// Description : storeArray for TRG GRL, only the match info.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGGRLMATCH_H
#define TRGGRLMATCH_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  class TRGGRLMATCH : public RelationsObject {
  public:

    TRGGRLMATCH(): m_deltar(-999.), m_deltaz(-999.) {}
    ~TRGGRLMATCH() {}

    double getDeltaR() const {return m_deltar;}

    /**Get the Delta Z (distance along Z-axis between the ECL cluster and the expected position
    which is calculated from CDC) for CDC track and ECL cluster*/
    double getDeltaZ() const {return m_deltaz;}

    /**Get dphi_d*/
    double get_dphi_d() const {return m_dphi_d;}

    /** set the Delta R*/
    void setDeltaR(double deltar) { m_deltar = deltar; }

    /** set the Delta Z*/
    void setDeltaZ(double deltaz) { m_deltaz = deltaz; }

    /** set the dphi_d*/
    void set_dphi_d(double dphi_d) { m_dphi_d = dphi_d; }

  private:


    /**Distance in X-Y plane between the ECL cluster and the expected position
    which is calculated from CDC*/
    double m_deltar;

    /**Distance along Z-axis between the ECL cluster and the expected position
    which is calculated from CDC*/
    double m_deltaz;

    /**Phi angle difference between the ECL cluster and the expected position
    which is calculated from CDC*/
    double m_dphi_d;

    /**! The Class title*/
    ClassDef(TRGGRLMATCH, 2); /*< the class title */

  };

} // end namespace Belle2

#endif
