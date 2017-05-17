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
//#include <boost/multi_array.hpp>
#include <boost/functional/hash.hpp>
#include <framework/gearbox/Unit.h>


namespace Belle2 {

  typedef std::tuple<short, short, short, short, short> key_type;
  typedef std::tuple<short, short, short, short, short, short> key_type2;

  struct key_hash : public std::unary_function<key_type, std::size_t> {
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

  struct key_hash2 : public std::unary_function<key_type2, std::size_t> {
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

  struct key_equal : public std::binary_function<key_type, key_type, bool> {
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

  struct key_equal2 : public std::binary_function<key_type2, key_type2, bool> {
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

  typedef std::unordered_map<key_type, double, key_hash, key_equal> map_typeCorrs;
  typedef std::unordered_map<key_type2, double, key_hash2, key_equal2> map_typeInPics;

  // Data
//  typedef struct {
//    double u;
//    double v;
//  } data_type;

  /**
   * Class implementing PXD cluster shape calibration algorithm
   */
  class PXDClusterShapeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor
    PXDClusterShapeCalibrationAlgorithm();

    /// Destructor
    virtual ~PXDClusterShapeCalibrationAlgorithm() {}

    /// Set external parameters and functions

    const int m_shapes = 15;    // 1 .. 15, shapeID = 0: not setting shape
    const int m_pixelkinds = 8;
    const int m_dimensions = 2; // cases: 15 x 8 x 2 = 240
    const int m_anglesU = 18;   // 18 x 18 = 324 (angles)
    const int m_anglesV = 18;
    const int m_in_pixelU = 7;  // 7 x 7 = 49 (in-pixel positions)
    const int m_in_pixelV = 7;
    const int m_MinClustersCorrections = 100; // Minimum clusters for calculation of corrections

    const int m_MinHitsInPixels = -1;       // TODO set to > 5000, deactivate this function: set to -1
    const int m_MinHitsAcceptInPixels = 5;  // TODO set to > 5

    const int m_cases = m_shapes * m_pixelkinds *
                        m_dimensions;    // 15 x 8 x 2 = 240 (shapes x pixelkinds x dimensions): bias corr., error estim. corr., in-pixel maps))

    /** Expert setting, production of monitoring histograms, default=False */
    void setDoExpertHistograms(bool DoExpertHistograms) { m_DoExpertHistograms = DoExpertHistograms; }

    /** To use track information (default) or simulations, default=True */
    void setUseTracks(bool UseTracks) { m_UseTracks = UseTracks; }
    /** To use real data without simulations or simulations, default=False */
    void setUseRealData(bool UseRealData) { m_UseRealData = UseRealData; }
    /** To compare true point and track position in simulations, default=False */
    void setCompareTruePointTracks(bool CompareTruePointTracks) { m_CompareTruePointTracks = CompareTruePointTracks; }
    /**
     * 1: standard calibration based on realistic physics or real data (default)
     * 2: special, for full range of angles for every kind of pixel
     */
    void setCalibrationKind(int CalibrationKind) { m_CalibrationKind = CalibrationKind; }
    /** For CalibrationKind=2 set pixel kind (pixel size) in range 1..4, default=1 */
    void setPixelKind(int PixelKind) { m_PixelKind = PixelKind; }

    /** For conversion from table to vector for calibration */
    void Table2Vector(map_typeCorrs& TCorrection, TVectorD* CorVector);
    void Table2Vector(map_typeInPics& TInPix, TVectorD* InPixVector);

    /** For conversion from table to vector for calibration */
    void Vector2Table(TVectorD* CorVector, map_typeCorrs& TCorrection);
    void Vector2Table(TVectorD* InPixVector, map_typeInPics& TInPix);

    /** get index of angle base on split */
    int getPhiIndex(float phi);
    int getThetaIndex(float theta);

    /** Calculate mean value and its error from array of values */
    int CalculateCorrection(int CorCase, int n, std::vector<double> array, double* val, double* valError, double* rms);

  protected:

    /// Run algo on data
    virtual EResult calibrate();

  private:
    /** Expert setting, production of monitoring histograms, default=False */
    Bool_t m_DoExpertHistograms = kFALSE;

    /** To use track information (default) or simulations, default=True */
    Bool_t m_UseTracks = kTRUE;

    /** To use real data without simulations or simulations, default=False */
    Bool_t m_UseRealData = kFALSE;

    /** To compare true point and track position in simulations, default=False */
    Bool_t m_CompareTruePointTracks = kFALSE;

    /**
     * 1: standard calibration based on realistic physics or real data (default)
     * 2: special, for full range of angles for every kind of pixel
     */
    int m_CalibrationKind = 1;

    /** For CalibrationKind=2 set pixel kind (pixel size) in range 1..4, default=1 */
    int m_PixelKind = 1;

    /** Current event id */
    int m_evt = -1;
    /** Current run id */
    int m_run = -1;
    /** Current experiment id */
    int m_exp = -1;
    /** Current process id */
    int m_procId = -1;

    /** Name of variable for layer ID (1, 2) */
    short m_layer = 0;
    /** Name of variable for sensor ID (1, 2) */
    short m_sensor = 0;
    /** Name of variable for pixel kind ID (1..8) */
    short m_pixelKind = 0;
    /** Name of variable for mark, if cluster is touch edge or masked pixel  */
    short m_closeEdge = 0;
    /** Name of variable for ID of Cluster Shape */
    short m_shape = 0;
    /** Name of variable for theta angle of track direction via sensor */
    float m_thetaTrack = 0.0;
    /** Name of variable for phi angle of track direction via sensor */
    float m_phiTrack = 0.0;
    /** Name of variable for theta angle from true hit */
    float m_thetaTrue = 0.0;
    /** Name of variable for phi angle from true hit */
    float m_phiTrue = 0.0;
    /** Name of variable for collected charge - signal */
    double m_signal = 0.0;
    /** Name of variable for seed - maximum of signal */
    double m_seed = 0.0;
    /** Name of variable for in-pixel position U from track (if possible) or simulation true hit */
    double m_InPixUTrue = 0.0;
    /** Name of variable for in-pixel position V from track (if possible) or simulation true hit */
    double m_InPixVTrue = 0.0;
    /** Name of variable for in-pixel reco position U */
    double m_InPixU = 0.0;
    /** Name of variable for in-pixel reco position V */
    double m_InPixV = 0.0;
    /** Name of variable for difference in U: reco - true position */
    double m_ResidUTrue = 0.0;
    /** Name of variable for difference in V: reco - true position */
    double m_ResidVTrue = 0.0;
    /** Name of variable for estimated sigma in U from reco position */
    double m_SigmaU = 0.0;
    /** Name of variable for estimated sigma in V from reco position */
    double m_SigmaV = 0.0;
    /** Name of variable for defference in U: reco - track position */
    double m_ResidUTrack = 0.0;
    /** Name of variable for defference in V: reco - track position */
    double m_ResidVTrack = 0.0;
    /** Name of variable for estimated sigma in U of track position */
    double m_SigmaUTrack = 0.0;
    /** Name of variable for estimated sigma in V of track position */
    double m_SigmaVTrack = 0.0;

    ClassDef(PXDClusterShapeCalibrationAlgorithm, 1); /**< Class implementing PXD cluster shape calibration algorithm */

  };
} // namespace Belle2


