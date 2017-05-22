#include <mdst/dataobjects/MCParticle.h>
#include <TObject.h>

#pragma once


namespace Belle2 {

  class hitToTrueXP: public TObject {

    //  This class collects some information of a TrueHit, using
    //  SVDCLuster and MCParticle information too.
    //  These informations are: position in local and global coordinates
    //  at entry/middle/exit position of the sensor, momentum, analytical
    //  track parameters (evaluated from posion and momentum), geometrical
    //  infomations (layed,ladder,sensor) and time.
    //
    //  The purpose of this class is to obtain a single "hit-like object"
    //  with all the useful information for a track-parameters study.
    //  It is used in evaluate the NoKickCuts.
    //
    //  To build a hitToTrueXP object is necessary to use the constructor
    //  of the derivate class hitToTrueXPDerivate.

  public:

    TVector3 m_positionMid;
    TVector3 m_positionEntry;
    TVector3 m_positionExit;
    TVector3 m_momentumMid;
    TVector3 m_momentumEntry;
    TVector3 m_momentumExit;
    TVector3 m_positionLocalMid;
    TVector3 m_positionLocalEntry;
    TVector3 m_positionLocalExit;
    TVector3 m_momentumLocalMid;
    TVector3 m_momentumLocalEntry;
    TVector3 m_momentumLocalExit;
    int m_PDGID;
    TVector3 m_position0;
    TVector3 m_momentum0;
    double m_time;
    int m_sensorSensor;
    int m_sensorLayer;
    int m_sensorLadder;
    double m_info_d0;
    double m_info_z0;
    double m_info_phi0;
    double m_info_tanlambda;
    double m_info_omega;
    int m_clusterU;
    int m_clusterV;

    bool m_reconstructed;
    double m_charge;





    hitToTrueXP() {}

    TVector3 getPositionMid() const { return m_positionMid; }

    TVector3 getPositionEntry() const { return m_positionEntry; }

    TVector3 getPositionExit() const { return m_positionExit; }

    TVector3 getMomentumMid() const { return m_momentumMid; }

    TVector3 getMomentumEntry() const { return m_momentumEntry; }

    TVector3 getMomentumExit() const { return m_momentumExit; }

    TVector3 getPositionLocalMid() const { return m_positionLocalMid; }

    TVector3 getPositionLocalEntry() const { return m_positionLocalEntry; }

    TVector3 getPositionLocalExit() const { return m_positionLocalExit; }

    TVector3 getMomentumLocalMid() const { return m_momentumLocalMid; }

    TVector3 getMomentumLocalEntry() const { return m_momentumLocalEntry; }

    TVector3 getMomentumLocalExit() const { return m_momentumLocalExit; }

    int getPDGID() const { return m_PDGID; }

    TVector3 getPosition0() const { return m_position0; }

    TVector3 getMomentum0() const { return m_momentum0; }

    double getTime() const { return m_time; }

    int getSensorSensor() const { return m_sensorSensor; }

    int getSensorLadder() const { return m_sensorLadder; }

    int getSensorLayer() const { return m_sensorLayer; }

    int getClusterU() const { return m_clusterU; }

    int getClusterV() const { return m_clusterV; }

    bool isReconstructed() const { return m_reconstructed; }

    double getCharge() const { return m_charge; }

    void setPositionMid(TVector3 position)
    {
      m_positionMid.SetX(position.X());
      m_positionMid.SetY(position.Y());
      m_positionMid.SetZ(position.Z());
    }

    void setPositionEntry(TVector3 position)
    {
      m_positionEntry.SetX(position.X());
      m_positionEntry.SetY(position.Y());
      m_positionEntry.SetZ(position.Z());
    }

    void setPositionExit(TVector3 position)
    {
      m_positionExit.SetX(position.X());
      m_positionExit.SetY(position.Y());
      m_positionExit.SetZ(position.Z());
    }

    void setMomentumMid(TVector3 momentum)
    {
      m_momentumMid.SetX(momentum.X());
      m_momentumMid.SetY(momentum.Y());
      m_momentumMid.SetZ(momentum.Z());
    }

    void setMomentumEntry(TVector3 momentum)
    {
      m_momentumEntry.SetX(momentum.X());
      m_momentumEntry.SetY(momentum.Y());
      m_momentumEntry.SetZ(momentum.Z());
    }

    void setMomentumExit(TVector3 momentum)
    {
      m_momentumExit.SetX(momentum.X());
      m_momentumExit.SetY(momentum.Y());
      m_momentumExit.SetZ(momentum.Z());
    }

    void setPositionLocalMid(TVector3 position)
    {
      m_positionLocalMid.SetX(position.X());
      m_positionLocalMid.SetY(position.Y());
      m_positionLocalMid.SetZ(position.Z());
    }

    void setPositionLocalEntry(TVector3 position)
    {
      m_positionLocalEntry.SetX(position.X());
      m_positionLocalEntry.SetY(position.Y());
      m_positionLocalEntry.SetZ(position.Z());
    }
    void setPositionLocalExit(TVector3 position)
    {
      m_positionLocalExit.SetX(position.X());
      m_positionLocalExit.SetY(position.Y());
      m_positionLocalExit.SetZ(position.Z());
    }

    void setMomentumLocalMid(TVector3 momentum)
    {
      m_momentumLocalMid.SetX(momentum.X());
      m_momentumLocalMid.SetY(momentum.Y());
      m_momentumLocalMid.SetZ(momentum.Z());
    }

    void setMomentumLocalEntry(TVector3 momentum)
    {
      m_momentumLocalEntry.SetX(momentum.X());
      m_momentumLocalEntry.SetY(momentum.Y());
      m_momentumLocalEntry.SetZ(momentum.Z());
    }

    void setMomentumLocalExit(TVector3 momentum)
    {
      m_momentumLocalExit.SetX(momentum.X());
      m_momentumLocalExit.SetY(momentum.Y());
      m_momentumLocalExit.SetZ(momentum.Z());
    }

    void setPDGID(int pdgid) { m_PDGID = pdgid; }

    void setPosition0(TVector3 position)
    {
      m_position0.SetX(position.X());
      m_position0.SetY(position.Y());
      m_position0.SetZ(position.Z());
    }

    void setMomentum0(TVector3 momentum)
    {
      m_momentum0.SetX(momentum.X());
      m_momentum0.SetY(momentum.Y());
      m_momentum0.SetZ(momentum.Z());
    }

    void setTime(double Time) { m_time = Time; }

    void setSensorSensor(int sensor) { m_sensorSensor = sensor; }

    void setSensorLayer(int layer) { m_sensorLayer = layer; }

    void setSensorLadder(int ladder) { m_sensorLadder = ladder; }

    void setClusterU(int cluster)
    {
      m_clusterU = cluster;
    }

    void setClusterV(int cluster)
    {
      m_clusterV = cluster;
    }

    void setReconstructed(bool isReconstructed) { m_reconstructed = isReconstructed; }

    void setCharge(double charge) { m_charge = charge; }

    //evaluates the parameter omega (1/curvature radius) of a helicoidal track
    //starting from position and momentum of the hit
    double omega(TVector3 xx, TVector3 p, double charge) const
    {
      TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
      double Bz = 1.5;
      double Bze = Bz * charge * 0.299792458;
      double aux = Bze / sqrt(p.X() * p.X() + p.Y() * p.Y());
      aux = aux / 100;
      return aux;
    }

    //evaluates the parameter tanLambda (pz/pt) of a helicoidal track
    //starting from position and momentum of the hit
    double tanLambda(TVector3 xx, TVector3 p) const
    {
      TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
      double aux = p.Z() / sqrt(p.X() * p.X() + p.Y() * p.Y());
      return aux;
    }

    //evaluates the parameter d0 (impact parameter) of a helicoidal track
    //starting from position and momentum of the hit
    double d0(TVector3 xx, TVector3 p, double charge) const
    {
      TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
      double Bz = 1.5;
      double Bze = Bz * charge * 0.299792458;
      double aux = sqrt((p.Y() / (Bze) + x.X()) * (p.Y() / (Bze) + x.X()) + (x.Y() - p.X() / (Bze)) * (x.Y() - p.X() / (Bze))) - sqrt((
                     p.X() * p.X() + p.Y() * p.Y()) / (Bze * Bze));
      if (Bze < 0) {
        aux = -aux;
      }
      aux = 100 * aux;
      return aux;
    }
    //evaluates the parameter phi0 (angle between pT and x) of a helicoidal track
    //starting from position and momentum of the hit
    double phi0(const TVector3& xx, TVector3 p, double charge) const
    {
      TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
      double Bz = 1.5;
      double Bze = Bz * charge * 0.299792458;
      double aux;
      double chi;
      if (Bze > 0) {
        chi = atan2((-p.X() * x.X() - p.Y() * x.Y()), ((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
      } else {
        chi = atan2(-(-p.X() * x.X() - p.Y() * x.Y()), -((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
      }
      aux = atan2(p.Y(), p.X()) - chi;
      return aux;
    }

    //evaluates the parameter z0 (distance transverse plane-POCA) of a helicoidal track
    //starting from position and momentum of the hit
    double z0(TVector3 xx, TVector3 p, double charge) const
    {
      TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
      double Bz = 1.5;
      double Bze = Bz * charge * 0.299792458;
      double aux;
      double chi;
      if (Bze > 0) {
        chi = atan2((-p.X() * x.X() - p.Y() * x.Y()), ((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
      } else {
        chi = atan2(-(-p.X() * x.X() - p.Y() * x.Y()), -((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
      }
      aux = p.Z() * chi / (Bze) + x.Z();
      aux = 100 * aux;
      return aux;
    }

    double getOmegaMid() const { return omega(m_positionMid, m_momentumMid, m_charge); }

    double getOmegaEntry() const { return omega(m_positionEntry, m_momentumEntry, m_charge); }

    double getOmegaExit() const { return omega(m_positionExit, m_momentumExit, m_charge); }

    double getOmega0() const { return omega(m_position0, m_momentum0, m_charge); }

    double getTanLambdaMid() const { return tanLambda(m_positionMid, m_momentumMid); }

    double getTanLambdaEntry() const { return tanLambda(m_positionEntry, m_momentumEntry); }

    double getTanLambdaExit() const { return tanLambda(m_positionExit, m_momentumExit); }

    double getTanLambda0() const { return tanLambda(m_position0, m_momentum0); }

    double getD0Mid() const { return d0(m_positionMid, m_momentumMid, m_charge); }

    double getD0Entry() const { return d0(m_positionEntry, m_momentumEntry, m_charge); }

    double getD0Exit() const { return d0(m_positionExit, m_momentumExit, m_charge); }

    double getD00() const { return d0(m_position0, m_momentum0, m_charge); }

    double getPhi0Mid() const { return phi0(m_positionMid, m_momentumMid, m_charge); }

    double getPhi0Entry() const { return phi0(m_positionEntry, m_momentumEntry, m_charge); }

    double getPhi0Exit() const { return phi0(m_positionExit, m_momentumExit, m_charge); }

    double getPhi00() const { return phi0(m_position0, m_momentum0, m_charge); }

    double getZ0Mid() const { return z0(m_positionMid, m_momentumMid, m_charge); }

    double getZ0Entry() const { return z0(m_positionEntry, m_momentumEntry, m_charge); }

    double getZ0Exit() const { return z0(m_positionExit, m_momentumExit, m_charge); }

    double getZ00() const { return z0(m_position0, m_momentum0, m_charge); }


    //This structure allows to compare times of 2 hitToTrueXP point
    struct timeCompare {
      bool operator()(hitToTrueXP v, hitToTrueXP u) const
      {return v.m_time < u.m_time;}
    };

    ClassDef(hitToTrueXP, 1);
  };
} //end namespace Belle2
