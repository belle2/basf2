/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
