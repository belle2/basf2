/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/ADCCountTranslatorBase.h>
#include <cdc/dataobjects/CDCGeometryTranslatorBase.h>
#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <framework/geometry/B2Vector3.h>

#include <memory>


namespace Belle2 {
  /** This class is used to transfer CDC information to the track fit. */
  class CDCRecoHit : public genfit::AbsMeasurement  {

  public:
    /** Default Constructor for ROOT IO.*/
    CDCRecoHit();

    /** Constructor.
     */
    CDCRecoHit(const CDCHit* cdcHit, const genfit::TrackCandHit* trackCandHit);

    /** Destructor. */
    ~CDCRecoHit() {}

    /** Creating a copy of this hit. */
    CDCRecoHit* clone() const override;

    /** Getter for WireID object. */
    WireID getWireID() const
    {
      return m_wireID;
    }

    /** Setter for the Translators. */
    static void setTranslators(CDC::ADCCountTranslatorBase*    const adcCountTranslator,
                               CDC::CDCGeometryTranslatorBase* const cdcGeometryTranslator,
                               CDC::TDCCountTranslatorBase*    const tdcCountTranslator,
                               //temp4cosmics                               bool useTrackTime = false);
                               bool useTrackTime = false, bool cosmics = false);

    /** Methods that actually interface to Genfit.
     */
    genfit::SharedPlanePtr constructPlane(const genfit::StateOnPlane& state) const override;

    /** build MeasurementsOnPlane
    */
    std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const override;

    /** construct error matrix
     */
    virtual const genfit::HMatrixU* constructHMatrix(const genfit::AbsTrackRep*) const override;

    /** Get the time derivative of the MesuredStateOnPlane (derived from the track fit).  */
    std::vector<double> timeDerivativesMeasurementsOnPlane(const genfit::StateOnPlane& state) const;

    /**
     * Get the vector pointing from the wire to the fitted trajectory
     * as well as the direction of the track in the fitted point.
     * Uses the cardinal TrackRep if rep == NULL.  Returns false if
     * the track is not fitted in this point for the requested
     * TrackRep.  If the hit does not belong to a track, an error is
     * issued and false returned.  If usePlaneFromFit is used,
     * constructPlane is not called to evaluate the closest point on
     * the wire to the track.  Instead the origin of the plane of the
     * fitted state is used (which will be the same point if the wire
     * is not bent).
     */
    bool getFlyByDistanceVector(B2Vector3D& pointingVector, B2Vector3D& trackDir,
                                const genfit::AbsTrackRep* rep = nullptr,
                                bool usePlaneFromFit = false);

    /**
     * select how to resolve the left/right ambiguity:
     * -1: negative (left) side on vector (wire direction) x (track direction)
     * 0: mirrors enter with same weight, DAF will decide.
     * 1: positive (right) side on vector (wire direction) x (track direction)
     * where the wire direction is pointing towards +z except for small
     * corrections such as stereo angle, sagging
     */
    void setLeftRightResolution(int lr) { m_leftRight = lr; }

    /** CDC RecoHits always have left-right ambiguity */
    bool isLeftRightMeasurement() const override { return true; }

    /** Getter for left/right passage flag. */
    int getLeftRightResolution() const override { return m_leftRight; }


    /** get the pointer to the CDCHit object that was used to create this CDCRecoHit object.
     * Can be NULL if CDCRecoHit was not created with the CDCRecoHit(const CDCHit* cdcHit) constructor
     */
    const CDCHit* getCDCHit() const
    {
      return m_cdcHit;
    }

  protected:
#ifndef __CINT__ // rootcint doesn't know smart pointers
    /** Object for ADC Count translation. */
    static std::unique_ptr<CDC::ADCCountTranslatorBase>     s_adcCountTranslator;

    /** Object for geometry translation. */
    static std::unique_ptr<CDC::CDCGeometryTranslatorBase>  s_cdcGeometryTranslator;

    /** Object for getting drift-length and -resolution. */
    static std::unique_ptr<CDC::TDCCountTranslatorBase>    s_tdcCountTranslator;

    /** Whether to use the track time or not when building the
     * measurementOnPlane.  This needs to be in sync with the
     * TDCCountTranslator.  */
    static bool s_useTrackTime;
    /**
     * Switch to use cosmic events, or physics events from IP.
     * Default value is 0, which means "physics event" mode.
     */
    static bool s_cosmics;

#endif

    /** TDC Count as out of CDCHit. */
    unsigned short m_tdcCount;

    /** ADC Count as out of CDCHit. */
    unsigned short m_adcCount;

    /** Wire Identifier. */
    WireID m_wireID;

    /** Pointer to the CDCHit used to created this CDCRecoHit */
    const CDCHit* m_cdcHit;  //!

    /** Flag showing left/right passage */
    signed char m_leftRight;

    /** ROOT Macro.*/
    ClassDefOverride(CDCRecoHit, 10);
    // Version history:
    // ver 10: ICalibrationParametersDerivatives interface moved to derived class.
    //         ClassDef -> ClassDefOverride + consistent override keyword usage.
    //         Private members -> protected for access from derived class.
    // ver 9: Derives from ICalibrationParametersDerivatives to expose
    //        alignment/calibration interface
    // ver 8: Rewrite to deal with realistic translators.  No longer
    //        derives from genfit::WireMeasurement.
  };
}
