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

  /**  most trivial node compatibility checker, says always true*/
  template<class NodeType>
  struct NodeCompatibilityCheckerBase {

    /** declares all nodes passed as compatible */
    bool areCompatible(NodeType*, NodeType*) { return true; }
  };

} //Belle2 namespace
