/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Sara Neuhaus                            *
 *                                                                        *
 **************************************************************************/

#ifndef CDCTrigger2DFinderModule_H
#define CDCTrigger2DFinderModule_H

#include <framework/core/Module.h>

#include <utility>
#include <map>
#include <vector>
#include <fstream>

#include <root/TVector2.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerHoughCluster.h>

namespace Belle2 {
  /** Pair of <iSuperLayer, (x, y)>, for hits in conformal space */
  typedef std::pair<unsigned short, TVector2> cdcPair;
  /** Map of <counter, cdcPair>, for hits with indices */
  typedef std::map<int, cdcPair> cdcMap;
  /** Hough Tuples */
  typedef std::pair<TVector2, TVector2> coord2dPair;

  /**
   * Hough Candidates class.
   * A candidate is a single Hough cells which passes the minimum hit criterion.
   * It contains the coordinates of the cell corners, a list of hit indices
   * and the super layer count. Connecting candidates are combined to a track.
   */
  class CDCTriggerHoughCand {
  public:
    /** Constructor for hough candidates */
    CDCTriggerHoughCand(std::vector<unsigned>& _list, coord2dPair _coord,
                        unsigned short _slcount, unsigned _id):
      hitList(_list), coord(_coord), SLcount(_slcount), id(_id) {}

    ~CDCTriggerHoughCand() {}

    /** Get Index list */
    std::vector<unsigned> getIdList() const { return hitList; }
    /** Get rectangle coordinates */
    coord2dPair getCoord() const { return coord; }
    /** Get super layer count */
    unsigned short getSLcount() const { return SLcount; }
    /** Get candidate number */
    unsigned getID() const { return id; }

  private:
    /** list of hit indices contained in the Hough cell */
    std::vector<unsigned> hitList;
    /** Coordinate of rectangle for this candidate */
    coord2dPair coord;
    /** Super layer count (number of hits from different super layers) */
    unsigned short SLcount;
    /** candidate number, for debugging */
    unsigned id;
  };

  /** Two cells are identical if they have the same coordinates */
  inline bool operator==(const CDCTriggerHoughCand& a, const CDCTriggerHoughCand& b)
  {
    return (a.getCoord().first.X() == b.getCoord().first.X() &&
            a.getCoord().first.Y() == b.getCoord().first.Y());
  }

  class CDCTrigger2DFinderModule : public Module {
  public:
    /** Constructor.  */
    CDCTrigger2DFinderModule();

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();
    /** Clean up */
    virtual void terminate();

    /** Fast intercept finder
     *  Divide Hough plane recursively to find cells with enough crossing lines.
     *  @param hits                 map of hit coordinates in conformal space
     *  @param x1_s,x2_s,y1_s,y2_s  limits of starting rectangle
     *  @param iterations           current iteration index
     *  @param ix_s,iy_s            indices of starting rectangle */
    int fastInterceptFinder(cdcMap& hits,
                            double x1_s, double x2_s, double y1_s, double y2_s,
                            unsigned iterations,
                            unsigned ix_s, unsigned iy_s);

    /** count the number of super layers with hits
     *  @param array of hit/no hit for all super layers */
    unsigned short countSL(bool*);
    /** check the short track condition
     *  (= hits in the inner super layers rather than any super layers)
     *  @param array of hit/no hit for all super layers */
    bool shortTrack(bool*);

    /** Combine Hough candidates to tracks by merging connected cells.
     *  The track coordinate is the center of gravity of the resulting cell cluster. */
    void connectedRegions();
    /** Recursive function to add combine connected cells.
     *  All candidates are compared to center. Connected candidates are added
     *  to the merged list. If the module option onlyLocalMax is on, connected cells
     *  with SLcount lower than the cluster maximum are added to the rejected list
     *  instead. The function is called recursively for all connected candidates.
     *  @param center      starting candidate
     *  @param candidates  list of candidates compared to center
     *  @param merged      list for storing all candidates belonging to the cluster
     *  @param rejected    list for storing rejected candidates
     *                     (connected, but lower SLcount, not in any other cluster either)
     *  @param nSLmax      highest SLcount for this cluster */
    void addNeighbors(const CDCTriggerHoughCand& center,
                      const std::vector<CDCTriggerHoughCand>& candidates,
                      std::vector<CDCTriggerHoughCand>& merged,
                      std::vector<CDCTriggerHoughCand>& rejected,
                      unsigned short nSLmax) const;
    /** Check if candidate is in list. */
    bool inList(const CDCTriggerHoughCand& a, const std::vector<CDCTriggerHoughCand>& list) const;
    /** Check if candidates are connected. */
    bool connected(const CDCTriggerHoughCand& a, const CDCTriggerHoughCand& b) const;
    /** Merge lists a and b and put the result in merged. */
    void mergeIdList(std::vector<unsigned>& merged, std::vector<unsigned>& a, std::vector<unsigned>& b);

    /** Combine Hough candidates to tracks by a fixed pattern algorithm.
     *  The Hough plane is first divided in 2 x 2 squares, then squares are combined. */
    void patternClustering();
    /** Check for left/right connection of patterns in 2 x 2 squares */
    bool connectedLR(unsigned patternL, unsigned patternR);
    /** Check for up/down connection of patterns in 2 x 2 squares */
    bool connectedUD(unsigned patternD, unsigned patternU);
    /** Check for diagonal connected of patterns in 2 x 2 squares */
    bool connectedDiag(unsigned patternLD, unsigned patternRU);
    /** Find the top right square within a cluster of 2 x 2 squares
     *  In case of ambiguity, top is favored over right
     *  @ return   index of corner within pattern vector */
    unsigned topRightSquare(std::vector<unsigned>& pattern);
    /** Find the top right corner within 2 x 2 square.
     *  In case of ambiguity right corner is returned.
     *  x .
     *  . x   -> return this one
     *  @ return   index of corner within pattern */
    unsigned topRightCorner(unsigned pattern);
    /** Find the bottom left corner within 2 x 2 square.
     *  In case of ambiguity left corner is returned.
     *  x .   -> return this one
     *  . x
     *  @ return   index of corner within pattern */
    unsigned bottomLeftCorner(unsigned pattern);

    /** Find all hits whose Hough curve crosses the rectangle
     *  with corners (x1, y1) and (x2, y2) and add the hit indices to list. */
    void findAllCrossingHits(std::vector<unsigned>& list,
                             double x1, double x2, double y1, double y2);
    /** Select one hit per super layer.
     *  @param list        input list of hit Ids
     *  @param selected    selected hit Ids are added to selected
     *  @param unselected  the rest of the hit Ids are added to unselected */
    void selectHits(std::vector<unsigned>& list, std::vector<unsigned>& selected,
                    std::vector<unsigned>& unselected);

  protected:

    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;
    /** Name of the StoreArray containing the tracks found by the Hough tracking. */
    std::string m_outputCollectionName;
    /** Name of the StoreArray containing the clusters formed in the Hough plane. */
    std::string m_clusterCollectionName;
    /** number of Hough cells in phi */
    unsigned m_nCellsPhi;
    /** number of Hough cells in 1/r */
    unsigned m_nCellsR;
    /** Hough plane limit in Pt [GeV] */
    double m_minPt;
    /** shift the Hough plane in 1/r to avoid curvature 0 tracks
     *  < 0: shift in negative direction (negative half is larger)
     *    0: no shift (same limits for negative and positive half)
     *  > 0: shift in positive direction (positive half is larger) */
    int m_shiftPt;
    /** Hough plane limit in 1/r [1/cm] */
    double maxR;
    /** Hough plane shift in 1/r [1/cm] */
    double shiftR;
    /** number of iterations for the fast peak finder,
     *  smallest n such that 2^(n+1) > max(nCellsPhi, nCellsR) */
    unsigned maxIterations;
    /** number of cells for the fast peak finder: 2^(maxIterations + 1) */
    unsigned nCells;

    /** minimum number of hits from different super layers in a Hough cell
     *  to form a candidate */
    unsigned m_minHits;
    /** short tracks require hits in the first minHitsShort super layers
     *  to form a candidate */
    unsigned m_minHitsShort;
    /** minimum number of cells in a cluster to form a track */
    unsigned m_minCells;
    /** switch to ignore candidates connected to cells with higher super layer count */
    bool m_onlyLocalMax;
    /** number of neighbors to check for connection
     *  (4: direct, 6: direct + upper right and lower left corner,
     *   8: direct + all corners) */
    unsigned m_connect;
    /** switch to skip second priority hits */
    bool m_ignore2nd;
    /** switch between priority position and center position of track segment */
    bool m_usePriority;
    /** switch to check separately for a hit in the innermost super layer */
    bool m_requireSL0;

    /** switch to save the Hough plane in DataStore
     *  (0: don't save, 1: save only peaks, 2: save full plane) */
    unsigned m_storePlane;
    /** switch for clustering algorithm (if true use nested patterns) */
    bool m_clusterPattern;
    /** maximum cluster size for pattern algorithm */
    unsigned m_clusterSizeX;
    /** maximum cluster size for pattern algorithm */
    unsigned m_clusterSizeY;
    /** switch for creating relations to hits in the pattern clustering algorithm.
     *   true: create relations for all hits passing through the corners of
     *         a cluster,
     *   false: create relations for all hits passing though the estimated
     *          center of the cluster (can be 0 hits if center is not part
     *          of the cluster) */
    bool m_hitRelationsFromCorners;

    /** filename for test output for firmware debugging */
    std::string m_testFilename;
    /** filestream for test output for firmware debugging */
    std::ofstream testFile;

    /** map of TS hits containing <iHit, <iSL, (x, y)>> with
     *  iHit: hit index in StoreArray
     *  iSL: super layer index
     *  (x, y): coordinates in conformal space */
    cdcMap hitMap;
    /** Hough Candidates */
    std::vector<CDCTriggerHoughCand> houghCand;

    /** Radius of the CDC layers with priority wires (2 per super layer) */
    double radius[9][2];
    /** Number of track segments up to super layer */
    unsigned TSoffset[10];

    /** list of track segment hits */
    StoreArray<CDCTriggerSegmentHit> m_segmentHits;
    /** list of found tracks */
    StoreArray<CDCTriggerTrack> m_tracks;
    /** list of clusters in the Hough map */
    StoreArray<CDCTriggerHoughCluster> m_clusters;
    /** matrix containing the Hough plane */
    StoreObjPtr<TMatrix> m_houghPlane;
  };//end class declaration
} // end namespace Belle2

#endif // CDCTrigger2DFinderModule_H
