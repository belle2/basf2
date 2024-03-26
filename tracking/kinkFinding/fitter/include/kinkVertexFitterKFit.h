/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 * External Contributor: J. Tanaka                                        *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/kinkFinding/fitter/KFitConst.h>
#include <tracking/kinkFinding/fitter/KFitError.h>
#include <tracking/kinkFinding/fitter/KFitBase.h>

#include <framework/geometry/B2Vector3.h>

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif


namespace Belle2 {


  /**
   * kinkVertexFitterKFit is a derived class from KFitBase to perform vertex-constraint kinematical fit.
   */
  class kinkVertexFitterKFit : public KFitBase {
  public:
    /** Construct an object with no argument. */
    kinkVertexFitterKFit(void);

    /** Destruct the object. */
    ~kinkVertexFitterKFit(void);


  public:
    /** Set an initial vertex point for the vertex-vertex constraint fit.
     * @param v initial vertex point
     * @return error code (zero if success)
     */
    enum KFitError::ECode setInitialVertex(const HepPoint3D& v);

    /** Set an initial vertex point for the mass-vertex constraint fit.
     * @param v initial vertex point
     * @return error code (zero if success)
     */
    enum KFitError::ECode setInitialVertex(const B2Vector3D& v);

    /** Tell the object to perform a fit with vertex position fixed.
     * @param flag true for fixed vertex, false for otherwise
     * @return error code (zero if success)
     */
    enum KFitError::ECode setKnownVertex(const bool flag = true);

    /** Tell the object to perform a fit with track correlations.
     * @param m true for with correlation, false for otherwise
     * @return error code (zero if success)
     */
    enum KFitError::ECode setCorrelationMode(const bool m);


  public:
    /** Get a vertex position.
     * @param flag KFitConst::kBeforeFit or KFitConst::kAfterFit
     * @return vertex position
     */
    const HepPoint3D getVertex(const int flag = KFitConst::kAfterFit) const;

    /** Get a fitted vertex error matrix.
     * @return vertex error matrix
     */
    const CLHEP::HepSymMatrix getVertexError(void) const;

    double getCHIsq(void) const override;

    /** Get a chi-square of the fit excluding IP-constraint part.
     * @return chi-square of the fit excluding IP-constraint part.
     */
    double getCHIsqVertex(void) const;

    /** Get a vertex error matrix of the track
     * @param id track id
     * @return vertex error matrix
     */
    const CLHEP::HepMatrix getTrackVertexError(const int id) const;

    double getTrackCHIsq(const int id) const override;

    /** Get a sum of the chi-square associated to the input tracks.
     *  The return value should be the same as the one from getCHIsqVertex().
     * @return sum of the chi-square associated to the input tracks
     */
    double getTrackPartCHIsq(void) const;

    /** Get an NDF relevant to the getTrackPartCHIsq().
     * @return NDF relevant to the getTrackPartCHIsq()
     */
    int getTrackPartNDF(void) const;


  public:
    /** Perform a vertex-constraint fit.
     * @return error code (zero if success)
     */
    enum KFitError::ECode doFit(void);

    /**
     * Update mother particle.
     * @param[in] mother Mother particle.
     */
    //enum KFitError::ECode updateMother(Particle* mother);

  private:

    /** Perform a standard vertex-constraint fit including IP-tube constraint.
    * @return error code (zero if success)
    */
    enum KFitError::ECode doFit3(void);


    /** Perform a fixed-vertex-position fit mainly for slow pion.
    * @return error code (zero if success)
    */
    enum KFitError::ECode doFit5(void);


  private:
    enum KFitError::ECode prepareInputMatrix(void) override;

    enum KFitError::ECode prepareInputSubMatrix(void) override;

    enum KFitError::ECode prepareOutputMatrix(void) override;

    enum KFitError::ECode makeCoreMatrix(void) override;

    enum KFitError::ECode calculateNDF(void) override;


  private:
    /** Flag controlled by setCorrelationMode(). */
    bool m_CorrelationMode;
    /** Container of chi-square's of the input tracks. */
    double m_EachCHIsq[KFitConst::kMaxTrackCount2];
    /** chi-square of the fit excluding IP-constraint part. */
    double m_CHIsqVertex;

    /** Vertex position before the fit. */
    HepPoint3D m_BeforeVertex;

    /** Vertex position after the fit. */
    HepPoint3D m_AfterVertex;
    /** Vertex error matrix after the fit. */
    CLHEP::HepSymMatrix m_AfterVertexError;
    /** Array of vertex error matrices after the fit. */
    std::vector <CLHEP::HepMatrix> m_AfterTrackVertexError;

    /** Flag controlled by setKnownVertex(). */
    bool m_FlagKnownVertex;

  };


} // namespace Belle2
