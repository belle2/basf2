/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDSIMHIT_H
#define PXDSIMHIT_H

// ROOT
#include <TObject.h>
#include <TVector3.h>

#define DEFAULT_PXDSIMHITS "PXDSimHits"
#define DEFAULT_PXDSIMHITSREL "PXDSimHitsToMCParticles"

namespace Belle2 {

  /**
    * ClassPXDSimHit - Geant4 simulated hit for the PXD.
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
        m_momIn(0, 0, 0),    /* Momentum at entry. */
        m_PDGcode(0),        /* The PDG value of the particle that created the hit. */
        m_PDGmass(0.0),    /* The PDG mass of the particle. */
        m_PDGcharge(0.0),  /* The PDG charge of the particle. */
        m_trackID(0),        /* The ID of the track that created the hit. */
        m_energyDep(0),      /* Deposited energy. */
        m_stepLength(0),     /* Step length. */
        m_globalTime(0) {    /* Global time. */
      /*! Does nothing */
    }

    /** Full constructor.
     * @param layerID ID of the layer in which the hit was created.
     * @param ladderID ID of the ladder in which the hit was created.
     * @param sensorID ID of the sensor in which the hit was created.
     * @param posIn Point of entry into the detector, in local coordinates.
     * @param posOut Point of exit from the detector, in local coordinates.
     * @param theta Angle theta (wrt. the global z axis).
     * @param momIn The momentum of particle on entry into the detector, in local coordinates.
     * @param PDGcode The PDG code of particle that produced this track.
     * @param PDGmass The PDG mass of the particle that produced the track.
     * @param PDGcharge The PDG charge of the particle that produced the track.
     * @param trackID ID of the track.
     * @param energyDep Energy deposition by the particle in the detector.
     * @param stepLength Length of G4 step.
     * @param globalTime Global time.
     */
    PXDSimHit(
      int layerID,
      int ladderID,
      int sensorID,
      TVector3 posIn,
      TVector3 posOut,
      float theta,
      TVector3 momIn,
      int PDGcode,
      double PDGmass,
      double PDGcharge,
      int trackID,
      float energyDep,
      float stepLength,
      float globalTime):
        m_layerID(layerID),
        m_ladderID(ladderID),
        m_sensorID(sensorID),
        m_posIn(posIn),             /* Position at entry. */
        m_posOut(posOut),           /* Position at exit. */
        m_theta(theta),             /* Theta angle.*/
        m_momIn(momIn),             /* Momentum at entry. */
        m_PDGcode(PDGcode),         /* The PDG value of the particle that created the hit. */
        m_PDGmass(PDGmass),     /* The PDG mass of the particle that produced the hit. */
        m_PDGcharge(PDGcharge),   /* The PDG charge of the particle that produced the hit. */
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

    /** The method to set LayerID.*/
    void setLayerID(int layerID) { m_layerID = layerID; }

    /** The method to set LadderID.*/
    void setLadderID(int ladderID) { m_ladderID = ladderID; }

    /** The method to set SensorID.*/
    void setSensorID(int sensorID) { m_sensorID = sensorID; }

    /** The method to set entry position.*/
    void setPosIn(float x, float y, float z) { m_posIn.SetXYZ(x, y, z); }

    /** The method to set exit position.*/
    void setPosOut(float x, float y, float z) { m_posOut.SetXYZ(x, y, z); }

    /** The method to set theta.*/
    void setTheta(float aTheta) { m_theta = aTheta; }

    /** The method to set entry momentum.*/
    void setMomIn(float px, float py, float pz) { m_momIn.SetXYZ(px, py, pz); }

    /** The method to set track id.*/
    void setTrackID(int trackID) { m_trackID = trackID; }

    /** The method to set PDG code.*/
    void setPDGcode(int pdg) { m_PDGcode = pdg; }

    /** The method to set PDG mass.*/
    void setPDGmass(double pdgmass) { m_PDGmass = pdgmass; }

    /** The method to set PDG charge.*/
    void setPDGcharge(double pdgcharge) { m_PDGcharge = pdgcharge; }

    /** The method to set deposited energy.*/
    void setEnergyDep(float energyDep) { m_energyDep = energyDep; }

    /** The method to set step length.*/
    void setStepLength(float stepLength) { m_stepLength = stepLength; }

    /** The method to set GlobalTime.*/
    void setGlobalTime(float globalTime) { m_globalTime = globalTime; }

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
    float getTheta() const { return m_theta; }

    /** The method to get entry momentum.*/
    const TVector3& getMomIn() const { return m_momIn; }

    /** The method to get track id.*/
    int getTrackID() const { return m_trackID; }

    /** The method to get PDG code.*/
    int getPDGcode() const { return m_PDGcode; }

    /** The method to get PDG mass.*/
    double getPDGmass() const { return m_PDGmass; }

    /** The method to get PDG charge.*/
    double getPDGcharge() const { return m_PDGcharge; }

    /** The method to get deposited energy.*/
    float getEnergyDep() const { return m_energyDep; }

    /** The method to get step length.*/
    float getStepLength() const { return m_stepLength; }

    /** The method to get GlobalTime.*/
    float getGlobalTime() const { return m_globalTime; }

    /** Assingment operator.*/
    PXDSimHit& operator=(const PXDSimHit& other);

  private:

    int m_layerID;           /**< Layer number. */
    int m_ladderID;          /**< Ladder number. */
    int m_sensorID;          /**< Sensor number. */

    TVector3 m_posIn;      /**< LRF position at entry. */
    TVector3 m_posOut;     /**< LRF position at exit. */
    float m_theta;          /**< Theta angle (wrt global z). */
    TVector3 m_momIn;      /**< LRF Momentum at entry. */

    int m_PDGcode;         /**< The PDG value of the particle that created the hit. */
    double m_PDGmass;    /**< The PDG mass of the particle that produced the hit. */
    double m_PDGcharge;    /**< The PDG charge of the particle that produced the hit. */
    int m_trackID;         /**< The ID of the track that created the hit. */
    float m_energyDep;    /**< Deposited energy. */
    float m_stepLength;   /**< Step length. */
    float m_globalTime;   /**< Global time. */

    ClassDef(PXDSimHit, 1)

  };

} // end namespace Belle2

#endif
