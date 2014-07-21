/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell                                          *
 *                                                                        *
 **************************************************************************/

#ifndef SVDHoughtrackingModule_H
#define SVDHoughtrackingModule_H

#include <framework/core/Module.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/simulation/SVDSignal.h>
#include <svd/geometry/SensorInfo.h>
#include <svd/reconstruction/Sample.h>
#include <pxd/geometry/SensorInfo.h>
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

/* Forward declaration */
extern "C" struct found_tracks* basf2_tracking_pipeline(struct svd_hit*, int);

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
    typedef std::pair<VxdID, TVector3> svdClusterPair;
    typedef std::map<int, svdClusterPair> svdClusterMap;
    typedef std::map<int, svdClusterPair>::iterator svdClusterMapIter;
    typedef std::map<int, sensorMap> clusterSensorMap;
    /** Hough Tuples */
    typedef std::pair<VxdID, TVector2> houghPair;
    typedef std::map<int, houghPair> houghMap;
    typedef std::pair<TVector2, TVector2> coord2dPair;
    typedef std::pair<unsigned int, coord2dPair> houghDbgPair;

    /*
     * Hough ROI class.
     */
    class SVDHoughROI {
    public:
      /** Constructor for hough ROI */
      SVDHoughROI(VxdID _sensorID, TVector2 _v1, TVector2 _v2): sensorID(_sensorID),
        v1(_v1), v2(_v2) {
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
      SVDHoughTrackCand(std::vector<unsigned int>& _list, TVector2 _coord): hitList(_list), coord(_coord) {
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
    private:
      /** ID list of points */
      std::vector<unsigned int> hitList;
      /** Coordinate of rectangle for this candidate */
      TVector2 coord;
      /** Hash for id list */
      unsigned int hash;
      unsigned int hitSize;
    };

    /*
     * Hough Candidates class.
     */
    class SVDHoughCand {
    public:
      /** Constructor for hough candidates */
      SVDHoughCand(std::vector<unsigned int>& _list, coord2dPair _coord): hitList(_list), coord(_coord) {
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
    private:
      /** ID list of points */
      std::vector<unsigned int> hitList;
      /** Coordinate of rectangle for this candidate */
      coord2dPair coord;
      /** Hash for id list */
      unsigned int hash;
      unsigned int hitSize;
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

    class SVDHoughtrackingModule : public Module {
    public:
      /** Constructor.  */
      SVDHoughtrackingModule();

      /** Process one SVDSimHit by dividing the step in smaller steps and drifting the charge */
      void processHit();

      /* Processing hits */
      void save_hits();

      /** Cluster strips */
      void clusterStrips();

      /** noise Filter */
      bool noiseFilter(float*, int, float*);

      /** Add cluster */
      void clusterAdd(VxdID, bool, short, int);

      /** Calculate Cluster position */
      double clusterPosition(double, short, int, double);

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
      void houghTrafo2d(svdClusterMap&, bool, bool);

      /** Fast intercept finder */
      int fastInterceptFinder2d(houghMap&, bool, TVector2, TVector2, TVector2, TVector2,
                                unsigned int, unsigned int, unsigned int, std::vector<houghDbgPair>&,
                                unsigned int);

      /** Layer filter */
      bool layerFilter(bool*, unsigned int);

      /** Print Hough candidates */
      void printHoughCandidates();

      /** Purify track candidates */
      void purifyTrackCands(); /* Wrapper to select purify method */
      void purifyTrackCandsList();
      void purifyTrackCandsHash();
      void oldpurifyTrackCands();
      bool compareList(std::vector<unsigned int>&, std::vector<unsigned int>&);
      void mergeIdList(std::vector<unsigned int>&, std::vector<unsigned int>&, std::vector<unsigned int>&);

      /** Print Track candidates */
      void printTrackCandidates();

      /** Compare tracking performance with MCParticle */
      void trackAnalyseMCParticle();

      /** Track merger */
      void trackMerger();

      /** Find and Combine track candidates */
      void fac3d();

      /** Print tracks */
      void printTracks();

      void print_num();

      /** Print Clusters */
      void printClusters(svdClusterMap&, bool);

      void clustering(bool);
      void create_sector_map();
      sensorMap get_next_hit(sensorMap, unsigned short, bool*);
      int sector_track_fit(sensorMap*);
      void fullHoughTracking();
      /** Extrapolation */
      void pxdExtrapolation();
      void pxdExtrapolationPhi();
      void pxdExtrapolationTheta();
      void pxdExtrapolationFull();
      void pxdTestExtrapolationPhi();
      void pxdSingleExtrapolation();
      void pxdStraightExtrapolation();
      void createResiduals();
      void createTbResiduals();
      void analyseClusterStrips();
      void analyseExtrapolatedHits();
      void analyseExtrapolatedPhi();
      void analyseExtrapolatedFull();

      /** Plot hough lines in gnuplot */
      void houghTrafoPlot(bool);

      /** Write rect dbg values to gnuplot file */
      void gplotRect(const std::string, std::vector<houghDbgPair>&);

      /** Functions to create gnuplot files */
      FILE* gplotCreate(const char*, char*, int);
      void gplotSetOpt(FILE*, const char*);
      void gplotSetLoad(FILE*, const char*);
      void gplotInsert(FILE*, const char*, int);
      void gplotClose(FILE*, int);

      /** Purify tracks for both sides.  */
      bool hashSort(const SVDHoughCand& a, const SVDHoughCand& b) const {
        if (a.getHitSize() == b.getHitSize() && a.getHash() == b.getHash())  {
          return (true);
        } else {
          return (false);
        }
      }

    protected:

      // Members holding module parameters:

      // 1. Collections
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
      /** Name of root file */
      std::string m_statisticsFileName;
      /** Name of root file */
      std::string m_rootFilename;


      // 2. VXD Output
      std::string m_svdPosName;
      std::string m_svdStripName;
      std::ofstream of_pos;
      std::ofstream of_strip;

      /** Statistics output stream */
      std::ofstream statout;

      // 3. Options
      /* Clustering */
      bool m_useClusters;
      bool m_useFPGAClusters;
      bool m_analyseFPGAClusters;
      bool m_useTrueHitClusters;
      bool m_useSimHitClusters;
      /* Tracking */
      bool m_fullTrackingPipeline;
      bool m_writeHoughSpace;
      bool m_writeHoughSectors;
      bool m_useSensorFilter;
      bool m_useRadiusFilter;
      bool m_useHashPurify;
      bool m_useTrackMerger;
      bool m_usePhiOnly;
      bool m_useThetaOnly;
      bool m_compareMCParticle;
      bool m_compareMCParticleVerbose;
      bool m_usePhiExtrapolation;
      bool m_useThetaExtrapolation;
      /* ROIs */
      bool m_createROI;
      bool m_createPXDMap;
      bool m_analyseROIVerbose;
      bool m_PXDExtrapolation;
      bool m_PXDTbExtrapolation;
      /* Verbose */
      bool m_printTrackInfo;
      bool m_printStatistics;

      // 4. Noise options
      unsigned int m_noiseFactor;
      unsigned int m_noiseUpThreshold;
      unsigned int m_noiseDownThreshold;

      // 5. Hough Trafo
      unsigned int m_minimumLines;
      unsigned int m_critIterationsN;
      unsigned int m_critIterationsP;
      unsigned int m_maxIterationsN;
      unsigned int m_maxIterationsP;
      double m_radiusThreshold;
      double m_rectSizeN;
      double m_rectSizeP;
      double m_rectScaleN;
      double m_rectScaleP;
      double m_mergeThreshold;

      // Other data members:
      /** Store peak samples for center of gravity */
      float peak_samples[8];

      /** Total time for Hough Tracking pipeline */
      double totClockCycles;

      /** Structure containing signals in all existing sensors */
      Sensors pxd_sensors;
      Sensors svd_sensors;

      /** Clusters */
      svdClusterMap p_clusters;
      svdClusterMap n_clusters;

      /** Hough Map */
      houghMap p_hough;
      houghMap n_hough;

      /** Hough Candidates */
      std::vector<SVDHoughCand> n_houghCand;
      std::vector<SVDHoughCand> p_houghCand;

      /** Purified Hough Candidates */
      std::vector<SVDHoughTrackCand> n_houghTrackCand;
      std::vector<SVDHoughTrackCand> p_houghTrackCand;

      /** Extrapolated local hits */
      std::vector<houghPair> extrapolatedHits;

      /* ROIs */
      std::vector<SVDHoughROI> pxdROI;

      /** Pointer to the SensorInfo of the current sensor */
      const SensorInfo*  m_currentSensorInfo;

      /** Current tracking efficency */
      double curTrackEff, curTrackEffN, curTrackEffP;
      double totFakeTracks;
      unsigned int totTracks;
      unsigned int runNumber;

      /** Current ROI performance efficiency */
      double curROIEff;
      unsigned int totROITrueHits;
      unsigned int curHitsInROIs;

      /** Const for testbeam radius */
      double dist_thres[3];
      double tb_radius[2];

      // ROOT stuff
      /** Pointer to the ROOT filename for statistics */
      TFile* m_rootFile;
      /** Histogram for Cluster spread position */
      TH1D* m_histClusterU;
      TH1D* m_histClusterV;
      TH1D* m_histTrueClusterU;
      TH1D* m_histTrueClusterV;
      /** Plots for Hough transformation */
      TH1D* m_histHoughDiffPhi;
      TH1D* m_histHoughDiffTheta;
      /** Histogram for ROI spread */
      TH1D* m_histROIy;
      TH1D* m_histROIz;
      TH1D* m_histDist;
      TH1I* m_histSimHits;

      /** Histogram for pT-distribution */
      TH1D* m_histPtDist;
      /** Histogram for correctly reconstructed tracks in pT */
      TH1D* m_histPtPhiRecon;
      TH1D* m_histPtThetaRecon;
      /** Histogram for efficiency vs pT (only in phi while reconstruction in theta is not proved) */
      TH1D* m_histPtEffPhi;
      TH1D* m_histPtEffTheta;
      /** Histogram for Phi-distribution */
      TH1D* m_histPhiDist;
      /** Histogram for correctly reconstructed tracks in Phi */
      TH1D* m_histPhiRecon;
      /** Histogram for efficiency vs Phi */
      TH1D* m_histEffPhi;
      /** Histogram for Theta-distribution */
      TH1D* m_histThetaDist;
      TH1D* m_histProjectedThetaDist;
      TH1D* m_histThetaPhiDist;
      /** Histogram for correctly reconstructed tracks in Theta */
      TH1D* m_histThetaRecon;
      TH1D* m_histProjectedThetaRecon;
      /** Histogram for efficiency vs Theta */
      TH1D* m_histEffTheta;
      TH1D* m_histEffProjectedTheta;
      TH1D* m_histEffThetaPhi;
      /** Histogram for fake rate vs pT */
      TH1D* m_histPtFakePhi;
      TH1D* m_histFakePhi;
      TH1D* m_histFakeTheta;
      /** Histogram for fake rate vs pT*/
      TH1D* m_histPtFake;
      /** Missed hit distribution */
      TH1D* m_histMissedTheta;
      TH1D* m_histMissedPhi;
      /** Average Fake histograms */
      TH1D* m_histPtAverageFake;
      TH1D* m_histPhiAverageFake;
      TH1D* m_histThetaAverageFake;

      /** Histograms for ROIs */
      TH1D* m_histROIDiffPhi;
      TH1D* m_histROIDiffPhiPx;
      TH1D* m_histROIDiffTheta;
      TH1D* m_histROIDiffThetaPx;
      /** Histograms for PXD occupancy plots */
      TH2D* m_histROIPXD_l1;
      TH2D* m_histROIPXD_l2;

      /* List of coord SVD clusters */
      clusterMap cluster_map;
      /* List of PXD clusters for extrapolation crosscheck */
      clusterMap pxd_cluster_map;
      clusterSensorMap svd_sensor_cluster_map, svd_real_cluster_map;

      /* Idx counters */
      int p_idx;
      int n_idx;

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
                                "\n"
                                "\n";

      std::string gplot_geo = "plot [0:2*pi] 0.038 * cos(t), 0.038 * sin(t) notitle with lines linestyle 3\n"
                              "replot 0.080 * cos(t), 0.080 * sin(t) notitle with lines linestyle 3\n"
                              "replot 0.115 * cos(t), 0.115 * sin(t) notitle with lines linestyle 3\n"
                              "replot 0.140 * cos(t), 0.140 * sin(t) notitle with lines linestyle 3\n"
                              "\n";

    };//end class declaration

    /** @}*/

  } // end namespace SVD
} // end namespace Belle2

#endif // SVDHoughtrackingModule_H

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */
