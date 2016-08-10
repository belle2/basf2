/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDHOUGHTRACKINGMODULE_H
#define SVDHOUGHTRACKINGMODULE_H

#include <tracking/modules/SVDHoughTracking/SVDHoughClusterCand.h>
//#include <tracking/modules/SVDHoughTracking/SVDHoughCand.h>
//#include <tracking/modules/SVDHoughTracking/SVDHoughTrackCand.h>
//#include <tracking/modules/SVDHoughTracking/SVDHoughROI.h>
//#include <tracking/modules/SVDHoughTracking/SVDHoughRootOutput.h>


#include <tracking/dataobjects/SVDHoughCluster.h>
#include <tracking/dataobjects/SVDHoughTrack.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/dataobjects/RelationElement.h>
#include <boost/array.hpp>
#include <string>
#include <set>
#include <vector>
#include <fstream>

#include <root/TVector3.h>
#include <root/TVector2.h>
#include <root/TFile.h>
#include <root/TTree.h>
#include <root/TH1D.h>
#include <root/TH2D.h>
#include <root/TF1.h>

/* Structs needed for hough tracking interface */
struct coord {
  double x;
  double y;
  double z;
};

/* SVD id and coordinates of the hitted strip */
struct svd_hit {
  int strip_id;
  struct coord coord;
  struct svd_hit* next;
};

/* Track for track creation with radius and angle */
struct track_r {
  double r;
  double angle;
  double theta;
  struct coord offset;
};

/* Found tracks with hough coordinates */
struct found_tracks {
  int id;
  struct track_r track;
  struct found_tracks* next;
};

#define GPLOT_OPT "notitle with lines lt 1"
#define GPLOT_GEO_OPT "notitle with lines lt -1"
#define GPLOT_INIT_OPT "notitle with lines lt 2"

#define GPLOT_HEADER 1
#define GPLOT_PLOT 2
#define GPLOT_INPLOT 3
#define GPLOT_PLOTGEO 4
#define GPLOT_LINE 5
#define GPLOT_FOOTER 9

#define GPLOT_TMPL1 10

#define DEINEMUDDA 2000000


namespace Belle2 {
  namespace SVD {
    /**
     * Enum to flag R-Phi vs. Z strips.
     */
    enum StripDirection {
      stripRPhi = 0, /** for strips in R-Phi */
      stripZ = 1     /** for strips in Z     */
    };
    /**
     * Enum to flag charge carriers.
     */
    enum CarrierType {
      electron = -1, /** electrons */
      hole = +1      /** holes */
    };


    /** Container to sort the digits by strip number and time */
    typedef std::set<Sample> SensorSide;
    /** Container to hold the data of u and v strips of a sensor. */
    typedef boost::array<SensorSide, 2> Sensor;
    /** Structure to hold the data of all SVD sensors. */
    typedef std::map<VxdID, Sensor > Sensors;
    /** Iterator type for the map of sensors. */
    typedef std::map<VxdID, Sensor >::iterator SensorIterator;
    /** Cluster map */
    typedef std::map<int, TVector3> clusterMap;
    /** Cluster sensor map */
    typedef std::pair<VxdID, TVector3> sensorMap;
    /** SVD Clusters */
    /** Pair containing VxdID and corresponding position vector */
    typedef std::pair<VxdID, TVector3> svdClusterPair;
    /** Map containing integer ID and svdClusterPair */
    typedef std::map<int, svdClusterPair> svdClusterMap;
    /** Iterator of svdClusterMap */
    typedef std::map<int, svdClusterPair>::iterator svdClusterMapIter;
    /** Map containing integer ID and sensorMap */
    typedef std::map<int, sensorMap> clusterSensorMap;
    /** Hough Tuples */
    /** Pair containing VxdID ant TVector2 of TODO */
    typedef std::pair<VxdID, TVector2> houghPair;
    /** Map containing integer ID and corresponding houghPair for the HS TODO make this description better */
    typedef std::map<int, houghPair> houghMap;
    /** Pair of two TVector2 containing the lower left and upper right
     * corner of a Hough Space (sub) sector
     */
    typedef std::pair<TVector2, TVector2> coord2dPair;
    /** Pair containing integer ID and coord2dPair forming
     * a debug pair for HS visualization
     */
    typedef std::pair<unsigned int, coord2dPair> houghDbgPair;
    /** Hough Cluster
     * Pair of integer ID and TVector2 building up a cluster
     * in the HS
     */
    typedef std::pair<unsigned int, TVector2> houghCluster;


    //classes go here
    /*
     * Hough ROI class.
     */
    class SVDHoughROI {
    public:
      /** Constructor for hough ROI */
      SVDHoughROI(VxdID _sensorID, TVector2 _v1, TVector2 _v2): sensorID(_sensorID),
        v1(_v1), v2(_v2)
      {
      }

      ~SVDHoughROI() {}

      /** Get sensor ID */
      VxdID getSensorID() { return sensorID; }
      /** Get v1 and v2 */
      TVector2 getV1() { return v1; }
      TVector2 getV2() { return v2; }
    private:
      /** Sensor ID */
      VxdID sensorID;
      /** v1 (down left edge) and v2 (upper right edge) */
      TVector2 v1;
      TVector2 v2;
      /** Pixel IDs */
    };

    /*
     * Hough Candidates class.
     */
    class SVDHoughTrackCand {
    public:
      /** Constructor for hough candidates */
      SVDHoughTrackCand(std::vector<unsigned int>& _list, TVector2 _coord, bool _left = false): hitList(_list), coord(_coord), left(_left)
      {
        hash = 0;
        hitSize = 0;
        for (unsigned int i = 0; i < hitList.size(); ++i) {
          hash += hitList[i] + i * 10000000;
          ++hitSize;
        }
      }

      ~SVDHoughTrackCand() {}

      /** Get Index list */
      std::vector<unsigned int> getIdList() { return hitList; }

      /** Get Index list */
      TVector2 getCoord() { return coord; }

      /** Get Hash of hit list */
      unsigned int getHash() const { return hash; }

      /** Get Size of hit list */
      unsigned int getHitSize() const { return hitSize; }

      /** Get track orientation */
      bool getTrackOrientation() { return left; }

    private:
      /** ID list of points */
      std::vector<unsigned int> hitList;
      /** Coordinate of rectangle for this candidate */
      TVector2 coord;
      /** Hash for id list */
      unsigned int hash;
      unsigned int hitSize;
      /** Left handed track */
      bool left;
    };

    /*
     * Hough Candidates class.
     */
    class SVDHoughCand {
    public:
      /** Constructor for hough candidates */
      SVDHoughCand(std::vector<unsigned int>& _list, coord2dPair _coord, bool _left = false): hitList(_list), coord(_coord), left(_left)
      {
        hash = 0;
        hitSize = 0;
        for (unsigned int i = 0; i < hitList.size(); ++i) {
          hash += (hitList[i] + i * 10000000);
          ++hitSize;
        }
      }

      ~SVDHoughCand() {}

      /** Get Index list */
      std::vector<unsigned int> getIdList() { return hitList; }

      /** Get Index list */
      coord2dPair getCoord() { return coord; }

      /** Get Hash of hit list */
      unsigned int getHash() const { return hash; }

      /** Get Size of hit list */
      unsigned int getHitSize() const { return hitSize; }

      /** Get track orientation */
      bool getTrackOrientation() { return left; }

    private:
      /** ID list of points */
      std::vector<unsigned int> hitList;
      /** Coordinate of rectangle for this candidate */
      coord2dPair coord;
      /** Hash for id list */
      unsigned int hash;
      unsigned int hitSize;
      /** Left handed track */
      bool left;
    };

    inline bool operator<(const SVDHoughCand& a, const SVDHoughCand& b)
    {
      /*if (a.getHash() == b.getHash())  {
        B2DEBUG(1, "XXX a: " << a.getHitSize() << " " << b.getHash() << " b: " << b.getHash());
        B2DEBUG(1, "  true");
        return (true);
      } else {
        B2DEBUG(1, "  false");
        return (false);
      }*/
      return (a.getHash() > b.getHash());
    }


    /** SVDHoughTrackingModule class description */
    class SVDHoughTrackingModule : public Module {
    public:
      /** Constructor.  */
      SVDHoughTrackingModule();

      /** Standard Deconstructor */
      virtual ~SVDHoughTrackingModule();

      /** Process one SVDSimHit by dividing the step in smaller steps and drifting the charge */
      void processHit();

      /** Processing hits, save hits to TODO */
      void save_hits();

      /** Save hits for FPGA tests */
      void saveHits();

      /** Convert integer into verilog 32bit unsigned */
      char* convertInt(int32_t);

      /** Cluster strips */
      void clusterStrips();

      /** noise Filter */
      bool noiseFilter(float*, int, float*);

      /** Add cluster */
      void clusterAdd(VxdID, bool, short, int);

      /** Calculate Cluster position */
      double clusterPosition(double, short, int, double);

      /** Save Strips for FPGA LUT */
      void saveStrips();

      /** Helper function to convert and assign total strips to apv and rest strip */
      void writeStrip(VxdID, bool, short, float*, std::ofstream&);
      /** Helper function to convert and assign total strips to apv and rest strip */
      void writeStrip(VxdID, bool, short, float*);


      /** Convert TrueHits into the cluster format */
      void convertTrueHits();

      /** Convert SimHits into the cluster format */
      void convertSimHits();

      /** Create ROI */
      void createROI();

      /** Create PXD Pixel map with ROIs */
      void createPXDMap();

      /** Analyse ROI */
      void analyseROI();

      /** Mix True and Sim hits for background simulation */
      void mixTrueSimHits();

      /** Create ghosts hits */
      void createGhosts();

      /** Tracking Pipeline */
      void trackingPipeline();

      /** Initialize the module and check module parameters */
      virtual void initialize();
      /** Initialize variables, open root file... */
      virtual void beginRun();
      /** End run, mostly used for statistics */
      virtual void endRun();
      /** Run tracking */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

      /** Hough transformation */
      /** New Hough transformation of Christian Wessel with some major changes */
      void houghTrafo2d(svdClusterMap&, bool, bool);

      /** Intercept Finder functions */
      /** fastInterceptFinder2d uses iterative / recursive approach, only subdividing "active"
       * sectors with at least m_minimumLines of m_minimumLines different detector layers
       * passing through the sector
       */
      /** New fastInterceptFinder2d written by Christian Wessel, up-to-date */
      int fastInterceptFinder2d(houghMap&, bool, TVector2, TVector2, TVector2, TVector2,
                                unsigned int, unsigned int, unsigned int, std::vector<houghDbgPair>&,
                                unsigned int);
      /** FPGA-like intercept finder with all the sectors given / "festgelegt" a priori
       * so no subdivision of sectors is needed, but thus this intercept finder is slower
       * since all sectors have to be checked and not only active ones
       */
      int slowInterceptFinder2d(houghMap&, bool, std::vector<houghDbgPair>&, unsigned int);

      /** Caculate intersections of HS lines analytically after performing fast/slowInterceptFinder2d */
      void AnalyticalIntersection(std::vector<unsigned int>&, bool, std::vector<double>&);

      /** Layer filter, checking for hits from different SVD layers */
      bool layerFilter(bool*, unsigned int);

      /** Curling detection */
      bool curlingDetection(svdClusterMap&, std::vector<unsigned int>&);

      /** Curvature Estimation */
      int CurvatureEstimation(std::vector<unsigned int>&);

      /** Print Hough candidates */
      void printHoughCandidates();

      /** Functions to purify track candidates */
      /** Wrapper to select purify method */
      void purifyTrackCands();
      /** Purify track candidates by checking list of strip_id
       * (specific id calculated in this module)
       */
      void purifyTrackCandsList();

      /*
       * Empty purifier function only converting x_houghCand into x_houghTrackCand
       */
      void convertHoughCandToHoughTrackCand();

      /** Purify track candidates using clustering algorithm written by
       * Christian Wessel (older approach)
       */
      void purifyTrackCandsClusteringCW();
      /** Purify track candidates using clustering algorithm written by
       * Michael Schnell (newer approach)
       */
      void purifyTrackCandsClustering();
      /** Iterate over all clusters of active sectors in HS */
      unsigned int iterCluster(std::vector<SVDHoughCand>*, SVDHoughClusterCand*, coord2dPair);
      /** compare for equal hits / equal coord2dPair */
      bool hitsEqual(coord2dPair&, coord2dPair&);

      /** Compare strip_id lists / lists of counter variable */
      bool compareList(std::vector<unsigned int>&, std::vector<unsigned int>&);
      /** Merge strip_id lists / lists of counter variables to combine
       * large lists to smaller ones and thus purify track candidates
       * and avoid (too) many fakes
       */
      void mergeIdList(std::vector<unsigned int>&, std::vector<unsigned int>&, std::vector<unsigned int>&);

      /** Print Track candidates */
      void printTrackCandidates();

      /** Compare tracking performance with MCParticle */
      void trackAnalyseMCParticle();

      /** Track merger to merge SVDHoughTrackCand with similar properties
       * and to avoid / reduce number of fakes
       */
      void trackMerger();

      /** Find and Combine track candidates
       * Combine found values of phi and r from p-side HS
       * with found values of theta from n-side HS into track candidates
       */
      void fac3d();

      /** Save calculated Tracks as RecoTracks */
      void saveRecoTrack(RecoTrack& recoTrack, std::vector<unsigned int>& idList, bool p_side);

      /** Calculate track momentum and covariance matrix for the RecoTrack momentum
       * @param double r                    track radius
       * @param double phi                  track initial azimuthal angle
       * @param double theta                track initial polar angle
       * @param TVector3 momentum           track initial momentum, to be calculated
       * @param vector<double> covariance   covariance matrix as vector of length 9
       */
      void calculateMomentum(double r, double phi, double theta, TVector3& momentum, std::vector<double>& positionCovariance,
                             std::vector<double>& momentumCovariance);


      /** Save Hits to RecoTrack */
      void saveHitsToRecoTrack(RecoTrack* recoTrack, std::vector<unsigned int>& idList, bool p_side);

      /** Print tracks to terminal / stdout */
      void printTracks();

      /** TODO: weiß gerade nicht, was die Funktion genau macht */
      void print_num();

      /** Print summary at end of SVDHoughtrackingModule (obsolete?) */
      void printCustomSummary();

      /** Print Clusters */
      void printClusters(svdClusterMap&, bool);

      /** TODO: weiß gerade nicht, was diese Funktion machen soll */
      void clustering(bool);
      /** Create sector map with ROI and extrapolated hits TODO: Correct?*/
      void create_sector_map();
      sensorMap get_next_hit(sensorMap, unsigned short, bool*);
      int sector_track_fit(sensorMap*);

      /** Extrapolation functions */
      /** Obsolete / non-working pxdExtrapolation function */
      void pxdExtrapolation();
      /** Extrapolation for phi only, "subfunction" of pxdExtrapolationFullCW
       * and not used except for debugging of phi extrapolation (=>obsolete)
       */
      void pxdExtrapolationPhi();
      /** Extrapolation for theta only, "subfunction" of pxdExtrapolationFullCW
       * and not used except for debugging of theta extrapolation (=>obsolete)
       */
      void pxdExtrapolationTheta();
      /** Old full extrapolation written by Michael Schnell
       * Extrapolation here only works for negatively charged particles
       * Obsolete and replaced by pxdExtrapolationFullCW
       */
      void pxdExtrapolationFullMS();
      /** Full extrapolation of svdHoughTracks to PXD, most up-to-date */
      void pxdExtrapolationFullCW();
      /** No idea why only for testing, what does this function do and what is it used for? */
      void pxdTestExtrapolationPhi();
      /** No idea what this function should do, probably obsolete */
      void pxdSingleExtrapolation();
      /** Extrapolation using straight tracks (for TB only?) => inaccurate/wrong and obsolete */
      void pxdStraightExtrapolation();
      /** What is this used for? Should be similar to analyse functions below */
      void createResiduals();
      /** Same as last function, but for TB only */
      void createTbResiduals();
      /** TODO: Weiß gerade nicht, was die Funktion macht */
      void analyseClusterStrips();
      /** Analyse extrapolated hits (obsolete?) */
      void analyseExtrapolatedHits();
      /** Analyse extrapolated hits for phi only, goes with pxdExtrapolationPhi and thus obsolete */
      void analyseExtrapolatedPhi();
      /** Full analysis of extrapolated hits, most up-to-date */
      void analyseExtrapolatedFull();

      /** Plot hough lines in gnuplot */
      void houghTrafoPlot(bool);

      /** Write rect dbg values to gnuplot file */
      /** Write initial HS dbg data (rectangles) to dbg file */
      void gplotRect(const std::string, std::vector<houghDbgPair>&);
      /** Write clustering result rectangles to dbg file */
      void gplotRect(const std::string , std::vector<SVDHoughCand>&);

      /** Functions to create gnuplot files */
      /** Pointer to output file function */
      FILE* gplotCreate(const char*, char*, int);
      /** Set Options for gnuplot debugging file */
      void gplotSetOpt(FILE*, const char*);
      /** Load file / options for gnuplot debugging file */
      void gplotSetLoad(FILE*, const char*);
      /** Insert debugging stuff TODO */
      void gplotInsert(FILE*, const char*, int);
      /** Close gnuplot debugging output file */
      void gplotClose(FILE*, int);

      /** Purify tracks for both sides.  */
      bool hashSort(const SVDHoughCand& a, const SVDHoughCand& b) const
      {
        if (a.getHitSize() == b.getHitSize() && a.getHash() == b.getHash())  {
          return (true);
        } else {
          return (false);
        }
      }

    protected:

      /** Members holding module parameters: */

      /** 1. Collections */
      /** Name of the collection for the MCParticles */
      std::string m_storeMCParticlesName;
      /** Name of the collection for the SVDTrueHits */
      std::string m_storeSVDTrueHitsName;
      /** Name of the collection for the SVDSimhits */
      std::string m_storeSVDSimHitsName;
      /** Name of the collection for the SVDDigits */
      std::string m_storeSVDDigitsName;
      /** Name of the collection for the SVDCluster */
      std::string m_storeSVDClusterName;
      /** Name of the collection for the SVDDigits */
      std::string m_storePXDDigitsName;
      /** Name of the collection for the SVDSimhits */
      std::string m_storePXDSimHitsName;
      /** Name of the collection for the SVDTrueHits */
      std::string m_storePXDTrueHitsName;
      /** Name of the collection for the SVD Clusters */
      std::string m_storeHoughCluster;
      /** Name of the collection for the SVD Hough tracks */
      std::string m_storeHoughTrack;
      /** Name of the collection for the Extrapolated Hits */
      std::string m_storeExtrapolatedHitsName;

      /** Name of the relation between MCParticles and SVDSimHits */
      std::string m_relMCParticleSimHitName;
      /** Name of the relation between SVDDigits and MCParticles */
      std::string m_relDigitMCParticleName;
      /** Name of the relation between SVDTrueHits and SVDSimHits */
      std::string m_relTrueHitSimHitName;
      /** Name of the relation between SVDDigits and SVDTrueHits */
      std::string m_relDigitTrueHitName;
      /** Name of the RecoTrack array */
      std::string m_recoTracksStoreArrayName;

      /** 2. VXD Output */
      /** TODO */
      std::string m_svdPosName;
      /** TODO */
      std::string m_svdStripName;

      /** Standalone Analysis of data */
      bool m_StandAloneAnalysis;

      /** 3. Options */
      //TODO: write default values in variable descriptions
      /** Clustering of incoming data from SVD */
      /** Use SVDHoughtrackingModule SVD clustering */
      bool m_useClusters;
      /** Use clustering like performed on FPGA (obsolete?) */
      bool m_useFPGAClusters;
      /** Analyse those FPGA-like cluster */
      bool m_analyseFPGAClusters;
      /** Use TrueHitClusters */
      bool m_useTrueHitClusters;
      /** Use SimHitClusters */
      bool m_useSimHitClusters;
      /** Save hits to file */
      bool m_saveHits;
      /** Save strips to file
       * For now (01062016) only used in TB simulations
       */
      bool m_saveStrips;

      bool m_countStrips;

      bool m_useAllStripCombinations;

      /** Tracking */
      /** Run the full tracking pipeline */
      bool m_fullTrackingPipeline;
      /** Use radius filter for merging / purifying tracks / track candidates
       * TODO: make better description
       */
      bool m_useRadiusFilter;
      /** Use purifying algortihm at all */
      bool m_usePurifier;
      /** Use hashPurify function */
      bool m_useEmptyPurify;
      /** Use clustering based purifier function */
      bool m_useClusteringPurify;
      /** TODO */
      int m_minHoughClusters;
      /** TODO */
      double m_maxClusterSizeX;
      /** TODO */
      double m_maxClusterSizeY;
      /** Use TrackMerger to merge found tracks (candidates) to avoid / reduce fakes */
      bool m_useTrackMerger;
      /** Use TrackMerger for p-side tracks (candidates) */
      bool m_useTrackMergerP;
      /** Use TrackMerger for n-side tracks (candidates) */
      bool m_useTrackMergerN;
      /** Global track merger merging threshold */
      double m_mergeThreshold;
      /** Merging threshold for n-side trackMerger */
      double m_mergeThresholdP;
      /** Merging threshold for p-side trackMerger */
      double m_mergeThresholdN;
      /** Calculate interceations in HS analytically */
      bool m_analyticalIntersection;
      /** Use only phi in track finding and combining (fac3d) and analysis of found tracks */
      bool m_usePhiOnly;
      /** Use only theta in track finding and combining (fac3d) and analysis of found tracks */
      bool m_useThetaOnly;
      /** Compare / analyse found tracks with MCParticle tracks */
      bool m_compareMCParticle;
      /** Print analysed values to stdout */
      bool m_compareMCParticleVerbose;
      /** Use TB mapping / reconstruction (default: false) */
      bool m_tbMapping;
      /** Use straight tracks (obsolete) (default: false) */
      bool m_straightTracks;
      /** Tolerance for phi for analysing tracks and accept a track as being found (default: 1deg) */
      double m_phiTolerance;
      /** Tolerance for theta for analysing tracks and accept a track as being found (default: 2deg) */
      double m_thetaTolerance;
      /** Tolerance for pT for analysing tracks and accept a track as being found (default: yet none) */
      double m_pTTolerance;
      /** Tolerance for track radius for analysing tracks and accept a track as being found (default: yet none) */
      double m_radiusTolerance;

      /** ROIs */
      /** Create ROI (default: TODO) */
      bool m_createROI;
      /** Create PXDMap (default: TODO) */
      bool m_createPXDMap;
      /** Analyse calculated ROI (default: TODO) */
      bool m_analyseROIVerbose;
      /** Perform extrapolation to PXD (default: false) */
      bool m_PXDExtrapolation;
      /** Perform extrapolation to PXD for TB mode (default: false) */
      bool m_PXDTbExtrapolation;
      /** Perform extrapolation to PXD for phi (default: TODO) */
      bool m_usePhiExtrapolation;
      /** Perform extrapolation to PXD for theta (default: TODO) */
      bool m_useThetaExtrapolation;
      /** If using fixed ROI sizes: size of ROI in u-direction (default: 70) */
      int m_fixedSizeUCells;
      /** If using fixed ROI sizes: size of ROI in v-direction (default: 30) */
      int m_fixedSizeVCells;

      /** Verbose */
      /** Print Track Info to stdout (TODO: improve this) */
      bool m_printTrackInfo;
      /** Print end-of-run SVDHoughtrackingModule statistics to stdout (default: true) */
      bool m_printStatistics;

      /** Statistics and ROOT output */
      /** Name of statistics file */
      std::string m_statisticsFileName;
      /** Name of root file */
      std::string m_rootFilename;

      /** 4. Noise options */
      /** Disable NoiseFilter */
      bool m_disableNoiseFilter;
      /** TODO */
      unsigned int m_noiseFactor;
      /** Upper noise threshold */
      unsigned int m_noiseUpThreshold;
      /** Lower noise threshold */
      unsigned int m_noiseDownThreshold;

      /** 5. Hough Trafo Variables */
      /** Use conformal transformation for p-side (default: true) */
      bool m_conformalTrafoP;
      /** Use conformal transformation for n-side (default: false) */
      bool m_conformalTrafoN;
      /** Use old projection HS reconstruction for n-side (for debugging and comparison) (default: false) */
      bool m_projectionRecoN;
      /** Use conformal transformation with (x',y') = (x,y)/r^2 */
      bool m_xyHoughPside;
      /** Use conformal transformation with (r, phi0) */
      bool m_rphiHoughPside;
      /** Minimum number of lines required for an active sector (default: 3) */
      unsigned int m_minimumLines;
      /** Number of iterations performed for n-side (default: 7) */
      unsigned int m_critIterationsN;
      /** Number of iterations performed for p-side (default: 7) */
      unsigned int m_critIterationsP;
      /** Maximum number of iterations allowed for n-side (obsolete?) (default: 12) */
      unsigned int m_maxIterationsN;
      /** Maximum number of iterations allowed for p-side (obsolete?) (default: 12) */
      unsigned int m_maxIterationsP;
      /** Use tracking with independent sectors in HS like it is possible to do on FPGA
       * (with slowInterceptFinder2d)
       * Independent means: number of sectors is not necessary equal to 2^n with
       * n=1,2,3,... number of iteration steps and number of sectors for horizontal and
       * vertical axis in HS does not have to be equal but different
       * (default: false)
       */
      bool m_independentSectors;
      /** Using independent sectors: Number of horizontal sectors for p-side (default: 256) */
      unsigned int m_angleSectorsP;
      /** Using independent sectors: Number of horizontal sectors for n-side (default: 256) */
      unsigned int m_angleSectorsN;
      /** Using independent sectors: Number of vertical sectors for p-side (default: 256) */
      unsigned int m_vertSectorsP;
      /** Using independent sectors: Number of vertical sectors for n-side (default: 256) */
      unsigned int m_vertSectorsN;
      /** Radius threshold for merging tracks TODO: is this correct? */
      double m_radiusThreshold;
      /** For fastInterceptFinder2d: vertical size of HS for n-side (default: 0.1) */
      double m_rectSizeN;
      /** For fastInterceptFinder2d: vertical size of HS for p-side (default: 0.1) */
      double m_rectSizeP;
      double m_rectXP1;
      double m_rectXP2;
      double m_rectXN1;
      double m_rectXN2;
      /** Write HS lines to file (for debugging) */
      bool m_writeHoughSpace;
      /** Write HS sectors to file (for debugging) */
      bool m_writeHoughSectors;
      /** Use SensorFilter / LayerFilter TODO: rename variable and replace where used */
      bool m_useSensorFilter;

      /** Various output streams */
      /** Output file for TODO */
      std::ofstream of_pos;
      /** Output file for firing strips (today (01062016) only used in TB mode) */
      std::ofstream of_strip;
      /** Statistics output stream */
      std::ofstream statout;


      // Other data members:
      /** Store peak samples for center of gravity */
      float peak_samples[8];

      /** Total time for Hough Tracking pipeline */
      double totClockCycles;

      /** Structure containing signals in all existing sensors */
      /** For PXD */
      Sensors pxd_sensors;
      /** For SVD */
      Sensors svd_sensors;

      /** Clusters */
      /** SVD p-side clusters */
      svdClusterMap p_clusters;
      /** SVD n-side clusters */
      svdClusterMap n_clusters;

      /** Hough Map */
      /** p-side HoughMap */
      houghMap p_hough;
      /** n-side HoughMap */
      houghMap n_hough;

      /** Hough Candidates */
      /** p-side */
      std::vector<SVDHoughCand> p_houghCand;
      /** n-side */
      std::vector<SVDHoughCand> n_houghCand;

      /** Purified Hough Candidates */
      /** p-side */
      std::vector<SVDHoughTrackCand> p_houghTrackCand;
      /** n-side */
      std::vector<SVDHoughTrackCand> n_houghTrackCand;

      /** Extrapolated local hits */
      std::vector<houghPair> extrapolatedHits;

      /** ROIs */
      std::vector<SVDHoughROI> pxdROI;

      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo;

      /** Current tracking efficency */
      /** Total */
      double curTrackEff;
      /** n-side */
      double curTrackEffN;
      /** p-side */
      double curTrackEffP;
      /** Total number of fake tracks */
      double ntotFakeTracks;
      /** Total number of tracks TODO: richtig? */
      unsigned int ntotTracks;
      /** All tracks TODO: Unterschied zu ntotTracks */
      unsigned int allTracks;
      /** Run number (= event number) */
      unsigned int runNumber;
      /** Number of valid events processed */
      unsigned int validEvents;
      /** Event number for module ROOT-File */
      unsigned int evtNumber;
      /** Number of MCParticles for module ROOT-File */
      unsigned int MCParticleNumber;
      /** Number of HoughTracks for module ROOT-File */
      unsigned int HoughTrackNumber;
      /** Number of extrapolated Hits for module ROOT-File */
      unsigned int ExtrapolatedHitNumber;

      /** Variables for ROI performance estimation */
      /** Current ROI performance efficiency */
      double curROIEff;
      /** number of TrueHits inside ROI */
      unsigned int ntotROITrueHits;
      /** number of SimHits inside ROI */
      unsigned int ntotROISimHits;
      /** number of current hits in ROI (probably redundant) */
      unsigned int ncurHitsInROIs;
      /** total number of all active pixel on PXD */
      unsigned int ntotPXDactivePixel;
      /** total number of active pixel inside ROI (TrueHit or SimHit) */
      unsigned int ntotPXDinsideROIPixel;
      /** Event ROI effficiency */
      double eventROIEff;
      /** Event DRF */
      double eventDRF;

      /** Const for testbeam radius */
      double dist_thres[3];
      double tb_radius[2];


      /************************************ ROOT stuff ****************************************/
      /** Basics */
      /** Pointer to the ROOT filename for statistics */
      TFile* m_rootFile;
      /** TTree for the data */
      TTree* m_rootTreeTracking;
      TTree* m_rootTreeExtHitAna;
      TTree* m_rootTreeMCData;
      TTree* m_rootTreeHoughData;
      TTree* m_rootTreeExtrapolatedHits;
      TTree* m_rootTreeROI;
      TTree* m_rootTreeEfficiency;

      /*********************************** Histograms *************************************/
      /** Histogram for Cluster spread position */
      TH1D* m_histClusterU;
      TH1D* m_histClusterV;
      TH1D* m_histTrueClusterU;
      TH1D* m_histTrueClusterV;

      /** Event efficiency */
      TH1D* m_histEventEfficiency;
      /** Event fake rate */
      TH1D* m_histEventFakeRate;
      /** Event ROI efficency */
      TH1D* m_histEventROIEfficiency;
      /** Event DRF */
      TH1D* m_histEventDRF;


      /** Hough Trafo Histogram Naming Convention
       * ...Houghxxx: xxx gained from Hough Transformation results, contains all values gained
       * ...MCxxx: xxx from MCParticle datastore
       * ...vsxxx: xxx is on horizontal axis, independent of 1D or 2D histogram
       * ...CorrectRecon...: There is a track with both phi and theta being found,
       *    meaning fabs(MCangle - Houghangle) < angleThreshold for angle = phi, theta (falsche beschreibung!!!)
       * Efficiency histograms (m_hist...Eff... ): m_histCorrectRecon... / m_histMC...
       */

      /** Histograms for direct HoughTrafo results and MC comparison */
      TH1D* m_histMCTrackR ;
      TH1D* m_histHoughTrackR;
      TH1D* m_histHoughTrackRCorrectRecon;
      TH1D* m_histCorrectReconHoughTrackR;
      TH1D* m_histCorrectReconMCTrackR;
      TH1D* m_histCorrectReconTrackR;

      TH1D* m_histHoughTrackREff;
      TH1D* m_histMCTrackREff;
      TH1D* m_histCorrectReconTrackREff;

      TH1D* m_histMCCurvatureSign;
      TH1D* m_histHoughCurvatureSign;
      TH1D* m_histCorrectReconMCCurvatureSign;
      TH1D* m_histCorrectReconHoughCurvatureSign;
      TH1D* m_histCorrectReconCurvatureSign;
      TH1D* m_histCurvatureSignEff;

      TH1D* m_histMCCharge;
      TH1D* m_histHoughCharge;
      TH1D* m_histCorrectReconMCCharge;
      TH1D* m_histCorrectReconHoughCharge;
      TH1D* m_histCorrectReconCharge;
      TH1D* m_histChargeEff;


      /** PT Histograms */
      /** Histogram for MCPT-distribution */
      TH1D* m_histMCPTDist;
      TH1D* m_histHoughPTDist;
      TH1D* m_histCorrectReconMCPTDist;
      TH1D* m_histCorrectReconHoughPTDist;
      TH1D* m_histCorrectReconPTDist;
      TH1D* m_histEffHoughPTvsMCPT;
      /** Histogram for correctly reconstructed tracks vs MCPT */
      TH1D* m_histHoughPhivsMCPTDist;
      TH1D* m_histHoughThetavsMCPTDist;

      //TH1D* m_histCorrectReconHoughvsMCPTDist;              // For total efficiency as function of PT
      //TH1D* m_histCorrectReconHoughPhivsMCPTDist;           // For efficency of the reconstruction of Phi as function of pT
      //TH1D* m_histCorrectReconHoughThetavsMCPTDist;         // For efficency of the reconstruction of Theta as function of pT
      TH1D* m_histCorrectReconvsMCPTDist;              // For total efficiency as function of PT
      TH1D* m_histCorrectReconPhivsMCPTDist;           // For efficency of the reconstruction of Phi as function of pT
      TH1D* m_histCorrectReconThetavsMCPTDist;         // For efficency of the reconstruction of Theta as function of pT
      //TH1D* m_histPTEffvsMCPT;                              // Total efficiency vs PT
      //TH1D* m_histHoughPhiEffvsMCPT;                        // Efficency of the reconstruction of Phi as function of pT
      //TH1D* m_histHoughThetaEffvsMCPT;                      // Efficency of the reconstruction of Theta as function of pT
      TH1D* m_histPTEffvsMCPT;                         // Total efficiency vs PT
      TH1D* m_histPhiEffvsMCPT;                        // Efficency of the reconstruction of Phi as function of pT
      TH1D* m_histThetaEffvsMCPT;                      // Efficency of the reconstruction of Theta as function of pT

      /** Phi Histograms */
      /** Histogram for MCPhi-distribution */
      TH1D* m_histMCPhiDist;
      TH1D* m_histHoughPhiDist;
      TH1D* m_histCorrectReconMCPhiDist;
      TH1D* m_histCorrectReconHoughPhiDist;
      TH1D* m_histCorrectReconPhiDist;
      TH1D* m_histEffHoughPhivsMCPhi;
      /** Histogram for correctly reconstructed tracks in Phi */
      //TH1D* m_histHoughPhiReconDist;      // diese und nächste Zeile stehen für das gleiche Histogramm
      //TH1D* m_histHoughPhiReconvsMCPhi;   // aber ich glaube gerade, dass ich das so nicht brauche, 11032016


      /** Theta Histograms */
      /** Histogram for MCTheta-distribution */
      TH1D* m_histMCThetaDist;
      TH1D* m_histHoughThetaDist;
      TH1D* m_histCorrectReconMCThetaDist;
      TH1D* m_histCorrectReconHoughThetaDist;
      TH1D* m_histCorrectReconThetaDist;
      TH1D* m_histEffHoughThetavsMCTheta;
      /** Histogram for correctly reconstructed tracks in Theta */
      //TH1D* m_histHoughThetaReconvsMCTheta;
      //TH1D* m_histHoughThetaCorrectReconvsMCTheta;
      //TH1D* m_histHoughThetaReconDist;
      //TH1D* m_histThetaReconDistvsMCPhi;
      //TH1D* m_histHoughThetaReconvsMCPhi;
      //TH1D* m_histHoughThetaCorrectReconvsMCPhi;
      TH1D* m_histProjectedThetaDist;
      TH1D* m_histProjectedThetaRecon;
      TH1D* m_histEffProjectedTheta;

      TH1D* m_histHoughThetavsMCPhiDist;
      TH1D* m_histCorrectReconHoughThetavsMCPhiDist;
      TH1D* m_histHoughThetaEffvsMCPhi;

      /** Plots for Resolution of Hough Transformation results */
      TH1D* m_histHoughDiffPhi;
      TH1D* m_histHoughDiffTheta;
      TH2D* m_histHoughDiffPhivsPhi;
      TH2D* m_histHoughDiffThetavsTheta;

      /* Hough angle alpha vs MC angle beta or vice versa */
      TH2D* m_histHoughPhivsMCPhi2D_all;
      TH2D* m_histHoughPhivsMCPhi2D_reco;
      TH2D* m_histHoughThetavsMCTheta2D_all;
      TH2D* m_histHoughThetavsMCTheta2D_reco;
      TH2D* m_histHoughPhivsMCTheta2D_all;
      TH2D* m_histHoughPhivsMCTheta2D_reco;
      TH2D* m_histHoughThetavsMCPhi2D_all;
      TH2D* m_histHoughThetavsMCPhi2D_reco;

      /** Histogram for 2D distribution and effficiency of Theta vs Phi, CWessel 29.10.2015 */
      TH2D* m_histHoughThetaDistvsMCPhiDist2D;
      TH2D* m_histMCThetaCorrectReconvsMCPhi2D;
      TH2D* m_histHoughThetaCorrectReconvsMCPhi2D;
      TH2D* m_histThetaCorrectReconvsMCPhi2D;
      TH2D* m_histEffMCThetavsMCPhi2D;
      TH2D* m_histEffHoughThetavsMCPhi2D;

      /** Efficiency Histograms */
      /** Histogram for efficiency vs MCPT */
      /** Histogram for efficiency vs Phi */
      /** Histogram for efficiency vs Theta */


      /** Fake and Missed Hits Histograms */
      /** Histogram for fake rate vs pT*/
      TH1D* m_histFakesinPTvsMCPT;
      TH1D* m_histFakesinPhivsMCPhi;
      TH1D* m_histFakesinThetavsMCTheta;
      //TH1D* m_histFakesinPhivsMCPT;
      //TH1D* m_histFakesinThetavsMCPT;
      /** Average Fake histograms */
      TH1D* m_histAverageFakesvsMCPT;
      TH1D* m_histAverageFakesvsMCPhi;
      TH1D* m_histAverageFakesvsMCTheta;
      /** Missed hit distribution */
      TH1D* m_histMissedHitsvsMCPT;
      TH1D* m_histMissedHitsvsMCPhi;
      TH1D* m_histMissedHitsvsMCTheta;

      /** ROI stuff */
      /** Histogram for ROI spread */
      TH1D* m_histROIy;
      TH1D* m_histROIz;
      TH1D* m_histDist;
      TH1I* m_histSimHits;
      /** Histograms for ROIs */
      /** Total */
      TH1D* m_histROIDiffPhi;     // deviation of calculated pixel from MC pixel in r-phi in cm
      TH1D* m_histROIDiffPhiPx;     // deviation of calculated pixel from MC pixel in r-phi in pixel
      TH1D* m_histROIDiffTheta;     // deviation of calculated pixel from MC pixel in theta in cm
      TH1D* m_histROIDiffThetaPx;   // deviation of calculated pixel from MC pixel in theta in pixel
      TH2D* m_histROIDiffPhiTheta;
      TH2D* m_histROIDiffPhiThetaPx;
      TH2D* m_histROIDiffPTPhi;
      TH2D* m_histROIDiffPTPhiPx;
      TH2D* m_histROIDiffPTTheta;
      TH2D* m_histROIDiffPTThetaPx;
      /** PXD Layer 1 */
      TH1D* m_histROIDiffPhi_pxdl1;     // deviation of calculated pixel from MC pixel in r-phi in cm
      TH1D* m_histROIDiffPhiPx_pxdl1;     // deviation of calculated pixel from MC pixel in r-phi in pixel
      TH1D* m_histROIDiffTheta_pxdl1;     // deviation of calculated pixel from MC pixel in theta in cm
      TH1D* m_histROIDiffThetaPx_pxdl1;   // deviation of calculated pixel from MC pixel in theta in pixel
      TH2D* m_histROIDiffPhiTheta_pxdl1;
      TH2D* m_histROIDiffPhiThetaPx_pxdl1;
      TH2D* m_histROIDiffPTPhi_pxdl1;
      TH2D* m_histROIDiffPTPhiPx_pxdl1;
      TH2D* m_histROIDiffPTTheta_pxdl1;
      TH2D* m_histROIDiffPTThetaPx_pxdl1;
      /** PXD Layer 2 */
      TH1D* m_histROIDiffPhi_pxdl2;     // deviation of calculated pixel from MC pixel in r-phi in cm
      TH1D* m_histROIDiffPhiPx_pxdl2;     // deviation of calculated pixel from MC pixel in r-phi in pixel
      TH1D* m_histROIDiffTheta_pxdl2;     // deviation of calculated pixel from MC pixel in theta in cm
      TH1D* m_histROIDiffThetaPx_pxdl2;   // deviation of calculated pixel from MC pixel in theta in pixel
      TH2D* m_histROIDiffPhiTheta_pxdl2;
      TH2D* m_histROIDiffPhiThetaPx_pxdl2;
      TH2D* m_histROIDiffPTPhi_pxdl2;
      TH2D* m_histROIDiffPTPhiPx_pxdl2;
      TH2D* m_histROIDiffPTTheta_pxdl2;
      TH2D* m_histROIDiffPTThetaPx_pxdl2;
      /** Histograms with absolute deviations */
      /*      TH1D* m_histROIDiffPhiabs;    // deviation of calculated pixel from MC pixel in r-phi in cm
            TH1D* m_histROIDiffPhiPxabs;    // deviation of calculated pixel from MC pixel in r-phi in pixel
            TH1D* m_histROIDiffThetaabs;    // deviation of calculated pixel from MC pixel in theta in cm
            TH1D* m_histROIDiffThetaPxabs;    // deviation of calculated pixel from MC pixel in theta in pixel
            TH2D* m_histROIDiffPhiThetaabs;
            TH2D* m_histROIDiffPhiThetaPxabs;
            TH2D* m_histROIDiffPTabs;
            TH2D* m_histROIDiffPTPhiabs;
            TH2D* m_histROIDiffPTPhiPxabs;
            TH2D* m_histROIDiffPTThetaabs;
            TH2D* m_histROIDiffPTThetaPxabs;              */
      /** Histograms for PXD occupancy plots */
      TH2D* m_histROIPXD_l1;
      TH2D* m_histROIPXD_l2;


      /*********************************** End of Histograms *************************************/


      /************* Create TTree variables and TTree Branches ***********************/
      /** Event efficiency */
      double m_treeEventEfficiency;
      /** Event fake rate */
      double m_treeEventFakeRate;
      /** Event ROI efficency */
      double m_treeEventROIEfficiency;
      /** Event DRF */
      double m_treeEventDRF;
      int m_treeEventEventNo;


      /** MC Particle Data */
      int m_treeMCDataEventNo;
      int m_treeMCDataEventParticleNo;
      int m_treeMCDataPDG;
      int m_treeMCDataCharge;
      double m_treeMCDataPhi;
      double m_treeMCDataTheta;
      double m_treeMCDataPx;
      double m_treeMCDataPy;
      double m_treeMCDataPz;
      double m_treeMCDataPT;
      double m_treeMCDataVx;
      double m_treeMCDataVy;
      double m_treeMCDataVz;

      /** Hough Track Data */
      int m_treeHoughDataEventNo;
      int m_treeHoughDataEventTrackNo;
      int m_treeHoughDataCharge;
      int m_treeHoughDataCurvSign;
      double m_treeHoughDataPhi;
      double m_treeHoughDataTheta;
      double m_treeHoughDataRadius;

      /** Extrapolated Hits Data */
      int m_treeExtHitEventNo;
      int m_treeExtHitHitNo;
      /** total information */
      double m_treeExtHitTheta;
      double m_treeExtHitX;
      double m_treeExtHitY;
      int m_treeExtHitLayerNo;
      int m_treeExtHitLadderNo;
      int m_treeExtHitSensorNo;
      double m_treeExtHitPosX;
      double m_treeExtHitPosY;
      double m_treeExtHitPosZ;
      double m_treeExtHitPosU;
      double m_treeExtHitPosV;
      int m_treeExtHitUCell;
      int m_treeExtHitVCell;
      /** layer 1 */
      double m_treeExtHitTheta_l1;
      double m_treeExtHitX_l1;
      double m_treeExtHitY_l1;
      int m_treeExtHitLadderNo_l1;
      int m_treeExtHitSensorNo_l1;
      double m_treeExtHitPosX_l1;
      double m_treeExtHitPosY_l1;
      double m_treeExtHitPosZ_l1;
      double m_treeExtHitPosU_l1;
      double m_treeExtHitPosV_l1;
      int m_treeExtHitUCell_l1;
      int m_treeExtHitVCell_l1;
      /** layer 1 */
      double m_treeExtHitTheta_l2;
      double m_treeExtHitX_l2;
      double m_treeExtHitY_l2;
      int m_treeExtHitLadderNo_l2;
      int m_treeExtHitSensorNo_l2;
      double m_treeExtHitPosX_l2;
      double m_treeExtHitPosY_l2;
      double m_treeExtHitPosZ_l2;
      double m_treeExtHitPosU_l2;
      double m_treeExtHitPosV_l2;
      int m_treeExtHitUCell_l2;
      int m_treeExtHitVCell_l2;

      /** Track Analysis Data */
      int m_treeTrackingEventNo = 0;
      int m_treeExtHitAnaEventNo = 0;
      /** TTree variables for Cluster spread position */
//      double m_treeClusterU;
//      double m_treeClusterV;
//      double m_treeTrueClusterU;
//      double m_treeTrueClusterV;
      /** Plots for Hough transformation */
      double m_treeHoughDiffPhi;
      double m_treeHoughDiffTheta;

      /** MCParticle information */
      int m_treeMCPDG_all;
      double m_treeMCVertexX_all;
      double m_treeMCVertexY_all;
      double m_treeMCVertexZ_all;
      double m_treeMCMomentumX_all;
      double m_treeMCMomentumY_all;
      double m_treeMCMomentumZ_all;
      int m_treeMCPDG;
      double m_treeMCVertexX;
      double m_treeMCVertexY;
      double m_treeMCVertexZ;
      double m_treeMCMomentumX;
      double m_treeMCMomentumY;
      double m_treeMCMomentumZ;

      /** TTree variables for MC variables and Hough / reconstructed variables */
      double m_treeMCTrackR;
      double m_treeHoughTrackR;
      double m_treeHoughTrackRCorrectRecon;
      double m_treeCorrectReconHoughTrackR;
      double m_treeCorrectReconMCTrackR;
      double m_treeCorrectReconTrackR;

      int m_treeMCCurvatureSign;
      int m_treeHoughCurvatureSign;
      int m_treeCorrectReconMCCurvatureSign;
      int m_treeCorrectReconHoughCurvatureSign;
      int m_treeCorrectReconCurvatureSign;

      int m_treeMCCharge;
      int m_treeHoughCharge;
      int m_treeCorrectReconMCCharge;
      int m_treeCorrectReconHoughCharge;
      int m_treeCorrectReconCharge;

      double m_treeMCPTDist;
      double m_treeHoughPTDist;
      double m_treeCorrectReconMCPTDist;
      double m_treeCorrectReconHoughPTDist;
      double m_treeCorrectReconPTDist;

      double m_treeMCPhiDist;
      double m_treeHoughPhiDist;
      double m_treeCorrectReconMCPhiDist;
      double m_treeCorrectReconHoughPhiDist;
      double m_treeCorrectReconPhiDist;

      double m_treeMCThetaDist;
      double m_treeHoughThetaDist;
      double m_treeCorrectReconMCThetaDist;
      double m_treeCorrectReconHoughThetaDist;
      double m_treeCorrectReconThetaDist;

      double m_treeMissedPT;
      double m_treeMissedPhi;
      double m_treeMissedTheta;
      /** Secondary variables */
      double m_treeHoughPhivsMCPTDist;
      double m_treeHoughThetavsMCPTDist;
      double m_treeTrackRRecon;             // Filled if phi and theta where correctly reconstructed
      double m_treeTrackRCorrectRecon;      // Filled if radius R was correctly reconstructed
      /** TTree variables for pT-distribution */
      double m_treePTDist;                  // equivalent to m_treeMCPT
      /** TTree variables for correctly reconstructed tracks in pT */
      double m_treePTPhiRecon;
      double m_treePTThetaRecon;
      double m_treePTRecon;
      /** TTree variables for Phi-distribution */
      double m_treePhiDist;                 // equivalent to m_treeMCPhi
      /** TTree variables for correctly reconstructed tracks in Phi */
      double m_treePhiRecon;
      /** TTree variables for Theta-distribution */
      double m_treeThetaDist;               // equivalent to m_treeMCTheta
      double m_treeProjectedThetaDist;
      double m_treeThetaReconPhiDist;
      /** TTree variables for efficiency of theta reco / reco in theta vs phi */
      double m_treeHoughThetavsMCPhiDist;
      double m_treeCorrectReconHoughThetavsMCPhiDist;
      /** TTree variables for correctly reconstructed tracks in Theta */
      double m_treeThetaRecon;
      double m_treeProjectedThetaRecon;
      /** TTree variables for fake rate vs pT */
      double m_treeFakes;
      double m_treePTFake;
      double m_treePTFakePhi;
      double m_treePTFakeTheta;
      double m_treeFakePhi;
      double m_treeFakeTheta;
      /** Average Fake treeograms */
      double m_treeAverageFakes;
      double m_treeAverageFakesPT;
      double m_treeAverageFakesPhi;
      double m_treeAverageFakesTheta;


      /** TTree variabless for Extrapolated Hit Analysis */
      double m_treeExtHitAnaDiffPhi;      // deviation of calculated pixel from MC pixel in r-phi in cm
      double m_treeExtHitAnaDiffPhiPx;    // deviation of calculated pixel from MC pixel in r-phi in pixel
      double m_treeExtHitAnaDiffTheta;    // deviation of calculated pixel from MC pixel in theta in cm
      double m_treeExtHitAnaDiffThetaPx;    // deviation of calculated pixel from MC pixel in theta in pixel
      double m_treeExtHitAnaDiffPTPhi;
      double m_treeExtHitAnaDiffPTPhiPx;
      double m_treeExtHitAnaDiffPTTheta;
      double m_treeExtHitAnaDiffPTThetaPx;
      double m_treeExtHitAnaPTDist;
      /** PXD Layer 1 */
      double m_treeExtHitAnaDiffPhi_pxdl1;      // deviation of calculated pixel from MC pixel in r-phi in cm
      double m_treeExtHitAnaDiffPhiPx_pxdl1;    // deviation of calculated pixel from MC pixel in r-phi in pixel
      double m_treeExtHitAnaDiffTheta_pxdl1;    // deviation of calculated pixel from MC pixel in theta in cm
      double m_treeExtHitAnaDiffThetaPx_pxdl1;    // deviation of calculated pixel from MC pixel in theta in pixel
      double m_treeExtHitAnaDiffPTPhi_pxdl1;
      double m_treeExtHitAnaDiffPTPhiPx_pxdl1;
      double m_treeExtHitAnaDiffPTTheta_pxdl1;
      double m_treeExtHitAnaDiffPTThetaPx_pxdl1;
      double m_treeExtHitAnaPTDist_pxdl1;
      /** PXD Layer 2 */
      double m_treeExtHitAnaDiffPhi_pxdl2;      // deviation of calculated pixel from MC pixel in r-phi in cm
      double m_treeExtHitAnaDiffPhiPx_pxdl2;    // deviation of calculated pixel from MC pixel in r-phi in pixel
      double m_treeExtHitAnaDiffTheta_pxdl2;    // deviation of calculated pixel from MC pixel in theta in cm
      double m_treeExtHitAnaDiffThetaPx_pxdl2;    // deviation of calculated pixel from MC pixel in theta in pixel
      double m_treeExtHitAnaDiffPTPhi_pxdl2;
      double m_treeExtHitAnaDiffPTPhiPx_pxdl2;
      double m_treeExtHitAnaDiffPTTheta_pxdl2;
      double m_treeExtHitAnaDiffPTThetaPx_pxdl2;
      double m_treeExtHitAnaPTDist_pxdl2;
      /** TTree variabless with absolute deviations */
      //double m_treeExtHitAnaDiffPhiabs;   // deviation of calculated pixel from MC pixel in r-phi in cm
      //double m_treeExtHitAnaDiffPhiPxabs;   // deviation of calculated pixel from MC pixel in r-phi in pixel
      //double m_treeExtHitAnaDiffThetaabs;   // deviation of calculated pixel from MC pixel in theta in cm
      //double m_treeExtHitAnaDiffThetaPxabs;   // deviation of calculated pixel from MC pixel in theta in pixel

      /************************************ End of ROOT stuff ****************************************/



      /* List of coord SVD clusters */
      clusterMap cluster_map;
      /* List of PXD clusters for extrapolation crosscheck */
      clusterMap pxd_cluster_map;
      clusterSensorMap svd_sensor_cluster_map, svd_real_cluster_map;

      /* Idx counters */
      int p_idx;
      int n_idx;
      int n_idxX;
      int n_idxY;

      /* Min and Max r for FPGA lookup table */
      double minR, maxR;

      /* Color for rectangle */
      /*const std::string rectColor[17] = {"aliceblue", "antiquewhite", "blue",
                    "brown", "chartreuse", "coral", "cyan", "gold", "greenyellow", "magenta", "magenta",
                    "pink", "skyblue", "violet", "green", "blue", "orange"};*/
      const std::string rectColor[8] = {"blue", "cyan", "green", "yellow", "orange", "violet", "skyblue", "pink"};

      std::string gplot_text = "set title \"2D tracks\" \n"
                               "\n"
                               "set xrange [-1e-0:1e-0] \n"
                               "set yrange [-1e-0:1e-0]\n";

      std::string gplot_text2 = "set notitle \n"
                                "\n"
                                "# Style options \n"
                                "set style line 80 lt rgb \"#808080\"\n"
                                "set style line 81 lt 0\n"
                                "set style line 81 lt rgb \"#808080\"\n"
                                "\n"
                                "# Lines\n"
                                "set style line 1 lt rgb \"#A00000\" lw 1 pt 1\n"
                                "set style line 2 lt rgb \"#00A000\" lw 1 pt 6\n"
                                "set style line 3 lt rgb \"#000000\" lw 1 pt 6\n"
                                "\n"
                                "# Points\n"
                                "#set style points 3 lt rgb \"#0000A0\" pt 1\n"
                                "\n"
                                "# Description position\n"
                                "set key top right\n"
                                "\n"
                                "# Grid and border style\n"
                                "set grid back linestyle 81\n"
                                "set border 3 back linestyle 80\n"
                                "\n"
                                "# No mirrors\n"
                                "set xtics nomirror\n"
                                "set ytics nomirror\n"
                                "\n"
                                "# Encoding\n"
                                "set encoding utf8\n"
                                "\n"
                                "set xlabel \"x\"\n"
                                "set ylabel \"y\"\n"
                                "\n"
                                "#set xrange [-0.2:0.2]\n"
                                "#set yrange [-0.2:0.2]\n"
                                "\n"
                                "\n";

      std::string gplot_tmpl1 = "set notitle \n"
                                "\n"
                                "# Style options \n"
                                "set style line 80 lt rgb \"#808080\"\n"
                                "set style line 81 lt 0\n"
                                "set style line 81 lt rgb \"#808080\"\n"
                                "\n"
                                "# Lines\n"
                                "set style line 1 lt rgb \"#A00000\" lw 1 pt 1\n"
                                "set style line 2 lt rgb \"#00A000\" lw 1 pt 6\n"
                                "set style line 3 lt rgb \"#000000\" lw 1 pt 6\n"
                                "set style line 13 lt rgb 'black' lw 1 pt 6\n"
                                "set style line 14 lt rgb 'yellow' lw 1 pt 6\n"
                                "set style line 15 lt rgb 'green' lw 1 pt 6\n"
                                "set style line 16 lt rgb 'red' lw 1 pt 6\n"
                                "\n"
                                "# Points\n"
                                "#set style points 3 lt rgb \"#0000A0\" pt 1\n"
                                "\n"
                                "# Description position\n"
                                "set key top right\n"
                                "\n"
                                "# Grid and border style\n"
                                "set grid back linestyle 81\n"
                                "set border 3 back linestyle 80\n"
                                "\n"
                                "# No mirrors\n"
                                "set xtics nomirror\n"
                                "set ytics nomirror\n"
                                "\n"
                                "# Encoding\n"
                                "set encoding utf8\n"
                                "\n"
                                "set xlabel \"x\"\n"
                                "set ylabel \"y\"\n"
                                "set xrange[-pi/2:pi/2]\n"
                                "\n"
                                "\n";

      std::string gplot_geo = "plot [0:2*pi] 0.038 * cos(t), 0.038 * sin(t) notitle with lines linestyle 3\n"
                              "replot 0.080 * cos(t), 0.080 * sin(t) notitle with lines linestyle 3\n"
                              "replot 0.115 * cos(t), 0.115 * sin(t) notitle with lines linestyle 3\n"
                              "replot 0.140 * cos(t), 0.140 * sin(t) notitle with lines linestyle 3\n"
                              "\n";

    };//end class declaration


  } // end namespace SVD
} // end namespace Belle2

#endif // SVDHoughtrackingModule_H

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */
