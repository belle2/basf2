/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/FrontEndMapper.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    FrontEndMapper::FrontEndMapper()
    {
      for (unsigned i = 0; i < c_numModules; i++) {
        for (unsigned k = 0; k < c_numColumns; k++) {
          m_fromBar[i][k] = 0;
        }
      }
    }


    FrontEndMapper::~FrontEndMapper()
    {
    }

    void FrontEndMapper::initialize(const GearDir& frontEndMapping)
    {

      // unordered sets used to check that ...

      unordered_set<unsigned short> scrodIDs; // all SCROD ID's are different
      unordered_set<string> coppers; // COPPER inputs are used only once
      unordered_set<int> bars; // (barID, col) mapped only once

      // read parameters from DB
      int numBars = 0; // counter of mapped bars
      for (const GearDir& topModule : frontEndMapping.getNodes("TOPModule")) {

        int barID = topModule.getInt("@barID");
        if (barID <= 0 or barID > c_numModules) {
          B2ERROR(topModule.getPath() << " barID=" << barID << " ***invalid ID");
          continue;
        }

        bool barMapped = false; // to count mapped bars
        for (const GearDir& boardstack : topModule.getNodes("Boardstack")) {

          int col = boardstack.getInt("@col");
          if (col < 0 or col >= c_numColumns) {
            B2ERROR(boardstack.getPath() << " col=" << col << " ***invalid number");
            continue;
          }
          if (!bars.insert(barID * c_numColumns + col).second) {
            B2ERROR(boardstack.getPath()
                    << " barID=" << barID
                    << " col=" << col
                    << " ***already mapped");
            continue;
          }

          unsigned short scrodID = (unsigned short) boardstack.getInt("SCRODid");
          if (!scrodIDs.insert(scrodID).second) {
            B2ERROR(boardstack.getPath() << "/SCRODid " << scrodID <<
                    " ***already used");
            continue;
          }

          string finesseSlot = boardstack.getString("FinesseSlot");
          int finesse = 0;
          if (finesseSlot == "A") {finesse = 0;}
          else if (finesseSlot == "B") {finesse = 1;}
          else if (finesseSlot == "C") {finesse = 2;}
          else if (finesseSlot == "D") {finesse = 3;}
          else {
            B2ERROR(boardstack.getPath() << "/FinesseSlot " << finesseSlot <<
                    " ***invalid slot (valid are A, B, C, D)");
            continue;
          }

          unsigned copperID = (unsigned) boardstack.getInt("COPPERid");
          m_copperIDs.insert(copperID);
          string copper = boardstack.getString("COPPERid") + " " + finesseSlot;
          if (!coppers.insert(copper).second) {
            B2ERROR(boardstack.getPath() << "/COPPERid " << copper <<
                    " ***input already used");
            continue;
          }

          TOPFrontEndMap feemap(barID, col, scrodID, copperID, finesse, m_mapping.size());
          m_mapping.push_back(feemap);
          barMapped = true;
        }
        if (barMapped) numBars++;
      }

      // set conversion objects

      for (const auto& feemap : m_mapping) {
        m_fromBar[feemap.getBarID() - 1][feemap.getBoardstackNumber()] = &feemap;
        m_fromScrod[feemap.getScrodID()] = &feemap;
        m_fromCopper[feemap.getCopperID() * 4 + feemap.getFinesseSlot()] = &feemap;
      }

      B2INFO("TOP::FrontEndMapper: " << m_mapping.size() << " SCROD's mapped to "
             << numBars << " TOP module(s)");

    }

  } // TOP namespace
} // Belle2 namespace
