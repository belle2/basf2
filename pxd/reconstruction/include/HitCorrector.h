/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Peter Kvasnicka, Peter Kodys                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef PXD_HITCORRECTOR_H
#define PXD_HITCORRECTOR_H

#include <pxd/dataobjects/PXDCalibrationMap.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <string>
#include <memory>
#include <fstream>
#include <functional>

namespace Belle2 {
  /** Namespace to provide code needed by PXD */
  namespace PXD {
    /**
    * Singleton class that provides hit reconstruction corrections.
    *
    * @see PXDClusterShape
    * @see PXDClusterShapeCalibration
    * @see PXDClusterShapeCalibrationAlgorithm
    * @see pxdMergeClusterShapeCorrections
    * @see PXDDQMClusterShape
    *
    */
    class HitCorrector {

    public:

      /** Clean up internal structures */
      void clear();

      /** Read correction data and initialize the corrector.
      * @filename Name of data file containing correction data.
      */
      void initialize(std::string filename);
      /** Read correction data from DataBase and initialize the corrector.
      */
      void initialize();

      /** If the name of log file is defined, HitCorrectorr records all corrections in a log file. .
      * @filename Name of log file
      */
      void setLogFile(std::string filename) { m_logFileName = filename; }

      /** Get initialization status of the corrector.
       * Optimally, could be used to switch off correcting altogether.
       */
      bool isInitialized() const { return m_isInitialized; }

      /** Return position-corrected cluster.
      * NB:
      * This interface allows corrections to be chained, and the internals are kept within the corrector class.
      * NB:
      * Create a temporary cluster to avoid accumulating corrections
      * Like this:
      * PXDCluster correctedCluster(*this->getCluster()); // "this" = PXDRecoHit
      * correctedCluster =
      *   PXD::HitCorrector::getInstance().correctCluster(correctedCluster, tu, tv);
      * correctedCluster is a local object and dies with its scope.
      * @param cluster Pointer to the cluster to be corrected
      * @param tu Track direction in u-coordinate
      * @param tv Track direction in v-coordinate
      * @return Corrected cluster
      */
      PXDCluster& correctCluster(PXDCluster& cluster, double tu, double tv);

      /** Main (and only) way to access the HitCorrectior. */
      static HitCorrector& getInstance();

      /** destructor to clean up structures */
      ~HitCorrector() { clear(); }

      /** Return type of corrector function */
      typedef std::function<PXDCluster&(PXDCluster&, double, double)> corrector_function_type;

      /** Return corrector function for this corrector */
      corrector_function_type getCorrectorFunction()
      {
        return ([this](PXDCluster & cluster, double tu, double tv)  -> PXDCluster& {
          return correctCluster(cluster, tu, tv);
        });
      }

    private:
      /** Packed position and error covariance corrections.
       * The ittems are bias_u, bias_v, error_u, error_v, rho_uv
       */
      typedef std::tuple<double, double, double, double, double> position_correction_type;

      /** Singleton class, hidden constructor */
      HitCorrector() {};
      /** Singleton class, forbidden copy constructor */
      HitCorrector(const HitCorrector&) = delete;
      /** Singleton class, forbidden assignment operator */
      HitCorrector& operator=(const HitCorrector&) = delete;

      /** Function to calculate position/error corrections
      * TODO:
      * Toto je samozauzlovacie. Naco mam prechadzat zakazdym cez funkciu
      * plnu ifov, ked chcem vzdy to iste?
      *
      * by interpolation on a rectangular mesh of values.
      * Return value for position u,v.
      * @param type Type of values:
      * 0: Bias of position (m_CorrectionBiasMap)
      * 1: Correction of error estimate (m_CorrectionErrorEstimationMap)
      * 2: Correction of error Covariance estimate (m_CorrectionErrorEstimationCovarianceMap)
      * 3: Error of position bias estimate (m_CorrectionBiasMapErr)
      * @param direction Direction in which we want correction: 0: u, 1: v.
      * @param shape Index of shape kind.
      * @param pixelKind Index of pixel kind.
      * @param iu Lowest index in u direction of matrix.
      * @param iv Lowest index in v direction of matrix.
      * @param u Normalized position in u direction in range iu .. iu + 1.
      * @param v Normalized position in v direction in range iv .. iv + 1.
      */
      double GetWeightedParameter(int type, int direction, int shape,
                                  int pixelKind, int iu, int iv, double u, double v);
      // TODO: Predsa takuto funkciu je treba:
      // double getFromMap(PXDCalibrationMap& source, int direction,
      // int shape, int pixelkind, int iu, int iv, double u, double v);

      /** Flag to indicate if  the calibration maps have been properly initialized. Set to true by a successful call to initialize(), otherwise false. */
      bool m_isInitialized = false;
      /** Name of log file name. If "", no logging takes place.*/
      std::string m_logFileName = "";
      /** Log file object */
      std::ofstream m_logFile;
      /** Number of shapes in current calibration file */
      int m_shapes = 0;     // 1 .. 15, 0: shape not set
      /** Number of pixel kinds in current calibration file */
      int m_pixelkinds = 0;
      /** Number of calibration settings in current file */
      int m_dimensions = 0; // cases: 15 x 8 x 2 = 240
      /** Size of u-angles grid */
      int m_anglesU = 0;    // 18 x 18 = 324 (angles)
      /** Size of v-angles grid */
      int m_anglesV = 0;
      /** Size of in-pixel u coordinate grid */
      int m_in_pixelU = 0;  // 7 x 7 = 49 (in-pixel positions)
      /** Size of in-pixel v coordinate grid */
      int m_in_pixelV = 0;

      /** Map holding corrections of position estimate biases */
      PXDCalibrationMap m_CorrectionBiasMap;
      /** Map holding corrections of position error estimates */
      PXDCalibrationMap m_CorrectionErrorEstimateMap;
      /** Map holding corrections of error Covariance estimates */
      PXDCalibrationMap m_CorrectionErrorEstimateCovarianceMap;
      /** Map holding estimates of bias correction errors */
      PXDCalibrationMap m_CorrectionBiasMapErr;
    };
  }
} //Belle2 namespace
#endif
