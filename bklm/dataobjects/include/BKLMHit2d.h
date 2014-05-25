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
#include <bklm/dataobjects/BKLMStatus.h>

namespace Belle2 {

  class BKLMHit1d;

  //! Store one BKLM strip hit as a ROOT object
  class BKLMHit2d : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMHit2d();

    //! Constructor with orthogonal 1D hits
    BKLMHit2d(const BKLMHit1d*, const BKLMHit1d*);

    //! Copy constructor
    BKLMHit2d(const BKLMHit2d&);

    //! Destructor
    virtual ~BKLMHit2d() {}

    //! returns flag whether this 2D hit is in RPC (true) or scintillator (false)
    bool inRPC() const { return ((m_ModuleID & BKLM_INRPC_MASK) != 0); }

    //! returns end (TRUE=forward or FALSE=backward) of this 2D hit
    bool isForward() const { return ((m_ModuleID & BKLM_END_MASK) != 0); }

    //! returns sector number (1..8) of this 2D hit
    int getSector() const { return (((m_ModuleID & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT) + 1); }

    //! returns layer number (1..15) of this 2D hit
    int getLayer() const { return (((m_ModuleID & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT) + 1); }

    //! returns lowest phi-measuring strip number of this 2D hit
    int getPhiStripMin() const { return (((m_ModuleID & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT) + 1); }

    //! returns highest phi-measuring strip number of this 2D hit
    int getPhiStripMax() const { return (((m_ModuleID & BKLM_MAXSTRIP_MASK) >> BKLM_MAXSTRIP_BIT) + 1); }

    //! returns phi-measuring strip number of this 2D hit
    float getPhiStripAve() const;

    //! returns lowest z-measuring strip number of this 2D hit
    int getZStripMin() const { return (((m_ZStrips & BKLM_ZSTRIP_MASK) >> BKLM_ZSTRIP_BIT) + 1); }

    //! returns highest z-measuring strip number of this 2D hit
    int getZStripMax() const { return (((m_ZStrips & BKLM_ZMAXSTRIP_MASK) >> BKLM_ZMAXSTRIP_BIT) + 1); }

    //! returns z-measuring strip number of this 2D hit
    float getZStripAve() const;

    //! returns detector-module identifier
    int getModuleID() const { return m_ModuleID; }

    //! get the OutOfTime bit for this hit
    bool isOutOfTime() { return (m_ModuleID & BKLM_OUTOFTIME_MASK) != 0; }

    //! get the OnTrack bit for this hit
    bool isOnTrack() { return (m_ModuleID & BKLM_ONTRACK_MASK) != 0; }

    //! get the Inefficient bit for this hit
    bool isInefficient() { return (m_ModuleID & BKLM_INEFFICIENT_MASK) != 0; }

    //! returns 3D hit position in global coordinates (cm)
    TVector3 getGlobalPosition(void) const { return TVector3(m_GlobalPosition[0], m_GlobalPosition[1], m_GlobalPosition[2]); }

    //! returns 2D hit position and uncertainty in local coordinates (cm)
    //void getLocalPosition(double position[2], double positionError[2]) const;

    //! returns 2D reconstructed hit time (average of the 2 projections)
    float getTime() const { return m_Time; }

    //! returns 2D reconstructed energy deposition (sum of the 2 projections)
    float getEDep() const { return m_EDep; }

    //! determines if two BKLMHit2ds are equal based on geometry only
    bool match(const BKLMHit2d* h) const { return (((h->getModuleID() ^ m_ModuleID) & BKLM_MODULEID_MASK) == 0); }

    //! set the OutOfTime bit for this hit
    void isOutOfTime(bool flag) { if (flag) { m_ModuleID |= BKLM_OUTOFTIME_MASK; } else { m_ModuleID &= ~BKLM_OUTOFTIME_MASK; } }

    //! set the OnTrack bit for this hit
    void isOnTrack(bool flag) { if (flag) { m_ModuleID |= BKLM_ONTRACK_MASK; } else { m_ModuleID &= ~BKLM_ONTRACK_MASK; } }

    //! set the Inefficient bit for this hit
    void isInefficient(bool flag) { if (flag) { m_ModuleID |= BKLM_INEFFICIENT_MASK; } else { m_ModuleID &= ~BKLM_INEFFICIENT_MASK; } }

    //! set global position
    void setGlobalPosition(double, double, double);

    //! set local position
    void setLocalPosition(double, double, double);

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
    //! bit 29     = hit is assigned to a track (from muid reconstruction)
    int m_ModuleID;

    //! z-measuring strip numbers of the 2D hit
    int m_ZStrips;

    //! position of the 2D hit in global coordinates (cm)
    float m_GlobalPosition[3];

    //! position of the 2D hit in local coordinates (cm)
    float m_LocalPosition[3];

    //! reconstructed hit time relative to trigger (ns)
    float m_Time;

    //! reconstructed pulse height (MeV)
    float m_EDep;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit2d, 3)

  };

} // end of namespace Belle2

#endif //BKLMHIT2D_H
