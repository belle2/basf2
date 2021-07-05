/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/eventTimeExtraction/findlets/BaseEventTimeExtractor.dcl.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vector>
#include <limits>

namespace Belle2 {

  /// Findlet to extract the T0 time of an event only using CDC Hits
  class HitBasedT0Extractor final : public BaseEventTimeExtractor<TrackFindingCDC::CDCWireHit&> {

  private:
    /// Type of the base class
    using Super = BaseEventTimeExtractor<TrackFindingCDC::CDCWireHit&>;

  public:
    /// Constructor registering the subordinary findlets to the processing signal distribution machinery
    HitBasedT0Extractor() = default;

    /// Make destructor of interface virtual
    virtual ~HitBasedT0Extractor() = default;

    /// Short description of the findlet
    std::string getDescription() override final;

    /// Expose the parameters to a module
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final;

    /// Collects all Wire Hits and executes the t0 fit
    void apply(std::vector<TrackFindingCDC::CDCWireHit>& inputWireHits) override final;

    /// Initialize the event meta data
    void initialize() override final;

  private:

    unsigned int m_param_minHitCount = 20; /**< minimum number of hits */

    float m_param_searchWindow = 70.0f; /**< the window (+-m_param_searchWindow) in ns where to search for the best t0 */

    float m_param_fitWindow = 100.0f; /**< the space (+-m_param_fitWindow) in ns used to fit the t0 */

    int m_param_binCountTimeHistogram = 50; /**< number of bins in the timing histogram */

    bool m_param_rejectByBackgroundFlag = false; /**< don't use hits for the fit which have been flagged as background */

    bool m_param_rejectIfNotTakenFlag = true; /**< don't use hits for the fit which have not been assigned to any track */

    bool m_param_storeAllFits = false; /**< store an image of the histogram and fit result, for debug purpose only */

    // don't use fit results which have a chi2 larger than configured here
    float m_param_rejectIfChiSquareLargerThan = std::numeric_limits<float>::max(); /**< largest allowable chi2 value */

    float m_param_rejectIfUncertaintyLargerThan = 12.0f; /**< largest allowable uncertainty value */

    float m_param_refitWindow = 50.0f; /**< the width of the window in ns (+- m_param_refitWindow) used to refit the final t0 value*/

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< access to event nr for debugging purposes */
  };
}
