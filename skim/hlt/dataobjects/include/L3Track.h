/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationsObject.h>
#include <TVectorD.h>

namespace Belle2 {

  //! Store the Level 3 trigger information
  class L3Track : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    L3Track();

    //! Constructor with flag
    explicit L3Track(const TVectorD& h);

    //! Destructor
    virtual ~L3Track() {}

    //! returns Helix parameter at the origin
    TVectorD getHelix() const { return TVectorD(5, m_helix); }

    //! returns drho
    double getDr() const { return m_helix[0]; }

    //! returns phi0
    double getPhi0() const { return m_helix[1]; }

    //! returns kappa
    double getKappa() const { return m_helix[2]; }

    //! returns dz
    double getDz() const { return m_helix[3]; }

    //! returns tan(lambda)
    double getTanL() const { return m_helix[4]; }

    //! assigns Helix parameter of the track
    void setHelix(const TVectorD& h);

  private:

    //! Level 3 Trigger information
    double m_helix[5];

    //
    ClassDef(L3Track, 1)

  };
}
