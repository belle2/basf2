/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRECOHIT_H
#define CDCRECOHIT_H

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/ADCCountTranslatorBase.h>
#include <cdc/dataobjects/CDCGeometryTranslatorBase.h>
#include <cdc/dataobjects/TDCCountTranslatorBase.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <genfit/ICalibrationParametersDerivatives.h>

#include <tracking/vectorTools/B2Vector3.h>

#include <memory>


namespace Belle2 {
  /** This class is used to transfer CDC information to the track fit. */
  class CDCRecoHit : public genfit::AbsMeasurement, public genfit::ICalibrationParametersDerivatives  {

  public:
    /** Default Constructor for ROOT IO.*/
    CDCRecoHit();

    /** Constructor.
     */
    CDCRecoHit(const CDCHit* cdcHit, const genfit::TrackCandHit* trackCandHit);

    /** Destructor. */
    ~CDCRecoHit() {}

    /** Creating a copy of this hit. */
    CDCRecoHit* clone() const;

    /** Getter for WireID object. */
    WireID getWireID() const
    {
      return m_wireID;
    }

    /** Setter for the Translators. */
    static void setTranslators(CDC::ADCCountTranslatorBase*    const adcCountTranslator,
                               CDC::CDCGeometryTranslatorBase* const cdcGeometryTranslator,
                               CDC::TDCCountTranslatorBase*    const tdcCountTranslator,
                               bool useTrackTime = false);

    /** Methods that actually interface to Genfit.
     */
    genfit::SharedPlanePtr constructPlane(const genfit::StateOnPlane& state) const;
    std::vector<genfit::MeasurementOnPlane*> constructMeasurementsOnPlane(const genfit::StateOnPlane& state) const;
    virtual const genfit::HMatrixU* constructHMatrix(const genfit::AbsTrackRep*) const;


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
                                const genfit::AbsTrackRep* rep = NULL,
                                bool usePlaneFromFit = false);

    /**
     * select how to resolve the left/right ambiguity:
     * -1: negative (left) side on vector (wire direction) x (track direction)
     * 0: mirrors enter with same weight, DAF will decide.
     * 1: positive (right) side on vector (wire direction) x (track direction)
     * where the wire direction is pointing towards +z except for small
     * corrections such as stereo angle, sagging
     */
    virtual void setLeftRightResolution(int lr) { m_leftRight = lr; }

    virtual bool isLeftRightMeasurement() const override { return true; }
    /** Getter for left/right passage flag. */
    virtual int getLeftRightResolution() const override { return m_leftRight; }


    /** get the pointer to the CDCHit object that was used to create this CDCRecoHit object.
     * Can be NULL if CDCRecoHit was not created with the CDCRecoHit(const CDCHit* cdcHit) constructor
     */
    const CDCHit* getCDCHit() const
    {
      return m_cdcHit;
    }

    /**
     * @brief Labels for (global) alignment/calibration parameters
     *
     * @return Vector of ints, one per each parameter
     */
    virtual std::vector< int > labels();
    /**
     * @brief Derivatives for (global) alignment/calibration parameters
     *
     * @param sop State on virtual plane to calculate derivatives
     * @return TMatrixD of global derivatives, #columns=#params, #row=2 (or measurement dimension if > 2)
     */
    virtual TMatrixD derivatives(const genfit::StateOnPlane* sop);
    /**
     * @brief Derivatives for (local) fit parameters
     *
     * @param sop State on virtual plane to calculate derivatives
     * @return TMatrixD of local derivatives, #columns=#params, #row=2 (or measurement dimension if > 2)
     */
    virtual TMatrixD localDerivatives(const genfit::StateOnPlane* sop);
    /**
     * @brief Labels for (local) alignment/calibration parameters
     *
     * @return Vector of ints, one per each parameter
     */
    virtual std::vector< int > localLabels();

  private:
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

#endif

    //---------------------------------------------------------------------------------------------------------------------------
    /** TDC Count as out of CDCHit. */
    unsigned short m_tdcCount;

    /** Wire Identifier. */
    WireID m_wireID;

    /** Pointer to the CDCHit used to created this CDCRecoHit */
    const CDCHit* m_cdcHit;  //!

    /** Flag showing left/right passage */
    signed char m_leftRight;

    /** ROOT Macro.*/
    ClassDef(CDCRecoHit, 9);
    // Version history:
    // ver 9: Derives from ICalibrationParametersDerivatives to expose
    //        alignment/calibration interface
    // ver 8: Rewrite to deal with realistic translators.  No longer
    //        derives from genfit::WireMeasurement.
  };
}
#endif
