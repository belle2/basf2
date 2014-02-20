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
#include <TMatrixD.h>
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

    //! returns status word of this 2D hit
    unsigned int getStatus() const { return m_Status; }

    //! returns flag whether this 2D hit is in RPC (true) or scintillator (false)
    bool isInRPC() const { return ((m_Status & STATUS_INRPC) != 0); }

    //! returns end (TRUE=forward or FALSE=backward) of this 2D hit
    bool isForward() const { return m_IsForward; }

    //! returns sector number (1..8) of this 2D hit
    int getSector() const { return m_Sector; }

    //! returns layer number (1..15) of this 2D hit
    int getLayer() const { return m_Layer; }

    //! returns lowest phi-measuring strip number of this 2D hit
    int getPhiStripMin() const { return m_PhiStripMin; }

    //! returns highest phi-measuring strip number of this 2D hit
    int getPhiStripMax() const { return m_PhiStripMax; }

    //! returns number of phi-measuring strips of this 2D hit
    int getPhiStripCount() const { return m_PhiStripCount; }

    //! returns phi-measuring strip number of this 2D hit
    double getPhiStripAve() const { return m_PhiStripAve; }

    //! returns phi-measuring strip position error of this 2D hit
    double getPhiStripErr() const { return m_PhiStripErr; }

    //! returns lowest z-measuring strip number of this 2D hit
    int getZStripMin() const { return m_ZStripMin; }

    //! returns highest z-measuring strip number of this 2D hit
    int getZStripMax() const { return m_ZStripMax; }

    //! returns number of z-measuring strips of this 2D hit
    int getZStripCount() const { return m_ZStripCount; }

    //! returns z-measuring strip number of this 2D hit
    double getZStripAve() const { return m_ZStripAve; }

    //! returns z-measuring strip position error of this 2D hit
    double getZStripErr() const { return m_ZStripErr; }

    //! returns detector-module identifier
    int getModuleID() const { return m_ModuleID; }

    //! returns 3D hit position in global coordinates (cm)
    TVector3 getGlobalPosition(void) const { return m_GlobalPosition; }

    //! returns 2D hit position and uncertainty in local coordinates (cm)
    void getLocalPosition(double position[2], double positionError[2]) const;

    //! returns 2D reconstructed hit time (average of the 2 projections)
    double getTime() const { return m_Time; }

    //! returns 2D reconstructed energy deposition (sum of the 2 projections)
    double getEDep() const { return m_EDep; }

    //! determines if two BKLMHit2ds are equal based on geometry only
    bool match(const BKLMHit2d* h) const { return (m_ModuleID == h->getModuleID()); }

    //! sets status word
    void setStatus(int status) { m_Status = status; }

    //! sets some status bit(s)
    void setStatusBits(unsigned int status) { m_Status |= status; }

    //! clears status word (all bits)
    void clearStatus(void) { m_Status = 0; }

    //! clears some status bit(s)
    void clearStatusBits(unsigned int status) { m_Status &= (~status); }

    //! set global position
    void setGlobalPosition(double, double, double);

    //! set local position
    void setLocalPosition(double, double, double);

    //! set local-position covariance matrix
    //! unused function    void setLocalVariance(double, double, double, double);

  private:

    //! status word
    int m_Status;

    //! axial end (true=forward or false=backward) of the 2D hit
    bool m_IsForward;

    //! sector number (1..8) of the 2D hit
    int m_Sector;

    //! layer number (1..15) of the 2D hit
    int m_Layer;

    //! unique detector-module identifier
    int m_ModuleID;

    //! lowest phi-measuring strip number of the 2D hit
    int m_PhiStripMin;

    //! highest phi-measuring strip number of the 2D hit
    int m_PhiStripMax;

    //! number of phi-measuring strips of the 2D hit
    int m_PhiStripCount;

    //! average phi-measuring strip of the 2D hit
    double m_PhiStripAve;

    //! phi-measuring strip position uncertainty of the 2D hit
    double m_PhiStripErr;

    //! lowest z-measuring strip number of the 2D hit
    int m_ZStripMin;

    //! highest z-measuring strip number of the 2D hit
    int m_ZStripMax;

    //! number of z-measuring strips of the 2D hit
    int m_ZStripCount;

    //! average z-measuring strip of the 2D hit
    double m_ZStripAve;

    //! z-measuring strip position uncertainty of the 2D hit
    double m_ZStripErr;

    //! position of the 2D hit in global coordinates (cm)
    TVector3 m_GlobalPosition;

    //! position of the 2D hit in local coordinates (cm)
    TVector3 m_LocalPosition;

    //! covariance matrix of the 2D hit' position in local coordinates (cm)
    TMatrixD m_LocalVariance;

    //! reconstructed hit time relative to trigger (ns)
    double m_Time;

    //! reconstructed pulse height (MeV)
    double m_EDep;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMHit2d, 2)

  };

} // end of namespace Belle2

#endif //BKLMHIT2D_H
