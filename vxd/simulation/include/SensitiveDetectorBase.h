/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef VXD_SENSITIVEDETECTORBASE_H
#define VXD_SENSITIVEDETECTORBASE_H

//If this is defined, we create a root file with all information necessary to
//check functionality of the implementation
//#define VXD_SENSITIVEDETECTOR_DEBUG

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <vxd/simulation/SensorTraversal.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <stack>
#include <vector>

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD */
  namespace VXD {
    /** Base class for Sensitive Detector implementation of PXD and SVD.
     *
     * This base class provides common access to sensor information (like
     * transformation) and performs the the collection of energy depositing
     * when particles cross the sensor. Most of the calculations are contained
     * here while the detector dependent hit and relation creation is delegated
     * to the derived VXD::SensitiveDetector<SimHitClass, TrueHitClass>.
     */
    class SensitiveDetectorBase: public Simulation::SensitiveDetectorBase {
    public:
      /** Constructor
       * @param info pointer to a valid SensorInfo instance providing
       * information about the sensor this sensitive detector instance will
       * handle. Ownership of the SensorInfo goes to the sensitive detector
       * instance
       */
      explicit SensitiveDetectorBase(SensorInfoBase* info):
        Simulation::SensitiveDetectorBase((info->getType() == SensorInfoBase::PXD ? "PXD " : "SVD ") + (std::string)info->getID(),
                                          info->getType() == SensorInfoBase::PXD ? Const::PXD : Const::SVD), m_info(info) {};

      /** Set all common options.
       * @param seeNeutrons if true, simhits are also stored for neutrons
       * @param onlyPrimaryTrueHits if true, truehits will only be created for
       * primary particles
       * @param distanceTolerance maximal distance of step position from linear
       * approximation.
       * @param electronTolerance maximum deviation of energy deposition from
       * linear approximation in electrons
       * @param minimumElectrons minimum number of electrons to be deposited
       * before SimHits are created.
       */
      void setOptions(bool seeNeutrons, bool onlyPrimaryTrueHits, float distanceTolerance,
                      float electronTolerance, float minimumElectrons)
      {
        m_seeNeutrons = seeNeutrons;
        m_onlyPrimaryTrueHits = onlyPrimaryTrueHits;
        m_distanceTolerance = distanceTolerance;
        m_electronTolerance = electronTolerance;
        m_minimumElectrons = minimumElectrons;
      }

      /** Destructor freeing the sensor Info */
      virtual ~SensitiveDetectorBase() override
      {
        if (m_info) delete m_info;
      }

      /** Return a pointer to the SensorInfo associated with this instance */
      SensorInfoBase* getSensorInfo() { return m_info; }

      /** Return the VxdID belonging to this sensitive detector */
      VxdID getSensorID() const { return m_info->getID(); }

    protected:
      /** Save the actual TrueHit for this sensor traversal.
       * This function depends on the actual TrueHit implementation and is
       * implemented in VXD::SensitiveDetector.
       * @param traversal information on the particle traversal to be used when
       * creating the TrueHit
       * @return the index of the created TrueHit in its StoreArray
       */
      virtual int saveTrueHit(const SensorTraversal& traversal) = 0;

      /** Save a SimHit for this track including the given points.
       * This function depends on the actual SimHit implementation and is
       * implemented in VXD::SensitiveDetector.
       * @param traversal information on the particle traversal to be used when
       * creating the SimHit
       * @param points pair of iterators to the first and last step position to
       * be used for the SimHit
       * @return the index of the created SimHit in its StoreArray
       */
      virtual int saveSimHit(const SensorTraversal& traversal, const SensorTraversal::range& points) = 0;

      /** Save the relations between MCParticle, TrueHit and SimHits.
       * @param traversal information on the particle traversal to be used when
       * creating the Relations
       * @param trueHitIndex index of the TrueHit, <0 if no TrueHit was created
       * @param simHitIndices indices of the SimHits along with the number of
       * electrons deposited in each SimHit
       */
      virtual void saveRelations(const SensorTraversal& traversal, int trueHitIndex,
                                 std::vector<std::pair<unsigned int, float>> simHitIndices) = 0;

      /** Simplify the energy deposition profile using Douglas-Peuker-Algorithm
       * We normally force a Geant4 step after 5µm but energy deposition does
       * not necessarily vary much between these short steps. Saving all steps
       * would be a waste of space so we define a tolerance (in electrons) and
       * keep only the points needed so that no point is further away from the
       * profile than this tolerance.
       *
       * @param points pair of iterators to the first and last point of the
       * energy deposition profile
       * @return list of encoded EnergyDeposit values representing the
       * simplified energy deposition profile.
       */
      std::vector<unsigned int> simplifyEnergyDeposit(const SensorTraversal::range& points);

      /** Find the mid-point of the track traversal.
       * This function will return the position and momentum at the center of
       * the track traversal, using cubic spline interpolation between the
       * actual geant4 steps. Center is defined as "half the flight length" in
       * this case.
       *
       * @param traversal information on the particle traversal to be used when
       * finding the midpoint
       * @return position and momentum at the mid point
       */
      StepInformation findMidPoint(const SensorTraversal& traversal);
    private:
      /** Process a single Geant4 Step.
       * This function stores the necessary information to create the TrueHits
       * and SimHits and will call finishTrack() if a track leaves the volume
       * or is stopped inside the volume.
       */
      bool step(G4Step* step, G4TouchableHistory*) override;

      /** Process a track once all steps are known.
       * This function decides whether TrueHit/SimHits will be saved
       */
      bool finishTrack();

      /** Determine which SimHits to create.
       * A SimHit is a linear approximation of the particle trajectory. As such
       * we try to combine as many Geant4 steps as possible by defining a
       * distance tolerance and using the Douglas-Peucker algortihm to
       * determine the required number of SimHits to keep the maximum distance
       * of all Geant4 steps below that tolerance.
       *
       * @return indices and electrons of all created simhits
       */
      std::vector<std::pair<unsigned int, float>> createSimHits();

      /** Pointer to the SensorInfo associated with this instance */
      SensorInfoBase* m_info {0};
      /** stack of SensorTraversal information for all tracks not finished so far */
      std::stack<SensorTraversal> m_tracks;
      /** maximum distance between step point and linear interpolation of sensor
       * traversal before a new simhit object is created */
      float m_distanceTolerance {0};
      /** maximum relative difference between electron density of two steps where
       * they can be considered similar enough to be merged */
      float m_electronTolerance {0};
      /** minimum number of electrons a track must deposit for SimHit/TrueHits to
       * be created */
      float m_minimumElectrons {0};
      /** also create SimHit/TrueHit objects for neutrons (or charged particles
       * which deposit less than m_minimumElectrons electrons */
      bool m_seeNeutrons {false};
      /** only create TrueHits for primary particles */
      bool m_onlyPrimaryTrueHits {false};
    };
  }
} //Belle2 namespace
#endif
