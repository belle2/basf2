/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <cdc/dataobjects/WireID.h>
#include <Math/Vector3D.h>
#include <cmath>

namespace Belle2 {

  /**
   * Class to store CDC hit information needed for dedx
   */

  class CDCDedxHit : public RelationsObject {

  public:

    /**
     * default constructor
     */
    CDCDedxHit()
    {}

    /**
     * full constructor
     * @param wireID wire identifier
     * @param TDC TDC count from CDCHit
     * @param ADC ADC count from CDCHit
     * @param pocaMomentum POCA momentum from recoTrack
     * @param pocaOnTrack POCA on track from recoTrack
     * @param pocaOnWire POCA on wire from recoTrack
     * @param foundByTrackFinder track finder ID (see enum Belle2::RecoHitInformation::OriginTrackFinder)
     * @param weightPionHypo Kalman fitter weight for pion hypothesis (from recoTrack)
     * @param weightKaonHypo Kalman fitter weight for kaon hypothesis (from recoTrack)
     * @param weightProtHypo Kalman fitter weight for proton hypothesis (from recoTrack)
     */
    CDCDedxHit(WireID wireID, short TDC, unsigned short ADC,
               const ROOT::Math::XYZVector& pocaMomentum,
               const ROOT::Math::XYZVector& pocaOnTrack,
               const ROOT::Math::XYZVector& pocaOnWire,
               int foundByTrackFinder, double weightPionHypo, double weightKaonHypo, double weightProtHypo):
      m_wireID(wireID), m_tdcCount(TDC), m_adcCount(ADC),
      m_px(pocaMomentum.X()), m_py(pocaMomentum.Y()), m_pz(pocaMomentum.Z()),
      m_x(pocaOnTrack.X()), m_y(pocaOnTrack.Y()), m_z(pocaOnTrack.Z()),
      m_dx(pocaOnWire.X() - pocaOnTrack.X()), m_dy(pocaOnWire.Y() - pocaOnTrack.Y()), m_dz(pocaOnWire.Z() - pocaOnTrack.Z()),
      m_foundByTrackFinder(foundByTrackFinder),
      m_weightPionHypo(weightPionHypo), m_weightKaonHypo(weightKaonHypo), m_weightProtHypo(weightProtHypo)
    {}

    /**
     * Returns wire identifier
     * @return wire identifier
     */
    const WireID& getWireID() const {return m_wireID;}

    /**
     * Returns TDC count
     * @return TDC count
     */
    short getTDCCount() const {return m_tdcCount;}

    /**
     * Returns ADC count
     * @return ADC count
     */
    unsigned short getADCCount() const {return m_adcCount;}

    /**
     * Returns momentum at point-of-closest-approach of track to wire
     * @return momentum at point-of-closest-approach of track to wire
     */
    ROOT::Math::XYZVector getPOCAMomentum() const {return ROOT::Math::XYZVector(m_px, m_py, m_pz);}

    /**
     * Returns point-of-closest-approach on track
     * @return point-of-closest-approach on track
     */
    ROOT::Math::XYZVector getPOCAOnTrack() const {return ROOT::Math::XYZVector(m_x, m_y, m_z);}

    /**
     * Returns point-of-closest-approach on wire
     * @return point-of-closest-approach on wire
     */
    ROOT::Math::XYZVector getPOCAOnWire() const {return ROOT::Math::XYZVector(m_x + m_dx, m_y + m_dy, m_z + m_dz);}

    /**
     * Returns distance-of-closest-approach vector (pointing from track to wire)
     * @return distance-of-closest-approach vector
     */
    ROOT::Math::XYZVector getDOCAVector() const {return ROOT::Math::XYZVector(m_dx, m_dy, m_dz);}

    /**
     * Returns distance-of-closest-approach
     * @return distance-of-closest-approach
     */
    double getDOCA() const {return sqrt(m_dx * m_dx + m_dy * m_dy + m_dz * m_dz);}


    /**
     * Returns signed distance-of-closest-approach in XY projection.
     * The sign is defined here to be positive in the +x dir in the cell.
     * @return signed DOCA in XY projection
     */
    double getSignedDOCAXY() const;

    /**
     * Returns entrance angle to the cell in XY projection.
     * @return entracne angle
     */
    double getEntranceAngle() const;

    /**
     * Returns ID of track finder which added this hit
     * @return track finder ID (see enum Belle2::RecoHitInformation::OriginTrackFinder)
     */
    int getFoundByTrackFinder() const {return m_foundByTrackFinder;}

    /**
     * Returns Kalman fitter weight for pion hypothesis
     * @return Kalman fitter weight for pion hypothesis
     */
    double getWeightPionHypo() const {return m_weightPionHypo;}

    /**
     * Returns Kalman fitter weight for kaon hypothesis
     * @return Kalman fitter weight for kaon hypothesis
     */
    double getWeightKaonHypo() const {return m_weightKaonHypo;}

    /**
     * Returns Kalman fitter weight for proton hypothesis
     * @return Kalman fitter weight for proton hypothesis
     */
    double getWeightProtonHypo() const {return m_weightProtHypo;}

  private:

    WireID m_wireID; /**< wire identifier */
    short m_tdcCount = 0; /**< TDC count from CDCHit */
    unsigned short m_adcCount = 0; /**< ADC count from CDCHit */
    float m_px = 0; /**< POCA momentum, x coordinate */
    float m_py = 0; /**< POCA momentum, y coordinate */
    float m_pz = 0; /**< POCA momentum, z coordinate */
    float m_x = 0; /**< POCA on track, x cordinate */
    float m_y = 0; /**< POCA on track, y cordinate */
    float m_z = 0; /**< POCA on track, z cordinate */
    float m_dx = 0; /**< DOCA, x cordinate */
    float m_dy = 0; /**< DOCA, y cordinate */
    float m_dz = 0; /**< DOCA, z cordinate */
    int m_foundByTrackFinder = 0; /**< track finder ID (see enum Belle2::RecoHitInformation::OriginTrackFinder) */
    float m_weightPionHypo = 0; /**< Kalman fitter weight for pion hypothesis */
    float m_weightKaonHypo = 0; /**< Kalman fitter weight for kaon hypothesis */
    float m_weightProtHypo = 0; /**< Kalman fitter weight for proton hypothesis */

    ClassDef(CDCDedxHit, 1); /**< ClassDef */

  };

} // end namespace Belle2
