/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDSTPI0_H
#define MDSTPI0_H

#include <framework/datastore/DataStore.h>

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class MdstPi0 : public TObject {
  public:


    //! The shower id of 1st Gamma.
    int m_showerId1;

    //! The shower id of 2nd Gamma.
    int m_showerId2;


    //! The energy of this Pi0.
    double m_energy;

    //! The px of this Pi0.
    double m_px;

    //! The py of this Pi0.
    double m_py;

    //! The pz of this Pi0.
    double m_pz;

    //! The mass of this Pi0.
    double m_mass;

    //! The massfit of this Pi0.
    double m_massfit;

    //! The chi2 of this Pi0.
    double m_chi2;


    //! The method to set gamma1
    void setShowerId1(int showerId1) { m_showerId1 = showerId1; }
    //! The method to set gamma2
    void setShowerId2(int showerId2) { m_showerId2 = showerId2; }


    //! The method to set energy
    void setenergy(double energy) { m_energy = energy; }

    //! The method to set px
    void setpx(double px) { m_px = px; }

    //! The method to set py
    void setpy(double py) { m_py = py; }

    //! The method to set pz
    void setpz(double pz) { m_pz = pz; }

    //! The method to set mass
    void setmass(double mass) { m_mass = mass; }

    //! The method to set massfit
    void setmassfit(double massfit) { m_massfit = massfit; }

    //! The method to set fit chi2
    void setchi2(double chi2) { m_chi2 = chi2; }



    //! The method to get shower id1
    int getShowerId1() const { return m_showerId1; }
    //! The method to get shower id2
    int getShowerId2() const { return m_showerId2; }

    //! The method to get energy
    double getenergy() const { return m_energy; }
    //! The method to get px
    double getpx() const { return m_px; }
    //! The method to get py
    double getpy() const { return m_py; }
    //! The method to get pz
    double getpz() const { return m_pz; }

    //! The method to get mass
    double getmass() const { return m_mass; }
    //! The method to get massfit
    double getmassfit() const { return m_massfit; }
    //! The method to get energy
    double getchi2() const { return m_chi2; }


    //! The method to get momentum
    TVector3 getp() const { TVector3 momentum(m_px, m_py, m_pz); return momentum ; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    MdstPi0() {;}

    ClassDef(MdstPi0, 1);

  };

} // end namespace Belle2

#endif
