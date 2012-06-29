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
#include "ecl/dataobjects/MdstShower.h"
#include <math.h>


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class MdstGamma : public TObject {
  public:

    //! The shower point of this gamma.
    MdstShower* m_aECLShower;

    //! The method to set shower id
    void setShower(MdstShower* aECLShower) { m_aECLShower = aECLShower;}

    //! The method to get shower
    MdstShower* getShower() {return m_aECLShower; }

    //! The method to get shower id
    int getShowerId() const { return  m_aECLShower->GetShowerId(); }

    //! The method to get px
    float getpx() const { return   m_aECLShower->GetEnergy() * sin(m_aECLShower->GetTheta()) * cos(m_aECLShower->GetPhi()) ; }

    //! The method to get py
    float getpy() const { return   m_aECLShower->GetEnergy() * sin(m_aECLShower->GetTheta()) * sin(m_aECLShower->GetPhi()) ; }

    //! The method to get pz
    float getpz() const { return   m_aECLShower->GetEnergy() * cos(m_aECLShower->GetTheta()); }

    //! The method to get momentum
    TVector3 getp() const { TVector3 momentum(getpx(), getpy(), getpz()); return momentum ; }


    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    MdstGamma() {;}
    /** ROOT Macro.*/
    ClassDef(MdstGamma, 1);

  };

} // end namespace Belle2

#endif
