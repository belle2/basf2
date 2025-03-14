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
#include <utility>

namespace Belle2 {

  /* Data type to collect a binning */
  struct SectorBinning {
    c5elem omega;
    c5elem phi;
    c5elem theta;
    c5elem nHitIDs;
    c5elem nPriorityWires;
  };

  /* Data type for a cluster cell */
  struct CellWeight {
    cell_index index;
    unsigned short weight;
  };

  /* Collection of the hit information needed for the hit representations */
  struct HitInfo {
    unsigned short relativeWireID;
    unsigned short priorityWire;
    short phiOffset;
  };

  /* Store track parameters of found tracks. */
  class NDFinderTrack {
  public:
    NDFinderTrack(std::vector<double> values,
                  SimpleCluster&& cluster,
                  std::vector<ROOT::Math::XYZVector>&& readoutHoughSpace,
                  std::vector<ROOT::Math::XYZVector>&& readoutCluster)
      : m_cluster(std::move(cluster)),
        m_houghSpace(std::move(readoutHoughSpace)),
        m_readoutCluster(std::move(readoutCluster))
    {
      m_omega = values[0];
      m_phi = values[1];
      m_cotTheta = values[2];
    }

    /* Default destructor. */
    virtual ~NDFinderTrack() {}
    double getOmega() const
    {
      return m_omega;
    }
    double getPhi0() const
    {
      return m_phi;
    }
    double getCot() const
    {
      return m_cotTheta;
    }
    double getNRelHits() const
    {
      return m_cluster.getHits().size();
    }
    std::vector<unsigned short> getRelHits() const
    {
      return m_cluster.getHits();
    }
    std::vector<unsigned short> getRelHitsWeights() const
    {
      return m_cluster.getWeights();
    }
    void reset()
    {
      m_omega = 0.;
      m_phi = 0.;
      m_cotTheta = 0.;
    }
    SimpleCluster getCluster() const
    {
      return m_cluster;
    }
    std::vector<ROOT::Math::XYZVector> getHoughSpace() const
    {
      return m_houghSpace;
    }
    std::vector<ROOT::Math::XYZVector> getClusterReadout() const
    {
      return m_readoutCluster;
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
    std::vector<ROOT::Math::XYZVector> m_houghSpace;
    /* Vector storing cluster informations for analysis */
    std::vector<ROOT::Math::XYZVector> m_readoutCluster;
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
      unsigned char minSuperAxial = 4;
      /* Required number of stereo super layers */
      unsigned char minSuperStereo = 3;
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
      /* Switch for writing the full Hough space and the cluster information to the 3DFinderInfo class */
      bool storeAdditionalReadout = false;
    };

    /* Default constructor. */
    NDFinder() {}

    /* Destructor. */
    virtual ~NDFinder()
    {
      delete m_axialHitContributions;
      delete m_stereoHitContributions;
      delete m_houghSpace;
      delete m_hitToSectorIDs;
      delete m_compAxialHitReps;
      delete m_compStereoHitReps;
      delete m_expAxialHitReps;
      delete m_expStereoHitReps;
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
     * @param storeAdditionalReadout switch for Hough space + cluster readout
     * @param axialFile axial hit data
     * @param stereoFile stereo hit data
     * */
    void init(unsigned char minSuperAxial, unsigned char minSuperStereo, float thresh,
              unsigned short minTotalWeight, unsigned short minPeakWeight, unsigned char iterations,
              unsigned char omegaTrim, unsigned char phiTrim, unsigned char thetaTrim,
              bool storeAdditionalReadout, std::string& axialFile, std::string& stereoFile);

    /* NDFinder reset data structure to process next event */
    void reset()
    {
      m_NDFinderTracks.clear();
      m_hitIDs.clear();
      m_hitSLIDs.clear();
      m_priorityWirePos.clear();
      m_priorityWireTime.clear();
      m_nHits = 0;
      m_vecDstart.clear();
      m_hitOrients.clear();
      delete m_houghSpace;
      std::array<c3index, 3> shapeHough = {{ m_nOmega, m_nPhi, m_nTheta }};
      m_houghSpace = new c3array(shapeHough);
    }

    /* Fill hit info of the event */
    void addHit(unsigned short hitID, unsigned short hitSLID, unsigned short hitPrioPos, long hitPrioTime)
    {
      if (hitPrioPos > 0) { // skip "no hit"
        m_hitIDs.push_back(hitID);
        m_hitSLIDs.push_back(hitSLID);
        m_priorityWirePos.push_back(3 - hitPrioPos);
        m_priorityWireTime.push_back(hitPrioTime);
        ++m_nHits;
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
    void initLookUpArrays();

    /* Fills the m_hitToSectorIDs (see below) array with the hit to orientation/sectorWire/sectorID relations */
    void initHitToSectorMap(c2array& hitsToSectors);

    /* Fills the m_compAxialHitReps/m_compStereoHitReps (see below) arrays with the hit representations (hits to weights) */
    void loadCompressedHitReps(const std::string& fileName, const SectorBinning& compBins, c5array& compHitsToWeights);

    /* Fills the m_expAxialHitReps/m_expStereoHitReps (see below) arrays with the expanded hit representations (hits to weights) */
    void fillExpandedHitReps(const SectorBinning& compBins, const c5array& compHitsToWeights, c5array& expHitsToWeights);

    /* Loop over all hits and theta bins and squeeze all */
    /* 2D (omega,phi) planes */
    void squeezeAll(const SectorBinning& writebins, c5array& writeArray, const c5array& expHitsToWeights);

    /* Squeeze phi-axis in a 2D (omega,phi) plane */
    /* @param inparcels number of 1/32 sectors in input plane */
    /* @param outparcels number of 1/32 sectors in output plane */
    void squeezeOne(c5array& writeArray, const c5array& expHitsToWeights, c5index hitIdx, c5index priorityIndex, c5index itheta,
                    c5elem nomega);

    /* Core track finding logic in the constructed houghmap */
    void getCM();

    /* Add a single axial or stereo hit to the houghmap. */
    /* Determines the phi window of the hit in the full houghmap (Dstart, Dend). */
    /* Uses: m_arrayHitMod */
    /* Fills: m_vecDstart, m_hitOrients */
    void addLookup(unsigned short hitIdx);

    /* Write (add) a single hit (Hough curve) to the Hough space */
    void writeHitToHoughSpace(const HitInfo& hitInfo, const c5array& expHitsToWeights);

    /* Create hits to clusters confusion matrix */
    std::vector<std::vector<unsigned short>> getHitsVsClusters(std::vector<SimpleCluster>& clusters);

    /* Peak cell in cluster */
    cell_index getMax(const std::vector<cell_index>&);

    /* Determine weight contribution of a single hit to a single cell. */
    /* Used to create the hitsVsClusters confusion matrix. */
    unsigned short hitContrib(cell_index peak, unsigned short hitIdx);

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
    std::vector<unsigned short> m_hitSLIDs;

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

    /* Configuration parameters of the 3DFinder */
    ndparameters m_params;

    /* Configuration of the clustering module */
    clustererParams m_clustererParams;

    /* Clustering module */
    Belle2::Clusterizend m_clusterer;

    /* Track segments */
    static constexpr unsigned short m_nTS = 2336; // Number of track segments
    static constexpr unsigned short m_nSL = 9; // Number of super layers
    static constexpr unsigned short m_nAxial = 41; // Number of unique axial track segments
    static constexpr unsigned short m_nStereo = 32; // Number of unique stereo track segments
    static constexpr unsigned short m_nPrio = 3; // Number of priority wires

    /* Full Hough space bins */
    static constexpr unsigned short m_nOmega = 40; // Bins in the phi dimension
    static constexpr unsigned short m_nPhi = 384; // Bins in the omega dimension
    static constexpr unsigned short m_nTheta = 9; // Bins in the theta dimension

    /* CDC symmetry in phi */
    static constexpr unsigned short m_phiGeo = 32; // Repetition of the wire pattern
    static constexpr unsigned short m_parcels = 7; // phi range: hit data
    static constexpr unsigned short m_parcelsExp = 11; // phi range: expanded hit data

    /* Phi sectors in the CDC */
    static constexpr unsigned short m_nPhiSector = m_nPhi / m_phiGeo; // Bins of one phi sector (12)
    static constexpr unsigned short m_nPhiComp = 15; // Bins of compressed phi: phi_start, phi_width, phi_0, ..., phi_12
    static constexpr unsigned short m_nPhiUse =  m_parcels * m_nPhiSector; // Bins of 7 phi sectors (84)
    static constexpr unsigned short m_nPhiExp =  m_parcelsExp * m_nPhiSector; // Bins of 11 phi sectors (132)

    /* Binnings in different hit pattern arrays */
    static constexpr SectorBinning m_compAxialBins = {m_nOmega, m_nPhiComp, 1, m_nAxial, m_nPrio}; // 40, 15, 1, 41, 3
    static constexpr SectorBinning m_compStereoBins = {m_nOmega, m_nPhiComp, m_nTheta, m_nStereo, m_nPrio}; // 40, 15, 9, 32, 3
    static constexpr SectorBinning m_expAxialBins = {m_nOmega, m_nPhiExp, m_nTheta, m_nAxial, m_nPrio}; // 40, 132, 9, 32, 3
    static constexpr SectorBinning m_expStereoBins = {m_nOmega, m_nPhiExp, m_nTheta, m_nStereo, m_nPrio}; // 40, 132, 9, 32, 3
    static constexpr SectorBinning m_axialBins = {m_nOmega, m_nPhiUse, m_nTheta, m_nAxial, m_nPrio}; // 40, 84, 9, 41, 3
    static constexpr SectorBinning m_stereoBins = {m_nOmega, m_nPhiUse, m_nTheta, m_nStereo, m_nPrio}; // 40, 84, 9, 32, 3
    static constexpr SectorBinning m_fullBins = {m_nOmega, m_nPhi, m_nTheta, m_nTS, m_nPrio}; // 40, 384, 9, 2336, 3

    /* Acceptance ranges + slot sizes to convert bins to track parameters (for getBinToVal method) */
    static constexpr std::array<float, 2> m_omegaRange = {-5., 5.};
    static constexpr std::array<float, 2> m_phiRange = {0., 11.25};
    static constexpr std::array<float, 2> m_thetaRange = {19., 140.};
    static constexpr float m_binSizeOmega = (m_omegaRange[1] - m_omegaRange[0]) / m_nOmega; // 0.25
    static constexpr float m_binSizePhi = (m_phiRange[1] - m_phiRange[0]) / m_nPhiSector; // 0.9375
    static constexpr float m_binSizeTheta = (m_thetaRange[1] - m_thetaRange[0]) / m_nTheta; // 13.444
    static constexpr std::array<std::array<float, 2>, 3> m_acceptanceRanges = {m_omegaRange, m_phiRange, m_thetaRange};
    static constexpr std::array<float, 3> m_binSizes = {m_binSizeOmega, m_binSizePhi, m_binSizeTheta};

    /* Array pointers to the hit patterns */
    /* m_hitToSectorIDs: 2D array mapping TS-ID ([0, 2335]) to: */
    /*   - [0]: Orientation (1 = axial, 0 = stereo) */
    /*   - [1]: Relative wire ID in the sector ([0, 40] for axials, [0, 31] for stereos) */
    /*   - [2]: Relative phi-sector ID in the super layer ([0, 31] in each SL) */
    c2array* m_hitToSectorIDs = nullptr;
    /* m_compAxialHitReps/m_compStereoHitReps (~ Compressed in phi (width, start, values)) 5D array mapping: */
    /* 1. [hitID]: Relative hit number of the track segment (axial [0, 40], stereo [0, 31]) */
    /* 2. [priorityWire]: Hit priority wire ([0, 2]) */
    /* 3. [omegaIdx]: Omega index of the Hough space ([0, 39]) */
    /* 4. [phiIdx]: Phi start value, number of phi bins, phi values (0, 1, [2, 14]) */
    /* 5. [thetaIdx]: Theta index of the Hough space (0 for axial TS, [0, 8] for stereo TS) */
    /* to the Hough space weight contribution at the corresponding bin (int, [0, 7]) */
    c5array* m_compAxialHitReps = nullptr;
    c5array* m_compStereoHitReps = nullptr;
    /* m_expAxialHitReps/m_expStereoHitReps (~ Expands to 11/32 phi sectors) 5D array mapping: */
    /* 1. [hitID]: Relative hit number of the track segment (axial [0, 40], stereo [0, 31]) */
    /* 2. [priorityWire]: Hit priority wire ([0, 2]) */
    /* 3. [omegaIdx]: Omega index of the Hough space ([0, 39]) */
    /* 4. [phiIdx]: The actual relative phi index in the Hough space ([0, 131]), Dstart must be added */
    /* 5. [thetaIdx]: Theta index of the Hough space ([0, 8] for both axial and stereo!) */
    /* to the Hough space weight contribution at the corresponding bin (int, [0, 7]) */
    c5array* m_expAxialHitReps = nullptr;
    c5array* m_expStereoHitReps = nullptr;
    c5array* m_axialHitContributions = nullptr;
    c5array* m_stereoHitContributions = nullptr;
    c3array* m_houghSpace = nullptr;
  };
}
