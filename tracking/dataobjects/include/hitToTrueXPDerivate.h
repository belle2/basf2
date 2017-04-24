#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <TFile.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/dataobjects/MCParticleInfo.h>
#include <tracking/dataobjects/hitToTrueXP.h>



namespace Belle2 {

  class hitToTrueXPDerivate: public hitToTrueXP {

    //  This class is the derivate of HitToTrueXP, and complete it with a constructor that use
    //  all other complex types (classes) of basf2. It is necessary to buld a hitToTrueXP object.


  public:
    hitToTrueXPDerivate() {}

    hitToTrueXPDerivate(const SVDTrueHit& hit, const SVDCluster cluster, const MCParticle& particle, const VXD::SensorInfoBase& sensor)
    {
      m_positionMid = sensor.pointToGlobal(TVector3(hit.getU(), hit.getV(), hit.getW()), false);
      m_positionEntry = sensor.pointToGlobal(TVector3(hit.getEntryU(), hit.getEntryV(), hit.getEntryW()), false);
      m_positionExit = sensor.pointToGlobal(TVector3(hit.getExitU(), hit.getExitV(), hit.getExitW()), false);
      m_momentumMid = sensor.vectorToGlobal(hit.getMomentum(), false);
      m_momentumEntry = sensor.vectorToGlobal(hit.getEntryMomentum(), false);
      m_momentumExit = sensor.vectorToGlobal(hit.getExitMomentum(), false);
      m_positionLocalMid = TVector3(hit.getU(), hit.getV(), hit.getW());
      m_positionLocalEntry = TVector3(hit.getEntryU(), hit.getEntryV(), hit.getEntryW());
      m_positionLocalExit = TVector3(hit.getExitU(), hit.getExitV(), hit.getExitW());
      m_momentumLocalMid = hit.getMomentum();
      m_momentumLocalEntry = hit.getEntryMomentum();
      m_momentumLocalExit = hit.getExitMomentum();
      m_PDGID = particle.getPDG();
      m_position0 = particle.getVertex();
      m_momentum0 = particle.getMomentum();
      m_time = hit.getGlobalTime();
      m_sensorSensor = hit.getSensorID().getSensorNumber();
      m_sensorLayer = hit.getSensorID().getLayerNumber();
      m_sensorLadder = hit.getSensorID().getLadderNumber();
      m_info_d0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getD0();
      m_info_z0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getZ0();
      m_info_phi0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getPhi();
      m_info_tanlambda = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getOmega();
      m_info_omega = tan(MCParticleInfo(particle, TVector3(0, 0, 1.5)).getLambda());
      m_clusterU = cluster.isUCluster();
      m_clusterV = cluster.isUCluster() ? 0 : 1; //return 1 if it is an V cluster
      m_reconstructed = false;
      m_charge = particle.getCharge();
    }

    ClassDef(hitToTrueXPDerivate, 1);
  };
} //end namespace Belle2
