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

#include <TVector3.h>

#define BKLM_INNER 1
#define BKLM_OUTER 2

namespace Belle2 {

  class BKLMSimHit;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMDigit();

    //! Constructor with initial values for an RPC or scint hit
//    BKLMDigit(unsigned int, int, bool, int, int, bool, int, int, const TVector3&, const TVector3&, double, double);

    //! Constructor with initial values for an RPC hit
    BKLMDigit(const BKLMSimHit*, int);

    //! Constructor with initial values for a scint hit
    BKLMDigit(const BKLMSimHit*);

    //! Copy constructor
    BKLMDigit(const BKLMDigit&);

    //! Destructor
    virtual ~BKLMDigit() {}

    //! returns status word
    unsigned int getStatus() const { return m_Status; }

    //! returns flag whether hit is in RPC (true) or scintillator (false)
    bool isInRPC() const { return ((m_Status & STATUS_INRPC) != 0); }

    //! returns the PDG code of the leading particle
    int getPDG(void) { return m_PDG; }

    //! returns whether the scint hit is usable in fit
    bool isGood() const { return m_IsAboveThreshold; }

    //! returns end (TRUE=forward or FALSE=backward) of this strip
    bool isForward() const { return m_IsForward; }

    //! returns sector number of this strip
    int getSector() const { return m_Sector; }

    //! returns layer number of this strip
    int getLayer() const { return m_Layer; }

    //! returns readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const { return m_IsPhiReadout; }

    //! returns plane number
    int getPlane() const { return (m_IsPhiReadout ? BKLM_INNER : BKLM_OUTER); }

    //! returns strip number
    int getStrip() const { return m_Strip; }

    //! returns unique detector-module ID
    int getModuleID() const { return m_ModuleID; }

    //! returns the MC-simulation global position of the hit
    TVector3 getSimGlobalPosition(void) const { return m_SimGlobalPosition; }

    //! returns the MC-simulation local position of the hit
    TVector3 getSimLocalPosition(void) const { return m_SimLocalPosition; }

    //! returns MC-simulation hit time
    double getSimTime() const { return m_SimTime; }

    //! returns reconstructed hit time
    double getTime() const { return m_Time; }

    //! returns MC-simulation energy deposition
    double getSimEDep() const { return m_SimEDep; }

    //! returns reconstructed energy deposition
    double getEDep() const { return m_EDep; }

    //! returns the number of simulated MPPC pixels
    double getSimNPixel() const { return m_SimNPixel; }

    //! returns the number of reconstructed MPPC pixels
    double getNPixel() const { return m_NPixel; }

    //! returns status of pulse-fit (enum EKLM::FPGAFitStatus returned as int!)
    int getFitStatus() { return m_FitStatus; }

    //! determines if two BKLMDigits are equal based on geometry only
    bool match(const BKLMDigit* d) const { return ((m_ModuleID == d->getModuleID()) && (m_Strip == d->getStrip())); }

    //! sets status word (all bits)
    void setStatus(unsigned int status) { m_Status = status; }

    //! sets some status bit(s)
    void setStatusBits(unsigned int status) { m_Status |= status; }

    //! clears status word (all bits)
    void clearStatus(void) { m_Status = 0; }

    //! clears some status bit(s)
    void clearStatusBits(unsigned int status) { m_Status &= (~status); }

    //! sets whether scint hit is usable in fit
    void isGood(bool flag) { m_IsAboveThreshold = flag; }

    //! sets reconstructed time (ns)
    void setTime(double time) { m_Time = time; }

    //! sets reconstructed energy deposition (MeV)
    void setEDep(double eDep) { m_EDep = eDep; }

    //! sets the number of simulated MPPC pixels (scintillator only)
    void setSimNPixel(int nPixel) { m_SimNPixel = nPixel; }

    //! sets the number of reconstructed MPPC pixels (scintillator only)
    void setNPixel(double nPixel) { m_NPixel = nPixel; }

    //! set the status of the pulse-fit (enum EKLM::FPGAFitStatus --> int!)
    void setFitStatus(int status) { m_FitStatus = status; }

  private:

    //! status word
    unsigned int m_Status;

    //! PDG code of the (leading) particle
    int m_PDG;

    //! axial end of the BKLM (TRUE=forward or FALSE=backward) of the strip
    bool m_IsForward;

    //! sector number of the strip
    int m_Sector;

    //! layer number of the strip
    int m_Layer;

    //! readout coordinate of the strip (TRUE=phi, FALSE=z)
    bool m_IsPhiReadout;

    //! strip number
    int m_Strip;

    //! unique detector-module identifier
    int m_ModuleID;

    //! MC-simulation Global position of the hit
    TVector3 m_SimGlobalPosition;

    //! MC-simulation Local position of the hit
    TVector3 m_SimLocalPosition;

    //! MC-simulation event hit time (ns)
    double m_SimTime;

    //! reconstructed hit time relative to trigger (ns)
    double m_Time;

    //! MC-simulation pulse height (MeV)
    double m_SimEDep;

    //! reconstructed pulse height (MeV)
    double m_EDep;

    //! flag to indicate whether the hit pulse height is above threshold
    bool m_IsAboveThreshold;

    //! simulated number of MPPC pixels
    int m_SimNPixel;

    //! reconstructed number of MPPC pixels (=photoelectrons in EKLM)
    double m_NPixel;

    //! pulse-fit status
    int m_FitStatus;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMDigit, 1)

  };

} // end of namespace Belle2

#endif //BKLMDIGIT_H
