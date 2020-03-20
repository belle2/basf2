/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMStatus.h>
#include <klm/dataobjects/KLMDigit.h>

namespace Belle2 {
  class BKLMSimHit;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public KLMDigit {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMDigit();

    //! Constructor with initial values for an RPC hit
    //! @param simHit pointer to the BKLMSimHit
    //! @param strip RPC strip number in a contiguous set
    explicit BKLMDigit(const BKLMSimHit* simHit, int strip);

    //! Constructor with initial values for a scint hit
    //! @param simHit pointer to the BKLMSimHit
    explicit BKLMDigit(const BKLMSimHit* simHit);

    //! Destructor
    virtual ~BKLMDigit() {}

    //! Get unique channel identifier.
    //!@return unique channel id
    unsigned int getUniqueChannelID() const override;

    //! The pile-up method.
    //! @return if the bg digit should be appended
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg) override;

    //! Determine whether hit is in RPC or scintillator
    //! @return whether hit is in RPC (true) or scintillator (false)
    bool inRPC() const
    {
      return getLayer() >= BKLMElementNumbers::c_FirstRPCLayer;
    }

    //! Get readout coordinate
    //! @return readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const
    {
      return m_Plane == BKLMElementNumbers::c_PhiPlane;
    }

    //! Get MC-simulation hit time
    //! @return MC-simulation hit time (ns)
    float getSimTime() const { return m_SimTime; }

    //! Get MC-simulation energy deposition
    //! @return MC-simulation energy deposition (MeV)
    float getSimEnergyDeposit() const { return m_SimEnergyDeposit; }

    //! Get the number of simulated MPPC pixels
    //! @return the number of simulated MPPC pixels
    float getSimNPixel() const { return m_SimNPixel; }

    //! Get the number of reconstructed MPPC pixels
    //! @return the number of reconstructed MPPC pixels
    float getNPixel() const { return m_NPixel; }

    //! Set the number of simulated MPPC pixels (scintillator only)
    //! @param nPixel number of simulated MPPC pixels
    void setSimNPixel(int nPixel) { m_SimNPixel = nPixel; }

    //! Set the number of reconstructed MPPC pixels (scintillator only)
    //! @param nPixel number of reconstructed MPPC pixels from the pulse-shape analysis
    void setNPixel(float nPixel) { m_NPixel = nPixel; }

  private:

    //! MC-simulation event hit time (ns)
    float m_SimTime;

    //! MC-simulation pulse height (MeV)
    float m_SimEnergyDeposit;

    //! simulated number of MPPC pixels
    int m_SimNPixel;

    //! reconstructed number of MPPC pixels (=photoelectrons in EKLM)
    float m_NPixel;

    //! Needed to make the ROOT object storable
    //! version 4 adds ctime etc
    ClassDefOverride(BKLMDigit, 7)

  };

} // end of namespace Belle2
