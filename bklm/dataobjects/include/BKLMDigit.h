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

//#include <framework/datastore/RelationsObject.h>
#include <framework/dataobjects/DigitBase.h>
#include <bklm/dataobjects/BKLMStatus.h>

namespace Belle2 {
  class BKLMSimHit;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public DigitBase {

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

    //!Constructor with initial values for a real data hit
    //! @param moduleID gives sector, layer and strip
    //! @param ctime gives lower 16 bits of B2TT CTIME
    //! @param tdc the tdc value
    //! @@param charge the charge (number of photons for the scintillator strips)
    BKLMDigit(int moduleID, int ctime, short tdc, short charge);


    //! Copy constructor
    BKLMDigit(const BKLMDigit&);

    //! Assignment operator
    BKLMDigit& operator=(const BKLMDigit&);

    //! Destructor
    virtual ~BKLMDigit() {}



    //! Get unique channel identifier.
    //!@return unique channel id
    unsigned int getUniqueChannelID() const;



    //! The pile-up method.
    //! @return if the bg digit should be appended
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg);


    //! Determine whether hit is in RPC or scintillator
    //! @return whether hit is in RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! Determine whether the scint hit is usable in fit
    //! @return whether the scint hit is usable (true) or not (false) in pulse-shape fit
    bool isAboveThreshold() const { return ((m_ModuleID & BKLM_ABOVETHRESHOLD_MASK) != 0); }

    //! Get detector end
    //! @return detector end (TRUE=forward or FALSE=backward) of this strip
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! Get sector number
    //! @return sector number of this strip (1..8)
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! Get layer number
    //! @return layer number of this strip (1..15)
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! Get readout coordinate
    //! @return readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! Get strip number
    //! @return strip number (1..64)
    int getStrip() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! Get detector-module ID
    //! @return unique detector-module ID (internally calculated)
    int getModuleID() const { return m_ModuleID; }

    //! Get MC-simulation hit time
    //! @return MC-simulation hit time (ns)
    float getSimTime() const { return m_SimTime; }

    //! Get scint's reconstructed hit time
    //! @return reconstructed hit time (ns) from scint pulse-shape analysis
    float getTime() const { return m_Time; }

    //! Get MC-simulation energy deposition
    //! @return MC-simulation energy deposition (MeV)
    float getSimEDep() const { return m_SimEDep; }

    //! Get scint's reconstructed energy deposition
    //! @return reconstructed energy deposition (MeV)
    float getEDep() const { return m_EDep; }

    //! Get the number of simulated MPPC pixels
    //! @return the number of simulated MPPC pixels
    float getSimNPixel() const { return m_SimNPixel; }

    //! Get the number of reconstructed MPPC pixels
    //! @return the number of reconstructed MPPC pixels
    float getNPixel() const { return m_NPixel; }

    //! Get the charge value
    int getCharge() const { return m_Charge; }

    //! @return ctime
    int getCTime() const { return m_CTime; }

    //! Get the status of scint pulse-shape fit
    //! @return status of scint pulse-shape fit (enum EKLM::FPGAFitStatus returned as int!)
    int getFitStatus() { return m_FitStatus; }

    //! Determine whether two BKLMDigits refer to the same strip
    //! @return whether two BKLMDigits refer to the same strip (true) or not (false)
    bool match(const BKLMDigit* d) const { return (((d->getModuleID() ^ m_ModuleID) & BKLM_MODULESTRIPID_MASK) == 0); }

    //! Sets whether scint hit is usable in fit
    //! @param flag above threshold (true) or not (false)
    void isAboveThreshold(bool flag) { m_ModuleID = (flag ? m_ModuleID | BKLM_ABOVETHRESHOLD_MASK : m_ModuleID & ~BKLM_ABOVETHRESHOLD_MASK); }

    //! Set reconstructed time
    //! @param time reconstructed time (ns) from scint pulse-shape analysis
    void setTime(float time) { m_Time = time; }

    //! Set reconstructed energy deposition
    //! @param eDep reconstructed energy (MeV) from scint pulse-shape analysis
    void setEDep(float eDep) { m_EDep = eDep; }

    //! Set the number of simulated MPPC pixels (scintillator only)
    //! @param nPixel number of simulated MPPC pixels
    void setSimNPixel(int nPixel) { m_SimNPixel = nPixel; }

    //! Set the number of reconstructed MPPC pixels (scintillator only)
    //! @param nPixel number of reconstructed MPPC pixels from the pulse-shape analysis
    void setNPixel(float nPixel) { m_NPixel = nPixel; }

    //! Set the charge value
    void setCharge(int charge) { m_Charge = charge; }

    //! Set the status of the pulse-shape fit (enum EKLM::FPGAFitStatus --> int!)
    //! @param status completion status of the pulse-shape analysis
    void setFitStatus(int status) { m_FitStatus = status; }

  private:

    //!lowest 16 bits of the B2TT CTIME signal
    int m_CTime;

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

    //! reconstructed charge value of MPPC
    int m_Charge;

    //! pulse-fit status
    int m_FitStatus;

    //! Needed to make the ROOT object storable
    //! version 4 adds ctime etc
    ClassDef(BKLMDigit, 7)

  };

} // end of namespace Belle2

#endif //BKLMDIGIT_H
