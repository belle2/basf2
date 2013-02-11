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
    BKLMSimHit(const TVector3&, double, double, double, unsigned int, bool, int, int, int, bool, int, int);

    //! Destructor
    virtual ~BKLMSimHit() {}

    //! returns position of the hit
    TVector3 getPosition() const { return m_Position; }

    //! returns position of the hit (alias for EVEVisualization.cc)
    TVector3 getHitPosition() const { return m_Position; }

    //! returns hit time
    double getTime() const { return m_Time; }

    //! returns energy deposition
    double getDeltaE() const { return m_DeltaE; }

    //! returns kinetic energy of throughgoing particle
    double getKE() const { return m_KE; }

    //! returns status word
    unsigned int getStatus() const { return m_Status; }

    //! returns axial end (TRUE=forward or FALSE=backward) of this hit
    bool isForward() const { return m_IsForward; }

    //! returns sector number of this hit
    int getSector() const { return m_Sector; }

    //! returns layer number of this hit
    int getLayer() const { return m_Layer; }

    //! returns plane (0=inner or 1=outer) of this hit
    int getPlane() const { return m_Plane; }

    //! returns readout-coordinate (TRUE=phi, FALSE=z) of this hit
    bool isPhiReadout() const { return m_IsPhiReadout; }

    //! returns lowest readout strip number of this hit
    int getStripMin() const { return m_StripMin; }

    //! returns highest readout strip number of this hit
    int getStripMax() const { return m_StripMax; }

    //! global-coordinates hit position (cm)
    TVector3 m_Position;

    //! global-coordinates hit time (ns)
    double m_Time;

    //! energy deposition (MeV)
    double m_DeltaE;

    //! kinetic energy (MeV) of throughgoing particle
    double m_KE;

    //! status word
    unsigned int m_Status;

    //! axial end (TRUE=forward or FALSE=backward) of the hit
    bool m_IsForward;

    //! sector number of the hit
    int m_Sector;

    //! layer number of the hit
    int m_Layer;

    //! inner or outer plane of the hit's module
    int m_Plane;

    //! readout-coordinate (TRUE=phi, FALSE=z) of this hit
    bool m_IsPhiReadout;

    //! lowest readout strip number for this hit
    int m_StripMin;

    //! highest readout strip number for this hit
    int m_StripMax;

    //! Needed to make the ROOT object storable
    ClassDef(BKLMSimHit, 1);

  private:

  };

} // end of namespace Belle2

#endif //BKLMSIMHIT_H
