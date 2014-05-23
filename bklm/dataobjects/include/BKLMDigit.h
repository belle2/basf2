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

//#define BKLM_INNER 1
//#define BKLM_OUTER 2

namespace Belle2 {

  class BKLMSimHit;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMDigit : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMDigit();

    //! Constructor with initial values for an RPC hit
    BKLMDigit(const BKLMSimHit*, int);

    //! Constructor with initial values for a scint hit
    BKLMDigit(const BKLMSimHit*);

    //! Copy constructor
    BKLMDigit(const BKLMDigit&);

    //! Destructor
    virtual ~BKLMDigit() {}

    //! returns flag whether hit is in RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! returns whether the scint hit is usable in fit
    bool isAboveThreshold() const { return ((m_ModuleID & BKLM_ABOVETHRESHOLD_MASK) != 0); }

    //! returns end (TRUE=forward or FALSE=backward) of this strip
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! returns sector number of this strip (1..8)
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! returns layer number of this strip (1..15)
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! returns readout coordinate (TRUE=phi, FALSE=z) of this strip
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! returns strip number (1..64)
    int getStrip() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! returns unique detector-module ID (internally calculated)
    int getModuleID() const { return m_ModuleID; }

    //! returns MC-simulation hit time
    float getSimTime() const { return m_SimTime; }

    //! returns reconstructed hit time
    float getTime() const { return m_Time; }

    //! returns MC-simulation energy deposition
    float getSimEDep() const { return m_SimEDep; }

    //! returns reconstructed energy deposition
    float getEDep() const { return m_EDep; }

    //! returns the number of simulated MPPC pixels
    float getSimNPixel() const { return m_SimNPixel; }

    //! returns the number of reconstructed MPPC pixels
    float getNPixel() const { return m_NPixel; }

    //! returns status of pulse-fit (enum EKLM::FPGAFitStatus returned as int!)
    int getFitStatus() { return m_FitStatus; }

    //! determines if two BKLMDigits are equal based on geometry only
    bool match(const BKLMDigit* d) const { return (((d->getModuleID() ^ m_ModuleID) & BKLM_MODULESTRIPID_MASK) == 0); }

    //! sets whether scint hit is usable in fit
    void isAboveThreshold(bool flag) { m_ModuleID = (flag ? m_ModuleID | BKLM_ABOVETHRESHOLD_MASK : m_ModuleID & ~BKLM_ABOVETHRESHOLD_MASK); }

    //! sets reconstructed time (ns)
    void setTime(float time) { m_Time = time; }

    //! sets reconstructed energy deposition (MeV)
    void setEDep(float eDep) { m_EDep = eDep; }

    //! sets the number of simulated MPPC pixels (scintillator only)
    void setSimNPixel(int nPixel) { m_SimNPixel = nPixel; }

    //! sets the number of reconstructed MPPC pixels (scintillator only)
    void setNPixel(float nPixel) { m_NPixel = nPixel; }

    //! set the status of the pulse-fit (enum EKLM::FPGAFitStatus --> int!)
    void setFitStatus(int status) { m_FitStatus = status; }

  private:

    //! detector-module identifier, internally calculated (see BKLMStatus)
    //! bit 0      = end-1 [0..1]; forward is 0
    //! bits 1-3   = sector-1 [0..7]
    //! bits 4-7   = layer-1 [0..14]
    //! bit 8      = plane-1 [0..1]; inner is 0 and phiReadout
    //! bits 9-15  = strip-1 [0..95]
    //! bits 16-22 = maxStrip-1 [0..95] for RPCs only
    //! bit 23     = inRPC flag
    //! bit 24     = MC-generated hit
    //! bit 25     = MC decay-point hit
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
