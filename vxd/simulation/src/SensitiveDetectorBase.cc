/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/kernel/UserInfo.h>
#include <vxd/simulation/SensitiveDetectorBase.h>
#include <vxd/dataobjects/VXDElectronDeposit.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>

#ifdef VXD_SENSITIVEDETECTOR_DEBUG
#include <vxd/simulation/SensitiveDetectorDebugHelper.h>
#endif

#include <G4Step.hh>

namespace Belle2 {
  namespace VXD {

    bool SensitiveDetectorBase::step(G4Step* step, G4TouchableHistory*)
    {
      // Get track
      const G4Track& track = *step->GetTrack();
      // Get particle PDG code
      const int pdgCode = track.GetDefinition()->GetPDGEncoding();
      // Get particle charge (only keep charged tracks, photons and magnetic monopoles)
      const bool isNeutral = track.GetDefinition()->GetPDGCharge() == 0;
      const bool isAllowedNeutral = (pdgCode == Const::photon.getPDGCode()) || (m_seeNeutrons
                                    && (abs(pdgCode) == Const::neutron.getPDGCode())) || (abs(pdgCode) == 99666);
      //Not interested in neutral particles except photons, magnetic monopoles and maybe neutrons
      if (isNeutral && !isAllowedNeutral) return false;

      // Get track ID
      const int trackID = track.GetTrackID();
      //Get deposited energy
      const double electrons = step->GetTotalEnergyDeposit() * Unit::MeV / Const::ehEnergy;
      // Get step information
      const G4StepPoint& postStep = *step->GetPostStepPoint();
      const G4StepPoint& preStep = *step->GetPreStepPoint();
      const G4AffineTransform& topTransform = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform();
      const G4ThreeVector postStepPos = topTransform.TransformPoint(postStep.GetPosition()) * Unit::mm;
      const G4ThreeVector postStepMom = topTransform.TransformAxis(postStep.GetMomentum()) * Unit::MeV;

      //Get the current track info
      //if none present or not matching current trackID create one first
      //if trackID is 0 we can reuse the existing top
      if (m_tracks.empty() || (m_tracks.top().getTrackID() > 0 && m_tracks.top().getTrackID() != trackID)) {
        m_tracks.push(SensorTraversal());
      }
      SensorTraversal& traversal = m_tracks.top();
      if (traversal.getTrackID() == 0) {
        bool isPrimary = Simulation::TrackInfo::getInfo(track).hasStatus(MCParticle::c_PrimaryParticle);
        //If new track, remember values
        traversal.setInitial(trackID, pdgCode, isPrimary);
        //Remember if the track came from the outside
        if (preStep.GetStepStatus() == fGeomBoundary) traversal.hasEntered();
        //Add start position
        const G4ThreeVector preStepPos = topTransform.TransformPoint(preStep.GetPosition()) * Unit::mm;
        const G4ThreeVector preStepMom = topTransform.TransformAxis(preStep.GetMomentum()) * Unit::MeV;
        traversal.add(preStepPos, preStepMom, 0, preStep.GetGlobalTime() * Unit::ns, 0);
      }
      //Add new track
      traversal.add(postStepPos, postStepMom, electrons,
                    postStep.GetGlobalTime() * Unit::ns,
                    step->GetStepLength() * Unit::mm);
      //check if we are leaving the volume
      bool isLeaving = (postStep.GetStepStatus() == fGeomBoundary);
      //remember that in the track info
      if (isLeaving) traversal.hasLeft();
      //If this step is to the boundary or track gets killed, save hits and
      //return whether or not we saved something
      if (isLeaving || track.GetTrackStatus() >= fStopAndKill) {
        bool saved = finishTrack();
        //If we saved at least one simhit and the track is not contained inside
        //the sensor volume: keep MCParticle
        if (saved && !traversal.isContained()) {
          Simulation::TrackInfo::getInfo(track).setIgnore(false);
        }
        return saved;
      }
      //Track not finished so we do not save anything, let's return false for
      //now
      return false;
    }

    bool SensitiveDetectorBase::finishTrack()
    {
      SensorTraversal& traversal = m_tracks.top();
#ifdef VXD_SENSITIVEDETECTOR_DEBUG
      SensitiveDetectorDebugHelper& debug = SensitiveDetectorDebugHelper::getInstance();
      debug.startTraversal(getSensorID(), traversal);
#endif
      bool save = traversal.getElectrons() >= m_minimumElectrons || m_seeNeutrons;
      if (save) {
        int trueHitIndex = -1;
        if (!m_onlyPrimaryTrueHits || traversal.isPrimary()) {
          trueHitIndex = saveTrueHit(traversal);
        }
        std::vector<std::pair<unsigned int, float>> simhits = createSimHits();
        saveRelations(traversal, trueHitIndex, simhits);
      }
#ifdef VXD_SENSITIVEDETECTOR_DEBUG
      debug.finishTraversal();
#endif
      //No we just need to take care of the stack of traversals
      if (m_tracks.size() == 1) {
        //reuse traversal to keep memory if this is the first one
        traversal.reset();
      } else {
        //this should only happen if the parent track got suspended. As this
        //rarely happens in PXD we do not care for re-usability here
        m_tracks.pop();
      }
      return save;
    }

    std::vector<std::pair<unsigned int, float>> SensitiveDetectorBase::createSimHits()
    {
      SensorTraversal& traversal = m_tracks.top();
      //List of created simhit indices to be able to create relations
      std::vector<std::pair<unsigned int, float>> simhits;

      //We need to check how close the steps would be to a linear approximation
      //of the traversal and split the track if they are too far away. To do
      //this we check the whole track and split it at the point with the
      //largest distance recursively until the largest distance is inside the
      //tolerance. For that we need a stack of segments and check each segment
      //in turn until they fulfill the criteria.
      static std::stack<SensorTraversal::range> stack;
      //Lets push the full segment, first step point to last step point, on the
      //stack. We use inclusive range, so the second iterator is still included
      stack.push(make_pair(traversal.begin(), traversal.end() - 1));
      //Iterators needed for checking
      SensorTraversal::iterator firstPoint, finalPoint, splitPoint;

      //Check all segments ...
      while (!stack.empty()) {
        //Get first and last point
        std::tie(firstPoint, finalPoint) = stack.top();
        //Remove segment from stack
        stack.pop();
        //Direction of the segment
        const G4ThreeVector n = (finalPoint->position - firstPoint->position).unit();
        //find largest distance to segment by looping over all intermediate points
        double maxDistance(0);
        for (auto nextPoint = firstPoint + 1; nextPoint != finalPoint; ++nextPoint) {
          //Calculate distances between point p and line segment,
          //x = a + t*n, where a is a point on the line and n is the unit vector
          //pointing in line direction. distance = ||(p-a) - ((p-a)*n)*n||
          const G4ThreeVector pa = nextPoint->position - firstPoint->position;
          const double dist = (pa - (pa * n) * n).mag();
          //Update splitpoint if distance is larger then previously known
          if (dist > maxDistance) {
            splitPoint = nextPoint;
            maxDistance = dist;
          }
        }
        //If we are above the tolerance we split the track
        if (maxDistance > m_distanceTolerance) {
          //If we split in this order, all created simhits will be in correct
          //order. That is not a requirement but a nice side effect.
          stack.push(make_pair(splitPoint, finalPoint));
          stack.push(make_pair(firstPoint, splitPoint));
          continue;
        }
        //Otherwise we create a SimHit
        //FIXME: check for m_minimumElectrons?
        int simHitIndex = saveSimHit(traversal, std::make_pair(firstPoint, finalPoint));
        simhits.push_back(std::make_pair(simHitIndex, finalPoint->electrons - firstPoint->electrons));
      }
      return simhits;
    }

    std::vector<unsigned int> SensitiveDetectorBase::simplifyEnergyDeposit(const SensorTraversal::range& points)
    {
      //At the end we want a list of points indication how many electrons where
      //deposited so far along the track, e.g. [(0.5,100), (1.0, 2000)] would
      //indicate that after half the step length 100 electrons were deposited
      //and at the end of the step we have 2000 electrons. To save some memory
      //we encode this information as unsigned int using ElectronDeposit later
      //on.
      std::vector<unsigned int> electronProfile;

      //We need an iterator to the first and last point in a segment
      SensorTraversal::iterator firstPoint, finalPoint;
      //for now let's extract the full segment
      std::tie(firstPoint, finalPoint) = points;
      //If this is not the first SimHit for a sensor traversal we need to
      //subtract the electrons already taken care of
      const double electronsOffset = (firstPoint->electrons);
      //We also need the length of the step
      const double length = finalPoint->length - firstPoint->length;
      //And the start length
      const double lengthOffset = firstPoint->length;

      //We need to keep track of sub segments and where they should insert a
      //midpoint if required. So we need a tuple of three iterators: insert
      //position, first point and last point of the segment. We store those
      //in a stack which we declare static to avoid some relocations if
      //possible.
      static std::stack <SensorTraversal::range> stack;
      //And we push the full segment on the stack
      stack.push(points);

      //Now we check all segments we encounter until none exceed the maximum
      //tolerance anymore
      while (!stack.empty()) {
        //Get next segment to be checked
        std::tie(firstPoint, finalPoint) = stack.top();
        //And remove it from the stack
        stack.pop();

        //Some variables we need
        const double startElectrons = firstPoint->electrons;
        const double startLength = firstPoint->length;
        const double segmentLength = finalPoint->length - startLength;
        const double segmentElectrons = finalPoint->electrons - startElectrons;
        //We want to give the tolerance in electrons so we need to convert the
        //step length into electrons. We do this by using the average number of
        //electrons created per micrometer for a minimum ionizing particle.
        const double lengthScale = 1. / Unit::um * 80;
        //we need the slope for the linear approximation:
        //electrons= slope*(length-startLength)*lengthScale+startElectrons;
        const double slope = segmentElectrons / segmentLength / lengthScale;
        //Distance between point x0,y0 and line a*x+b*y+c=0 is given as
        //abs(a*x0+b*y0+c)/sqrt(a^2+b^2). In our case:
        //x0=(length-startLength)*lengthScale, y0=electrons
        //a=slope, b=-1, c=startElectrons.
        //Nominator is independent of the actual point so we calculate it now
        const double distanceConstant = std::sqrt(slope * slope + 1);

        //Variable to remember maximum distance from linear approximation
        double maxDistance(0);
        //and also the point with the largest distance
        SensorTraversal::iterator splitPoint;

        //No go through all intermediate points
        for (auto nextPoint = firstPoint + 1; nextPoint != finalPoint; ++nextPoint) {
          //and check their distance from the linear approximation
          const double x = (nextPoint->length - startLength) * lengthScale;
          const double dist = fabs(x * slope - nextPoint->electrons + startElectrons) / distanceConstant;
          //and remember if it is the largest distance
          if (dist > maxDistance) {
            splitPoint = nextPoint;
            maxDistance = dist;
          }
        }
        //if the largest distance is above the tolerance we split the segment
        if (maxDistance > m_electronTolerance) {
          //And add the two sub segments to the stack in the correcto order to
          //ensure sorted processing: We always add the segments at the front
          //last so they will be processed first
          stack.push(make_pair(splitPoint, finalPoint));
          stack.push(make_pair(firstPoint, splitPoint));
          continue;
        }
        //otherwise we add the endpoint of the current segment to the list of points.
        const double fraction = (finalPoint->length - lengthOffset) / length;
        const double electrons = (finalPoint->electrons - electronsOffset);
        electronProfile.push_back(VXDElectronDeposit(fraction, electrons));
      }
      return electronProfile;
    }

    StepInformation SensitiveDetectorBase::findMidPoint(const SensorTraversal& traversal)
    {
      //We want the middle of the track so we need to find the two steps
      //which enclose this
      const double midLength = traversal.getLength() * 0.5;
      auto after = traversal.begin();
      while (after->length < midLength) ++after;
      //Now we have an iterator after the half length. We know that the first
      //step contains length 0 so we can savely subtract one to get the one
      //before
      auto before = after - 1;
      //the midpoint is in between so calculate the fractions from both sides
      const double fl = (after->length - midLength) / (after->length - before->length);
      const double fr = (1 - fl);
      //we calculate the time and electrons using weighted average
      const double midTime = fl * before->time + fr * after->time;
      const double midElectrons = fl * before->electrons + fr * after->electrons;
      //now we use 3rd order bezier curve to approximate mid position and momentum.
      //The two base points are easy ...
      const G4ThreeVector& p0 = before->position;
      const G4ThreeVector& p3 = after->position;
      //And the two control points are the base points +- the appropriately
      //scaled momenta
      const double momentumScale = (p3 - p0).mag() / before->momentum.mag() / 3;
      const G4ThreeVector p1 = p0 + momentumScale * before->momentum;
      const G4ThreeVector p2 = p3 - momentumScale * after->momentum;
      // The curve is B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
      const G4ThreeVector midPos = (
                                     fl * fl * fl * p0
                                     + 3 * fl * fl * fr * p1
                                     + 3 * fl * fr * fr * p2
                                     + fr * fr * fr * p3
                                   );
      // The derivative is dB(t)/dt = 3*[(1-t)^2*(P1-P0)+2*(1-t)*t*(P2-P1)+t^2*(P3-P2)]
      const G4ThreeVector midMom = 1.0 / momentumScale * (
                                     fl * fl * (p1 - p0)
                                     + 2 * fl * fr * (p2 - p1)
                                     + fr * fr * (p3 - p2)
                                   );
      //Okay, we now have everything
      return StepInformation(midPos, midMom, midElectrons, midTime, midLength);
    }

  } //VXD namespace
} //Belle2 namespace
