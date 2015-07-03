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
