/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLPI0_H
#define ECLPI0_H

#include <framework/datastore/DataStore.h>
#include "ecl/dataobjects/ECLShower.h"

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class ECLPi0 : public TObject {
  public:


    //! The shower point of this gamma1.
    ECLShower* m_aECLShower1;

    //! The method to set shower1 id
    void setShower1(ECLShower* aECLShower) { m_aECLShower1 = aECLShower;}

    //! The method to get shower1
    ECLShower* getShower1() {return m_aECLShower1; }

    //! The method to get shower1 id
    int getShower1Id() const { return  m_aECLShower1->GetShowerId(); }

    //! The shower point of this gamma2.
    ECLShower* m_aECLShower2;

    //! The method to set shower1 id
    void setShower2(ECLShower* aECLShower) { m_aECLShower2 = aECLShower;}

    //! The method to get shower1
    ECLShower* getShower2() {return m_aECLShower2; }

    //! The method to get shower1 id
    int getShower2Id() const { return  m_aECLShower2->GetShowerId(); }




    //! The energy of this Pi0.
    float m_energy;

    //! The px of this Pi0.
    float m_px;

    //! The py of this Pi0.
    float m_py;

    //! The pz of this Pi0.
    float m_pz;

    //! The mass of this Pi0.
    float m_mass;

    //! The massfit of this Pi0.
    float m_massfit;

    //! The chi2 of this Pi0.
    float m_chi2;


    //! The method to set energy
    void setenergy(float energy) { m_energy = energy; }

    //! The method to set px
    void setpx(float px) { m_px = px; }

    //! The method to set py
    void setpy(float py) { m_py = py; }

    //! The method to set pz
    void setpz(float pz) { m_pz = pz; }

    //! The method to set mass
    void setmass(float mass) { m_mass = mass; }

    //! The method to set massfit
    void setmassfit(float massfit) { m_massfit = massfit; }

    //! The method to set fit chi2
    void setchi2(float chi2) { m_chi2 = chi2; }


    //! The method to get energy
    float getenergy() const { return m_energy; }
    //! The method to get px
    float getpx() const { return m_px; }
    //! The method to get py
    float getpy() const { return m_py; }
    //! The method to get pz
    float getpz() const { return m_pz; }

    //! The method to get mass
    float getmass() const { return m_mass; }
    //! The method to get massfit
    float getmassfit() const { return m_massfit; }
    //! The method to get energy
    float getchi2() const { return m_chi2; }


    //! The method to get momentum
    TVector3 getp() const { TVector3 momentum(m_px, m_py, m_pz); return momentum ; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLPi0() {;}
    /** ROOT Macro.*/
    ClassDef(ECLPi0, 1);

  };

} // end namespace Belle2

#endif
