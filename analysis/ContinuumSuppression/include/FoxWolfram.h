/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <Math/Vector3D.h>
#include <vector>


namespace Belle2 {

  /**
   * Class to calculate the Fox-Wolfram moments up to order 8.
   * Since the most common user case is the calculation of the moments up to order 4, and the
   * calculation of the momenta 5-8 takes much longer, two methods have been implemented. FoxWolfram::calculateBasicMoments
   * will calculate the moments up to 4, while FoxWolfram::calculateAllMoments will perform the calculation
   * up to order 8. The two options have been implemented in two separate methods instead of using an if condition simply
   * to minimize the computing time.
   */

  class FoxWolfram {
  public:

    /**
     * Default constructor.
     */
    FoxWolfram();


    /**
     * Constructor with an array of 3-momenta.
     */
    explicit FoxWolfram(const std::vector<ROOT::Math::XYZVector>& momenta)
    {
      m_momenta.clear();
      m_momenta = momenta;
    };


    /**
     * Default destructor
     */
    ~FoxWolfram() {};


    /**
     * Method to  perform the calculation of the moments up to order 4,
     * which are the most relevant ones. The momenta up to order 8 can be calculated
     * calling FoxWolfram::calculateAllMoments().
     */
    void calculateBasicMoments();


    /**
     * Method to  perform the calculation of the moments up to order 8.
     * It adds a significant overhead to the total FW moments calculation time, so it should be
     * used for debugging or development studies.
     */
    void calculateAllMoments();


    /**
     * Sets the list of momenta used for the FW moment calculation, overwriting whatever list
     * has been set before.
     */
    void setMomenta(const std::vector<ROOT::Math::XYZVector>& momenta)
    {
      m_momenta.clear();
      m_momenta = momenta;
      return;
    };

    /**
     * Returns the i-th moment
     */
    double getH(int i) const { return (i < 0 || i > 8) ? NAN : m_moment[i]; }

    /**
     * Returns the i-th moment normalized to the 0th-order moment. These are the quantites
     * normally used for the event shape characterization and the continuum suppression.
     */
    double getR(int i) const { return (i < 0 || i > 8 || m_moment[0] == 0) ? NAN : m_moment[i] / m_moment[0]; }


  private:
    double m_moment[9] = {0.}; /**< The moments */
    std::vector<ROOT::Math::XYZVector> m_momenta; /**< The particle's momenta */
  };

} // Belle2 namespace
