/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
