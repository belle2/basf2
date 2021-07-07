/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
     * @param x x coordinate of position
     * @param y y coordinate of position
     * @param z z coordinate of position
     * @param px x coordinate of momentum
     * @param py y coordinate of momentum
     * @param pz z coordinate of momentum
     */
    void addPoint(float x, float y, float z, float px, float py, float pz)
    {
      //Reserve a decent amount of space to avoid lots of relocations
      m_points.emplace_back(x, y, z, px, py, pz);
    }

    /** Simplify the trajectory using the Ramer-Douglas-Peuker algorithm.
     * @param distanceTolerance maximum distance of the simplified path to
     * the points of the original path
     */
    void simplify(float distanceTolerance);

  private:
    //! Collection of points along the trajectory
    std::vector<MCTrajectoryPoint> m_points;

    //! Needed to make the ROOT object storable
    ClassDef(MCParticleTrajectory, 1);
  };

} //Belle2 namespace
