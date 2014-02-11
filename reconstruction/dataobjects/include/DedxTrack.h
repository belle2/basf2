/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXTRACK_H
#define DEDXTRACK_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <TVector3.h>

#include <vector>

namespace Belle2 {
  /** Debug output for DedxPID module.
   *
   * Contains information of individual hits belonging to a track
   * as well as calculated dE/dx values.
   */
  class DedxTrack : public RelationsObject {
    friend class DedxPIDModule;
  public:
    // default constructor
    DedxTrack():
      RelationsObject(),
      m_event_id(0), m_track_id(0), m_pdg(0), m_slow_pion(false),
      m_p_vec(TVector3()), m_p(0), m_p_true(0),
      m_charge(0),
      m_chi2(0),
      m_mother_pdg(0),
      m_last_layer(-1),
      m_length(0.0) {
      //for all detectors
      for (int i = 0; i < Dedx::c_num_detectors; i++)
        m_dedx_avg[i] = m_dedx_avg_truncated[i] = m_dedx_avg_truncated_err[i] = 0.0;

      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcLogl[i] = 0.0;
        m_svdLogl[i] = 0.0;
      }
    }

    /** add a single hit to the object */
    void addHit(const TVector3& pos, int layer, int sensorid, float phi, float energydep, bool extrapFailed, float drift_length = 0.0) {
      wx.push_back(pos.X());
      wy.push_back(pos.Y());
      wz.push_back(pos.Z());
      flayer.push_back(layer);
      sensorUID.push_back(sensorid);
      phiWireTrack.push_back(phi);
      edep.push_back(energydep);
      extrapolationFailed.push_back(extrapFailed);
      driftLength.push_back(drift_length);
      if ((layer < 0 and layer < m_last_layer) or (layer >= 0 and layer > m_last_layer))
        m_last_layer = layer;
    }

    /** add a dE/dx value (one per layer for non-curling tracks) */
    void addDedx(int layer, float distance, float dedx_value) {
      dedx_flayer.push_back(layer);
      dist.push_back(distance);
      dedx.push_back(dedx_value);
      m_length += distance;
    }

  private:
    int m_event_id; /**< event this track was found in */
    int m_track_id; /**< equal to Track id. */
    int m_pdg; /**< PDG code (MC truth) */
    int m_pdg_hyp; /**< PDG code (fit hypothesis, >0) */
    bool m_slow_pion; /**< does this particle belong to a slow pion (MC truth) */
    TVector3 m_p_vec; /**< momentum */
    float m_p; /**< total momentum at point of closest approach to origin */
    float m_p_true; /**< true momentum */
    short m_charge; /**< particle charge from tracking (+1 or -1) */
    float m_chi2; /**< chi^2 from track fitting */
    int m_mother_pdg; /**< PDG code of mother particle */
    int m_last_layer; /**< layer id of outermost hit */
    double m_length; /**< total distance travelled by the track */

    //arrays with one entry per hit
    std::vector<float> edep; /**< uncorrected energy deposition (or charge for CDC hits) */
    std::vector<int> flayer; /**< full layer id, -1..-2 for PXD -2..-6 for SVD */
    std::vector<float> phiWireTrack; /**< for PXD/SVD: angle between sensor normal and track; for CDC angle of track in x/y plane */
    std::vector<float> wx; /**< hit position */
    std::vector<float> wy; /**< hit position */
    std::vector<float> wz; /**< hit position */
    std::vector<int> sensorUID; /**< unique sensor ID (wire ID in CDC) */
    std::vector<float> driftLength; /**< drift length in CDC (0 for other hits) */
    std::vector<bool> extrapolationFailed; /**< true for hits where the extrapolation failed. */

    //arrays with one entry per layer (or so. just don't mix them with the hit arrays)
    std::vector<float> dedx; /**< extracted specific energy loss (arbitrary units, different between detectors) */
    std::vector<float> dist; /**< distance flown through active medium in current segment */
    std::vector<int> dedx_flayer; /**< layer id corresponding to dedx & dist */

    float m_dedx_avg[Dedx::c_num_detectors]; /**< dEdX averaged for one subdetector */
    float m_dedx_avg_truncated[Dedx::c_num_detectors]; /**< dEdX averaged for one subdetector, truncated mean */
    float m_dedx_avg_truncated_err[Dedx::c_num_detectors]; /**< standard deviation of m_dedx_avg_truncated */

    float m_cdcLogl[Const::ChargedStable::c_SetSize]; /**< CDC log likelihood for each particle, not including momentum prior */
    float m_svdLogl[Const::ChargedStable::c_SetSize]; /**< SVD log likelihood for each particle, not including momentum prior */

    ClassDef(DedxTrack, 6); /**< Debug output for DedxPID module. */
  };
}
#endif
