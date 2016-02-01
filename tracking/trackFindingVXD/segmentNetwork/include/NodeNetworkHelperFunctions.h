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

#include <tracking/trackFindingVXD/segmentNetwork/CACell.h>
#include <tracking/trackFindingVXD/segmentNetwork/VoidMetaInfo.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNode.h>
#include <tracking/trackFindingVXD/segmentNetwork/DirectedNodeNetwork.h>

#include <fstream>
#include <string>
#include <sstream>

namespace Belle2 {

  /** namespace for DirectedNodeNetwork-related stuff */
  namespace DNN {

    /** TODO
     *
     * - code isSeed and getState to colors for fill and for border
     *
     * */



    /** function for printing networks with CACells:
     *
     * takes network and prints it to given fileName.
     *
     * prerequisite for NodeEntryType:
     * - '<<' operator overload.
     */
    template<class NodeEntryType>
    void printCANetwork(DirectedNodeNetwork<NodeEntryType, CACell>& network, std::string fName/*, bool useGetName = false*/)
    {
      std::string fullOut = "digraph G {\n";
      fullOut +=
        "ranksep=\"0.2\" edge[labelfontsize=\"8\" fontsize=\"8\" arrowsize=\"0.9\"] nodesep=\"0.2\" node[shape=\"box\" width=\"0\" height=\"0\" fontsize=\"10\"]\n";
      for (auto* entry : network) { // write vertices:

        std::stringstream  outStream;
        outStream << entry->getEntry();

        fullOut += std::to_string(entry->getIndex()) +
                   " [label=\"" +
                   outStream.str() +
                   " State,Seed: " +
                   std::to_string(entry->getMetaInfo().getState()) +
                   "," +
                   std::to_string(entry->getMetaInfo().isSeed()) +
                   "\"];\n";
      }
      for (auto* entry : network) { // write edges:
        for (auto* nb : entry->getInnerNodes()) {
          std::string arrowStyle = (entry->getMetaInfo().getState() == (nb->getMetaInfo().getState() + 1)) ? "" : " [style=dashed]";
          fullOut += std::to_string(entry->getIndex()) + " -> " + std::to_string(nb->getIndex()) + arrowStyle + ";\n";
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
    void printNetwork(DirectedNodeNetwork<NodeEntryType, AnyMetaInfo>& network, std::string fName/*, bool useGetName = false*/)
    {
      std::string fullOut = "digraph G {\n";
      fullOut +=
        "ranksep=\"0.2\" edge[labelfontsize=\"8\" fontsize=\"8\" arrowsize=\"0.9\"] nodesep=\"0.2\" node[shape=\"box\" width=\"0\" height=\"0\" fontsize=\"10\"]\n";

      for (auto* entry : network) { // write vertices:

        std::stringstream  outStream;
        outStream << entry->getEntry();
        fullOut += std::to_string(entry->getIndex()) +
                   " [label=\"" +
                   outStream.str() +
                   "\"];\n";
      }
      for (auto* entry : network) { // write edges:
        for (auto* nb : entry->getInnerNodes()) {
          std::string arrowStyle = "";
          fullOut += std::to_string(entry->getIndex()) + " -> " + std::to_string(nb->getIndex()) + arrowStyle + ";\n";
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
