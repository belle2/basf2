/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <Math/Vector3D.h>
#include <TObject.h>
#include <TString.h>

namespace Belle2 {


  //! Class BeamBackHit - Stores hits from beam backgound simulation
  /*!
   This is a class to store beam backgound hits hits in datastore.
   It is a general class for all subdetectors
   */

  class BeamBackHit : public TObject {
  public:

    //! Default constructor
    BeamBackHit(): m_subDet(0), m_identifier(0), m_PDG(0), m_trackID(0),
      m_positionX(0.), m_positionY(0.), m_positionZ(0.), m_momentumX(0.),
      m_momentumY(0.), m_momentumZ(0.), m_t(0.), m_E_start(0.),
      m_E_end(0.), m_energyDeposit(0), m_trackLength(0.), m_neutronWeight(0.)
    {}

    /*!  Full constructor
     * @param subDet the ID of the individual subdetector
     * @param iden the ID of the particular volume
     * @param PDG the PDG code of the particle
     * @param trackID the trackID of the track
     * @param position the position of the hit
     * @param momentum the momentum of particle at the time of the hit
     * @param t time at which the hit occured
     * @param E_start the energy of the particle at the entrance to the volume
     * @param E_end the energy of the particle at the exit of the volume
     * @param eDep the energy deposited in the volume
     * @param trackLength the length of the track in the volume
     * @param nWeight the effective neutron weight
     */
    BeamBackHit(int subDet, int iden, int PDG, int trackID,
                const ROOT::Math::XYZVector& position,
                const ROOT::Math::XYZVector& momentum,
                double t, double E_start, double E_end, double eDep,
                double trackLength, double nWeight):
      m_subDet(subDet), m_identifier(iden), m_PDG(PDG), m_trackID(trackID),
      m_t(t), m_E_start(E_start), m_E_end(E_end), m_energyDeposit(eDep),
      m_trackLength(trackLength), m_neutronWeight(nWeight)
    {
      setPosition(position);
      setMomentum(momentum);
    }

    //! Get the subdetector name in which the hit occured
    TString getSubDetName() const
    {
      switch (m_subDet) {
        case 0: return "IR";
        case 1: return "PDX";
        case 2: return "SVD";
        case 3: return "CDC";
        case 4: return "ARICH";
        case 5: return "TOP";
        case 6: return "ECL";
        case 7: return "EKLM";
        case 8: return "BKLM";
      }
      return "";
    }

    //! Get the identifier of subdetector component in which hit occured
    int getIdentifier() const { return m_identifier;    }

    //! Det the index of subdetector in which hit occured
    int getSubDet() const {return m_subDet; }

    //! Get the lund code of the particle that hit the sensitive area
    int getPDG() const { return m_PDG; }

    //! the traci ID of the particle
    int getTrackID() const { return m_trackID;}

    //! Get global position of the particle hit
    ROOT::Math::XYZVector getPosition() const
    {
      return ROOT::Math::XYZVector(m_positionX, m_positionY, m_positionZ);
    }

    //! Get momentum of the particle hit
    ROOT::Math::XYZVector getMomentum() const
    {
      return ROOT::Math::XYZVector(m_momentumX, m_momentumY, m_momentumZ);
    }

    //! Get the time at which the hit occured
    double getTime() const { return m_t; }

    //! Get energy of the particle
    double getEnergy() const { return m_E_start; }

    //! Get energy of the particle
    double getEnergyAtExit() const { return m_E_end; }

    //! Get particle energy deposit in sensitive volume
    double getEnergyDeposit() const { return m_energyDeposit; }

    //! the length of the track in the volume
    double getTrackLength() const { return m_trackLength; }

    //! get the effective neutron weigth
    double getNeutronWeight() const {return m_neutronWeight;}

    //! Set the subdetector group in which the hit occured
    void setSubDet(int subDet) { m_subDet = subDet; }

    //! Set the subdetector component identifier
    void setIdentifier(int iden) { m_identifier = iden; }

    //! Set the lund code of the particle that hit the sensitive area
    void setPDG(int PDG)  { m_PDG = PDG; }

    //! set the trackID of the track
    void setTrackID(int trackID) {m_trackID = trackID;}

    //! Set global position of the particle hit
    void setPosition(ROOT::Math::XYZVector position)
    {
      m_positionX = position.X();
      m_positionY = position.Y();
      m_positionZ = position.Z();
    }

    //! Set momentum of the particle hit
    void setMomentum(ROOT::Math::XYZVector momentum)
    {
      m_momentumX = momentum.X();
      m_momentumY = momentum.Y();
      m_momentumZ = momentum.Z();
    }

    //! Set the time at which the hit occured
    void setTime(double t)  {  m_t = t; }

    //! Set energy of the particle
    void setEnergy(double E)  {  m_E_start = E; }

    //! Set energy of track at exit point of volume
    void setEnergyAtExit(double E) {m_E_end = E;}

    //! Set particle energy deposit in volume
    void setEnergyDeposit(double eDep) { m_energyDeposit = eDep; }

    //! set the length of the track in the volume
    void setTrackLength(double l) {m_trackLength = l;}

    //! set the neutron weight
    void setNeutronWeight(double w) {m_neutronWeight = w;}

    /** shift in time for beambkgmixer */
    void shiftInTime(double time) { m_t += time; }

  private:

    /** The name of the subdetector. */
    int m_subDet;

    /** The identifier of subdetector component. */
    int m_identifier;

    /** The PDG code of the particle that hit the sensitive area. */
    int m_PDG;

    /** The trackID of the hit. */
    int m_trackID;

    /** Global position of the hit (X coordinate). */
    float m_positionX;

    /** Global position of the hit (Y coordinate). */
    float m_positionY;

    /** Global position of the hit (Z coordinate). */
    float m_positionZ;

    /** Momentum of the hit (X coordinate). */
    float m_momentumX;

    /** Momentum of the hit (Y coordinate). */
    float m_momentumY;

    /** Momentum of the hit (Z coordinate). */
    float m_momentumZ;

    /** Time at which the hit occured. */
    double m_t;

    /** Energy of particle at entrance into the volume. */
    double m_E_start;

    /** Energy of the particle when leaving the volume. */
    double m_E_end;

    /** Energy deposited in sensitive volume. */
    double m_energyDeposit;

    /** Length of the track in the volume. */
    double m_trackLength;

    /** The effective neutron weight. */
    double m_neutronWeight;

    /** Class version. */
    ClassDef(BeamBackHit, 2);

  };

} // end namespace Belle2
