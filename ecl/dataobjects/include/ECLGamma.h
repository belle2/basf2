/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLGAMMA_H
#define ECLGAMMA_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>
#include "ecl/dataobjects/ECLShower.h"
#include <math.h>


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class ECLGamma : public TObject {
  public:

    /**shower id of this Gamma. */
    int m_showerId;

    //! The method to set showerId
    void setShowerId(int showerId) { m_showerId = showerId; }

    //! The method to get showerId
    int GetShowerId() const { return m_showerId ; }

    //! The method to get return  TVector3 Momentum
    TVector3 getMomentum() const {
      TVector3 momentum(0., 0., 0.);
      StoreArray<ECLShower> eclRecShowerArray;
      ECLShower* aECLShower = eclRecShowerArray[m_showerId];
      double m_energy = aECLShower->GetEnergy();
      double m_theta = aECLShower->GetTheta();
      double m_phi = aECLShower->GetPhi();
      double m_px = m_energy * sin(m_theta) * cos(m_phi);
      double m_py = m_energy * sin(m_theta) * sin(m_phi);
      double m_pz = m_energy * cos(m_theta);

      momentum.SetX(m_px);
      momentum.SetY(m_py);
      momentum.SetZ(m_pz);
      return momentum;
    }


    float getPx() const {
      StoreArray<ECLShower> eclRecShowerArray;
      ECLShower* aECLShower = eclRecShowerArray[m_showerId];
      double m_energy = aECLShower->GetEnergy();
      double m_theta = aECLShower->GetTheta();
      double m_phi = aECLShower->GetPhi();
      double m_px = m_energy * sin(m_theta) * cos(m_phi);
      return (float)m_px;
    }


    float getPy() const {
      StoreArray<ECLShower> eclRecShowerArray;
      ECLShower* aECLShower = eclRecShowerArray[m_showerId];
      double m_energy = aECLShower->GetEnergy();
      double m_theta = aECLShower->GetTheta();
      double m_phi = aECLShower->GetPhi();
      double m_py = m_energy * sin(m_theta) * sin(m_phi);
      return (float)m_py;
    }


    float getPz() const {
      StoreArray<ECLShower> eclRecShowerArray;
      ECLShower* aECLShower = eclRecShowerArray[m_showerId];
      double m_energy = aECLShower->GetEnergy();
      double m_theta = aECLShower->GetTheta();
      double m_pz = m_energy * cos(m_theta);

      return (float)m_pz;
    }


    float getEnergy() const {
      TVector3 momentum(0., 0., 0.);
      StoreArray<ECLShower> eclRecShowerArray;
      ECLShower* aECLShower = eclRecShowerArray[m_showerId];
      double m_energy = aECLShower->GetEnergy();
      return (float)m_energy;
    }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLGamma() {;}
    /** ROOT Macro.*/
    ClassDef(ECLGamma, 1);

  };

} // end namespace Belle2

#endif
