/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticle.h>
#include <TObject.h>



namespace Belle2 {

  /**  This class collects some information of a TrueHit, using
  *  SVDCLuster and MCParticle information too.
  *  These informations are: position in local and global coordinates
  *  at entry/middle/exit position of the sensor, momentum, analytical
  *  track parameters (evaluated from posion and momentum), geometrical
  *  infomations (layed,ladder,sensor) and time.
  *
  *  The purpose of this class is to obtain a single "hit-like object"
  *  with all the useful information for a track-parameters study.
  *  It is used in evaluate the NoKickCuts.
  *
  *  To build a hitXP object is necessary to use the constructor
  *  of the derivate class hitXPDerivate. */

  class hitXP: public TObject {

  public:

    TVector3 m_positionMid; /**< poition of hit at midpoint of layer */
    TVector3 m_positionEntry; /**< poition of hit at entrypoint of layer */
    TVector3 m_positionExit; /**< poition of hit at exitpoint of layer */
    TVector3 m_momentumMid; /**< momentum of hit at midpoint of layer */
    TVector3 m_momentumEntry; /**< momentum of hit at entry of layer */
    TVector3 m_momentumExit; /**< momentum of hit at exit of layer */
    TVector3 m_positionLocalMid; /**< poition of hit at midpoint of layer, local coordinate */
    TVector3 m_positionLocalEntry; /**< poition of hit at entrypoint of layer, local coordinate */
    TVector3 m_positionLocalExit; /**< poition of hit at exitpoint of layer, local coordinate */
    TVector3 m_momentumLocalMid; /**< momentum of hit at midpoint of layer, local coordinate */
    TVector3 m_momentumLocalEntry; /**< momentum of hit at entrypoint of layer, local coordinate */
    TVector3 m_momentumLocalExit; /**< momentum of hit at exitpoint of layer, local coordinate */
    int m_PDGID = 0; /**< PDGID */
    TVector3 m_position0; /**< position at IP */
    TVector3 m_momentum0; /**< momentum at IP */
    double m_time = 0; /**< time of the hit */
    int m_sensorSensor = 0; /**< sensor of the hit */
    int m_sensorLayer = 0; /**< layer of the hit */
    int m_sensorLadder = 0; /**< ladder of the hit */
    double m_info_d0 = 0; /**< d0 from MCParticle info evaluation. DO NOT USE, use the get */
    double m_info_z0 = 0; /**< z0 from MCParticle info evaluation. DO NOT USE, use the get */
    double m_info_phi0 = 0; /**< phi0 from MCParticle info evaluation. DO NOT USE, use the get */
    double m_info_tanlambda = 0; /**< tanlambda from MCParticle info evaluation. DO NOT USE, use the get */
    double m_info_omega = 0; /**< omega from MCParticle info evaluation. DO NOT USE, use the get */
    int m_clusterU = 0; /**< flag for u cluster */
    int m_clusterV = 0; /**< flag for v cluster */
    bool m_reconstructed = 0; /**< flag for reconstructed track */
    double m_charge = 0; /**< charge of the track */

    /** empty constructor of the class */
    hitXP() {}

    /** get the relative memeber */
    TVector3 getPositionMid() const { return m_positionMid; }

    /** get the relative memeber */
    TVector3 getPositionEntry() const { return m_positionEntry; }

    /** get the relative memeber */
    TVector3 getPositionExit() const { return m_positionExit; }

    /** get the relative memeber */
    TVector3 getMomentumMid() const { return m_momentumMid; }

    /** get the relative memeber */
    TVector3 getMomentumEntry() const { return m_momentumEntry; }

    /** get the relative memeber */
    TVector3 getMomentumExit() const { return m_momentumExit; }

    /** get the relative memeber */
    TVector3 getPositionLocalMid() const { return m_positionLocalMid; }

    /** get the relative memeber */
    TVector3 getPositionLocalEntry() const { return m_positionLocalEntry; }

    /** get the relative memeber */
    TVector3 getPositionLocalExit() const { return m_positionLocalExit; }

    /** get the relative memeber */
    TVector3 getMomentumLocalMid() const { return m_momentumLocalMid; }

    /** get the relative memeber */
    TVector3 getMomentumLocalEntry() const { return m_momentumLocalEntry; }

    /** get the relative memeber */
    TVector3 getMomentumLocalExit() const { return m_momentumLocalExit; }

    /** get the relative memeber */
    int getPDGID() const { return m_PDGID; }

    /** get the relative memeber */
    TVector3 getPosition0() const { return m_position0; }

    /** get the relative memeber */
    TVector3 getMomentum0() const { return m_momentum0; }

    /** get the relative memeber */
    double getTime() const { return m_time; }

    /** get the relative memeber */
    int getSensorSensor() const { return m_sensorSensor; }

    /** get the relative memeber */
    int getSensorLadder() const { return m_sensorLadder; }

    /** get the relative memeber */
    int getSensorLayer() const { return m_sensorLayer; }

    /** get the relative memeber */
    int getClusterU() const { return m_clusterU; }

    /** get the relative memeber */
    int getClusterV() const { return m_clusterV; }

    /** get the relative memeber */
    bool isReconstructed() const { return m_reconstructed; }

    /** get the relative memeber */
    double getCharge() const { return m_charge; }

    /** get the relative memeber */
    void setPositionMid(TVector3 position)
    {
      m_positionMid.SetX(position.X());
      m_positionMid.SetY(position.Y());
      m_positionMid.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setPositionEntry(TVector3 position)
    {
      m_positionEntry.SetX(position.X());
      m_positionEntry.SetY(position.Y());
      m_positionEntry.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setPositionExit(TVector3 position)
    {
      m_positionExit.SetX(position.X());
      m_positionExit.SetY(position.Y());
      m_positionExit.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setMomentumMid(TVector3 momentum)
    {
      m_momentumMid.SetX(momentum.X());
      m_momentumMid.SetY(momentum.Y());
      m_momentumMid.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setMomentumEntry(TVector3 momentum)
    {
      m_momentumEntry.SetX(momentum.X());
      m_momentumEntry.SetY(momentum.Y());
      m_momentumEntry.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setMomentumExit(TVector3 momentum)
    {
      m_momentumExit.SetX(momentum.X());
      m_momentumExit.SetY(momentum.Y());
      m_momentumExit.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setPositionLocalMid(TVector3 position)
    {
      m_positionLocalMid.SetX(position.X());
      m_positionLocalMid.SetY(position.Y());
      m_positionLocalMid.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setPositionLocalEntry(TVector3 position)
    {
      m_positionLocalEntry.SetX(position.X());
      m_positionLocalEntry.SetY(position.Y());
      m_positionLocalEntry.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setPositionLocalExit(TVector3 position)
    {
      m_positionLocalExit.SetX(position.X());
      m_positionLocalExit.SetY(position.Y());
      m_positionLocalExit.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setMomentumLocalMid(TVector3 momentum)
    {
      m_momentumLocalMid.SetX(momentum.X());
      m_momentumLocalMid.SetY(momentum.Y());
      m_momentumLocalMid.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setMomentumLocalEntry(TVector3 momentum)
    {
      m_momentumLocalEntry.SetX(momentum.X());
      m_momentumLocalEntry.SetY(momentum.Y());
      m_momentumLocalEntry.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setMomentumLocalExit(TVector3 momentum)
    {
      m_momentumLocalExit.SetX(momentum.X());
      m_momentumLocalExit.SetY(momentum.Y());
      m_momentumLocalExit.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setPDGID(int pdgid) { m_PDGID = pdgid; }

    /** get the relative memeber */
    void setPosition0(TVector3 position)
    {
      m_position0.SetX(position.X());
      m_position0.SetY(position.Y());
      m_position0.SetZ(position.Z());
    }

    /** get the relative memeber */
    void setMomentum0(TVector3 momentum)
    {
      m_momentum0.SetX(momentum.X());
      m_momentum0.SetY(momentum.Y());
      m_momentum0.SetZ(momentum.Z());
    }

    /** get the relative memeber */
    void setTime(double Time) { m_time = Time; }

    /** get the relative memeber */
    void setSensorSensor(int sensor) { m_sensorSensor = sensor; }

    /** get the relative memeber */
    void setSensorLayer(int layer) { m_sensorLayer = layer; }

    /** get the relative memeber */
    void setSensorLadder(int ladder) { m_sensorLadder = ladder; }

    /** get the relative memeber */
    void setClusterU(int cluster)
    {
      m_clusterU = cluster;
    }

    /** get the relative memeber */
    void setClusterV(int cluster)
    {
      m_clusterV = cluster;
    }

    /** get the relative memeber */
    void setReconstructed(bool isReconstructed) { m_reconstructed = isReconstructed; }

    /** get the relative memeber */
    void setCharge(double charge) { m_charge = charge; }

    /** evaluates the parameter omega (1/curvature radius) of a helicoidal track
    * starting from position and momentum of the hit
    * input(hit postion, hit momentum, hit charge)
    * output(omega value)
    */
    double omega(TVector3 xx, TVector3 p, double charge) const;


    /** evaluates the parameter tanLambda (pz/pt) of a helicoidal track
    * starting from position and momentum of the hit
    * input(hit postion, hit momentum)
    * output(tanlambda value)
    */
    double tanLambda(TVector3 xx, TVector3 p) const;


    /** evaluates the parameter d0 (impact parameter) of a helicoidal track
    * starting from position and momentum of the hit
    * input(hit postion, hit momentum, hit charge)
    * output(d0 value)
    */
    double d0(TVector3 xx, TVector3 p, double charge) const;


    /** evaluates the parameter phi0 (angle between pT and x) of a helicoidal track
    * starting from position and momentum of the hit
    * input(hit postion, hit momentum, hit charge)
    * output(phi0 value)
    */
    double phi0(const TVector3& xx, TVector3 p, double charge) const;


    /** evaluates the parameter z0 (distance transverse plane-POCA) of a helicoidal track
    * starting from position and momentum of the hit
    * input(hit postion, hit momentum, hit charge)
    * output(z0 value)
    */
    double z0(TVector3 xx, TVector3 p, double charge) const;


    /** evaluate relative parameter using midpoint position and momentum */
    double getOmegaMid() const { return omega(m_positionMid, m_momentumMid, m_charge); }

    /** evaluate relative parameter using entrypoint position and momentum */
    double getOmegaEntry() const { return omega(m_positionEntry, m_momentumEntry, m_charge); }

    /** evaluate relative parameter using exitpoint position and momentum */
    double getOmegaExit() const { return omega(m_positionExit, m_momentumExit, m_charge); }

    /** evaluate relative parameter using IP position and momentum */
    double getOmega0() const { return omega(m_position0, m_momentum0, m_charge); }

    /** evaluate relative parameter using midpoint position and momentum */
    double getTanLambdaMid() const { return tanLambda(m_positionMid, m_momentumMid); }

    /** evaluate relative parameter using entrypoint position and momentum */
    double getTanLambdaEntry() const { return tanLambda(m_positionEntry, m_momentumEntry); }

    /** evaluate relative parameter using exitpoint position and momentum */
    double getTanLambdaExit() const { return tanLambda(m_positionExit, m_momentumExit); }

    /** evaluate relative parameter using IP position and momentum */
    double getTanLambda0() const { return tanLambda(m_position0, m_momentum0); }

    /** evaluate relative parameter using midpoint position and momentum */
    double getD0Mid() const { return d0(m_positionMid, m_momentumMid, m_charge); }

    /** evaluate relative parameter using entrypoint position and momentum */
    double getD0Entry() const { return d0(m_positionEntry, m_momentumEntry, m_charge); }

    /** evaluate relative parameter using exitpoint position and momentum */
    double getD0Exit() const { return d0(m_positionExit, m_momentumExit, m_charge); }

    /** evaluate relative parameter using IP position and momentum */
    double getD00() const { return d0(m_position0, m_momentum0, m_charge); }

    /** evaluate relative parameter using midpoint position and momentum */
    double getPhi0Mid() const { return phi0(m_positionMid, m_momentumMid, m_charge); }

    /** evaluate relative parameter using entrypoint position and momentum */
    double getPhi0Entry() const { return phi0(m_positionEntry, m_momentumEntry, m_charge); }

    /** evaluate relative parameter using exitpoint position and momentum */
    double getPhi0Exit() const { return phi0(m_positionExit, m_momentumExit, m_charge); }

    /** evaluate relative parameter using IP position and momentum */
    double getPhi00() const { return phi0(m_position0, m_momentum0, m_charge); }

    /** evaluate relative parameter using midpoint position and momentum */
    double getZ0Mid() const { return z0(m_positionMid, m_momentumMid, m_charge); }

    /** evaluate relative parameter using entrypoint position and momentum */
    double getZ0Entry() const { return z0(m_positionEntry, m_momentumEntry, m_charge); }

    /** evaluate relative parameter using exitpoint position and momentum */
    double getZ0Exit() const { return z0(m_positionExit, m_momentumExit, m_charge); }

    /** evaluate relative parameter using IP position and momentum */
    double getZ00() const { return z0(m_position0, m_momentum0, m_charge); }


    /** This structure allows to compare times of 2 hitXP point
    * input (first hit, second hit)
    * output (boolean: true if time of the hit1< time of hit2)
    */
    struct timeCompare {
      bool operator()(hitXP v, hitXP u) const
      {return v.m_time < u.m_time;}
    };

    //! needed by root
    ClassDef(hitXP, 1);
  };
} /** end namespace Belle2 */
