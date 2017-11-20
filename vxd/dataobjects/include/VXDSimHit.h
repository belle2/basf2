/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef VXD_DATAOBJECTS_VXDSIMHIT_H
#define VXD_DATAOBJECTS_VXDSIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>
#include <vxd/dataobjects/VxdID.h>
#include <TVector3.h>
#include <vector>
#include <algorithm>

namespace Belle2 {
  /** Class VXDSimHit - Geant4 simulated hit for the VXD.
   *
   * This class holds particle hit data from geant4 simulation. As the simulated
   * hit classes are used to generate detector response, they contain _local_
   * information. It is a common base class for both, PXD and SVD
   */
  class VXDSimHit : public SimHitBase {
  public:
    /** default constructor for ROOT */
    VXDSimHit(): m_pdg(0), m_globalTime(0), m_posIn(), m_posOut(), m_sensorID(0) {}

    /** Standard constructor
     * @param sensorID ID of the sensor
     * @param pdg PDG Code of the particle producing the Hit
     * @param globalTime Time of electron deposition
     * @param posIn Start point of electron deposition in local coordinates
     * @param posOut End point of electron deposition in local coordinates
     */
    VXDSimHit(VxdID sensorID, int pdg, float globalTime, const float* posIn, const float* posOut):
      SimHitBase(), m_pdg(pdg), m_globalTime(globalTime), m_sensorID(sensorID)
    {
      std::copy_n(posIn, 3, m_posIn);
      std::copy_n(posOut, 3, m_posOut);
    }

    /** Set energy deposition profile
     * To avoid copying of the entries this will take over the contents of the
     * argument and after the call the argument will be empty.
     *
     * TODO: Normally this would be done with an lvalue reference but we need a
     * dictionary so we have to compile it with cint, might change for ROOT6.
     *
     * @param electronProfile list of energy depositions along the simhit,
     * encoded using ElectronDeposit class. Will be empty after call
     */
    void setEnergyDeposit(std::vector<unsigned int>& electronProfile)
    {
      m_electronProfile.clear();
      std::swap(m_electronProfile, electronProfile);
    }

    /** Return the sensorID of the sensor the electron was deposited in */
    VxdID getSensorID() const { return m_sensorID; }
    /** Return the PDG code of the particle causing the electron deposition */
    int getPDGcode() const { return m_pdg; }
    /** Return the start point of the electron deposition in local coordinates */
    TVector3 getPosIn() const { return TVector3(m_posIn[0], m_posIn[1], m_posIn[2]); }
    /** Return the end point of the electron deposition in local coordinates */
    TVector3 getPosOut() const { return TVector3(m_posOut[0], m_posOut[1], m_posOut[2]); }
    /** Return the start point of the electron deposition in local coordinates as float array */
    const float* getPosArrayIn() const { return m_posIn; }
    /** Return the end point of the electron deposition in local coordinates as float array */
    const float* getPosArrayOut() const { return m_posOut; }
    /** Return the time of the electron deposition */
    float getGlobalTime() const { return m_globalTime; }
    /** Shift the SimHit in time
     * @param delta The value of the time shift.
     */
    virtual void shiftInTime(float delta) { m_globalTime += delta; }

    /** Return the number of created electrons */
    float getElectrons() const;

    /** Get the decoded electron profile.
     * Each entry is the fraction between posIn and posOut and the number of
     * electrons deposited up to that point. It will contain at least one entry
     * with a fraction of one and the total number of deposited electrons.
     *
     * @return vector of (fraction, electron) pairs
     */
    std::vector<std::pair<float, float> > getElectronProfile() const;

    /** Get the electron deposition along constant stepsize.
     * The electron deposition will be sampled between posIn and posOut in
     * equal steps with a specified length. The acutal sampling length might
     * differ slightly from the given parameter but will be equal and below the
     * given value for all steps. The returned vector will contain fractions
     * between posIn and posOut and the electrons deposited up to that point.It
     * will contain at least one entry with a fraction of one and the total
     * number of deposited electrons.
     *
     * @param length target step size in default length unit (cm)
     * @return vector of (fraction, electron) pairs
     */
    std::vector<std::pair<float, float> > getElectronsConstantDistance(double length) const;

    /** Get the electron deposition with constant number of electrons between
     * sampling points. The number of electrons per step might differ from the
     * given parameter but will be equal and at most the given value for all
     * steps. The returned vector will contain fractions between posIn and
     * posOut and the electrons deposited up to that point. It will contain at
     * least one entry with a fraction of one and the total number of deposited
     * electrons.
     *
     * @param length target number of electrons per step
     * @return vector of (fraction, electron) pairs
     */
    std::vector<std::pair<float, float> > getElectronsConstantNumber(double electronsPerStep) const;

  private:
    /** Energy depsoition profile encoded using the ElectronDeposit class */
    std::vector<unsigned int> m_electronProfile;
    /** PDG Code of the particle producing the Hit */
    int m_pdg;
    /** Time of electron deposition */
    float m_globalTime;
    /** Start point of electron deposition in local coordinates */
    float m_posIn[3];
    /** End point of electron deposition in local coordinates */
    float m_posOut[3];
    /** ID of the sensor the electron was deposited in */
    unsigned short m_sensorID;

    ClassDef(VXDSimHit, 1)
  };
} // end namespace Belle2

#endif
