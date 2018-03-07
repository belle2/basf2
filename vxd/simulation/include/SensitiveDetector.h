/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - 2014 Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka             *
 *               Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef VXD_SIMULATION_SENSITIVEDETECTOR_H
#define VXD_SIMULATION_SENSITIVEDETECTOR_H

#include <vxd/simulation/SensitiveDetectorBase.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <array>

#ifdef VXD_SENSITIVEDETECTOR_DEBUG
#include <vxd/simulation/SensitiveDetectorDebugHelper.h>
#endif

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {

    /** Sensitive Detector implementation of PXD and SVD.
     * This class provides the actual implementation of the hit generation for
     * PXD and SVD. It is templated to be able to create the corresponding
     * output collection.
     *
     * It generates two different kinds of Hits:
     * - SimHits which correspond to Geant4 steps inside the sensitive volume.
     *   There will be several SimHits per traversal, since the step length of
     *   Geant4 is limited to provide detailed information about energy
     *   deposition.
     * - TrueHits are aggregated objects which store the position where a
     *   particle crossed the detector plane (local z=0). There can be only one
     *   TrueHit per traversal of one sensor, but there may be more than one
     *   TrueHit for curling Tracks. TrueHits also carry information about the
     *   particle momenta before and after entering the silicon (or at start/end
     *   point if the track started/ended inside the silicon.
     *   MODIFIED July 2013 (based largely on suggestions by Martin Ritter):
     *   1. Every particle that either enters or leaves the sensitive volume
     *   and deposits some energy there, will create a TrueHit.
     *   2. The (u,v,w) position of the TrueHit are the coordinates of the
     *   midpoint of the track inside the sensitive volume (rather of the
     *   x-ing point with z=0): the w (z) coordinate is added to TrueHits.
     *   3. Each MC particle entering the sensitive volume will be un-ignored,
     *   so that the TrueHits created by secondaries entering the sensitive
     *   volume will be attributed to the correct MCParticle (that is, not to
     *   its primary ancestor).
     *   4. SimHit relations to MCParticles that don't produce a TrueHit and get
     *   re-attributed (to their primary ancestor) will have negative sign. This
     *   concerns the secondaries created inside the sensitive volume that don't
     *   reach another sensitive volume.
     *
     * @tparam SimHitClass Class to use when generating SimHits
     * @tparam TrueHitClass Class to use when generating TrueHits
     */
    template <class SimHitClass, class TrueHitClass> class SensitiveDetector: public SensitiveDetectorBase {
    public:
      /** Construct instance and take over ownership of the sensorInfo pointer */
      explicit SensitiveDetector(VXD::SensorInfoBase* sensorInfo);

    private:
      /** Save the actual TrueHit for a given sensor traversal information.
       * @param traversal information on the particle traversal to be used when
       * creating the TrueHit
       * @return the index of the created TrueHit in its StoreArray
       */
      int saveTrueHit(const SensorTraversal& traversal) override;

      /** Save a SimHit for a track along the given points.
       * @param traversal information on the particle traversal to be used when
       * creating the SimHit
       * @param points pair of iterators to the first and last step position to
       * be used for the SimHit
       * @return the index of the created SimHit in its StoreArray
       */
      int saveSimHit(const SensorTraversal& traversal,
                     const SensorTraversal::range& points) override;

      /** Save the relations between MCParticle, TrueHit and SimHits.
       * @param traversal information on the particle traversal to be used when
       * creating the Relations
       * @param trueHitIndex index of the TrueHit, <0 if no TrueHit was created
       * @param simHitIndices indices of the SimHits along with the number of
       * electrons deposited in each SimHit
       */
      void saveRelations(const SensorTraversal& traversal, int trueHitIndex,
                         std::vector<std::pair<unsigned int, float>> simHitIndices) override;

      /** Convert G4ThreeVector (aka Hep3Vector) to float array to store in
       * TrueHit/SimHit classes.
       * @param vec vector to convert
       * @return array containing x,y,z as floats
       */
      std::array<float, 3> vecToFloat(const G4ThreeVector& vec)
      {
        return std::array<float, 3> {{(float)vec.x(), (float)vec.y(), (float)vec.z()}};
      }

      /** StoreArray for the SimHits */
      StoreArray<SimHitClass> m_simhits;
      /** StoreArray for the TrueHits */
      StoreArray<TrueHitClass> m_truehits;
      /** StoreArray for the MCParticles, needed by relations */
      StoreArray<MCParticle> m_mcparticles;
      /** Relation between MCParticle and SimHits */
      RelationArray m_relMCSimHits {m_mcparticles, m_simhits};
      /** Relation between MCParticle and TrueHits */
      RelationArray m_relMCTrueHits {m_mcparticles, m_truehits};
      /** Relation between TrueHits and SimHits */
      RelationArray m_relTrueSimHits {m_truehits, m_simhits};
    };

    template <class SimHitClass, class TrueHitClass>
    SensitiveDetector<SimHitClass, TrueHitClass>::SensitiveDetector(VXD::SensorInfoBase* sensorInfo):
      VXD::SensitiveDetectorBase(sensorInfo)
    {
      //Register output collections.
      //m_mcparticles.isRequired();
      m_simhits.registerInDataStore();
      m_truehits.registerInDataStore();
      m_relMCSimHits.registerInDataStore();
      m_relMCTrueHits.registerInDataStore();
      m_relTrueSimHits.registerInDataStore();
      registerMCParticleRelation(m_relMCSimHits, RelationArray::c_negativeWeight);
      registerMCParticleRelation(m_relMCTrueHits, RelationArray::c_deleteElement);
    }

    template <class SimHitClass, class TrueHitClass>
    int SensitiveDetector<SimHitClass, TrueHitClass>::saveTrueHit(const SensorTraversal& traversal)
    {
      //We have the full sensor traversal information, we only lack the midpoint so lets get it
      StepInformation midPoint = findMidPoint(traversal);

      //By now everything is done: just convert the position and momenta in float[3] and create a truehit
      auto posEntry = vecToFloat(traversal.front().position);
      auto posMidPoint = vecToFloat(midPoint.position);
      auto posExit = vecToFloat(traversal.back().position);
      auto momEntry = vecToFloat(traversal.front().momentum);
      auto momMidPoint = vecToFloat(midPoint.momentum);
      auto momExit = vecToFloat(traversal.back().momentum);
      //And we should convert the electrons back in energy ...
      const double energyDep = traversal.getElectrons() * Const::ehEnergy;

      //create the truehit
      int trueHitIndex = m_truehits.getEntries();
      m_truehits.appendNew(getSensorID(), posEntry.data(), posMidPoint.data(), posExit.data(),
                           momEntry.data(), momMidPoint.data(), momExit.data(), energyDep, midPoint.time);
#ifdef VXD_SENSITIVEDETECTOR_DEBUG
      SensitiveDetectorDebugHelper::getInstance().addTrueHit(m_truehits[trueHitIndex]);
#endif
      return trueHitIndex;
    }

    template <class SimHitClass, class TrueHitClass>
    int SensitiveDetector<SimHitClass, TrueHitClass>::saveSimHit(const SensorTraversal& traversal,
        const SensorTraversal::range& points)
    {
      //Convert position to floats here
      auto posIn = vecToFloat(points.first->position);
      auto posOut = vecToFloat(points.second->position);
      auto electronProfile = simplifyEnergyDeposit(points);

      //Create the simhit
      int simHitIndex = m_simhits.getEntries();
      SimHitClass* simhit = m_simhits.appendNew(getSensorID(), traversal.getPDGCode(), points.first->time,
                                                posIn.data(), posOut.data());
      simhit->setEnergyDeposit(electronProfile);
#ifdef VXD_SENSITIVEDETECTOR_DEBUG
      int startPoint = std::distance(traversal.begin(), (SensorTraversal::const_iterator)points.first);
      int endPoint = std::distance(traversal.begin(), (SensorTraversal::const_iterator)points.second);
      SensitiveDetectorDebugHelper::getInstance().addSimHit(simhit, startPoint, endPoint);
#endif
      return simHitIndex;
    }

    template <class SimHitClass, class TrueHitClass>
    void SensitiveDetector<SimHitClass, TrueHitClass>::saveRelations(const SensorTraversal& traversal,
        int trueHitIndex, std::vector<std::pair<unsigned int, float>> simHitIndices)
    {
      m_relMCSimHits.add(traversal.getTrackID(), simHitIndices.begin(), simHitIndices.end());
      //If there is no truehit there are obviously no relations ;)
      if (trueHitIndex >= 0) {
        m_relMCTrueHits.add(traversal.getTrackID(), trueHitIndex, traversal.getElectrons());
        m_relTrueSimHits.add(trueHitIndex, simHitIndices.begin(), simHitIndices.end());
      }
    }

  } //VXD Namespace
} //Belle2 namespace

#endif
