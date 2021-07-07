/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TVector3.h>
#include <vector>


namespace Belle2 {

  /**
   * Class to calculate the Harmonic moments up to order 8 with respect to a given axis.
   * Since the most common user case is the calculation of the moments up to order 4, and the
   * calculation of the momenta 5-8 takes much longer, two methods have been implemented. HarmonicMoments::calculateBasicMoments
   * will calculate the moments up to 4, while HarmonicMoments::calculateAllMoments will perform the calculation
   * up to order 8. The two options have been implemented in two separate methods instead of using an if condition simply
   * to minimize the computing time.
   */
  class HarmonicMoments {
  public:

    /**
     * Default constructor.
     */
    HarmonicMoments()
    {
      m_axis.SetXYZ(0., 0., 0.);
    };

    /**
     * Constructor.
     * @param momenta An std::vector<TVector3> containing the 3-momenta to be used to the moments' calculation
     * @param axis The reference axis
     */
    HarmonicMoments(const std::vector<TVector3>& momenta, const TVector3& axis)
    {
      m_momenta.clear();
      m_momenta = momenta;
      m_axis = axis;
    };

    /**
     * Default destructor.
     */
    ~HarmonicMoments() {};

    /**
     * Sets the list of momenta, overwriting whatever list has been set before.
     * @param momenta An std::vector<TVector3> containing the 3-momenta to be used to the moments' calculation
     */
    void setMomenta(const std::vector<TVector3>& momenta)
    {
      m_momenta.clear();
      m_momenta = momenta;
      return;
    };

    /**
     * Sets the reference axis.
     * @param axis The reference axis
     */
    void setAxis(TVector3 axis)
    {
      m_axis = axis;
      return;
    };

    /**
     * Calculates the moments up to order 4.
     */
    void calculateBasicMoments();

    /**
     * Calculates the moments up to order 8. The execution time of this function is
     * significantly longer than  calculateBasicMoments().
     */
    void calculateAllMoments();

    /**
     * Returns the moment of order i
     * @param i the order (0-8)
     * @param sqrts the center of mass energy
     * @returns the harmonic moment, not normalized to sqrt(s)
     */
    double getMoment(short i, double sqrts) const
    {
      if (i < 0 || i > 8)
        return NAN;
      else
        return m_moment[i] / sqrts;
    }

  private:
    double m_moment[9] = {0.}; /**< The harmonic moments */
    std::vector<TVector3> m_momenta; /**< The list of particles */
    TVector3 m_axis; /**< The reference axis */
  };

} // Belle2 namespace
