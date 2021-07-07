/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>

#include <fstream>
#include <string>

namespace Belle2 {

  /** namespace for DirectedNodeNetwork-related stuff */
  namespace DNN {

    /** TODO
     * - code isSeed and getState to colors for fill and for border
     * */

    /** function for printing networks with CACells:
     *
     * takes network and prints it to given fileName.
     *
     * prerequisite for NodeEntryType:
     * - std::string getName()
     */
    template<class NodeEntryType>
    // cppcheck-suppress constParameter
    void printCANetwork(DirectedNodeNetwork<NodeEntryType, CACell>& network, const std::string& fName)
    {
      std::string fullOut = "digraph G {\n";
      fullOut +=
        "ranksep=\"0.2\" edge[labelfontsize=\"8\" fontsize=\"8\" arrowsize=\"0.9\"] nodesep=\"0.2\" node[shape=\"box\" width=\"0\" height=\"0\" fontsize=\"10\"]\n";
      // write vertices:
      for (auto* node : network) {
        fullOut += "\"" + node->getEntry().getName() + "\"" +
                   " [label=\"" +
                   node->getEntry().getName() +
                   " State,Seed: " +
                   std::to_string(node->getMetaInfo().getState()) +
                   "," +
                   std::to_string(node->getMetaInfo().isSeed()) +
                   "\"];\n";
      }
      // write edges:
      for (auto* node : network) {
        for (auto* innerNode : node->getInnerNodes()) {
          auto innerEntry = innerNode->getEntry();
          std::string arrowStyle = (node->getMetaInfo().getState() == (innerNode->getMetaInfo().getState() + 1)) ? "" : " [style=dotted]";
          fullOut += "\"" + node->getEntry().getName() + "\" -> \"" + innerEntry.getName() + "\"" + arrowStyle + ";\n";
        }
      }
      fullOut += "labelloc=\"t\";\nlabel=\"" + fName + "\";\n";
      fullOut += "}\n";

      std::ofstream ofs;
      ofs.open(fName + ".gv", std::ofstream::out | std::ofstream::trunc);
      ofs << fullOut;
      ofs.close();
    };


    /** overloaded print-version for typical activeSector-networks without CACell-stuff */
    template<class NodeEntryType, class AnyMetaInfo>
    // cppcheck-suppress constParameter
    void printNetwork(DirectedNodeNetwork<NodeEntryType, AnyMetaInfo>& network, const std::string& fName)
    {
      std::string fullOut = "digraph G {\n";
      fullOut +=
        "ranksep=\"0.2\" edge[labelfontsize=\"8\" fontsize=\"8\" arrowsize=\"0.9\"] nodesep=\"0.2\" node[shape=\"box\" width=\"0\" height=\"0\" fontsize=\"10\"]\n";

      // write vertices:
      for (auto* node : network) {
        fullOut += "\"" + node->getEntry().getName() + "\"" +
                   " [label=\"" +
                   node->getEntry().getName() +
                   "\"];\n";
      }
      // write edges:
      for (auto* node : network) {
        for (auto* innerNode : node->getInnerNodes()) {
          auto innerEntry = innerNode->getEntry();
          std::string arrowStyle = "";
          fullOut += "\"" + node->getEntry().getName() + "\" -> \"" + innerEntry.getName() + "\"" + arrowStyle + ";\n";
        }
      }
      fullOut += "labelloc=\"t\";\nlabel=\"" + fName + "\";\n";
      fullOut += "}\n";

      std::ofstream ofs;
      ofs.open(fName + ".gv", std::ofstream::out | std::ofstream::trunc);
      ofs << fullOut;
      ofs.close();
    };

  } // DNN namespace
} //Belle2 namespace
