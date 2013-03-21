/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMDIGIT_H
#define BKLMDIGIT_H

#include <framework/datastore/RelationsObject.h>

// Definition of each bit in m_Status
// BKLMDigit originated from MC simulation (rather than real data)
#define STATUS_MC 0x40000000

// BKLMDigit represents the decay point of a simulated particle
#define STATUS_DECAYED 0x20000000

// BKLMDigit is located in an RPC (rather than a scintillator)
#define STATUS_INRPC 0x00000001

// BKLMDigit is outside the in-time coincidence window
#define STATUS_OUTOFTIME 0x00000002

// BKLMDigit has been assigned to a track
#define STATUS_ONTRACK 0x00000004

namespace Belle2 {

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMDigit();

    //! Constructor with initial values
    BKLMDigit(unsigned int, bool, int, int, bool, int, double, double);

    //! Copy constructor
    BKLMDigit(const BKLMDigit&);

    //! Destructor
    virtual ~BKLMDigit() {}

    //! returns status word
    unsigned int getStatus() const { return m_Status; }

    //! returns flag whether hit is in RPC (true) or scintillator (false)
    bool isInRPC() const { return ((m_Status & 0x00000001) != 0); }

    //! returns end (TRUE=forward or FALSE=backward) of this strip
    bool isForward() const { return m_IsForward; }

    //! returns sector number of this strip
    int getSector() const { return m_Sector; }

    //! returns layer number of this strip
    int getLayer() const { return m_Layer; }

    //! returns readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const { return m_IsPhiReadout; }

    //! returns strip number
    int getStrip() const { return m_Strip; }

    //! returns hit time
    double getTime() const { return m_Time; }

    //! returns pulse height
    double getEnergy() const { return m_Energy; }

    //! determines if two BKLMDigits are equal based on geometry only
    bool match(const BKLMDigit*) const;

    //! sets status word (all bits)
    void setStatus(unsigned int status) { m_Status = status; }

    //! sets some status bit(s)
    void setStatusBits(unsigned int status) { m_Status |= status; }

    //! clears status word (all bits)
    void clearStatus(void) { m_Status = 0; }

    //! clears some status bit(s)
    void clearStatusBits(unsigned int status) { m_Status &= (~status); }

    //! sets time (ns)
    void setTime(double time) { m_Time = time; }

    //! sets energy (MeV)
    void setEnergy(double energy) { m_Energy = energy; }

  private:

    //! status word
    unsigned int m_Status;

    //! axial end (TRUE=forward or FALSE=backward) of the strip
    bool m_IsForward;

    //! sector number of the strip
    int m_Sector;

    //! layer number of the strip
    int m_Layer;

    //! readout coordinate of the strip (TRUE=phi, FALSE=z)
    bool m_IsPhiReadout;

    //! strip number
    int m_Strip;

    //! global hit time relative to trigger (ns)
    double m_Time;

    //! pulse height (MeV)
    double m_Energy;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMDigit, 1)

  };

} // end of namespace Belle2

#endif //BKLMDIGIT_H
