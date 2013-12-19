/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCPARTICLETRAJECTORY_H
#define MCPARTICLETRAJECTORY_H

#include <framework/datastore/RelationsObject.h>
#include <simulation/dataobjects/MCTrajectoryPoint.h>
#include <vector>

namespace Belle2 {
  /** Class to save the full simulated trajectory of a particle */
  class MCParticleTrajectory: public RelationsObject {
  public:
    /** Default constructor */
    MCParticleTrajectory(): RelationsObject(), m_points() {}

    /** iterator definition to allow iteration */
    typedef std::vector<MCTrajectoryPoint>::iterator iterator;
    /** const iterator definition to allow iteration */
    typedef std::vector<MCTrajectoryPoint>::const_iterator const_iterator;
    /** return iterator to the first point */
    iterator begin() { return m_points.begin(); }
    /** return iterator beyond the last point */
    iterator end() { return m_points.end(); }
    /** return const iterator to the first point */
    const_iterator begin() const { return m_points.begin(); }
    /** return const iterator beyond the last point */
    const_iterator end() const { return m_points.end(); }
    /** return number of points */
    size_t size() const { return m_points.size(); }
    /** return true if size()==0 */
    bool empty() const { return m_points.empty(); }
    /** return a point */
    const MCTrajectoryPoint& operator[](size_t index) const { return m_points[index]; }
    /** return reference to the first point */
    const MCTrajectoryPoint& front() const { return m_points.front(); }
    /** return reference to the last point */
    const MCTrajectoryPoint& back() const { return m_points.back(); }
    /** Add a point to the trajectory
     * @param x x coordinate
     * @param y y coordinate
     * @param z z coordinate
     */
    void addPoint(float x, float y, float z, float px, float py, float pz) {
      //Reserve a decent amount of space to avoid lots of relocations
      m_points.emplace_back(x, y, z, px, py, pz);
    }

    /** Simplify the trajectory by merging all steps which do not change the
     * direction below a given tolerance.
     * @param angular_tolerance merge steps if their direction differs by less than
     * tolerance in radian
     * @param distance_tolerance maximum distance of the simplified path to
     * the points of the original path
     */
    void simplify(float angular_tolerance, float distance_tolerance);

  private:
    std::vector<MCTrajectoryPoint> m_points;

    ClassDef(MCParticleTrajectory, 1);
  };

} //Belle2 namespace
#endif
