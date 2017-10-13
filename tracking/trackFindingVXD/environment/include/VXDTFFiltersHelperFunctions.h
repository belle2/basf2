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

#include <fstream>
#include <string>
#include <sstream>

#include <framework/logging/Logger.h>
#include <tracking/trackFindingVXD/environment/VXDTFFilters.h>

namespace Belle2 {

  /** namespace for SectorMapHelper-related stuff */
  namespace SecMapHelper {

    /// TODO dot-compatible version of printStaticSectorRelations:
// // // //   /** overloaded print-version for typical activeSector-networks without CACell-stuff */
// // // //   template<class HitType>
// // // //   void printStaticSectorRelations(const VXDTFFilters<HitType>& filters, std::string configName, unsigned int nHitCombinations = 2, bool print2File = true, bool suppressDeadSectors = true)
// // // //   {
// // // //     if (nHitCombinations <2 or nHitCombinations > 4)
// // // //     B2FATAL("printStaticSectorRelations: input-parameter wrong ("
// // // //     << nHitCombinations
// // // //     << ", allowed only 2-4), skipping print-function!")
// // // //
// // // //     std::string fullOut = "digraph G {\n";
// // // //     fullOut += "ranksep=\"0.2\" edge[labelfontsize=\"8\" fontsize=\"8\" arrowsize=\"0.9\"] nodesep=\"0.2\" node[shape=\"box\" width=\"0\" height=\"0\" fontsize=\"10\"]";
// // // //     //    rankdir="LR" ranksep="0.2" edge[labelfontsize="8" fontsize="8" labeldistance="0.8" arrowsize="0.9" labelangle="-30" dir="none"] nodesep="0.2" node[width="0" height="0" fontsize="10"]
// // // //     for (auto* entry : network) { // write vertices:
// // // //
// // // //     std::stringstream  outStream;
// // // //     outStream << entry->getEntry();
// // // //     fullOut += std::to_string(entry->getIndex()) +
// // // //     " [label=\"" +
// // // //     outStream.str() +
// // // //     //    " State: " +
// // // //     //    std::to_string(entry->getMetaInfo().getState()) +
// // // //     //    " isSeed: " +
// // // //     //    std::to_string(entry->getMetaInfo().isSeed()) +
// // // //     "\"];\n";
// // // //     }
// // // //     for (auto* entry : network) { // write edges:
// // // //     for (auto* nb : entry->getInnerNodes()) {
// // // //       std::string arrowStyle = "";
// // // //       fullOut += std::to_string(entry->getIndex()) + " -> " + std::to_string(nb->getIndex()) + arrowStyle + ";\n";
// // // //     }
// // // //     }
// // // //     fullOut += "labelloc=\"t\";\nlabel=\"" + fName + "\";\n";
// // // //     fullOut += "}\n";
// // // //
// // // //     std::ofstream ofs;
// // // //     ofs.open(fName + ".gv", std::ofstream::out | std::ofstream::trunc);
// // // //     ofs << fullOut;
// // // //     ofs.close();
// // // //   };

    /** function for printing relationBetweenStaticSectors:
     *
     * - 'filters' contains the static sectors
    * - configName is used for the fileName to be able to recognize the outputs
    * - nHit-Combinations shows links of 2-, 3- or 4-static sectors in a row
    * - print2File prints to a file if yes or to screen if no
    * - suppressDeadSectors discards sectors without links if set to true
     */
    template<class HitType>
    void printStaticSectorRelations(const VXDTFFilters<HitType>& filters, std::string configName,
                                    unsigned int nHitCombinations = 2, bool print2File = true, bool suppressDeadSectors = true)
    {
      if (nHitCombinations <2 or nHitCombinations > 4)
        B2FATAL("printStaticSectorRelations: input-parameter wrong ("
                << nHitCombinations
                << ", allowed only 2-4), skipping print-function!");

      std::string secIDCombis = "## printed " + std::to_string(nHitCombinations) + "-sector-combi-output of secMap: " + configName +
                                "\n{";
      for (const auto* staticSector : filters.getStaticSectors()) {
        if (staticSector == nullptr) continue;
        std::string mainSecID = staticSector->getFullSecID().getFullSecString();

        if (nHitCombinations == 2) {
          const auto& innerSectors = staticSector->getInner2spSecIDs();
          if (innerSectors.empty()) {
            if (suppressDeadSectors == false) { secIDCombis += "\"" + mainSecID + "\",\n"; }
          } else {
            for (const auto& innerID : innerSectors) {
              secIDCombis += "\"" + mainSecID + "\" -> \"" + innerID.getFullSecString() + "\",\n";
            }
          }

        } else if (nHitCombinations == 3) {
          // TODO discuss: do we want to treat the nodes as sectors or as sector-pairs in this case? more correct would be sector-pair...)
          const auto& innerSectors = staticSector->getInner3spSecIDs();
          if (innerSectors.empty()) {
            if (suppressDeadSectors == false) { secIDCombis += "\"" + mainSecID + "\",\n"; }
          } else {
            for (const auto& innerIDpair : innerSectors) {
              secIDCombis += "\"" + mainSecID + "\" -> \"" + innerIDpair.first.getFullSecString() + "\",\n";
              secIDCombis += "\"" + innerIDpair.first.getFullSecString() + "\" -> \"" + innerIDpair.second.getFullSecString() + "\",\n";
            }
          }

        } else if (nHitCombinations == 4) {
          // TODO discuss: do we want to treat the nodes as sectors or as sector-pairs in this case? more correct would be sector-pair...)
          const auto& innerSectors = staticSector->getInner4spSecIDs();
          if (innerSectors.empty()) {
            if (suppressDeadSectors == false) { secIDCombis += "\"" + mainSecID + "\",\n"; }
          } else {
            for (const auto& innerIDtriplet : innerSectors) {
              secIDCombis += "\"" + mainSecID + "\" -> \"" + std::get<0>(innerIDtriplet).getFullSecString() + "\",\n";
              secIDCombis += "\"" + std::get<0>(innerIDtriplet).getFullSecString() + "\" -> \"" + std::get<1>
                             (innerIDtriplet).getFullSecString() + "\",\n";
              secIDCombis += "\"" + std::get<1>(innerIDtriplet).getFullSecString() + "\" -> \"" + std::get<2>
                             (innerIDtriplet).getFullSecString() + "\",\n";
            }
          }

        }
      }

      secIDCombis += "};";

      if (print2File == true) {
        B2DEBUG(99, "Printing static sector relations to file " << configName << "4Mathematica.txt...\n");
        std::ofstream ofs;
        ofs.open(configName + "4Mathematica.txt", std::ofstream::out | std::ofstream::trunc);
        ofs << secIDCombis;
        ofs.close();
      }
    }

  } // SecMapHelper namespace
} //Belle2 namespace
