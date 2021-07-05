/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/hitXPDerivate.h>
#include <tracking/dataobjects/MCParticleInfo.h>

using namespace Belle2;

hitXPDerivate::hitXPDerivate(const SVDTrueHit& hit, const SVDCluster& cluster, const MCParticle& particle,
                             const VXD::SensorInfoBase& sensor)
{
  m_positionMid = sensor.pointToGlobal(TVector3(hit.getU(), hit.getV(), hit.getW()), false); /** position at midpoint */
  m_positionEntry = sensor.pointToGlobal(TVector3(hit.getEntryU(), hit.getEntryV(), hit.getEntryW()),
                                         false); /** position at entrypoint */
  m_positionExit = sensor.pointToGlobal(TVector3(hit.getExitU(), hit.getExitV(), hit.getExitW()), false); /** position at exitpoint */
  m_momentumMid = sensor.vectorToGlobal(hit.getMomentum(), false); /** momentum ad midpoint */
  m_momentumEntry = sensor.vectorToGlobal(hit.getEntryMomentum(), false); /** momentum at entrypoint */
  m_momentumExit = sensor.vectorToGlobal(hit.getExitMomentum(), false); /** momentum at exitpoint */
  m_positionLocalMid = TVector3(hit.getU(), hit.getV(), hit.getW()); /** local position at midpoint */
  m_positionLocalEntry = TVector3(hit.getEntryU(), hit.getEntryV(), hit.getEntryW()); /** local position at entrypoint */
  m_positionLocalExit = TVector3(hit.getExitU(), hit.getExitV(), hit.getExitW()); /** local position at exitpoint */
  m_momentumLocalMid = hit.getMomentum(); /** local momentum at midpoint */
  m_momentumLocalEntry = hit.getEntryMomentum(); /** local momentum ad entrypoint */
  m_momentumLocalExit = hit.getExitMomentum(); /** local momentum at exitpoint */
  m_PDGID = particle.getPDG(); /** PDGID */
  m_position0 = particle.getVertex(); /** position at IP */
  m_momentum0 = particle.getMomentum(); /** momentum at IP */
  m_time = hit.getGlobalTime(); /** time of the hit */
  m_sensorSensor = hit.getSensorID().getSensorNumber(); /** sensor number */
  m_sensorLayer = hit.getSensorID().getLayerNumber(); /** layer number */
  m_sensorLadder = hit.getSensorID().getLadderNumber(); /** ladder number */
  m_info_d0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getD0(); /** d0 from particle info */
  m_info_z0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getZ0(); /** z0 from particle info */
  m_info_phi0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getPhi(); /** phi0 from particle info */
  m_info_tanlambda = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getOmega(); /** omega from particle info */
  m_info_omega = tan(MCParticleInfo(particle, TVector3(0, 0, 1.5)).getLambda()); /** omega from particle info */
  m_clusterU = cluster.isUCluster(); /** flag of u-cluser */
  m_clusterV = cluster.isUCluster() ? 0 : 1; /** return 1 if it is an V cluster. flag of v cluster */
  m_reconstructed = false; /** flag of reconstructed */
  m_charge = particle.getCharge(); /** charge of particle */
}


hitXPDerivate::hitXPDerivate(const PXDTrueHit& hit, const MCParticle& particle, const VXD::SensorInfoBase& sensor)
{
  m_positionMid = sensor.pointToGlobal(TVector3(hit.getU(), hit.getV(), hit.getW()), false); /** position at midpoint */
  m_positionEntry = sensor.pointToGlobal(TVector3(hit.getEntryU(), hit.getEntryV(), hit.getEntryW()),
                                         false); /** position at entrypoint */
  m_positionExit = sensor.pointToGlobal(TVector3(hit.getExitU(), hit.getExitV(), hit.getExitW()), false); /** position at exitpoint */
  m_momentumMid = sensor.vectorToGlobal(hit.getMomentum(), false); /** momentum ad midpoint */
  m_momentumEntry = sensor.vectorToGlobal(hit.getEntryMomentum(), false); /** momentum at entrypoint */
  m_momentumExit = sensor.vectorToGlobal(hit.getExitMomentum(), false); /** momentum at exitpoint */
  m_positionLocalMid = TVector3(hit.getU(), hit.getV(), hit.getW()); /** local position at midpoint */
  m_positionLocalEntry = TVector3(hit.getEntryU(), hit.getEntryV(), hit.getEntryW()); /** local position at entrypoint */
  m_positionLocalExit = TVector3(hit.getExitU(), hit.getExitV(), hit.getExitW()); /** local position at exitpoint */
  m_momentumLocalMid = hit.getMomentum(); /** local momentum at midpoint */
  m_momentumLocalEntry = hit.getEntryMomentum(); /** local momentum ad entrypoint */
  m_momentumLocalExit = hit.getExitMomentum(); /** local momentum at exitpoint */
  m_PDGID = particle.getPDG(); /** PDGID */
  m_position0 = particle.getVertex(); /** position at IP */
  m_momentum0 = particle.getMomentum(); /** momentum at IP */
  m_time = hit.getGlobalTime(); /** time of the hit */
  m_sensorSensor = hit.getSensorID().getSensorNumber(); /** sensor number */
  m_sensorLayer = hit.getSensorID().getLayerNumber(); /** layer number */
  m_sensorLadder = hit.getSensorID().getLadderNumber(); /** ladder number */
  m_info_d0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getD0(); /** d0 from particle info DO NOT USE, use the get */
  m_info_z0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getZ0(); /** z0 from particle info DO NOT USE, use the get */
  m_info_phi0 = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getPhi(); /** phi0 from particle info DO NOT USE, use the get */
  m_info_tanlambda = MCParticleInfo(particle, TVector3(0, 0, 1.5)).getOmega(); /** omega from particle info DO NOT USE, use the get */
  m_info_omega = tan(MCParticleInfo(particle, TVector3(0, 0,
                                                       1.5)).getLambda()); /** omega from particle info DO NOT USE, use the get */
  m_reconstructed = false; /** flag of reconstructed */
  m_charge = particle.getCharge(); /** charge of the particle */
}
