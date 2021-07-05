/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef NDFINDER_H
#define NDFINDER_H

#include <cmath>
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"

namespace Belle2 {

  struct cellweight {
    cell_index index;
    ushort weight;
  };

  /** Store track parameters of found tracks. */
  class NDFinderTrack {
  public:
    NDFinderTrack(std::vector<double> values, SimpleCluster cluster)
    {
      m_omega = values[0];
      m_phi = values[1];
      m_cotTheta = values[2];
      m_cluster = cluster;
    }

    /** Default destructor. */
    virtual ~NDFinderTrack() {}
    double getOmega()
    {
      return m_omega;
    }
    double getPhi0()
    {
      return m_phi;
    }
    double getCot()
    {
      return m_cotTheta;
    }
    double get_nRelHits()
    {
      return m_cluster.get_hits().size();
    }
    std::vector<unsigned short> get_relHits()
    {
      return m_cluster.get_hits();
    }
    std::vector<unsigned short> get_relHitsWeights()
    {
      return m_cluster.get_weights();
    }
    void reset()
    {
      m_omega = 0.;
      m_phi = 0.;
      m_cotTheta = 0.;
    }

  private:
    /** 2D track curvature */
    double m_omega;
    /** 2D azimuthal angle */
    double m_phi;
    /** 3D polar angle */
    double m_cotTheta;
    /** vector of the indices of the related hits
     *  in the list of CDC hits (StoreArray<CDCHits>) */
    std::vector<unsigned short> m_relHits;
    /** vector of the weights for each related hit. */
    std::vector<double> m_hitWeights;
    SimpleCluster m_cluster;
  };


  /** Class to represent the CDC NDFinder.
   *
   */
  class NDFinder {
  public:

    /** Struct of ndFinder parameters */
    struct ndparameters {
      /** Zero-Suppressed trained hit data */
      std::string axialFile = "data/trg/cdc/ndFinderArrayAxialComp.txt.gz";
      std::string stereoFile = "data/trg/cdc/ndFinderArrayStereoComp.txt.gz";

      /** Clustering options */
      /**  Only accept clusters with minhits */
      long unsigned int minhits  = 4;
      /**  Required number of axial hits per track */
      long unsigned int minhits_axial  = 0;
      /**  Members must have thresh x maxweight of cluster */
      float thresh            = 0.85;

      /** Required relative weight contribution to assign a hit to a cluster */
      float minassign = 0.2;

      /** Minimum numeber of cells in the track parameter space */
      long unsigned int mincells  = 5;

      /** CDC symmetry: repeat wire pattern 32 times in phi */
      unsigned short phigeo = 32;
      ///** CDC symmetry: phi range covered by hit data [0 .. phigeo] */
      unsigned short parcels = 7;
      /** CDC symmetry: phi range covered by expanded hit data [0 .. phigeo] */
      unsigned short parcelsExp = 11;
    };
    std::vector<std::vector<float>> m_acceptRanges;
    std::vector<float> m_slotsizes;



    /** Default constructor. */
    NDFinder() {}

    /** Default destructor. */
    virtual ~NDFinder()
    {
      delete m_parrayAxial;
      delete m_parrayStereo;
      delete m_phoughPlane;
      delete m_parrayHitMod;
      delete m_pcompAxial;
      delete m_pcompStereo;
      delete m_parrayAxialExp;
      delete m_parrayStereoExp;
    }


    /** initialization */

    /** Set parameters
     * @param minweight minimum weight of cluster member cells
     * @param minpts minimum neighboring cells with minweight for core cells
     * @param diagonal consider diagonal neighbor cells in the clustering
     * @param minhits minimum number of hits per cluster
     * @param minhits_axial minimum number of axial hits per cluster
     * @param thresh selection of cells for weighted mean track estimation
     * @param minassign hit to cluster assigment critical limit
     * @param mincells minumum number of cells per cluster
     * @param verbose print Hough planes and verbose output
     * @param axialFile axial hit data
     * @param stereoFile stereo hit data
     * */
    void init(int minweight, int minpts, bool diagonal,
              int minhits, int minhits_axial, double thresh, double minassign,
              int mincells, bool verbose, std::string axialFile, std::string stereoFile);

    /** Initialize the binnings and reserve the arrays */
    void initBins();

    /** Load an NDFinder array of hit representations in track phase space.
     * Used to load axial and stereo hit arrays.
     * Represented in a 7/32 phi sector of the CDC. */
    void loadArray(const std::string& filename, ndbinning bins, c5array& hitsToTracks);
    /** Restore non-zero suppressed hit curves.
     * will make m_params.arrayAxialFile and m_params.arrayStereoFile obsolete */
    void restoreZeros(ndbinning zerobins, ndbinning compbins, c5array& expArray, c5array& compArray);

    /** Squeeze phi-axis in a 2D (omega,phi) plane
     * @param inparcels number of 1/32 sectors in input plane
     * @param outparcels number of 1/32 sectors in output plane */
    void squeezeOne(c5array& writeArray, c5array& readArray, int outparcels, int inparcels, c5index ihit, c5index iprio, c5index itheta,
                    c5elem nomega);

    /** Loop over all hits and theta bins and squeeze all
     * 2D (omega,phi) planes */
    void squeezeAll(ndbinning writebins, c5array& writeArray, c5array& readArray, int outparcels, int inparcels);

    /** Initialize hit modulo mappings */
    void initHitModAxSt(c2array& hitMod);

    /** NDFinder reset data structure to process next event */
    void reset()
    {
      m_NDFinderTracks.clear();
      m_hitIds.clear();
      m_prioPos.clear();
      m_nHits = 0;
      m_vecDstart.clear();
      m_hitOrients.clear();
      delete m_phoughPlane;
      m_phoughPlane = new c3array(m_pc3shape);
    }

    /** Debug: print configured parameters */
    void printParams();

    /** fill hit info of the event */
    void addHit(unsigned short hitId, unsigned short hitPrioPos)
    {
      if (hitPrioPos > 0) { // skip "no hit"
        m_hitIds.push_back(hitId);
        m_prioPos.push_back(3 - hitPrioPos);
        m_nHits++;
      }
    }

    /** main function for track finding */
    void findTracks();

    /** retreive the results */
    std::vector<NDFinderTrack>* getFinderTracks()
    {
      return &m_NDFinderTracks;
    }

    /** Debug Tool: Print part of the houghmap */
    void printArray3D(c3array& hitsToTracks, ndbinning bins, ushort, ushort, ushort, ushort);

    /** NDFinder internal functions for track finding*/
  protected:

    /** Core track finding logic in the constructed houghmap */
    void getCM();

    /** Add a single axial or stereo hit to the houghmap.
     * Determines the phi window of the hit in the full houghmap (Dstart, Dend).
     * Uses: m_arrayHitMod
     * Fills: m_vecDstart, m_hitOrients   */
    void addLookup(unsigned short ihit);

    /** In place array addition to houghmap Comp: A = A + B */
    void addC3Comp(ushort hitr, ushort prio, c5array& hitsToTracks,
                   short Dstart, ndbinning bins);

    /** Create hits to clusters confusion matrix */
    std::vector<std::vector<unsigned short>> getHitsVsClusters(
                                            std::vector<SimpleCluster>& clusters);

    /** Peak cell in cluster */
    cell_index getMax(std::vector<cell_index>&);

    /** Determine weight contribution of a single hit to a single cell.
     * Used to create the hitsVsClusters confusion matrix. */
    ushort hitContrib(cell_index peak, ushort ihit);

    /** Use confusion matrix to related hits to clusters.
     * Remove small clusters with less than minhits related hits. */
    std::vector<SimpleCluster> relateHitsToClusters(
      std::vector<std::vector<unsigned short>>& hitsVsClusters,
      std::vector<SimpleCluster>& clusters);

    /** Determine the best cluster for a single hit,
     * given hitsVsClusters confusion matrix */
    int hitToCluster(std::vector<std::vector<unsigned short>>& hitsVsClusters,
                     unsigned short hit);

    /** Candidate cells as seed for the clustering.
     * Selects all cells with weight > minweight */
    std::vector<cellweight> getHighWeight(std::vector<cell_index> entries, float cutoff);

    /** Calculate the weighted center of a cluster */
    std::vector<double> getWeightedMean(std::vector<cellweight>);

    /** Scale the weighted center to track parameter values */
    std::vector<double> getBinToVal(std::vector<double>);

    /** Transverse momentum to radius */
    double cdc_track_radius(double pt)
    {
      return pt * 1e11 / (3e8 * 1.5); // div (c * B)
    }

    float m_pi_deg = 3.141592653589793 / 180.;

    /** Calculate physical units */
    float transformVar(float estVal, int idx);
    std::vector<double> transform(std::vector<double> estimate);

    /** NDFinder */
  private:

    /** Shapes of the arrays holding the hit patterns */
    boost::array<c5index, 5> m_pc5shapeax;
    boost::array<c5index, 5> m_pc5shapest;
    boost::array<c3index, 3> m_pc3shape;
    boost::array<c2index, 2> m_pc2shapeHitMod;
    boost::array<c5index, 5> m_pc5shapeCompAx;
    boost::array<c5index, 5> m_pc5shapeCompSt;
    boost::array<c5index, 5> m_pc5shapeExpAx;
    boost::array<c5index, 5> m_pc5shapeExpSt;

    /** Array pointers to the hit patterns */
    c5array* m_parrayAxial = nullptr;
    c5array* m_parrayStereo = nullptr;
    c3array* m_phoughPlane = nullptr;
    c2array* m_parrayHitMod = nullptr;
    c5array* m_pcompAxial = nullptr;
    c5array* m_pcompStereo = nullptr;
    c5array* m_parrayAxialExp = nullptr;
    c5array* m_parrayStereoExp = nullptr;

    /** Number of first priority wires in each super layer (TS per SL) */
    std::vector<int> m_nWires;

    /** Result: vector of the found tracks */
    std::vector<NDFinderTrack> m_NDFinderTracks;

    /** TS-Ids of the hits in the current event
     * elements: [0,2335] for 2336 TS in total*/
    std::vector<unsigned short> m_hitIds;

    /** Priority positon within the TS in the current event
     * elements basf2: [0,3] first, left, right, no hit
     * elements stored: 3 - basf2prio*/
    std::vector<unsigned short> m_prioPos;

    /** Orients
     * TS-Ids of the hits in the current event
     * elements: [0,2335] for 2336 TS in total*/
    std::vector<unsigned short> m_hitOrients;

    /** Phi-start of 7/32 hit representation in full track parameter space.
     * Used to get the weight contribution of a hit to a cluster. */
    std::vector<short> m_vecDstart;

    /** Counter for the number of hits in the current event */
    unsigned short m_nHits;

    /** Configuration parameters of the 3DFinder */
    ndparameters m_params;

    /** Binnings in different hit pattern arrays */
    ndbinning m_axbins;
    ndbinning m_stbins;
    ndbinning m_fullbins;
    ndbinning m_compaxbins;
    ndbinning m_compstbins;
    ndbinning m_expaxbins;
    ndbinning m_expstbins;

    /** Configuration of the clustering module */
    clusterer_params m_clusterer_params;
    std::vector<ushort> m_planeShape;

    /** Default bins */
    unsigned short m_nPhiFull;
    unsigned short m_nPhiOne;
    unsigned short m_nPhiComp;
    unsigned short m_nPhiExp;
    unsigned short m_nPhiUse;
    unsigned short m_nOmega;
    unsigned short m_nTheta;

    unsigned short m_nSL;
    unsigned short m_nTS;
    unsigned short m_nAx;
    unsigned short m_nSt;
    unsigned short m_nPrio;

    /** Clustering module */
    Belle2::Clusterizend m_clusterer;

    /** Print Hough planes and verbose output */
    bool m_verbose;
  };
}
#endif

