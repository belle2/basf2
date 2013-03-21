/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMHIT2D_H
#define BKLMHIT2D_H

#include <TVector3.h>
#include <framework/datastore/RelationsObject.h>
#include <bklm/dataobjects/BKLMDigit.h>

namespace Belle2 {

  //! Store one BKLM strip hit as a ROOT object
  class BKLMHit2d : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMHit2d();

    //! Constructor with orthogonal 1D hits
    BKLMHit2d(const BKLMDigit*, const BKLMDigit*);

    //! Constructor with initial values
    BKLMHit2d(int, bool, int, int, const double*, const double*, double, double);

    //! Copy constructor
    BKLMHit2d(const BKLMHit2d&);

    //! Destructor
    virtual ~BKLMHit2d() {}

    //! returns status word
    bool getStatus() const { return m_Status; }

    //! returns axial flag (true=forward or false=backward) of this 2D hit
    bool isForward() const { return m_IsForward; }

    //! returns sector number (1..8) of this 2D hit
    int getSector() const { return m_Sector; }

    //! returns layer number (1..15) of this 2D hit
    int getLayer() const { return m_Layer; }

    //! returns 3D hit position in global coordinates (cm)
    TVector3 getGlobalPosition(void) const { return m_GlobalPosition; }

    //! returns 2D hit position and uncertainty in local coordinates (cm)
    void getLocalPosition(double position[2], double positionError[2]) const;

    //! returns 2D hit time
    double getTime() const { return m_Time; }

    //! returns 2D hit energy
    double getEnergy() const { return m_Energy; }

    //! determines if two BKLMHit2ds are equal based on geometry only
    bool match(const BKLMHit2d*) const;

    //! sets status word
    void setStatus(int status) { m_Status = status; }

    //! sets axial end (true=forward or false=backward) of this 2D hit
    void setIsForward(bool isForward) { m_IsForward = isForward; }

    //! sets sector number (1..8) of this 2D hit
    void setSector(int sector) { m_Sector = sector; }

    //! sets layer number (1..15) of this 2D hit
    void setLayer(int layer) { m_Layer = layer; }

    //! sets hit position and uncertainty in local coordinates (cm)
    void setLocalPosition(const double*, const double*);

    //! sets hit time
    void setTime(double time) { m_Time = time; }

    //! sets pulse height
    void setEnergy(double energy) { m_Energy = energy; }

  private:

    //! status word
    int m_Status;

    //! axial end (true=forward or false=backward) of the 2D hit
    bool m_IsForward;

    //! sector number (1..8) of the 2D hit
    int m_Sector;

    //! layer number (1..15) of the 2D hit
    int m_Layer;

    //! position of the 2D hit in global coordinates (cm)
    TVector3 m_GlobalPosition;

    //! position of the 2D hit in local coordinates (cm)
    double m_Position[2];

    //! uncertainty in position of the 2D hit in local coordinates (cm)
    double m_PositionError[2];

    //! global hit time relative to trigger (ns)
    double m_Time;

    //! energy (MeV)
    double m_Energy;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit2d, 1)

  };

} // end of namespace Belle2

#endif //BKLMHIT2D_H
