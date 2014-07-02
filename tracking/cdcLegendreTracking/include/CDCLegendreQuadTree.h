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

#include <framework/logging/Logger.h>

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
     * Build neighborhood for leafs
     */
    void buildNeighborhood(int levelNeighborhood);

    /*
     * Returns level of the node in tree (i.e., how much ancestors the node has)
     */
    inline int getLevel() const {return m_level;};

    /*
     * Sets threshold on number of hits in the node
     */
    void setHitsThreshold(unsigned int hitsThreshold) {s_hitsThreshold = hitsThreshold;};

    /*
     * Sets threshold on pt of candidates
     */
    void setRThreshold(double rThreshold) {s_rThreshold = rThreshold;};

    /*
     * Sets threshold on pt of candidates
     */
    void setLastLevel(double lastLevel) {s_lastLevel = lastLevel;};

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
     * Forced filling of tree, skipping limitation on number of hits
     * Filling nodes which are parents to the current one
     */
    void fillChildrenForced();

    /*
     * Insert hit into node
     */
    void insertHit(CDCLegendreTrackHit* hit) {m_hits.push_back(hit); };

    /*
     * Reserve memory for holding hits
     */
    void reserveHitsVector(int nHits) {m_hits.reserve(nHits); };

    /*
     * Check if the node passes threshold on number of hits
     */
    bool checkNode() const {return m_hits.size() >= s_hitsThreshold;};

    /*
     * Get hits from node
     */
    inline std::vector<CDCLegendreTrackHit*>& getHits() {return m_hits;};

    /*
     * Check if the node passes threshold on number of hits
     */
    inline int getNHits() const {return m_hits.size();};

    /*
     * Removing used or bad hits
     */
    void cleanHitsInNode() ;

    /*
     * Check whether node is leaf (lowest node in the tree)
     */
    bool isLeaf() const {return s_lastLevel;};

    /*
     * Check whether node has been processed, i.e. children nodes has been filled
     */
    inline bool checkFilled() const {return m_filled; };

    /*
     * Set status of node to "filled" (children nodes has been filled)
     */
    void setFilled() {m_filled = true; };

    /*
     * Get mean value of theta
     */
    inline double getThetaMean() const {return (m_thetaMin + m_thetaMax) / 2. * m_PI / s_nbinsTheta;};

    /*
     * Get mean value of r
     */
    inline double getRMean() const {return (m_rMin + m_rMax) / 2.;};

    /*
     * get number of bins in "r" direction
     */
    inline int getRNbins() const {return m_nbins_r;};

    /*
     * Get minimal "r" value of the node
     */
    inline double getRMin() const {return m_rMin;};

    /*
     * Get maximal "r" value of the node
     */
    inline double getRMax() const {return m_rMax;};

    /*
     * get number of bins in "Theta" direction
     */
    inline int getThetaNbins() const {return m_nbins_theta;};

    /*
     * Get minimal "Theta" value of the node
     */
    inline int getThetaMin() const {return m_thetaMin;};

    /*
     * Get maximal "Theta" value of the node
     */
    inline int getThetaMax() const {return m_thetaMax;};

    /*
     * Return pointer to the parent of the node
     */
    inline CDCLegendreQuadTree* getParent() const {return m_parent;};

    /*
     * Get child of the node by index
     */
    CDCLegendreQuadTree* getChildren(int t_index, int r_index) const ;

    /*
     * Add pointer to some node to list of neighbors of current node
     */
    void addNeighbor(CDCLegendreQuadTree* node) {m_neighbors.push_back(node);};

    /*
     * Get number of neighbors of the current node (used mostly for debugging purposes)
     */
    int getNneighbors() const {return m_neighbors.size();};

    /*
     * Check whether neighbors of the node has been found
     */
    inline bool isNeighborsDefined() const {return m_neighborsDefined;};

    /*
     * Find and store neighbors of the node
     */
    void findNeighbors();

    /*
     * Get list of neighbors of the current node
     */
    std::vector<CDCLegendreQuadTree*>& getNeighborsVector();

    /*
     * Clear hits which the node holds
     */
    void clearNode() {m_hits.clear(); };

    /*
     * Clear hits which the node holds
     */
    void clearTree();

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

    std::vector<CDCLegendreQuadTree*> m_neighbors; // 8 neighbours of each node (or 5 at borders)

    CDCLegendreQuadTree*** m_children;
    bool m_isMaxLevel;

    double* m_r;          //bins range on r
    int* m_thetaBin;      //bins range on theta
    int m_nbins_r;        //number of r bins
    int m_nbins_theta;    //number of theta bins
    static double* s_sin_theta; /**< Lookup array for calculation of sin */
    static double* s_cos_theta; /**< Lookup array for calculation of cos */
    static bool s_sin_lookup_created; /**< Allows to use the same lookup table for sin and cos */
    static int s_nbinsTheta;
    static unsigned int s_hitsThreshold;
    static double s_rThreshold; /**< Threshold on r variable; allows to set threshold on pt of tracks */
    static int s_lastLevel;
    bool m_filled;
    bool m_neighborsDefined;


    /*
     * Check if we reach limitation on dr and dtheta; returns true when reached limit
     */
    bool checkLimitsR();


  };
}
