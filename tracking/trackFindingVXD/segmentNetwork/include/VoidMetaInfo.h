/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace Belle2 {

  /** The most CPU efficient MetaInfo for the DirectedNode-requirements (even if useless). */
  class VoidMetaInfo {
  public:

    /** Default constructor to meet DirectedNode-requirements */
    VoidMetaInfo() {}
  };
} //Belle2 namespace
