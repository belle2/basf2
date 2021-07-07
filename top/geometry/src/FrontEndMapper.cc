/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/geometry/FrontEndMapper.h>
#include <framework/database/DBImportArray.h>
#include <framework/logging/LogSystem.h>
#include <iostream>


using namespace std;

namespace Belle2 {
  namespace TOP {

    FrontEndMapper::FrontEndMapper()
    {
      for (unsigned i = 0; i < c_numModules; i++) {
        for (unsigned k = 0; k < c_numColumns; k++) {
          m_fromModule[i][k] = 0;
        }
      }
    }


    FrontEndMapper::~FrontEndMapper()
    {
      if (m_mappingDB) delete m_mappingDB;
    }


    void FrontEndMapper::initialize(const GearDir& frontEndMapping)
    {

      clear();

      // unordered sets used to check that ...

      unordered_set<unsigned short> scrodIDs; // all SCROD ID's are different
      unordered_set<string> coppers; // COPPER inputs are used only once
      unordered_set<int> modules; // (moduleID, col) mapped only once

      // read parameters from gerabox
      int numModules = 0; // counter of mapped modules
      for (const GearDir& topModule : frontEndMapping.getNodes("TOPModule")) {

        int moduleCNumber = topModule.getInt("@CNumber");
        int moduleID = topModule.getInt("moduleID");
        if (moduleID == 0) continue; // module is not installed into barrel
        if (moduleID < 0 or moduleID > c_numModules) {
          B2ERROR("TOP::FrontEndMapper: invalid moduleID in xml file"
                  << LogVar("moduleID", moduleID)
                  << LogVar("path", topModule.getPath()));
          return;
        }

        bool moduleMapped = false; // to count mapped modules
        for (const GearDir& boardstack : topModule.getNodes("Boardstack")) {

          int col = boardstack.getInt("@col");
          if (col < 0 or col >= c_numColumns) {
            B2ERROR("TOP::FrontEndMapper: invalid boardstack number in xml file"
                    << LogVar("moduleID", moduleID)
                    << LogVar("boardstack", col)
                    << LogVar("path", boardstack.getPath()));
            return;
          }
          if (!modules.insert(moduleID * c_numColumns + col).second) {
            B2ERROR("TOP::FrontEndMapper: this boardstack is already mapped."
                    << LogVar("moduleID", moduleID)
                    << LogVar("boardstack", col)
                    << LogVar("path", boardstack.getPath()));
            return;
          }

          unsigned short scrodID = (unsigned short) boardstack.getInt("SCRODid");
          if (!scrodIDs.insert(scrodID).second) {
            B2ERROR("TOP::FrontEndMapper: this SCROD ID is already used."
                    << LogVar("moduleID", moduleID)
                    << LogVar("boardstack", col)
                    << LogVar("scrod", scrodID)
                    << LogVar("path", boardstack.getPath() + "/SCRODid"));
            return;
          }

          string finesseSlot = boardstack.getString("FinesseSlot");
          int finesse = 0;
          if (finesseSlot == "A") {finesse = 0;}
          else if (finesseSlot == "B") {finesse = 1;}
          else if (finesseSlot == "C") {finesse = 2;}
          else if (finesseSlot == "D") {finesse = 3;}
          else {
            B2ERROR("TOP::FrontEndMapper: invalid slot (valid are A, B, C, D)."
                    << LogVar("FinesseSlot", finesseSlot)
                    << LogVar("path", boardstack.getPath() + "/FinesseSlot"));
            return;
          }

          unsigned copperID = (unsigned) boardstack.getInt("COPPERid");
          m_copperIDs.insert(copperID);
          string copper = boardstack.getString("COPPERid") + " " + finesseSlot;
          if (!coppers.insert(copper).second) {
            B2ERROR("TOP::FrontEndMapper: this COPPER ID is already used."
                    << LogVar("moduleID", moduleID)
                    << LogVar("boardstack", col)
                    << LogVar("copperID", copper)
                    << LogVar("path", boardstack.getPath() + "/COPPERid"));
            return;
          }

          TOPFrontEndMap feemap(moduleID, moduleCNumber, col, scrodID, copperID, finesse,
                                m_mapping.size());
          m_mapping.push_back(feemap);
          moduleMapped = true;
        }
        if (moduleMapped) numModules++;
      }

      // set conversion objects

      for (const auto& feemap : m_mapping) {
        m_fromModule[feemap.getModuleID() - 1][feemap.getBoardstackNumber()] = &feemap;
        m_fromScrod[feemap.getScrodID()] = &feemap;
        m_fromCopper[feemap.getCopperID() * 4 + feemap.getFinesseSlot()] = &feemap;
      }
      m_valid = true;

      B2INFO("TOP::FrontEndMapper: " << m_mapping.size() << " SCROD's mapped to "
             << numModules << " TOP module(s)");

      // print mappings if debug level for package 'top' is set to 100 or larger
      const auto& logSystem = LogSystem::Instance();
      if (logSystem.isLevelEnabled(LogConfig::c_Debug, 100, "top")) {
        print();
      }

    }


    void FrontEndMapper::initialize()
    {

      if (m_mappingDB) delete m_mappingDB;
      m_mappingDB = new DBArray<TOPFrontEndMap>();

      if (!m_mappingDB->isValid()) {
        clear();
        return;
      }
      update();

      m_mappingDB->addCallback(this, &FrontEndMapper::update);

      const auto& logSystem = LogSystem::Instance();
      if (logSystem.isLevelEnabled(LogConfig::c_Debug, 100, "top")) {
        print();
      }

    }


    void FrontEndMapper::import(const IntervalOfValidity& iov) const
    {
      DBImportArray<TOPFrontEndMap> array;
      for (const auto& map : m_mapping) {
        array.appendNew(map);
      }
      array.import(iov);
    }


    void FrontEndMapper::clear()
    {
      m_mapping.clear();
      m_copperIDs.clear();
      m_fromScrod.clear();
      m_fromCopper.clear();
      for (unsigned i = 0; i < c_numModules; i++) {
        for (unsigned k = 0; k < c_numColumns; k++) {
          m_fromModule[i][k] = 0;
        }
      }
      m_valid = false;
      m_fromDB = false;
    }


    void FrontEndMapper::update()
    {
      clear();
      if (!m_mappingDB->isValid()) return;

      for (const auto& feemap : *m_mappingDB) {
        m_copperIDs.insert(feemap.getCopperID());
        m_fromModule[feemap.getModuleID() - 1][feemap.getBoardstackNumber()] = &feemap;
        m_fromScrod[feemap.getScrodID()] = &feemap;
        m_fromCopper[feemap.getCopperID() * 4 + feemap.getFinesseSlot()] = &feemap;
      }
      m_valid = true;
      m_fromDB = true;
    }


    void FrontEndMapper::print() const
    {
      cout << endl;
      cout << "           Mapping of TOP front-end electronics" << endl << endl;

      char label[5] = "ABCD";
      for (int i = 0; i < c_numModules; i++) {
        int moduleID = i + 1;
        cout << " slot " << moduleID
             << " (module " << getModuleCNumber(moduleID) << "):" << endl;
        for (int bs = 0; bs < c_numColumns; bs++) {
          const auto* map = getMap(moduleID, bs);
          if (!map) continue;
          cout << "   BS" << bs;
          cout << " scrod " << map->getScrodID();
          if (map->getScrodID() < 10) cout << " ";
          if (map->getScrodID() < 100) cout << " ";
          cout << "  copper " << map->getCopperID();
          cout << " " << label[map->getFinesseSlot()];
          cout << endl;
        }
      }
      cout << endl;

    }



  } // TOP namespace
} // Belle2 namespace
