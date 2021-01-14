/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
//   class VxdID;

  class ModuleParamList;

  /**
   * Findlet for loading the seeds from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   *
   * If a direction != "invalid" is given, the relations of the tracks to the given store array are checked.
   * If there is a relation with the weight equal to the given direction (meaning there is already a
   * partner for this direction), the track is not passed on.
   */
  class DATCONFPGAFindlet : public TrackFindingCDC::Findlet<> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<>;

  public:
    /// Constructor, for setting module description and parameters.
    DATCONFPGAFindlet();

    /// Default desctructor
    ~DATCONFPGAFindlet();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
//     void initialize() override;

    /// Load in the reco tracks and the hits
    void apply() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    /// Findlets

  };
}
