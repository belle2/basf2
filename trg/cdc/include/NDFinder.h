/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Skambraks                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      /** Trained hit data */
      std::string arrayAxialFile = "data/trg/cdc/ndFinderArrayAxial.txt.gz";
      std::string arrayStereoFile = "data/trg/cdc/ndFinderArrayStereo.txt.gz";
      std::string hitModAxStFile = "data/trg/cdc/hitModAxSt.txt.gz";
      /** Zero-Suppressed trained hit data */
      std::string compAxialFile = "data/trg/cdc/ndFinderArrayAxialComp.txt.gz";
      std::string compStereoFile = "data/trg/cdc/ndFinderArrayStereoComp.txt.gz";
      /** TODO: reconstruct arrayAxial, arrayStereo from compAxial, compStereo */

      /** Clustering options */
      /**  only accept clusters with minhits */
      long unsigned int minhits  = 4; //6; //7;
      /**  members must have thresh x maxweight of cluster */
      float thresh            = 0.85; //0.9;

      float minassign = 0.2;

      long unsigned int mincells  = 5;

      /** CDC symmetry: repeat wire pattern 32 times in phi */
      unsigned short phigeo = 32;
      ///** CDC symmetry: phi range covered by hit data in training [0 .. phigeo] */
      //unsigned short parcels = 7;
      ///** CDC symmetry: phi range covered by compressed hit data [0 .. phigeo] */
      //unsigned short parcelsComp = 11;
    };
    /** Default binning in one parcel (7/32 CDC phi) */
    struct binning {
      c5elem omega = 40;
      c5elem phi = 84;
      c5elem theta = 9;
      c5elem hitid; // 41 for axial, 32 for stereo
      c5elem prio = 3;
    };
    std::vector<std::vector<float>> m_acceptRanges;
    std::vector<float> m_slotsizes;

    double cdc_track_radius(double pt)
    {
      return pt * 1e11 / (3e8 * 1.5); // div (c * B)
    }
    float xpi = 3.141592653589793; // TODO
    float transformVar(float estVal, int idx)
    {
      if (idx == 0) { //omega
        if (estVal == 0.) {
          return estVal;
        } else {
          return - 1 / cdc_track_radius(1. / estVal);
        }
      } else if (idx == 1) { // phi
        float phiMod = estVal;
        if (estVal > 180) {
          phiMod -= 360.;
        }
        return phiMod * xpi / 180.;
      } else { // theta
        float thetRad = estVal * xpi / 180.;
        return cos(thetRad) / sin(thetRad);
      }
    }
    std::vector<double> transform(std::vector<double> estimate)
    {
      std::vector<double> transformed;
      for (int idx = 0; idx < 3; idx++) {
        transformed.push_back(transformVar(estimate[idx], idx));
      }
      return transformed;
    }
    template<class T>
    std::string printVector(std::vector<T> vecX)
    {
      return m_clusterer.printVector(vecX);
    }


    /** Default constructor. */
    NDFinder() {}

    /** Default destructor. */
    virtual ~NDFinder() {}


    /** initialization */

    /** Set parameters
     * @param minhits minimum number of hits per cluster
     * @param minweight minimum weight of cluster member cells
     * @param minweight minimum neighboring cells with minweight for core cells
     * @param thresh selection of cells for weighted mean track estimation
     * @param minassign hit to cluster assigment critical limit
     * @param mincells minumum number of cells per cluster
     * */
    void init(int minweight, int minpts, bool diagonal,
              int minhits, double thresh, double minassign,
              int mincells);

    /** Load an NDFinder array of hit representations in track phase space.
     * Used to load axial and stereo hit arrays.
     * Represented in a 7/32 phi sector of the CDC. */
    bool loadArray(const std::string& filename, binning bins, c5array& hitsToTracks);
    /** Load NDFinder array of hit modulo mappings */
    bool loadHitModAxSt(const std::string& filename, binning bins, c2array& hitMod);

    /** NDFinder reset data structure to process next event */
    void reset()
    {
      m_NDFinderTracks.clear();
      m_hitIds.clear();
      m_prioPos.clear();
      m_nHits = 0;
      m_houghPlane = c3array(m_c3shape);
      m_vecDstart.clear();
      m_hitOrients.clear();
    }
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

    /** Debug Tool: Print (part) of the houghmap */
    bool printArray3D(c3array& hitsToTracks, binning bins, ushort, ushort);

    /** NDFinder internal functions for track finding*/
  protected:

    /** Core track finding logic in the constructed houghmap */
    void getCM();

    /** Add a single axial or stereo hit to the houghmap.
     * Determines the phi window of the hit in the full houghmap (Dstart, Dend).
     * Uses: m_arrayHitMod
     * Fills: m_vecDstart, m_hitOrients,// m_vecIhit   */
    void addLookup(unsigned short ihit);

    /** In place array addition to houghmap Comp: A = A + B */
    void addC3Comp(ushort hitr, ushort prio, c5array& hitsToTracks,
                   short Dstart, binning bins);

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

    /** TODO: check */
    std::vector<double> getWeightedMean(std::vector<cellweight>);
    std::vector<double> getBinToVal(std::vector<double>);



    /** NDFinder */
  private:
    boost::array<c5index, 5> m_c5shapeax = {{ 41, 3, 40, 84, 9 }};
    boost::array<c5index, 5> m_c5shapest =  {{ 32, 3, 40, 84, 9 }};
    c5array m_arrayAxial = c5array(m_c5shapeax);
    c5array m_arrayStereo = c5array(m_c5shapest);
    boost::array<c3index, 3> m_c3shape =  {{ 40, 384, 9 }};
    boost::array<c3index, 3> m_c3shapeSec =  {{ 40, 84, 9 }};
    c3array m_houghPlane = c3array(m_c3shape);
    boost::array<c2index, 2> m_c2shapeHitMod =  {{ 2336, 3}};
    c2array m_arrayHitMod = c2array(m_c2shapeHitMod);
    boost::array<c5index, 5> m_c5shapeCompAx = {{ 41, 3, 40, 15, 1 }};
    boost::array<c5index, 5> m_c5shapeCompSt =  {{ 32, 3, 40, 15, 9 }};
    c5array m_compAxial = c5array(m_c5shapeCompAx);
    c5array m_compStereo = c5array(m_c5shapeCompSt);


    /** core */

    /** result: vector of the found tracks */
    std::vector<NDFinderTrack> m_NDFinderTracks;
    /** TS-Ids of the hits in the current event
     * elements: [0,2335] for 2336 TS in total*/
    std::vector<unsigned short> m_hitIds;
    /** priority positon within the TS in the current event
     * elements basf2: [0,3] first, left, right, no hit
     * elements stored: 3 - basf2prio*/
    std::vector<unsigned short> m_prioPos;
    /** orients
     * TS-Ids of the hits in the current event
     * elements: [0,2335] for 2336 TS in total*/
    std::vector<unsigned short> m_hitOrients;
    unsigned short m_nHits;
    ndparameters m_params;

    binning m_axbins;
    binning m_stbins;
    binning m_fullbins;
    binning m_compaxbins;
    binning m_compstbins;
    clusterer_params m_clusterer_params;

    unsigned short m_nbinsPhi32;
    unsigned short m_nbinsPhi1;

    Belle2::Clusterizend m_clusterer;

    std::vector<short> m_vecDstart;

  };
}
#endif

