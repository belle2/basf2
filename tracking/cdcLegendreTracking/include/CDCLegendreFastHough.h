/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {

  class CDCLegendreTrackHit;

  class CDCLegendreFastHough {
  public:

    CDCLegendreFastHough(bool reconstructCurler, int maxLevel, int nbinsTheta, double rMin, double rMax);


    /**
     * @brief Recursively called function to perform the Fast Hough algorithm, modified to only deliver the candidate with the most contributing hits
     * @param candidate Pointer to the found track candidate. Will remain untouched, if no candidate is found
     * @param hits The hits, which are used for track finding
     * @param level the number, how often the function is called recursively. If it hits max_level, function will be left and the parameter candidate assigned
     * @param theta_min lower border of theta in bins
     * @param theta_max upper border of theta in bins
     * @param r_min lower border of r
     * @param r_max upper border of r
     * There are multiple description of the Fast Hough algorithm. This implementation follows not a special one, but tries to be as simple as possible.
     * At each step, the remaining voting plane is divided in 2x2 squares and the voting procedure is performed in each of them, following NIM A 592 (456 - 462).
     * Only bins with more bins than the current maximum are further investigated where the current maximum is determined of the configured threshold or the number of hits of an already found track candidate.
     */
    void MaxFastHough(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* candidate,
                      const std::vector<CDCLegendreTrackHit*>& hits, const int level, const int theta_min,
                      const int theta_max, const double r_min, const double r_max, const unsigned limit);

    /**
     * @brief small helper function, to check if four values have the same sign
     */
    inline bool sameSign(double, double, double, double);

  private:
    bool m_reconstructCurler;
    int m_maxLevel; /**< Maximum Level of FastHough Algorithm*/

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/
    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/
    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/
    double* m_sin_theta; /**< Lookup array for calculation of sin*/
    double* m_cos_theta; /**< Lookup array for calculation of cos*/


  };
}
