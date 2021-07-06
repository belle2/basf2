/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once


namespace Belle2 {

  /** simple NodeCompatibilityChecker, which checks for compatible Neighboring states of passed nodes (does no extended validation check) */
  template<class NodeType>
  struct NodeCompatibilityCheckerCA {

    /** returns true, if state of outerNode is state of innerNode.
    *
    * WARNING does not check if innerNode is in List of OuterNodes!
    */
    bool areCompatible(NodeType* outerNode, NodeType* innerNode)
    {
//    return true;
      return (outerNode->getMetaInfo().getState() == (innerNode->getMetaInfo().getState()));
    }
  };

} //Belle2 namespace
