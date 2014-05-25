/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSIMHIT_H
#define BKLMSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>
#include <framework/datastore/RelationsObject.h>
#include <bklm/dataobjects/BKLMStatus.h>

//#define BKLM_INNER 1
//#define BKLM_OUTER 2

namespace Belle2 {

  //! Store one simulation hit as a ROOT object
  class BKLMSimHit : public SimHitBase {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMSimHit();

    //! Constructor with initial values (from simulation step)
    BKLMSimHit(int, double, double, double);

    //! Destructor
    virtual ~BKLMSimHit() {}

    //! Copy constructor
    BKLMSimHit(const BKLMSimHit&);

    //! returns true if this hit is in an RPC
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! returns axial end (TRUE=forward or FALSE=backward) of this hit
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! returns sector number of this hit
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! returns layer number of this hit
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! returns plane (0=inner or 1=outer) of this hit
    //int getPlane() const { return (((m_ModuleID & BKLM_PLANE_MASK) != 0) ? BKLM_INNER : BKLM_OUTER); }

    //! returns readout-coordinate (TRUE=phi, FALSE=z) of this hit
    bool isPhiReadout() const { return ((m_ModuleID & BKLM_PLANE_MASK) != 0); }

    //! returns unique readout strip number of this hit (assumes one hit)
    int getStrip() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! returns lowest readout strip number of this hit
    int getStripMin() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! returns highest readout strip number of this hit
    int getStripMax() const { return (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1); }

    //! returns unique detector-module identifier
    int getModuleID() const { return m_ModuleID; }

    //! returns x coordinate of the local position of the hit (for time-of-propagation)
    float getLocalPositionX() const { return m_LocalX; }

    //! returns the event hit time
    double getTime() const { return (double)m_Time; }

    //! returns energy deposition
    double getEDep() const { return (double)m_EDep; }

    //! increase energy deposition
    void increaseEDep(double eDep) { m_EDep += eDep; }

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

    //! event hit time (ns)
    float m_Time;

    //! energy deposition (MeV)
    float m_EDep;

    //! local x coordinate of hit position (cm) for time-of-propagation
    float m_LocalX;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMSimHit, 3);

  };

} // end of namespace Belle2

#endif //BKLMSIMHIT_H
