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
  class CDCLegendreQuadTree;
  class CDCLegendreConformalPosition;

  class CDCLegendreFastHough {
  public:

    /**
     * @brief Constructor
     * @param reconstructCurler Sets whether curlers should be reconstructed
     * @param maxLevel Maximal level of Fast Hough algorithm
     * @param nbinsTheta Number of theta bins (usually depends on maxlevel and should be not less than 2^maxlevel)
     * @param rMax Maximal value of r used in algorithm
     */
    CDCLegendreFastHough(bool reconstructCurler, int maxLevel, int nbinsTheta, double rMax);

    ~CDCLegendreFastHough();

    /**
     * @brief small helper function, to check if four values have the same sign
     */
    static inline bool sameSign(double n1, double n2, double n3, double n4)
    {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

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
    void FastHoughNormal(std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >* candidate,
                         const std::vector<CDCLegendreTrackHit*>& hits, const int level, const int theta_min,
                         const int theta_max, const double r_min, const double r_max, const unsigned limit);

    /**
     * @brief Recursively called function to perform the Fast Hough algorithm, modified to deliver the candidates which passes limitation on number of hits
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
    void MaxFastHough(const std::vector<CDCLegendreTrackHit*>& hits, const int level, const int theta_min, const int theta_max,
                      const double r_min, const double r_max);

    /**
     * Same logic as in MaxFastHough, but looks only for HighPt tracks;
     * starts with 2x2 binning, but when reached desired dr resolution (which defined as 2*Rcell) makes 1xN binning and finish algorithm in one iteration
     */
    void MaxFastHoughHighPt(const std::vector<CDCLegendreTrackHit*>& hits, const int theta_min, const int theta_max,
                            double r_min, double r_max, int level);


    /** Set limitation on number of hits in candidate */
    void setLimit(int limit) {m_limit = limit;};

    /** Provide set of hits */
    void setAxialHits(std::vector<CDCLegendreTrackHit*>& hits) {m_hits = hits;};

    /** Provides pointer to list of track candidates */
    void initializeCandidatesVector(std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > >* candidates) {m_candidates = candidates;};

  private:

//    bool m_useHitPrecalculatedR; /**< To store r values inside hit objects or recalculate it each step */
    bool m_reconstructCurler; /**< Set if curlers should be reconstructed */
    int m_maxLevel; /**< Maximum Level of FastHough Algorithm*/

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.01667; /**< threshold of r, which defines curlers */
    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel */
//    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module */
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module */
    double* m_sin_theta; /**< Lookup array for calculation of sin */
    double* m_cos_theta; /**< Lookup array for calculation of cos */
    unsigned m_limit;  /**< Limitation on number of hits in candidate */
    std::vector<CDCLegendreTrackHit*> m_hits; /**< Holds vector of hits */
    double Rcell; /**< Rough estimation of cell size */

    std::vector< std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > >* m_candidates; /**< Holds list of track candidates */
//    CDCLegendreConformalPosition &m_cdcLegendreConformalPosition;


  };
}
