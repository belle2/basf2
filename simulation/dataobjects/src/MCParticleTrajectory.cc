/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <simulation/dataobjects/MCParticleTrajectory.h>
#include <TVector3.h>
#include <stack>
#include <tuple>

using namespace Belle2;

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

void MCParticleTrajectory::simplify(float distance_tolerance)
{
  // cannot simplify anything, return
  if (distance_tolerance <= 0 || m_points.size() < 3) return;

  // stack with all segments to be investigated
  std::stack<std::pair<iterator, iterator>> stack;
  // push full trajectory on the stack
  stack.push(make_pair(m_points.begin(), m_points.end() - 1));
  // next free point: we always want the starting point so start at index 1
  iterator nextFreePoint = m_points.begin() + 1;
  // iterators used for the segment inspection
  iterator firstPoint, splitPoint, finalPoint;
  // segment direction and vector between segment start and mid point
  TVector3 n, pa;
  // investigate all segments until all fulfill the distance requirement
  while (!stack.empty()) {
    //Get first and last point
    std::tie(firstPoint, finalPoint) = stack.top();
    //Remove segment from stack
    stack.pop();
    //Direction of the segment
    setVector(n, *firstPoint, *finalPoint, true);
    //Calculate maximum distance of all intermediate points to the segment
    double maxDistance(0);
    for (auto nextPoint = firstPoint + 1; nextPoint != finalPoint; ++nextPoint) {
      //vector from segment start (p) to point (a)
      setVector(pa, *firstPoint, *nextPoint);
      //3D distance between point a and line p + x*n
      const double dist = (pa - (pa * n) * n).Mag();
      //check if this is the maximum distance so far
      if (dist > maxDistance) {
        splitPoint = nextPoint;
        maxDistance = dist;
      }
    }
    //Are all points close enough? if not split the segment at the largest distance
    if (maxDistance > distance_tolerance) {
      //If we split in this order, all points will be in correct order since we
      //use a stack: last thing put in is first thing to get out. So initially
      //we inspect everything, if we divide we put in the second part and then
      //the first and in the next round we take out the first and repeat. So
      //effectively we look at all segments in an ordered way.
      stack.push(make_pair(splitPoint, finalPoint));
      stack.push(make_pair(firstPoint, splitPoint));
      continue;
    }
    //Ok, all points close enough, add the final point to list of points. Due
    //to the order in which we look at the points in a ordered way we can
    //replace the points in place
    *(nextFreePoint++) = *finalPoint;
  }

  //Now delete all remaining elements
  m_points.erase(nextFreePoint, end());
}
