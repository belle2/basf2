/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Sara Neuhaus                            *
 *                                                                        *
 **************************************************************************/

#ifndef CDCTriggerHoughtrackingModule_H
#define CDCTriggerHoughtrackingModule_H

#include <framework/core/Module.h>

#include <utility>
#include <map>
#include <vector>

#include <root/TVector2.h>

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

  /**
   * Hough Track class.
   * A track is the center of a cluster of candidates.
   * It contains the center coordinates and a hit list.
   */
  class CDCTriggerHoughTrack {
  public:
    /** Constructor for hough candidates */
    CDCTriggerHoughTrack(std::vector<unsigned>& _list, TVector2 _coord):
      hitList(_list), coord(_coord) {}

    ~CDCTriggerHoughTrack() {}

    /** Get Index list */
    std::vector<unsigned> getIdList() { return hitList; }
    /** Get center coordinates */
    TVector2 getCoord() { return coord; }

  private:
    /** ID list of points */
    std::vector<unsigned> hitList;
    /** Coordinate of cluster center */
    TVector2 coord;
  };

  class CDCTriggerHoughtrackingModule : public Module {
  public:
    /** Constructor.  */
    CDCTriggerHoughtrackingModule();

    /** Initialize the module and check module parameters */
    virtual void initialize();
    /** Run tracking */
    virtual void event();

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

  protected:

    /** Name of the StoreArray containing the tracks found by the Hough tracking. */
    std::string m_outputCollectionName;
    /** number of Hough cells in phi */
    unsigned m_nCellsPhi;
    /** number of Hough cells in 1/r */
    unsigned m_nCellsR;
    /** Hough plane limit in Pt [GeV] */
    double m_minPt;
    /** Hough plane limit in 1/r [1/cm] */
    double maxR;
    /** number of iterations for the fast peak finder,
     *  smallest n such that 2^(n+1) > max(nCellsPhi, nCellsR) */
    unsigned maxIterations;
    /** number of cells for the fast peak finder: 2^(maxIterations + 1) */
    unsigned nCells;

    /** minimum number of hits from different super layers in a Hough cell
     *  to form a candidate */
    unsigned m_minHits;
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

    /** switch to save the Hough plane in DataStore
     *  (0: don't save, 1: save only peaks, 2: save full plane) */
    unsigned m_storePlane;

    /** map of TS hits containing <iHit, <iSL, (x, y)>> with
     *  iHit: hit index in StoreArray
     *  iSL: super layer index
     *  (x, y): coordinates in conformal space */
    cdcMap hitMap;
    /** Hough Candidates */
    std::vector<CDCTriggerHoughCand> houghCand;
    /** Purified Hough Candidates */
    std::vector<CDCTriggerHoughTrack> houghTrack;

    /** Radius of the CDC layers with priority wires (2 per super layer) */
    double radius[9][2];
    /** Number of track segments up to super layer */
    unsigned TSoffset[10];
  };//end class declaration
} // end namespace Belle2

#endif // CDCTriggerHoughtrackingModule_H
