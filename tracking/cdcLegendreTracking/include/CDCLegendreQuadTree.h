/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include <set>
#include <vector>
#include <algorithm>

namespace Belle2 {

  class CDCLegendreTrackHit;

  class CDCLegendreQuadTree {

  public:

    CDCLegendreQuadTree();

    CDCLegendreQuadTree(double rMin, double rMax, int thetaMin, int thetaMax, int level, CDCLegendreQuadTree* parent);

    ~CDCLegendreQuadTree();


//    void setParameters(double rMin, double rMax, int thetaMin, int thetaMax, int level, CDCLegendreQuadTree* parent);

    /*
     * Initialize structure and prepare children
     */
    void initialize();

    /*
     * Sets threshold on number of hits in the node
     */
    void setThreshold(unsigned int hitsThreshold) {
      s_hitsThreshold = hitsThreshold;
      s_nCands = 0;
    };


    /*
     * Copy information about hits into member of class (node at level 0 should be used  because other levels fills by parents)
     */
    void provideHitSet(const std::set<CDCLegendreTrackHit*>& hits_set);

    /*
     * Fill the tree structure
     */
    void startFillingTree();

    /*
     * Fill children of node with hits (according to bin crossing criteria)
     */
    void fillChildren(/*const std::vector<CDCLegendreTrackHit*>& hits*/);

    /*
     * Reserve space which will be used for filling with pointers to the hits
     */
    void reserveHitsVector(int nHits) {m_hits.reserve(nHits); };

    /*
     * Insert hit into node
     */
    void insertHit(CDCLegendreTrackHit* hit) {m_hits.push_back(hit); };

    /*
     * Reserve memory for holding hits
     */
    void reserveHitsArray(int nHits) {m_hits.reserve(nHits); };

    /*
     * Clear hits which the node holds
     */
    void clearNode() {m_hits.clear(); };

    /*
     * Clear hits which the node holds
     */
    void clearTree();

    /*
     * Check if the node passes threshold on number of hits
     */
    bool checkNode() {return m_hits.size() >= s_hitsThreshold;};

    /*
     * Check if the node passes threshold on number of hits
     */
    inline int getNHits() const {return m_hits.size();};

    /*
     * Clean node from used or bad hits
     */
    void cleanHitsInNode() ;

    /*
     * Get hits from node
     */
    inline std::vector<CDCLegendreTrackHit*>& getHits() {return m_hits;};

    /*
     * Get mean value of theta
     */
    inline double getThetaMean() {return (m_thetaMin + m_thetaMax) / 2. * m_PI / s_nbinsTheta;};

    /*
     * Get mean value of r
     */
    inline double getRMean() {return (m_rMin + m_rMax) / 2.;};


    inline bool checkFilled() {return m_filled; };

    void setFilled() {m_filled = true; };

    int getNCands() {return s_nCands;};

  private:

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

    double m_rMin;
    double m_rMax;
    int m_thetaMin;
    int m_thetaMax;
    int m_level;
    double Rcell;
    double m_deltaR;

    std::vector<CDCLegendreTrackHit*> m_hits;

    CDCLegendreQuadTree* m_parent;

    CDCLegendreQuadTree*** m_children;
    bool m_lastLevel;

    double* m_r;          //bins range on r
    int* m_thetaBin;      //bins range on theta
    int m_nbins_r;        //number of r bins
    int m_nbins_theta;    //number of theta bins
    static double* s_sin_theta; /**< Lookup array for calculation of sin */
    static double* s_cos_theta; /**< Lookup array for calculation of cos */
    static bool s_sin_lookup_created; /**< Allows to use the same lookup table for sin and cos */
    static int s_nbinsTheta;
    static unsigned int s_hitsThreshold;
    static int s_nCands;
    bool m_filled;

    /*
     * Check if we reach limitation on dr and dtheta; returns true when reached limit
     */
    bool checkLimitsR();


  };
}
