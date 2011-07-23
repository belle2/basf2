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

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Store one simulation hit as a ROOT object
  /*! Per ROOT TObject convention, all data elements are public so that
      they can be seen in a ROOT browser.
  */
  class BKLMSimHit : public TObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    BKLMSimHit();

    //! Constructor with initial values
    BKLMSimHit(const TVector3, double, double, bool, bool, int, int, int, int);

    //! Destructor
    virtual ~BKLMSimHit() {}

    //! returns position of the hit
    TVector3 getHitPosition() const { return m_HitPosition; }

    //! returns hit time
    double getHitTime() const { return m_HitTime; }

    //! returns energy deposition
    double getDeltaE() const { return m_DeltaE; }

    //! returns flag whether hit is in RPC (true) or scintillator (false)
    bool getInRPC() const { return m_InRPC; }

    //! returns flag whether particle decayed in this hit
    bool getDecayed() const { return m_Decayed; }

    //! returns end (0=forward or 1=backward) of this hit
    int getFrontBack() const { return m_FrontBack; }

    //! returns sector number of this hit
    int getSector() const { return m_Sector; }

    //! returns layer number of this hit
    int getLayer() const { return m_Layer; }

    //! returns plane (0=inner or 1=outer) of this hit
    int getPlane() const { return m_Plane; }

    //! returns pointer to the vector of phiStrips
    const std::vector<int>* getPhiStrips() const { return &m_PhiStrips; }

    //! returns pointer to the vector of zStrips
    const std::vector<int>* getZStrips() const { return &m_ZStrips; }

    //! append a new strip to the vector of phi strips
    void appendPhiStrip(int strip) { m_PhiStrips.push_back(strip); }

    //! append a new strip to the vector of z strips
    void appendZStrip(int strip) { m_ZStrips.push_back(strip); }

    //! global-frame hit position (cm)
    TVector3 m_HitPosition;

    //! global hit time (ns)
    double m_HitTime;

    //! energy deposition (MeV)
    double m_DeltaE;

    //! flag to say whether the hit is in RPC (true) or scintillator (false)
    bool m_InRPC;

    //! flag to say whether the track decayed in this hit
    bool m_Decayed;

    //! barrel end (forward or backward) of the hit
    int m_FrontBack;

    //! sector number of the hit
    int m_Sector;

    //! layer number of the hit
    int m_Layer;

    //! inner or outer plane of the hit's module
    int m_Plane;

    //! List of phi strips associated with this hit
    std::vector<int> m_PhiStrips;

    //! List of z strips associated with this hit
    std::vector<int> m_ZStrips;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMSimHit, 1);

  private:

  };

} // end of namespace Belle2

#endif //BKLMSIMHIT_H
