/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMHITPOSITION_H
#define BKLMSIMHITPOSITION_H

#include <framework/datastore/RelationsObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Store one simulation hit's global position as a ROOT object
  class BKLMSimHitPosition : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMSimHitPosition() : RelationsObject() {}

    //! Constructor with initial values (from simulation step)
    //! @param x Cartesian x coordinate of global position
    //! @param y Cartesian y coordinate of global position
    //! @param z Cartesian z coordinate of global position
    BKLMSimHitPosition(double x, double y, double z) : RelationsObject() { m_Position.SetX(x); m_Position.SetY(y); m_Position.SetZ(z); }

    //! Destructor
    virtual ~BKLMSimHitPosition() {}

    //! Copy constructor
    BKLMSimHitPosition(const BKLMSimHitPosition& p) : RelationsObject(p), m_Position(p.m_Position) {}

    //! Assignment operator
    BKLMSimHitPosition& operator=(const BKLMSimHitPosition& p) { m_Position = p.m_Position; return *this;}

    //! Get the BKLMSimHit's global position
    //! @return Cartesian global position (cm)
    const TVector3& getPosition() { return m_Position; }

  private:

    //! BKLMSimHit's global position (cm)
    TVector3 m_Position;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMSimHitPosition, 1);

  };

} // end of namespace Belle2

#endif //BKLMSIMHITPOSITION_H
