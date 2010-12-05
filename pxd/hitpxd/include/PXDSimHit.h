/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Peter Kvasnicka                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDSIMHIT_H
#define PXDSIMHIT_H

// ROOT
#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  class PXDB4VHit;

  /**
    * ClassPXDSimHit - Geant4 simulated hit for the PXD
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class PXDSimHit : public TObject {

  public:

    /** Empty constructor, required for ROOT IO.*/
    PXDSimHit():
        m_layerID(-1),
        m_ladderID(-1),
        m_sensorID(-1),
        m_posIn(0, 0, 0),    /* Position at entry. */
        m_posOut(0, 0, 0),   /* Position at exit. */
        m_theta(0),          /* Theta angle. */
        m_momIn(0, 0, 0),    /* Momentum at entry (GRF). */
        m_PDGcode(0),        /* The PDG value of the particle that created the hit. */
        m_trackID(0),        /* The ID of the track that created the hit. */
        m_energyDep(0),      /* Deposited energy. */
        m_stepLength(0),     /* Step length. */
        m_globalTime(0) {    /* Global time. */
      /*! Does nothing */
    }

    /** Full constructor.*/
    PXDSimHit(
      int layerID,
      int ladderID,
      int sensorID,
      TVector3 posIn,
      TVector3 posOut,
      double theta,
      TVector3 momIn,
      int PDGcode,
      int trackID,
      double energyDep,
      double stepLength,
      double globalTime):
        m_layerID(layerID),
        m_ladderID(ladderID),
        m_sensorID(sensorID),
        m_posIn(posIn),             /* Position at entry. */
        m_posOut(posOut),           /* Position at exit. */
        m_theta(theta),             /* Theta angle.*/
        m_momIn(momIn),             /* Momentum at entry. */
        m_PDGcode(PDGcode),         /* The PDG value of the particle that created the hit. */
        m_trackID(trackID),         /* The ID of the track that created the hit. */
        m_energyDep(energyDep),     /* Deposited energy. */
        m_stepLength(stepLength),   /* Step length. */
        m_globalTime(globalTime) {  /* Global time. */
      /* Does nothing. */
    }

    //! Destructor
    ~PXDSimHit() {
      /* Does nothing. */
    }

    //! Constructor from a G4 simhit
    PXDSimHit(const PXDB4VHit & g4hit);


    /** The method to set LayerID.*/
    void setLayerID(int layerID) { m_layerID = layerID; }

    /** The method to set LadderID.*/
    void setLadderID(int ladderID) { m_ladderID = ladderID; }

    /** The method to set SensorID.*/
    void setSensorID(int sensorID) { m_sensorID = sensorID; }

    /** The method to set entry position.*/
    void setPosIn(double x, double y, double z) { m_posIn.SetXYZ(x, y, z); }

    /** The method to set exit position.*/
    void setPosOut(double x, double y, double z) { m_posOut.SetXYZ(x, y, z); }

    /** The method to set theta.*/
    void setTheta(double aTheta) { m_theta = aTheta; }

    /** The method to set entry momentum.*/
    void setMomIn(double px, double py, double pz) { m_momIn.SetXYZ(px, py, pz); }

    /** The method to set track id.*/
    void setTrackID(int trackID) { m_trackID = trackID; }

    /** The method to set PDG code.*/
    void setPDGcode(int pdg) { m_PDGcode = pdg; }

    /** The method to set deposited energy.*/
    void setEnergyDep(double energyDep) { m_energyDep = energyDep; }

    /** The method to set step length.*/
    void setStepLength(double stepLength) { m_stepLength = stepLength; }

    /** The method to set GlobalTime.*/
    void setGlobalTime(double globalTime) { m_globalTime = globalTime; }


    /** The method to get layer id.*/
    int getLayerID() const { return m_layerID; }

    /** The method to get ladder id.*/
    int getLadderID() const { return m_ladderID; }

    /** The method to get sensor id.*/
    int getSensorID() const { return m_sensorID; }

    /** The method to get entry position.*/
    const TVector3& getPosIn() const { return m_posIn; }

    /** The method to get exit position.*/
    const TVector3& getPosOut() const { return m_posOut; }

    /** The method to get theta.*/
    double getTheta() const { return m_theta; }

    /** The method to get entry momentum.*/
    const TVector3& getMomIn() const { return m_momIn; }

    /** The method to get track id.*/
    int getTrackID() const { return m_trackID; }

    /** The method to get PDG code.*/
    int getPDGcode() const { return m_PDGcode; }

    /** The method to get deposited energy.*/
    double getEnergyDep() const { return m_energyDep; }

    /** The method to get step length.*/
    double getStepLength() const { return m_stepLength; }

    /** The method to get GlobalTime.*/
    double getGlobalTime() const { return m_globalTime; }

    /** Assingment operator.*/
    PXDSimHit& operator=(const PXDSimHit& other);

  private:

    int m_layerID;           /**< Layer number. */
    int m_ladderID;          /**< Ladder number. */
    int m_sensorID;          /**< Sensor number. */

    TVector3 m_posIn;      /**< LRF position at entry. */
    TVector3 m_posOut;     /**< LRF position at exit. */
    double m_theta;          /**< Theta angle (wrt global z). */
    TVector3 m_momIn;      /**< GRF Momentum at entry. */

    int m_PDGcode;         /**< The PDG value of the particle that created the hit. */
    int m_trackID;         /**< The ID of the track that created the hit. */
    double m_energyDep;    /**< Deposited energy. */
    double m_stepLength;   /**< Step length. */
    double m_globalTime;   /**< Global time. */

    ClassDef(PXDSimHit, 1)

  };

} // end namespace Belle2

#endif
