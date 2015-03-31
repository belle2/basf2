/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle, Isabelle Ripp                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PLUMESIMHIT_H
#define PLUMESIMHIT_H

#include <simulation/dataobjects/SimHitBase.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {

  /**
    * Class PlumeSimHit - Geant4 simulated hit for the PLUME detector.
    *
    * This class holds particle hit data from geant4 simulation. As the simulated
    * hit classes are used to generate detector response, they contain _local_
    * information.
    */
  class PlumeSimHit : public SimHitBase {
  public:
    //typedef std::vector<unsigned int>::iterator iterator;
    //typedef std::vector<unsigned int>::const_iterator const_iterator;

    /** default constructor for ROOT */
    PlumeSimHit(): m_pdgID(0), m_sensorID(0), m_trackID(0), m_trackVx_x(0), m_trackVx_y(0),
      m_trackVx_z(0), m_energyDepMeV(0), m_nielDepMeV(0),
      m_posINmm_x(0), m_posINmm_y(0), m_posINmm_z(0), m_posINmm_u(0), m_posINmm_v(0), m_posINmm_w(0),
      m_posOUTmm_u(0), m_posOUTmm_v(0), m_posOUTmm_w(0),
      m_momGeV_x(0), m_momGeV_y(0), m_momGeV_z(0), m_thetaLoc(0), m_phiLoc(0), m_globalTime(0) {}


    /** Standard constructor
     See parameters definition below
     */
    PlumeSimHit(int a_pdgID, int a_sensorID, int a_trackID, float a_trackVx_x, float a_trackVx_y,
                float a_trackVx_z, float a_energyDep, float a_nielDep,
                float a_posIN_x, float a_posIN_y, float a_posIN_z, float a_posIN_u, float a_posIN_v, float a_posIN_w,
                float a_posOUT_u, float a_posOUT_v, float a_posOUT_w, float a_momentum_x, float a_momentum_y, float a_momentum_z,
                float a_thetaAngle, float a_phiAngle, float a_globalTime):
      SimHitBase(), m_pdgID(a_pdgID), m_sensorID(a_sensorID), m_trackID(a_trackID), m_trackVx_x(a_trackVx_x),
      m_trackVx_y(a_trackVx_y), m_trackVx_z(a_trackVx_z), m_energyDepMeV(a_energyDep),
      m_nielDepMeV(a_nielDep), m_posINmm_x(a_posIN_x), m_posINmm_y(a_posIN_y), m_posINmm_z(a_posIN_z),
      m_posINmm_u(a_posIN_u), m_posINmm_v(a_posIN_v), m_posINmm_w(a_posIN_w),
      m_posOUTmm_u(a_posOUT_u), m_posOUTmm_v(a_posOUT_v), m_posOUTmm_w(a_posOUT_w),
      m_momGeV_x(a_momentum_x), m_momGeV_y(a_momentum_y), m_momGeV_z(a_momentum_z),
      m_thetaLoc(a_thetaAngle), m_phiLoc(a_phiAngle), m_globalTime(a_globalTime)
    {
    }
    /** particle PDG id */
    int getpdgID()              const { return m_pdgID; }
    /** sensor ID */
    int getsensorID()           const { return m_sensorID; }
    /** track ID */
    int gettrackID()            const { return m_trackID; }
    /** track production vertex x coordinates in G4 ref */
    float gettrackVx_x()        const { return m_trackVx_x; }
    /** track production vertex y coordinates in G4 ref */
    float gettrackVx_y()        const { return m_trackVx_y; }
    /** track production vertex z coordinates in G4 ref */
    float gettrackVx_z()        const { return m_trackVx_z; }
    /** deposited energy in MeV */
    float getenergyDep()        const { return m_energyDepMeV; }
    /** non ionizing deposited energy */
    float getnielDep()          const { return m_nielDepMeV; }
    /** incoming track position x in G4 ref, in mm */
    float getposIN_x()          const { return m_posINmm_x; }
    /** incoming track position y in G4 ref, in mm */
    float getposIN_y()      const { return m_posINmm_y; }
    /** incoming track position z in G4 ref, in mm */
    float getposIN_z()      const { return m_posINmm_z; }
    /** incoming track position u in sensor ref,  in mm */
    float getposIN_u()      const { return m_posINmm_u; }
    /** incoming track position v in sensor ref,  in mm */
    float getposIN_v()          const { return m_posINmm_v; }
    /** incoming track position w in sensor ref,  in mm */
    float getposIN_w()          const { return m_posINmm_w; }
    /** outcoming track position u in sensor ref,  in mm */
    float getposOUT_u()   const { return m_posOUTmm_u; }
    /** outcoming track position v in sensor ref,  in mm */
    float getposOUT_v()   const { return m_posOUTmm_v; }
    /** outcoming track position w in sensor ref,  in mm */
    float getposOUT_w()     const { return m_posOUTmm_w; }
    /** incoming track momentum, x coordinates in G4 ref, in GeV */
    float getmomentum_x()       const { return m_momGeV_x; }
    /** incoming track momentum, y coordinates in G4 ref, in GeV */
    float getmomentum_y()       const { return m_momGeV_y; }
    /** incoming track momentum, z coordinates in G4 ref, in GeV */
    float getmomentum_z()       const { return m_momGeV_z; }
    /** local (sensor ref) theta angle, out of sensor plane, in degree */
    float getthetaAngle()       const { return m_thetaLoc; }
    /** local (sensor ref) phi angle, in sensor plane, in degree */
    float getphiAngle()         const { return m_phiLoc; }
    /** global time */
    float getglobalTime()       const { return m_globalTime; }


  private:

    /** particle PDG id */
    int m_pdgID;
    /** sensor ID */
    int m_sensorID;
    /** track ID */
    int m_trackID;
    /** track production vertex x coordinates in G4 ref */
    float m_trackVx_x;
    /** track production vertex y coordinates in G4 ref */
    float m_trackVx_y;
    /** track production vertex z coordinates in G4 ref */
    float m_trackVx_z;
    /** deposited energy in MeV */
    float m_energyDepMeV;
    /** non ionizing deposited energy */
    float m_nielDepMeV;
    /** incoming track position x in G4 ref, in mm */
    float m_posINmm_x;
    /** incoming track position y in G4 ref, in mm */
    float m_posINmm_y;
    /** incoming track position z in G4 ref, in mm */
    float m_posINmm_z;
    /** incoming track position u in sensor ref,  in mm */
    float m_posINmm_u;
    /** incoming track position v in sensor ref,  in mm */
    float m_posINmm_v;
    /** incoming track position w in sensor ref,  in mm */
    float m_posINmm_w;
    /** outcoming track position u in sensor ref,  in mm */
    float m_posOUTmm_u;
    /** outcoming track position v in sensor ref,  in mm */
    float m_posOUTmm_v;
    /** outcoming track position w in sensor ref,  in mm */
    float m_posOUTmm_w;
    /** incoming track momentum, x coordinates in G4 ref, in GeV */
    float m_momGeV_x;
    /** incoming track momentum, y coordinates in G4 ref, in GeV */
    float m_momGeV_y;
    /** incoming track momentum, z coordinates in G4 ref, in GeV */
    float m_momGeV_z;
    /** local (sensor ref) theta angle, out of sensor plane, in degree */
    float m_thetaLoc;
    /** local (sensor ref) phi angle, in sensor plane, in degree */
    float m_phiLoc;
    /** global time */
    float m_globalTime;


    ClassDef(PlumeSimHit, 1)
  };

} // end namespace Belle2

#endif
