/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/dataobjects/MCParticleTrajectory.h>
#include <TVector3.h>
#include <iostream>

using namespace Belle2;

ClassImp(MCParticleTrajectory);

namespace {
  /** Utility function to set an TVector3 from two points.
   * v = a - b
   * @param v TVector3 to modify
   * @param a first point
   * @param b second point
   * @param unit if true, return the unit vector in a-b direction
   */
  void setVector(TVector3& v, const MCTrajectoryPoint& a, const MCTrajectoryPoint& b, bool unit = false)
  {
    v.SetXYZ(a.x - b.x, a.y - b.y, a.z - b.z);
    if (unit) {
      v *= 1. / v.Mag();
    }
  }
}

void MCParticleTrajectory::simplify(float angular_tolerance, float distance_tolerance)
{
  //We can only simplify if there is a tolerance and at least three points
  if (angular_tolerance <= 0 || distance_tolerance <= 0 || m_points.size() < 3) return;

  //Now we loop over all points: we have a reference point r, a final point f
  //and at least one intermediate point, i_0...i_n, between r and f
  //
  // - if the angle between i_0 - r and f - i_n is larger than
  //   angular_tolerance, i_n is added to the list and becomes the new
  //   reference point
  // - if the maximum distance between the points i_0...i_n and the line f - r
  //   exceeds distance_tolerance, the point i_n is added to the list and
  //   becomes the new reference, r
  // - in both cases, f becomes r + 2

  //iterator to the current reference point
  iterator referencePoint = begin();
  //iterator to the current final point
  iterator finalPoint = referencePoint + 2;
  //iterator to the next "free" position in the vector
  iterator nextPoint = begin() + 1;

  //Temporary vectors for angle and distance calculations
  TVector3 vec1, vec2;

  for (; finalPoint != end(); ++finalPoint) {
    //Calculate angle between the two segments
    setVector(vec1, *(referencePoint + 1), *referencePoint);
    setVector(vec2, *finalPoint, *(finalPoint - 1));
    const double angle = vec1.Angle(vec2);
    //Calculate distances between intermediate points p and line segment,
    //x = a + t*n, where a is a point on the line and n is the unit vector
    //pointing in line direction. distance = ||(p-a) - ((p-a)*n)*n||
    setVector(vec1, *finalPoint, *referencePoint, true);
    double dist(0);
    for (iterator intermediate = referencePoint + 1;
         intermediate != finalPoint; ++intermediate) {
      setVector(vec2, *intermediate, *referencePoint);
      dist = std::max(dist, (vec2 - (vec2 * vec1) * vec1).Mag());
    }
    if (angle > angular_tolerance || dist > distance_tolerance) {
      //Update reference point
      referencePoint = finalPoint - 1;
      //add reference point to final list of points
      *(nextPoint++) = *referencePoint;
      //and set new final-point to be reference + 1, will get incremented to
      //reference +2 by the for loop to give us one intermediate point
      finalPoint = referencePoint + 1;
      continue;
    }
  }
  //We are still missing the last point
  *(nextPoint++) = m_points.back();

  //Now delete all remaining elements
  m_points.erase(nextPoint, end());
}
