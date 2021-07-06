/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>

#include <iostream>
#include <iomanip>
#include <vector>

namespace Belle2 {

  /**
   * Structure which holds apexes of the tessellation volumes.
   * Initially for ARICH detector (merger cooling bodies).
   */
  struct tessellatedSolidStr {
    int tessellatedSolidID;
    unsigned int nCells;
    unsigned int nApexPerCell;
    // Tessellated solid represented by the array (vector) of triangles
    std::vector<std::vector<double>> posV1; // x, y, z of apex1
    std::vector<std::vector<double>> posV2; // x, y, z of apex2
    std::vector<std::vector<double>> posV3; // x, y, z of apex3
    tessellatedSolidStr()
    {
      tessellatedSolidID = 0;
      nCells = 0;
      nApexPerCell = 0;
    }
    ~tessellatedSolidStr() {;}
    void defineDummyApex()
    {
      std::vector<double> apex;
      for (unsigned int i = 0; i < 3; i++)
        apex.push_back(i);
      posV1.push_back(apex);
      posV2.push_back(apex);
      posV3.push_back(apex);
    }
    void pushBackApexesCoordinates(const std::vector<double>& apex1x, const std::vector<double>& apex1y,
                                   const std::vector<double>& apex1z,
                                   const std::vector<double>& apex2x, const std::vector<double>& apex2y, const std::vector<double>& apex2z,
                                   const std::vector<double>& apex3x, const std::vector<double>& apex3y, const std::vector<double>& apex3z)
    {
      if (nCells != apex1x.size())
        B2ERROR("tessellatedSolidStr: apex1x.size() != nCells");
      if (nCells != apex1y.size())
        B2ERROR("tessellatedSolidStr: apex1y.size() != nCells");
      if (nCells != apex1z.size())
        B2ERROR("tessellatedSolidStr: apex1z.size() != nCells");
      if (nCells != apex2x.size())
        B2ERROR("tessellatedSolidStr: apex2x.size() != nCells");
      if (nCells != apex2y.size())
        B2ERROR("tessellatedSolidStr: apex2y.size() != nCells");
      if (nCells != apex2z.size())
        B2ERROR("tessellatedSolidStr: apex2z.size() != nCells");
      if (nCells != apex3x.size())
        B2ERROR("tessellatedSolidStr: apex3x.size() != nCells");
      if (nCells != apex3y.size())
        B2ERROR("tessellatedSolidStr: apex3y.size() != nCells");
      if (nCells != apex3z.size())
        B2ERROR("tessellatedSolidStr: apex3z.size() != nCells");
      for (unsigned int i = 0; i < nCells; i++) {
        std::vector<double> apex1;
        std::vector<double> apex2;
        std::vector<double> apex3;
        apex1.push_back(apex1x.at(i));
        apex1.push_back(apex1y.at(i));
        apex1.push_back(apex1z.at(i));
        apex2.push_back(apex2x.at(i));
        apex2.push_back(apex2y.at(i));
        apex2.push_back(apex2z.at(i));
        apex3.push_back(apex3x.at(i));
        apex3.push_back(apex3y.at(i));
        apex3.push_back(apex3z.at(i));
        posV1.push_back(apex1);
        posV2.push_back(apex2);
        posV3.push_back(apex3);
      }
    }
    void printInfo(int verboseLevel = 0)
    {
      std::cout << "tessellatedSolidID " << tessellatedSolidID << std::endl
                << "nCells             " << nCells << std::endl
                << "nApexPerCell       " << nApexPerCell << std::endl;
      if (verboseLevel > 0) {
        for (unsigned int i = 0; i < posV1.size(); i++) {
          std::cout << std::setw(20) << "apex1 : x y z " << " " << posV1[i][0] << " " << posV1[i][1] << " " << posV1[i][2] << std::endl
                    << std::setw(20) << "apex2 : x y z " << " " << posV2[i][0] << " " << posV2[i][1] << " " << posV2[i][2] << std::endl
                    << std::setw(20) << "apex3 : x y z " << " " << posV3[i][0] << " " << posV3[i][1] << " " << posV3[i][2] << std::endl;
        }
      }
    }
  };//struct tessellatedSolidStr {

}//namespace Belle2 {
