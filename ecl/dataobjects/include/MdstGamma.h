/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDSTGAMMA_H
#define MDSTGAMMA_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class MdstGamma : public TObject {
  public:


    //! The shower id of this gamma.
    int m_showerId;

    //! The px of this gamma.
    double m_px;

    //! The py of this gamma.
    double m_py;

    //! The pz of this gamma.
    double m_pz;

    //! The method to set shower id
    void setShowerId(int showerId) { m_showerId = showerId; }

    //! The method to set px
    void setpx(double px) { m_px = px; }

    //! The method to set py
    void setpy(double py) { m_py = py; }

    //! The method to set pz
    void setpz(double pz) { m_pz = pz; }

    //! The method to get shower id
    int getShowerId() const { return m_showerId; }

    //! The method to get px
    double getpx() const { return m_px; }

    //! The method to get py
    double getpy() const { return m_py; }

    //! The method to get pz
    double getpz() const { return m_pz; }

    //! The method to get momentum
    TVector3 getp() const { TVector3 momentum(m_px, m_py, m_pz); return momentum ; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    MdstGamma() {;}

    ClassDef(MdstGamma, 1);

  };

} // end namespace Belle2

#endif
