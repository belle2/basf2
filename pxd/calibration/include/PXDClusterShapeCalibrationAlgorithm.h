/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Peter Kodys                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <pxd/reconstruction/PXDClusterShape.h>
#include <iostream>
#include <stdio.h>
#include <tuple>
#include <unordered_map>
#include <TCut.h>
#include <TVectorD.h>
#include <boost/functional/hash.hpp>
#include <framework/gearbox/Unit.h>


namespace Belle2 {

  /**
    * Type definition for working with tuple structure for PXD cluster shape corrections:
    * shape ID, pixelkind ID, dimension (u or v), angle ID in u, angle ID in v
    */
  typedef std::tuple<short, short, short, short, short> key_type;
  /**
    * Type definition for working with tuple structure for PXD cluster shape corrections:
    * shape ID, pixelkind ID, angle ID in u, angle ID in v, in-pixel ID in u, in-pixel ID in v
    */
  typedef std::tuple<short, short, short, short, short, short> key_type2;

  /**
    * Structure for working with tuple structure for PXD cluster shape corrections:
    * shape ID, pixelkind ID, dimension (u or v), angle ID in u, angle ID in v
    * @see key_type
    */
  struct key_hash : public std::unary_function<key_type, std::size_t> {
    /** operator() returns hash for key k.
      * @param k The key for which we need a hash
      * @return Hash for key k.
      */
    std::size_t operator()(const key_type& k) const
    {
      using boost::hash_value;
      using boost::hash_combine;
      std::size_t seed = 0;

      hash_combine(seed, hash_value(std::get<0>(k)));
      hash_combine(seed, hash_value(std::get<1>(k)));
      hash_combine(seed, hash_value(std::get<2>(k)));
      hash_combine(seed, hash_value(std::get<3>(k)));
      hash_combine(seed, hash_value(std::get<4>(k)));
      return seed;
    }
  };

  /**
    * Structure for working with tuple structure for PXD cluster shape corrections:
    * shape ID, pixelkind ID, angle ID in u, angle ID in v, in-pixel ID in u, in-pixel ID in v
    * @see key_type2
    */
  struct key_hash2 : public std::unary_function<key_type2, std::size_t> {
    /** operator() returns hash for key k.
      * @param k The key for which we need a hash
      * @return Hash for key k.
      */
    std::size_t operator()(const key_type2& k) const
    {
      using boost::hash_value;
      using boost::hash_combine;
      std::size_t seed2 = 0;

      hash_combine(seed2, hash_value(std::get<0>(k)));
      hash_combine(seed2, hash_value(std::get<1>(k)));
      hash_combine(seed2, hash_value(std::get<2>(k)));
      hash_combine(seed2, hash_value(std::get<3>(k)));
      hash_combine(seed2, hash_value(std::get<4>(k)));
      hash_combine(seed2, hash_value(std::get<5>(k)));
      return seed2;
    }
  };

  /**
    * Structure for working with tuple structure for PXD cluster shape corrections:
    * Comparing of two elements of array with content:
    * shape ID, pixelkind ID, dimension (u or v), angle ID in u, angle ID in v
    * @see key_type
    */
  struct key_equal : public std::binary_function<key_type, key_type, bool> {
    /** operator() returns true if keys v0 and v1 are equal.
      * @param v0 First key for comparing
      * @param v1 Second key for comparing
      * @return true if keys v0 and v1 are equal.
      */
    bool operator()(const key_type& v0, const key_type& v1) const
    {
      return (
               std::get<0>(v0) == std::get<0>(v1) &&
               std::get<1>(v0) == std::get<1>(v1) &&
               std::get<2>(v0) == std::get<2>(v1) &&
               std::get<3>(v0) == std::get<3>(v1) &&
               std::get<4>(v0) == std::get<4>(v1)
             );
    }
  };

  /**
    * Structure for working with tuple structure for PXD cluster shape corrections:
    * Comparing of two elements of array with content:
    * shape ID, pixelkind ID, angle ID in u, angle ID in v, in-pixel ID in u, in-pixel ID in v
    * @see key_type2
    */
  struct key_equal2 : public std::binary_function<key_type2, key_type2, bool> {
    /** operator() returns true if keys v0 and v1 are equal.
      * @param v0 First key for comparing
      * @param v1 Second key for comparing
      * @return true if keys v0 and v1 are equal.
      */
    bool operator()(const key_type2& v0, const key_type2& v1) const
    {
      return (
               std::get<0>(v0) == std::get<0>(v1) &&
               std::get<1>(v0) == std::get<1>(v1) &&
               std::get<2>(v0) == std::get<2>(v1) &&
               std::get<3>(v0) == std::get<3>(v1) &&
               std::get<4>(v0) == std::get<4>(v1) &&
               std::get<5>(v0) == std::get<5>(v1)
             );
    }
  };

  /**
    * Type definition for working with tuple structure for PXD cluster shape corrections:
    * Creating of map with content:
    * shape ID, pixelkind ID, dimension (u or v), angle ID in u, angle ID in v
    */
  typedef std::unordered_map<key_type, double, key_hash, key_equal> map_typeCorrs;
  /**
    * Type definition for working with tuple structure for PXD cluster shape corrections:
    * Creating of map with content:
    * shape ID, pixelkind ID, angle ID in u, angle ID in v, in-pixel ID in u, in-pixel ID in v
    */
  typedef std::unordered_map<key_type2, double, key_hash2, key_equal2> map_typeInPics;

  /**
    * Class implementing PXD cluster shape calibration algorithm
    * @calibrate loop over all data with filters for individual cases:
    * shape ID, pixelkind ID, dimension (u or v), angle ID in u, angle ID in v
    * shape ID, pixelkind ID, angle ID in u, angle ID in v, in-pixel ID in u, in-pixel ID in v
    * For every case are calculate bias of position and error estimation corrections.
    * On calibrate are fill histograms for statistics of corrections and they are storing to histo-file.
    *
    * @see PXDClusterShape
    * @see PXDClusterShapeCalibration
    * @see pxdMergeClusterShapeCorrections
    * @see PXDDQMClusterShape
    *
    */
  class PXDClusterShapeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor
    PXDClusterShapeCalibrationAlgorithm();

    /// Destructor
    virtual ~PXDClusterShapeCalibrationAlgorithm() {}

    /// Set external parameters and functions

    /** 1 .. 15, shapeID = 0: not setting shape, @see PXDClusterShape */
    const int m_shapes = 15;
    /** kinds of pixels: sensors: 2x (in foreward and backward), pitch: 4x */
    const int m_pixelkinds = 8;
    /** dimension in local u, v, cases: 15 x 8 x 2 = 240 */
    const int m_dimensions = 2;
    /** 18 x 18 = 324 (angles in u x v) */
    const int m_anglesU = 18;
    /** 18 x 18 = 324 (angles in u x v) */
    const int m_anglesV = 18;
    /** 7 x 7 = 49 (in-pixel positions in u x v) */
    const int m_in_pixelU = 7;
    /** 7 x 7 = 49 (in-pixel positions in u x v) */
    const int m_in_pixelV = 7;
    /** Minimum clusters for calculation of corrections */
    const int m_MinClustersCorrections = 110;

    /** Setting of minimum of hits in pixel for analysis, TODO set to > 5000, deactivate this function: set to -1 */
    const int m_MinHitsInPixels = -1;
    /** Setting of minimum of hits in bin for acceptance, TODO set to > 5 */
    const int m_MinHitsAcceptInPixels = 5;
    /** 15 x 8 x 2 = 240 (shapes x pixelkinds x dimensions): bias corr., error estim. corr., in-pixel maps)) */
    const int m_cases = m_shapes * m_pixelkinds * m_dimensions;

    /** Expert setting, production of monitoring histograms, default=False */
    void setDoExpertHistograms(bool DoExpertHistograms) { m_DoExpertHistograms = DoExpertHistograms; }

    /** To use real data without simulations or simulations, default=False */
    void setUseRealData(bool UseRealData) { m_UseRealData = UseRealData; }
    /** Set pixel kind (pixel size) for corrections, range: -1: any, 0..3 <-> 55, 60, 70, 85 um, default=-1 */
    void setPixelKind(int PixelKind) { m_UsePixelKind = PixelKind; }
    /** Set EE correction calculate from 0: true EE, 1: EE from Track EE (real data), 2: EE from Track residuals, default=0 */
    void setStoreEECorrectionType(int StoreEECorrectionType) { m_StoreEECorrectionType = StoreEECorrectionType; }


    /** For conversion from table of corrections to vector format, for calibration for bias and error estimations */
    void Table2Vector(map_typeCorrs& TCorrection, TVectorD* CorVector);
    /** For conversion from table of corrections to vector format, for calibration for in-pixel level corrections */
    void Table2Vector(map_typeInPics& TInPix, TVectorD* InPixVector);

    /** For conversion from vector corrections to table format, for calibration for bias and error estimations */
    void Vector2Table(TVectorD* CorVector, map_typeCorrs& TCorrection);
    /** For conversion from vector corrections to table format, for calibration for in-pixel level corrections */
    void Vector2Table(TVectorD* InPixVector, map_typeInPics& TInPix);

    /** get angle Phi of index base on split */
    double getPhiFromIndex(int indPhi);
    /** get angle Theta of index base on split */
    double getThetaFromIndex(int indTheta);
    /** get index of angle Phi base on split */
    int getPhiIndex(float phi);
    /** get index of angle Theta base on split */
    int getThetaIndex(float theta);

    /** Calculate mean value and its error from array of values.
     * Calculates trimmed (5% on both tails) values of mean and standard deviation.
     * For obtaining of the best realistic value of distribution use correction factor for RMS:
     * Corr = 0.79, because we cut out 10% of full distribution.
     * Hardcoded CorCase parameter:
     *   0: General:         no conditions, always return
     *   1: Bias:            Minimal Correction = 0.5 micron,
     *                       factor of minimal distance in estimated error unit from 0 = 3.0
     *   2: ErrorEstimation  Minimal Correction = 0.05 (5 %)
     * @param CorCase: correction case general, for bias or for error estimation.
     * @param n: Number of samples
     * @param array: List of samples
     * @param val: Return mean of samples
     * @param valError: Return Error of mean of samples
     * @param rms: Return of RMS of samples
     * @return 1: if correction is successful,
     *         0: not enought samples or no significant correction obtain
     */
    int CalculateCorrection(int CorCase, int n,
                            std::vector<float> array, double* val, double* valError, double* rms);
    /** Calculate correction for obtain more realistic value of correction
     *   for histogram RMS of ratio of residual devided by Error Estimation.
     * Function is prepared for later using with real data correction,
     *   currently is not apply for using true information from simulation.
     * Function was confirm with value from rms(res/EE)*Thiscorrection ~= true value.
     *
     * In more detailed description: if:
     * sig(x): Sigma or RMS of x distribution, mean(x): Mean of x distribution
     * mean of real EE is derivate from sig(R/EE):
     * mean(EE) = sig(R) / sig(R/EE) * GetCorrectionResErrEst(1.0 / sig(R/EE))
     * If we want sig(R)/meanTrue(EE)->1.0:
     *     meanTrue(EE) = mean(EE) * sig(R/EE) / GetCorrectionResErrEst(1.0 / sig(R/EE))
     * Final value is more realistic, spread is up to +- 10 % for statistics > 110 samples
     * Correction is valid for imput value mean(EE)/sig(R) in range: <0.3 .. 2.2>
     * Final correction for ErrorEstimation is:
     *     CorEE = meanTrue(EE) / mean(EE) = sig(R/EE) / GetCorrectionResErrEst(1.0 / sig(R/EE))
     * and applying of EE correction is:
     *     TrueEE = EE * CorEE
     * Examples:
     * undreestimated est. error: mean(EE) = 0.3 * sig(R), sig(R)/mean(EE) = 3.3,
     *   sig(R/EE) = 1.6, 1.0/sig(R/EE) = 0.6
     *   1.0/sig(R/EE)*CorrREE(1.0/sig(R/EE)) = 0.2 ~ mean(EE)/sig(R)
     * overestimated est. error:   mean(EE) = 2.2 * sig(R), sig(R)/mean(EE) = .45,
     *   sig(R/EE) = 0.4, 1.0/sig(R/EE) = 2.2
     *   1.0/sig(R/EE)*CorrREE(1.0/sig(R/EE)) = 2.2 ~ mean(EE)/sig(R)
     * @param ResErrEst Value of 1.0 / sigma of ratio (Resiual / ErrorEstimation)
     * @return Correction for multiply EE to more realistic value
     */
    double GetCorrectionResErrEst(double ResErrEst);

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    /** Expert setting, production of monitoring histograms, default=False */
    Bool_t m_DoExpertHistograms = kFALSE;
    /** To use real data without simulations or simulations, default=False */
    Bool_t m_UseRealData = kFALSE;
    /** Use all pixel kinds (-1) or only one of specific: 0..3 correspond with pitch: 55, 60, 70, 85 um, default=-1 */
    short m_UsePixelKind = -1;
    /** Use EE correction calculate from 0: true EE, 1: EE from Track EE (real data), 2: EE from Track residuals, default=0 */
    short m_StoreEECorrectionType = 0;
    /** Apply only in case correction is higher than cut, default=0.1 */
    double m_ApplyEECorrectionCut = 0.1;

    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Current process id */
    int m_procId = -1;
    /** layer ID (1, 2) */
    short m_layer = 0;
    /** sensor ID (1, 2) */
    short m_sensor = 0;
    /** pixel kind ID (0..7) */
    short m_pixelKind = 0;
    /** mark, if cluster is touch edge or masked pixel  */
    short m_closeEdge = 0;
    /** ID of Cluster Shape */
    short m_shape = 0;
    /** theta angle of track direction via sensor */
    float m_thetaTrack = 0.0;
    /** phi angle of track direction via sensor */
    float m_phiTrack = 0.0;
    /** theta angle from true hit */
    float m_thetaTrue = 0.0;
    /** phi angle from true hit */
    float m_phiTrue = 0.0;
    /** collected charge - signal */
    float m_signal = 0.0;
    /** seed - maximum of signal */
    float m_seed = 0.0;
    /** in-pixel position U from track (if possible) or simulation true hit */
    float m_InPixUTrue = 0.0;
    /** in-pixel position V from track (if possible) or simulation true hit */
    float m_InPixVTrue = 0.0;
    /** in-pixel recohit position U */
    float m_InPixUReco = 0.0;
    /** in-pixel recohit position V */
    float m_InPixVReco = 0.0;

    /** difference in U: plane - recohit position for biased track */
    float m_ResidUPlaneRHBias = 0.0;
    /** difference in V: plane - recohit position for biased track */
    float m_ResidVPlaneRHBias = 0.0;
    /** difference in U: plane - recohit position for unbiased track*/
    float m_ResidUPlaneRHUnBias = 0.0;
    /** difference in V: plane - recohit position for unbiased track */
    float m_ResidVPlaneRHUnBias = 0.0;
    /** difference in U: plane - cluster position for biased track */
    float m_ResidUPlaneClBias = 0.0;
    /** difference in V: plane - cluster position for biased track */
    float m_ResidVPlaneClBias = 0.0;
    /** difference in U: true - plane position for biased track */
    float m_ResidUTruePlaneBias = 0.0;
    /** difference in V: true - plane position for biased track */
    float m_ResidVTruePlaneBias = 0.0;
    /** difference in U: true - plane position for unbiased track */
    float m_ResidUTruePlaneUnBias = 0.0;
    /** difference in V: true - plane position for unbiased track */
    float m_ResidVTruePlaneUnBias = 0.0;
    /** difference in U: true - recohit position */
    float m_ResidUTrueRH = 0.0;
    /** difference in V: true - recohit position */
    float m_ResidVTrueRH = 0.0;
    /** difference in U: true - cluster position */
    float m_ResidUTrueCl = 0.0;
    /** difference in V: true - cluster position */
    float m_ResidVTrueCl = 0.0;
    /** error estimation covariant element 0,1 for biased track */
    float m_ErrorEstimateCovarUVBias = 0.0;
    /** error estimation covariant element 0,1 for unbiased track */
    float m_ErrorEstimateCovarUVUnBias = 0.0;
    /** estimated sigma in U from recohit */
    float m_SigmaUReco = 0.0;
    /** estimated sigma in V from recohit */
    float m_SigmaVReco = 0.0;
    /** estimated sigma in U of track position for biased track */
    float m_SigmaUTrackBias = 0.0;
    /** estimated sigma in V of track position for biased track */
    float m_SigmaVTrackBias = 0.0;
    /** estimated sigma in U of residuals for biased track */
    float m_SigmaUResBias = 0.0;
    /** estimated sigma in V of residuals for biased track */
    float m_SigmaVResBias = 0.0;
    /** estimated sigma in U of track position for unbiased track */
    float m_SigmaUTrackUnBias = 0.0;
    /** estimated sigma in V of track position for unbiased track */
    float m_SigmaVTrackUnBias = 0.0;
    /** estimated sigma in U of residuals for unbiased track */
    float m_SigmaUResUnBias = 0.0;
    /** estimated sigma in V of residuals for unbiased track */
    float m_SigmaVResUnBias = 0.0;


  };
} // namespace Belle2


