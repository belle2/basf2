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
    PlumeSimHit(): pdgID(0), sensorID(0), trackID(0), energyDep(0), nielDep(0), posIN_x(0), posIN_y(0),
      posIN_z(0), posIN_u(0), posIN_v(0), posIN_w(0), posOUT_u(0), posOUT_v(0), posOUT_w(0),
      momentum_x(0), momentum_y(0), momentum_z(0), thetaAngle(0), phiAngle(0), globalTime(0) {}


    /** Standard constructor
     See parameters definition below
     */
    PlumeSimHit(int a_pdgID, int a_sensorID, int a_trackID, float a_energyDep, float a_nielDep,
                float a_posIN_x, float a_posIN_y, float a_posIN_z, float a_posIN_u, float a_posIN_v, float a_posIN_w,
                float a_posOUT_u, float a_posOUT_v, float a_posOUT_w, float a_momentum_x, float a_momentum_y, float a_momentum_z,
                float a_thetaAngle, float a_phiAngle, float a_globalTime): SimHitBase(), pdgID(a_pdgID), sensorID(a_sensorID),
      trackID(a_trackID), energyDep(a_energyDep), nielDep(a_nielDep), posIN_x(a_posIN_x), posIN_y(a_posIN_y), posIN_z(a_posIN_z),
      posIN_u(a_posIN_u), posIN_v(a_posIN_v), posIN_w(a_posIN_w),
      posOUT_u(a_posOUT_u), posOUT_v(a_posOUT_v), posOUT_w(a_posOUT_w),
      momentum_x(a_momentum_x), momentum_y(a_momentum_y), momentum_z(a_momentum_z),
      thetaAngle(a_thetaAngle), phiAngle(a_phiAngle), globalTime(a_globalTime)
    {
    }
    /** particle PDG id */
    int getpdgID()              const { return pdgID; }
    /** sensor ID */
    int getsensorID()           const { return sensorID; }
    /** track ID */
    int gettrackID()            const { return trackID; }
    /** deposited energy in electrons */
    float getenergyDep()        const { return energyDep; }
    /** non ionizing deposited energy */
    float getnielDep()          const { return nielDep; }
    /** incoming track position x ref G4*/
    float getposIN_x()          const { return posIN_x; }
    /** incoming track position y ref G4*/
    float getposIN_y()      const { return posIN_y; }
    /** incoming track position z ref G4*/
    float getposIN_z()      const { return posIN_z; }
    /** incoming track position u ref mimosa*/
    float getposIN_u()      const { return posIN_u; }
    /** incoming track position v ref mimosa*/
    float getposIN_v()          const { return posIN_v; }
    /** incoming track position w ref mimosa*/
    float getposIN_w()          const { return posIN_w; }
    /** outgoing track position u ref mimosa*/
    float getposOUT_u()   const { return posOUT_u; }
    /** outgoing track position v ref mimosa*/
    float getposOUT_v()   const { return posOUT_v; }
    /** outgoing track position w ref mimosa*/
    float getposOUT_w()     const { return posOUT_w; }
    /** incoming track momentum x*/
    float getmomentum_x()       const { return momentum_x; }
    /** incoming track momentum y*/
    float getmomentum_y()       const { return momentum_y; }
    /** incoming track momentum z*/
    float getmomentum_z()       const { return momentum_z; }
    /** local theta angle (out of plane) */
    float getthetaAngle()       const { return thetaAngle; }
    /** local phi angle (in plane) */
    float getphiAngle()         const { return phiAngle; }
    /** global time */
    float getglobalTime()       const { return globalTime; }


  private:

    /** particle PDG id */
    int pdgID;
    /** sensor ID */
    int sensorID;
    /** track ID */
    int trackID;
    /** deposited energy in electrons */
    float energyDep;
    /** non ionizing deposited energy */
    float nielDep;
    /** incoming track position x ref G4*/
    float posIN_x;
    /** incoming track position y ref G4*/
    float posIN_y;
    /** incoming track position z ref G4*/
    float posIN_z;
    /** incoming track position u ref mimosa*/
    float posIN_u;
    /** incoming track position v ref mimosa*/
    float posIN_v;
    /** incoming track position w ref mimosa*/
    float posIN_w;
    /** outgoing track position u ref mimosa */
    float posOUT_u;
    /** outgoing track position v ref mimosa */
    float posOUT_v;
    /** outgoing track position w ref mimosa */
    float posOUT_w;
    /** incoming track momentum x*/
    float momentum_x;
    /** incoming track momentum y*/
    float momentum_y;
    /** incoming track momentum z*/
    float momentum_z;
    /** local theta angle (out of plane) */
    float thetaAngle;
    /** local phi angle (in plane) */
    float phiAngle;
    /** global time */
    float globalTime;


    ClassDef(PlumeSimHit, 1)
  };

} // end namespace Belle2

#endif
