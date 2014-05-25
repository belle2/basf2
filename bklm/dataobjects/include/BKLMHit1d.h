/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMHIT1D_H
#define BKLMHIT1D_H

#include <TVector3.h>
#include <framework/datastore/RelationsObject.h>
#include <bklm/dataobjects/BKLMStatus.h>

#include <vector>

namespace Belle2 {

  class BKLMDigit;

  //! Store one reconstructed BKLM 1D hit as a ROOT object
  class BKLMHit1d : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMHit1d();

    //! Constructor from a vector of BKLMDigits
    BKLMHit1d(const std::vector<Belle2::BKLMDigit*>&);

    //! Copy constructor
    BKLMHit1d(const BKLMHit1d&);

    //! Destructor
    virtual ~BKLMHit1d() {}

    //! returns flag whether this 1D hit is in RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! returns end (TRUE=forward or FALSE=backward) of this 1D hit
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! returns sector number (1..8) of this 1D hit
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! returns layer number (1..15) of this 1D hit
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! returns readout plane of this 1D hit
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! returns lowest strip number of this 1D hit
    int getStripMin() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! returns lowest strip number of this 1D hit
    int getStripMax() const { return (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1); }

    //! returns detector-module identifier
    int getModuleID() const { return m_ModuleID; }

    //! returns reconstructed hit time
    float getTime() const { return m_Time; }

    //! returns energy deposition
    float getEDep() const { return m_EDep; }

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
    //! bit 26     = out-of-time hit (from BKLM hit reconstruction)
    //! bit 27     = inefficient hit (from BKLM hit reconstruction)
    //! bit 28     = pulse height above threshold (from BKLM scint hit reconstruction)
    int m_ModuleID;

    //! reconstructed hit time (ns)
    float m_Time;

    //! reconstructed pulse height (MeV)
    float m_EDep;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit1d, 3)

  };

} // end of namespace Belle2

#endif //BKLMHIT1D_H
