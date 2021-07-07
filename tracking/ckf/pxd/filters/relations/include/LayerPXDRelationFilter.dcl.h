/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/RelationFilter.dcl.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>
#include <framework/database/DBObjPtr.h>
#include <tracking/dbobjects/CKFParameters.h>

namespace Belle2 {
  /// Base filter for CKF PXD states
  template <class AFilter, class APrefilter = AFilter>
  class LayerPXDRelationFilter : public TrackFindingCDC::RelationFilter<CKFToPXDState> {
    /// The parent class
    using Super = TrackFindingCDC::RelationFilter<CKFToPXDState>;

  public:
    using Super::operator();

    /// Add the filter as listener
    LayerPXDRelationFilter();

    /// Default destructor
    ~LayerPXDRelationFilter();

    /// Return all states the given state is possible related to.
    std::vector<CKFToPXDState*> getPossibleTos(CKFToPXDState* from,
                                               const std::vector<CKFToPXDState*>& states) const override;

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Give a final weight to the possibilities by asking the filter.
    TrackFindingCDC::Weight operator()(const CKFToPXDState& from, const CKFToPXDState& to) override;

    void initialize() override;

    void beginRun() override;

  private:
    /// Parameter: Make it possible to jump over N layers (if set to -1, read from DB).
    int m_param_hitJumping = 0;
    /// This are values that are actually used ('m_param_hitJumping==-1' means parameter is read from DB).
    int m_layerJumpLowPt = m_param_hitJumping;
    /// This are values that are actually used ('m_param_hitJumping==-1' means parameter is read from DB).
    int m_layerJumpHighPt = m_param_hitJumping;
    /// This are values that are actually used ('m_param_hitJumping==-1' means parameter is read from DB).
    double m_layerJumpPtThreshold = -1;
    /// Filter for rejecting the states
    AFilter m_filter;
    /// Loose pre-filter to reject possibleTos
    APrefilter m_prefilter;
    /// Used to get correct payload
    std::string m_prefix = "";
    /// LayerJump parameter can be read from DB (use pointer as payload name contains 'prefix')
    std::unique_ptr<DBObjPtr<CKFParameters>> m_ckfParameters;
  };
}
