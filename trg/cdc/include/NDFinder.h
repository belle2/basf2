/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cmath>
#include "trg/cdc/NDFinderDefs.h"
#include "trg/cdc/Clusterizend.h"
#include <Math/Vector3D.h>

namespace Belle2 {

  /* Data type to collect a binning */
  struct SectorBinning {
    c5elem omega;
    c5elem phi;
    c5elem theta;
    c5elem hitid;
    c5elem priorityWire;
  };

  /* Data type for a cluster cell */
  struct CellWeight {
    cell_index index;
    unsigned short weight;
  };

  struct HitInfo {
    unsigned short relativeWireID;
    unsigned short priorityWire;
    short Dstart;
    SectorBinning bins;
  };

  /* Store track parameters of found tracks. */
  class NDFinderTrack {
  public:
    NDFinderTrack(std::vector<double> values, const SimpleCluster& cluster,
                  std::vector<ROOT::Math::XYZVector> houghspace, std::vector<ROOT::Math::XYZVector> ndreadout):
      m_cluster(cluster)
    {
      m_omega = values[0];
      m_phi = values[1];
      m_cotTheta = values[2];
      m_houghspace = houghspace;
      m_ndreadout = ndreadout;
    }

    /* Default destructor. */
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
    double getNRelHits()
    {
      return m_cluster.getHits().size();
    }
    std::vector<unsigned short> getRelHits()
    {
      return m_cluster.getHits();
    }
    std::vector<unsigned short> getRelHitsWeights()
    {
      return m_cluster.getWeights();
    }
    void reset()
    {
      m_omega = 0.;
      m_phi = 0.;
      m_cotTheta = 0.;
    }
    SimpleCluster getCluster()
    {
      return m_cluster;
    }
    std::vector<ROOT::Math::XYZVector> getHoughSpace()
    {
      return m_houghspace;
    }
    std::vector<ROOT::Math::XYZVector> getNDReadout()
    {
      return m_ndreadout;
    }

  private:
    /* 2D track curvature */
    double m_omega;
    /* 2D azimuthal angle */
    double m_phi;
    /* 3D polar angle */
    double m_cotTheta;
    /* Vector of the indices of the related hits */
    /* in the list of CDC hits (StoreArray<CDCHits>) */
    std::vector<unsigned short> m_relHits;
    /* Vector of the weights for each related hit. */
    std::vector<double> m_hitWeights;
    /* The found cluster of the track */
    SimpleCluster m_cluster;
    /* Vector storing the complete Hough space for analysis */
    std::vector<ROOT::Math::XYZVector> m_houghspace;
    /* Vector storing cluster informations for analysis */
    std::vector<ROOT::Math::XYZVector> m_ndreadout;
  };


  /* Class to represent the CDC NDFinder. */
  class NDFinder {
  public:

    /* Struct of ndFinder parameters */
    struct ndparameters {
      /* Zero-Suppressed trained hit data */
      std::string axialFile = "data/trg/cdc/ndFinderArrayAxialComp.txt.gz";
      std::string stereoFile = "data/trg/cdc/ndFinderArrayStereoComp.txt.gz";

      /* Required number of axial super layers */
      unsigned char minSuperAxial  = 4;
      /* Required number of stereo super layers */
      unsigned char minSuperStereo  = 3;
      /* Hough space cells must have (thresh * maxweight) to be considered */
      float thresh = 0.85;
      /* Clustering: Minimum of the total weight in all cells of the 3d volume */
      unsigned short minTotalWeight = 450;
      /* Clustering: Minimum peak cell weight */
      unsigned short minPeakWeight = 32;
      /* Clustering: Number of iterations for the cluster search in each Hough space quadrant */
      unsigned char iterations = 2;
      /* Clustering: Number of deleted cells in each omega direction from the maximum */
      unsigned char omegaTrim = 5;
      /* Clustering: Number of deleted cells in each phi direction from the maximum */
      unsigned char phiTrim = 4;
      /* Clustering: Number of deleted cells in each theta direction from the maximum */
      unsigned char thetaTrim = 4;
    };

    /* Default constructor. */
    NDFinder() {}

    /* Destructor. */
    virtual ~NDFinder()
    {
      delete m_parrayAxial;
      delete m_parrayStereo;
      delete m_phoughPlane;
      delete m_hitToSectorIDs;
      delete m_compAxialHitReps;
      delete m_compStereoHitReps;
      delete m_parrayAxialExp;
      delete m_parrayStereoExp;
    }

    /* Initialization */

    /** Set parameters
     * @param minSuperAxial minimum number of axial super layers per cluster
     * @param minSuperStereo minimum number of stereo super layers per cluster
     * @param thresh selection of cells for weighted mean track estimation
     * @param minTotalWeight minimum total weight of all cells in the 3d volume
     * @param minPeakWeight minimum peak cell weight
     * @param iterations number of cluster searches in each Hough space quadrant
     * @param omegaTrim number deleted cells in each omega direction from the maximum
     * @param phiTrim number deleted cells in each phi direction from the maximum
     * @param thetaTrim number deleted cells in each theta direction from the maximum
     * @param verbose print Hough planes and verbose output
     * @param axialFile axial hit data
     * @param stereoFile stereo hit data
     * */
    void init(unsigned char minSuperAxial, unsigned char minSuperStereo, float thresh,
              unsigned short minTotalWeight, unsigned short minPeakWeight, unsigned char iterations,
              unsigned char omegaTrim, unsigned char phiTrim, unsigned char thetaTrim,
              bool verbose, std::string& axialFile, std::string& stereoFile);

    /* NDFinder reset data structure to process next event */
    void reset()
    {
      m_NDFinderTracks.clear();
      m_hitIDs.clear();
      m_hitSLIds.clear();
      m_priorityWirePos.clear();
      m_priorityWireTime.clear();
      m_nHits = 0;
      m_vecDstart.clear();
      m_hitOrients.clear();
      delete m_phoughPlane;
      std::array<c3index, 3> shapeHough = {{ m_nOmega, m_nPhi, m_nTheta }};
      m_phoughPlane = new c3array(shapeHough);
    }

    /* fill hit info of the event */
    void addHit(unsigned short hitId, unsigned short hitSLId, unsigned short hitPrioPos, long hitPrioTime)
    {
      if (hitPrioPos > 0) { // skip "no hit"
        m_hitIDs.push_back(hitId);
        m_hitSLIds.push_back(hitSLId);
        m_priorityWirePos.push_back(3 - hitPrioPos);
        m_priorityWireTime.push_back(hitPrioTime);
        m_nHits++;
      }
    }

    /* main function for track finding */
    void findTracks();

    /* retreive the results */
    std::vector<NDFinderTrack>* getFinderTracks()
    {
      return &m_NDFinderTracks;
    }

    /* NDFinder internal functions for track finding*/
  protected:

    /* Initialize the binnings and reserve the arrays */
    void initBins();

    /* Fills the m_hitToSectorIDs array */
    void initHitToSectorMap(c2array& mapArray);

    /* Load an NDFinder array of hit representations in track phase space. */
    /* Used to load axial and stereo hit arrays. */
    /* Represented in a 7/32 phi sector of the CDC. */
    void loadHitRepresentations(const std::string& filename, SectorBinning bins, c5array& hitsToTracks);

    /* Restore non-zero suppressed hit curves. */
    /* will make m_params.arrayAxialFile and m_params.arrayStereoFile obsolete */
    void restoreZeros(SectorBinning zerobins, SectorBinning compbins, c5array& expArray, const c5array& compArray);

    /* Squeeze phi-axis in a 2D (omega,phi) plane */
    /* @param inparcels number of 1/32 sectors in input plane */
    /* @param outparcels number of 1/32 sectors in output plane */
    void squeezeOne(c5array& writeArray, c5array& readArray, int outparcels, int inparcels, c5index ihit, c5index priorityIndex,
                    c5index itheta,
                    c5elem nomega);

    /* Loop over all hits and theta bins and squeeze all */
    /* 2D (omega,phi) planes */
    void squeezeAll(SectorBinning writebins, c5array& writeArray, c5array& readArray, int outparcels, int inparcels);

    /* Core track finding logic in the constructed houghmap */
    void getCM();

    /* Add a single axial or stereo hit to the houghmap. */
    /* Determines the phi window of the hit in the full houghmap (Dstart, Dend). */
    /* Uses: m_arrayHitMod */
    /* Fills: m_vecDstart, m_hitOrients */
    void addLookup(unsigned short ihit);

    /* In place array addition to houghmap Comp: A = A + B */
    void addC3Comp(const HitInfo& hitInfo, const c5array& hitsToTracks);

    /* Create hits to clusters confusion matrix */
    std::vector<std::vector<unsigned short>> getHitsVsClusters(
                                            std::vector<SimpleCluster>& clusters);

    /* Peak cell in cluster */
    cell_index getMax(const std::vector<cell_index>&);

    /* Determine weight contribution of a single hit to a single cell. */
    /* Used to create the hitsVsClusters confusion matrix. */
    unsigned short hitContrib(cell_index peak, unsigned short ihit);

    /* Relate all hits in a cluster to the cluster */
    /* Remove small clusters with less than minsuper related hits. */
    std::vector<SimpleCluster> allHitsToClusters(
      std::vector<std::vector<unsigned short>>& hitsVsClusters,
      std::vector<SimpleCluster>& clusters);

    /* Candidate cells as seed for the clustering. */
    /* Selects all cells with weight > minWeight */
    std::vector<CellWeight> getHighWeight(std::vector<cell_index> entries, float cutoff);

    /* Calculate the weighted center of a cluster */
    std::vector<double> getWeightedMean(std::vector<CellWeight>);

    /* Scale the weighted center to track parameter values */
    std::vector<double> getBinToVal(std::vector<double>);

    /* Transverse momentum to radius */
    double cdcTrackRadius(double pt)
    {
      return pt * 1e11 / (3e8 * 1.5); // div (c * B)
    }

    /* Calculate physical units */
    float transformVar(float estVal, int idx);
    std::vector<double> transform(std::vector<double> estimate);

    /* NDFinder */
  private:
    /* Result: vector of the found tracks */
    std::vector<NDFinderTrack> m_NDFinderTracks;

    /* TS-Ids of the hits in the current event */
    /* elements: [0,2335] for 2336 TS in total */
    std::vector<unsigned short> m_hitIDs;

    /* SL-Ids of the hits in the current event */
    /* elements: super layer number in [0,1,...,8] */
    std::vector<unsigned short> m_hitSLIds;

    /* Priority positon within the TS in the current event */
    /* elements basf2: [0,3] first, left, right, no hit */
    /* elements stored: 3 - basf2prio */
    std::vector<unsigned short> m_priorityWirePos;

    /* Drift time of the priority wire */
    std::vector<long> m_priorityWireTime;

    /* Orients */
    /* TS-Ids of the hits in the current event */
    /* elements: [0,2335] for 2336 TS in total */
    std::vector<unsigned short> m_hitOrients;

    /* Phi-start of 7/32 hit representation in full track parameter space. */
    /* Used to get the weight contribution of a hit to a cluster. */
    std::vector<short> m_vecDstart;

    /* Counter for the number of hits in the current event */
    unsigned short m_nHits{0};

    /* Print Hough planes and verbose output */
    bool m_verbose{false};

    /* Configuration parameters of the 3DFinder */
    ndparameters m_params;

    /* Configuration of the clustering module */
    clustererParams m_clustererParams;

    /* Clustering module */
    Belle2::Clusterizend m_clusterer;

    /* Track segments */
    unsigned short m_nTS{0}; // Number of track segments
    unsigned short m_nSL{0}; // Number of super layers
    unsigned short m_nAxial{0}; // Number of unique axial track segments
    unsigned short m_nStereo{0}; // Number of unique stereo track segments
    unsigned short m_nPrio{0}; // Number of priority wires

    /* Full Hough space bins */
    unsigned short m_nPhi{0}; // Bins in the phi dimension
    unsigned short m_nOmega{0}; // Bins in the omega dimension
    unsigned short m_nTheta{0}; // Bins in the theta dimension

    /* CDC symmetry in phi */
    unsigned short m_phiGeo{0}; // Repetition of the wire pattern
    unsigned short m_parcels{0}; // phi range: hit data
    unsigned short m_parcelsExp{0}; // phi range: expanded hit data

    /* Phi sectors in the CDC */
    unsigned short m_nPhiSector{0}; // Bins of one phi sector
    unsigned short m_nPhiComp{0}; // Bins of compressed phi: phi_start, phi_width, phi_0, ..., phi_12
    unsigned short m_nPhiUse{0}; // Bins of 7 phi sectors
    unsigned short m_nPhiExp{0}; // Bins of 11 phi sectors

    /* Binnings in different hit pattern arrays */
    SectorBinning m_compAxialBins;
    SectorBinning m_compStereoBins;
    SectorBinning m_axialBins;
    SectorBinning m_stereoBins;
    SectorBinning m_expAxialBins;
    SectorBinning m_expStereoBins;
    SectorBinning m_fullBins;

    /* Acceptance Ranges to convert bins to track parameters */
    std::vector<std::vector<float>> m_acceptRanges;
    std::vector<float> m_slotSizes;

    /* Array pointers to the hit patterns */
    /* m_hitToSectorIDs: 2D array mapping TS-ID ([0, 2335]) to: */
    /*   - [0]: Orientation (1 = axial, 0 = stereo) */
    /*   - [1]: Relative wire ID in the sector ([0, 40] for axials, [0, 31] for stereos) */
    /*   - [2]: Relative phi-sector ID in the super layer ([0, 31] in each SL) */
    c2array* m_hitToSectorIDs = nullptr;
    c5array* m_parrayAxial = nullptr;
    c5array* m_parrayStereo = nullptr;
    c3array* m_phoughPlane = nullptr;
    c5array* m_compAxialHitReps = nullptr;
    c5array* m_compStereoHitReps = nullptr;
    c5array* m_parrayAxialExp = nullptr;
    c5array* m_parrayStereoExp = nullptr;
  };
}
