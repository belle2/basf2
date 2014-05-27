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
#include <bklm/dataobjects/BKLMStatus.h>

namespace Belle2 {

  class BKLMSimHit;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMDigit();

    //! Constructor with initial values for an RPC hit
    //! @param simHit pointer to the BKLMSimHit
    //! @param strip RPC strip number in a contiguous set
    BKLMDigit(const BKLMSimHit* simHit, int strip);

    //! Constructor with initial values for a scint hit
    //! @param simHit pointer to the BKLMSimHit
    BKLMDigit(const BKLMSimHit* simHit);

    //! Copy constructor
    BKLMDigit(const BKLMDigit&);

    //! Destructor
    virtual ~BKLMDigit() {}

    //! @return whether hit is in RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! @return whether the scint hit is usable in fit (true) or not (false)
    bool isAboveThreshold() const { return ((m_ModuleID & BKLM_ABOVETHRESHOLD_MASK) != 0); }

    //! @return detector end (TRUE=forward or FALSE=backward) of this strip
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! @return sector number of this strip (1..8)
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! @return layer number of this strip (1..15)
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! @return readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! @return strip number (1..64)
    int getStrip() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! @return unique detector-module ID (internally calculated)
    int getModuleID() const { return m_ModuleID; }

    //! @return MC-simulation hit time (ns)
    float getSimTime() const { return m_SimTime; }

    //! @return reconstructed hit time (ns) from scint pulse-shape analysis
    float getTime() const { return m_Time; }

    //! @return MC-simulation energy deposition (MeV)
    float getSimEDep() const { return m_SimEDep; }

    //! @return reconstructed energy deposition (MeV)
    float getEDep() const { return m_EDep; }

    //! @return the number of simulated MPPC pixels
    float getSimNPixel() const { return m_SimNPixel; }

    //! @return the number of reconstructed MPPC pixels
    float getNPixel() const { return m_NPixel; }

    //! @return status of scint pulse-shape fit (enum EKLM::FPGAFitStatus returned as int!)
    int getFitStatus() { return m_FitStatus; }

    //! @return whether two BKLMDigits refer to the same strip (true) or not (false)
    bool match(const BKLMDigit* d) const { return (((d->getModuleID() ^ m_ModuleID) & BKLM_MODULESTRIPID_MASK) == 0); }

    //! sets whether scint hit is usable in fit
    //! @param flag above threshold (true) or not (false)
    void isAboveThreshold(bool flag) { m_ModuleID = (flag ? m_ModuleID | BKLM_ABOVETHRESHOLD_MASK : m_ModuleID & ~BKLM_ABOVETHRESHOLD_MASK); }

    //! sets reconstructed time
    //! @param time reconstructed time (ns) from scint pulse-shape analysis
    void setTime(float time) { m_Time = time; }

    //! sets reconstructed energy deposition
    //! @param eDep reconstructed energy (MeV) from scint pulse-shape analysis
    void setEDep(float eDep) { m_EDep = eDep; }

    //! sets the number of simulated MPPC pixels (scintillator only)
    //! @param nPixel number of simulated MPPC pixels
    void setSimNPixel(int nPixel) { m_SimNPixel = nPixel; }

    //! sets the number of reconstructed MPPC pixels (scintillator only)
    //! @param nPixel number of reconstructed MPPC pixels from the pulse-shape analysis
    void setNPixel(float nPixel) { m_NPixel = nPixel; }

    //! set the status of the pulse-shape fit (enum EKLM::FPGAFitStatus --> int!)
    //! @param status completion status of the pulse-shape analysis
    void setFitStatus(int status) { m_FitStatus = status; }

  private:

    //! detector-module identifier
    //! @sa BKLMStatus.h
    int m_ModuleID;

    //! MC-simulation event hit time (ns)
    float m_SimTime;

    //! reconstructed hit time relative to trigger (ns)
    float m_Time;

    //! MC-simulation pulse height (MeV)
    float m_SimEDep;

    //! reconstructed pulse height (MeV)
    float m_EDep;

    //! simulated number of MPPC pixels
    int m_SimNPixel;

    //! reconstructed number of MPPC pixels (=photoelectrons in EKLM)
    float m_NPixel;

    //! pulse-fit status
    int m_FitStatus;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMDigit, 3)

  };

} // end of namespace Belle2

#endif //BKLMDIGIT_H
