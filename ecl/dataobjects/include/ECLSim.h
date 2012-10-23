/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSIM_H
#define ECLSIM_H

#include <TObject.h>
#include <TVector3.h>

#include <generators/dataobjects/SimHitBase.h>
// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {

  /**
    * ClassSVDSimHit - Geant4 simulated hit for the ECL.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class ECLSim : public SimHitBase {


////namespace Belle2 {

//  //! Example Detector
////  class ECLSim : public TObject {
  public:

    //! The cell id of this hit.
    int m_cellId;

    //! The track id of this hit.
    int m_trackId;

    //! Particle PDG (can be one of secondaries).
    int m_pdg;

    //!  Flight time from IP.
    double m_flightTime;

    //! Deposited energy of this hit.
    double m_edep;

    //! The momentum of pre-step.
    TVector3 m_momentum;

    //! Position of pre-step.
    TVector3 m_posIn;

    //! The method to set cell id
    void setCellId(int cellId) { m_cellId = cellId; }

    //! The method to set track id
    void setTrackId(int trackId) { m_trackId = trackId; }

    //! The method to set PDG code
    void setPDGCode(int pdg) { m_pdg = pdg; }


    //! The method to set flight time
    void setFlightTime(double flightTime) { m_flightTime = flightTime; }

    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set momentum
    void setMomentum(TVector3 momentum) { m_momentum = momentum; }


    //! The method to set position of pre-step
    void setPosIn(TVector3 posIn) { m_posIn = posIn; }


    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get track id
    int getTrackId() const { return m_trackId; }

    //! The method to get PDG code
    int getPDGCode() const { return m_pdg; }


    //! The method to get flight time
    double getFlightTime() const { return m_flightTime; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get momentum
    TVector3 getMomentum() const { return m_momentum; }

    //! The method to get position
    TVector3 getPosIn() const { return m_posIn; }

    //! Empty constructor
    /*! Recommended for ROOT IO
    */
    ECLSim() {;}

    ClassDef(ECLSim, 1);/**< the class title */

  };

} // end namespace Belle2

#endif
