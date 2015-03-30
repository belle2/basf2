/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef MCTRACKMATCHER_H_
#define MCTRACKMATCHER_H_

#include <tracking/modules/mcMatcher/MCMatcherTracksModule.h>

namespace Belle2 {

  /** This is a depricated alias for MCMatcherTracksModule */
  class MCTrackMatcherModule : public MCMatcherTracksModule {
  };

} // end namespace Belle2

#endif /* MCTRACKMATCHER_H_ */
