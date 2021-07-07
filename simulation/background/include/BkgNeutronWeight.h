/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BKGNEUTRONWEIGHT_H
#define BKGNEUTRONWEIGHT_H

#include <vector>

namespace Belle2 {

  /** The class to get the weighting factor for a 1-MeV-equivalent neutron flux
    * on silicon. Data from http://sesam.desy.de/members/gunnar/Si-dfuncs.html
    */
  class BkgNeutronWeight {

  public:

    /** Return a reference to the singleton BkgNeutronWeight instance. */
    static BkgNeutronWeight& getInstance();

    /** Get weighting factor to convert a neutron to its 1-MeV equivalent */
    double getWeight(double ke);

  private:

    /** Constructor is private for this singleton */
    BkgNeutronWeight(void);

    /** Function to add one (x,y) data point to the spline table */
    void addPoint(double, double);

    /** Function to calculate the cubic spline derivatives */
    void doSpline();

    /** Number of tabulated points */
    int m_n;

    /** Vector of tabulated x coordinates (logarithm of kinetic energy) */
    std::vector<double> m_x;

    /** Vector of tabulated y coordinates (weighting factors) */
    std::vector<double> m_y;

    /** Vector of first derivatives in spline fit (calculated) */
    std::vector<double> m_b;

    /** Vector of second derivatives in spline fit (calculated) */
    std::vector<double> m_c;

    /** Vector of third derivatives in spline fit (calculated) */
    std::vector<double> m_d;

    /** static pointer to the singleton instance of this class */
    static BkgNeutronWeight* m_Instance;

  };

} // end of namespace Belle2

#endif // BKGNEUTRONWEIGHT_H
