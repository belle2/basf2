/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//
#include <ecl/utility/ECLChannelMapper.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>
#include <framework/database/DBObjPtr.h>
#include <framework/utilities/FileSystem.h>
//
#include <fstream>
#include <string>

using namespace Belle2;
using namespace std;
using namespace ECL;

ECLChannelMapper::ECLChannelMapper()
{
  int i;
  for (i = 0; i < ECL_BARREL_CRATES * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER; i++)
    convertArrayBarrel[i] = 0;
  for (i = 0; i < ECL_FWD_CRATES * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER; i++)
    convertArrayFWD[i] = 0;
  for (i = 0; i < ECL_BKW_CRATES * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER; i++)
    convertArrayBKW[i] = 0;
  for (i = 0; i < ECL_TOTAL_CHANNELS; i++)
    for (int j = 0; j < 3; j++)
      convertArrayInv[i][j] = 0;

  isInitialized = false;

}

bool ECLChannelMapper::initFromFile()
{
  std::string filePath = FileSystem::findFile("ecl/data/ecl_channels_map.txt");
  return initFromFile(filePath.c_str());
}

ECLChannelMapper* ECLChannelMapper::instance = nullptr;
ECLChannelMapper* ECLChannelMapper::getInstance(bool use_db)
{
  if (!instance) {
    instance = new ECLChannelMapper();
    if (use_db) instance->initFromDB();
    else instance->initFromFile();
  }
  return instance;
}

bool ECLChannelMapper::initFromFile(const char* eclMapFileName)
{
  B2WARNING("Reading possibly outdated ECLChannelMap from " << eclMapFileName);

  ifstream mapFile(eclMapFileName);
  if (mapFile.is_open()) {

    float iCrate, iShaper, iChannel, thetaID, phiID, cellID;
    int arrayIndex = 0;
    int arrayCount = 0;
    while (mapFile.good()) {

      // Ignoring commented lines
      char ch = mapFile.get();
      if (ch == '#') {
        mapFile.ignore(256, '\n');
        continue;
      } else if (ch == '\n') {
        continue;
      } else {
        mapFile.unget();
      }

      mapFile >> iCrate >> iShaper >> iChannel >> thetaID >> phiID >> cellID;

      if (cellID > ECL_TOTAL_CHANNELS) {
        B2ERROR("ECLChannelMapper:: wrong cellID in the init file " << eclMapFileName);
        return false;
      }

      if (cellID > 0) {
        convertArrayInv[(int)cellID - 1][0] = (int)iCrate;
        convertArrayInv[(int)cellID - 1][1] = (int)iShaper;
        convertArrayInv[(int)cellID - 1][2] = (int)iChannel;
      }

      // Barrel
      if (iCrate >= 1 && iCrate <= 36) {
        arrayIndex = arrayCount;
        convertArrayBarrel[arrayIndex] = (int)cellID;
      }

      // Forward endcap
      if (iCrate > 36 && iCrate < 45) {
        arrayIndex = arrayCount - 36 * 12 * 16;

        if (arrayIndex >= 0 && arrayIndex < ECL_FWD_CRATES * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER)
          convertArrayFWD[arrayIndex] = (int)cellID;
      }

      // Backward endcap
      if (iCrate > 44) {
        arrayIndex = arrayCount - 36 * 12 * 16 - 8 * 10 * 16;
        if (arrayIndex >= 0 && arrayIndex < ECL_BKW_CRATES * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER)
          convertArrayBKW[arrayIndex] = (int)cellID;
      }
      arrayCount++;
    }
  } else {
    B2ERROR("ERROR:: file " << eclMapFileName << " doesn't found");
    return false;
  }

  isInitialized = true;

  return true;
}

bool ECLChannelMapper::initFromDB()
{
  DBObjPtr<Belle2::ECLChannelMap> channelMap("ECLChannelMap");
  // Re-initialize only if interval of validity was changed
  if (isInitialized && !channelMap.hasChanged()) {
    return true;
  }

  if (!channelMap.isValid()) {
    B2FATAL("ECLChannelMapper:: Could not get ECLChannelMap from the database.");
  }

  B2INFO("ECLChannelMapper:: loaded ECLChannelMap from the database"
         << LogVar("IoV", channelMap.getIoV()));

  const auto& mappingBAR = channelMap->getMappingBAR();
  const auto& mappingFWD = channelMap->getMappingFWD();
  const auto& mappingBWD = channelMap->getMappingBWD();

  int cellID;
  // Loop variables
  int iCrate = 1, iShaper = 1, iChannel = 1;
  // Index in currently used array (converArrayBarrel,*FWD,*BKW)
  int arrayIndex = 0;
  int iMaxShapers = ECL_BARREL_SHAPERS_IN_CRATE;

  while (iCrate <= ECL_CRATES) {
    if (iCrate <= ECL_BARREL_CRATES) {
      // Barrel
      cellID = mappingBAR[arrayIndex];
      convertArrayBarrel[arrayIndex] = cellID;
    } else if (iCrate <= ECL_BARREL_CRATES + ECL_FWD_CRATES) {
      // Forward endcap
      cellID = mappingFWD[arrayIndex];
      convertArrayFWD[arrayIndex] = cellID;
    } else {
      // Backward endcap
      cellID = mappingBWD[arrayIndex];
      convertArrayBKW[arrayIndex] = cellID;
    }

    if (cellID > ECL_TOTAL_CHANNELS) {
      B2FATAL("ECLChannelMapper:: wrong cellID (" << cellID << ") in the database payload");
      return false;
    }

    if (cellID > 0) {
      convertArrayInv[cellID - 1][0] = iCrate;
      convertArrayInv[cellID - 1][1] = iShaper;
      convertArrayInv[cellID - 1][2] = iChannel;
    }

    arrayIndex++;
    // Increment all indices, accounting for hierarchical
    // structure of channels <- shapers <- crates
    iChannel++;
    if (iChannel > ECL_CHANNELS_IN_SHAPER) {
      iChannel = 1;
      iShaper++;
      if (iShaper > iMaxShapers) {
        iShaper = 1;
        if (iCrate == ECL_BARREL_CRATES) {
          arrayIndex = 0;
          iMaxShapers = getNShapersInCrate(iCrate + 1);
        } else  if (iCrate == ECL_BARREL_CRATES + ECL_FWD_CRATES) {
          arrayIndex = 0;
          iMaxShapers = getNShapersInCrate(iCrate + 1);
        }
        iCrate++;
      }
    }
  }

  isInitialized = true;

  return true;
}

ECLChannelMap ECLChannelMapper::getDBObject()
{
  ECLChannelMap map;

  if (!isInitialized) {
    B2FATAL("ECLChannelMapper:: Tried to generate dbobject before initialization");
  }

  int mappingBAR_size = ECL_BARREL_CRATES * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER;
  int mappingFWD_size = ECL_FWD_CRATES    * ECL_FWD_SHAPERS_IN_CRATE    * ECL_CHANNELS_IN_SHAPER;
  int mappingBWD_size = ECL_BKW_CRATES    * ECL_BKW_SHAPERS_IN_CRATE    * ECL_CHANNELS_IN_SHAPER;

  std::vector<int> mappingBAR(mappingBAR_size);
  std::vector<int> mappingFWD(mappingFWD_size);
  std::vector<int> mappingBWD(mappingBWD_size);

  for (int i = 0; i < mappingBAR_size; i++)
    mappingBAR[i] = convertArrayBarrel[i];
  for (int i = 0; i < mappingFWD_size; i++)
    mappingFWD[i] = convertArrayFWD[i];
  for (int i = 0; i < mappingBWD_size; i++)
    mappingBWD[i] = convertArrayBKW[i];

  map.setMappingVectors(mappingBAR, mappingFWD, mappingBWD);

  return map;
}

int ECLChannelMapper::getCrateID(int iCOPPERNode, int iFINESSE)
{
  int iCrate;

  if (iFINESSE > ECL_FINESSES_IN_COPPER - 1) {
    B2ERROR("ECLChannelMapper::ERROR:: wrong FINESSE " << iFINESSE);
    return -1;
  }

  if ((iCOPPERNode & BECL_ID) == BECL_ID) {

    iCrate = (iCOPPERNode - BECL_ID - 1) * ECL_FINESSES_IN_COPPER + iFINESSE + 1;

  } else if ((iCOPPERNode & EECL_ID) == EECL_ID) {

    iCrate = ECL_BARREL_CRATES + iFINESSE * ECL_FWD_CRATES + (iCOPPERNode - EECL_ID - 1) + 1;

  } else {

    B2ERROR("ECLChannelMapper::ERROR:: wrong COPPER NodeID 0x" << std::hex << iCOPPERNode << " BECL_ID 0x" << BECL_ID << " EECL_ID 0x"
            << EECL_ID);
    return -1;
  }
//    B2DEBUG(100,"ECLChannelMapper:: " << std::hex << "0x"<<iCOPPERNode << " " << iFINESSE << " iCrate = " << std::dec << iCrate);
  if (iCrate > ECL_CRATES || iCrate < 1) {
    B2ERROR("ECLChannelMapper::getCrateID::ERROR:: wrong crate number " << iCrate << " return -1");
    return -1;
  }

  return iCrate;

}

int ECLChannelMapper::getCellId(int iCrate, int iShaper, int iChannel)
{
  // iCrate = 1  - 36  -- Barrel
  //          37 - 44  -- Forward
  //          45 - 52  -- Backward
  int cellID = 0;
  int arrayIndex;

  if (iCrate   < 1 || iCrate   > 52) return -1;
  if (iShaper  < 1 || iShaper  > 12) return -1;
  if (iChannel < 1 || iChannel > 16) return -1;

  if (iCrate >= 1 && iCrate <= 36) {
    //int thetaID = 1 + (iShaper - 1) * 4 + (iChannel - 1) % 4;
    //int phiID   = 1 + (iCrate  - 1) * 4 + (iChannel - 1) / 4;
    //cellID  = 1 + ECL_FWD_CHANNELS + (phiID - 1) + (thetaID - 1) * 144;

    arrayIndex = (iCrate - 1) * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
                 + (iShaper - 1) * ECL_CHANNELS_IN_SHAPER + (iChannel - 1);
    cellID = convertArrayBarrel[arrayIndex];

  }

  if (iCrate > 36 && iCrate < 45) {
    arrayIndex = (iCrate - 37) * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
                 + (iShaper - 1) * ECL_CHANNELS_IN_SHAPER + (iChannel - 1);
    cellID = convertArrayFWD[arrayIndex];
  }

  if (iCrate > 44) {
    arrayIndex = (iCrate - 45) * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
                 + (iShaper - 1) * ECL_CHANNELS_IN_SHAPER + (iChannel - 1);
    cellID = convertArrayBKW[arrayIndex];
  }


  return cellID;
}
int ECLChannelMapper::getCrateID(int cellID)
{
  if (cellID > 0 && cellID <= ECL_TOTAL_CHANNELS) {
    return convertArrayInv[cellID - 1 ][0];
  } else  return -1;
}

int ECLChannelMapper::getShaperPosition(int cellID)
{
  if (cellID > 0 && cellID <= ECL_TOTAL_CHANNELS) {
    return convertArrayInv[cellID - 1][1];
  } else  return -1;
}

int ECLChannelMapper::getShaperChannel(int cellID)
{
  if (cellID > 0 && cellID <= ECL_TOTAL_CHANNELS) {
    return convertArrayInv[cellID - 1][2];
  } else  return -1;
}

int ECLChannelMapper::getElectronicsID(int cellID)
{
  if (cellID < 1 || cellID > ECL_TOTAL_CHANNELS) return -1;

  int crate   = getCrateID(cellID);
  int shaper  = getShaperPosition(cellID);
  int channel = getShaperChannel(cellID);

  int arrayIndex = 0;
  // Total number of electronic IDs in barrel
  const int bar_eid_count = ECL_BARREL_CRATES * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER;
  // Total number of electronic IDs in forward endcap
  const int fwd_eid_count = ECL_FWD_CRATES    * ECL_FWD_SHAPERS_IN_CRATE    * ECL_CHANNELS_IN_SHAPER;

  if (crate <= 36) {
    arrayIndex += (crate - 1) * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
                  + (shaper - 1) * ECL_CHANNELS_IN_SHAPER + (channel - 1);
  } else if (crate <= 44) {
    arrayIndex += bar_eid_count;
    arrayIndex += (crate - 37) * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
                  + (shaper - 1) * ECL_CHANNELS_IN_SHAPER + (channel - 1);
  } else {
    arrayIndex += bar_eid_count + fwd_eid_count;
    arrayIndex += (crate - 45) * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER
                  + (shaper - 1) * ECL_CHANNELS_IN_SHAPER + (channel - 1);
  }

  return arrayIndex;
}

int ECLChannelMapper::getCOPPERNode(int iCrate)
{
  int systemID, iNode, iCOPPERNode;

  if (iCrate < 1 || iCrate > ECL_CRATES) return -1;
  systemID = (iCrate <= ECL_BARREL_CRATES) * BECL_ID + (iCrate > ECL_BARREL_CRATES) * EECL_ID;
  iNode = (iCrate <= ECL_BARREL_CRATES) ? (iCrate - 1) / 2 : (iCrate - ECL_BARREL_CRATES - 1) % 8;
  iCOPPERNode = systemID + iNode + 1;

  return iCOPPERNode;
}

int ECLChannelMapper::getFINESSE(int iCrate)
{
  if (iCrate < 1 || iCrate > ECL_CRATES) return -1;

  return (iCrate <= ECL_BARREL_CRATES) ? (iCrate - 1) % 2 : (iCrate - ECL_BARREL_CRATES - 1) / 8;
}

int ECLChannelMapper::getSubSystem(int iCrate)
{
  if (iCrate <= ECL_BARREL_CRATES) return 0;
  if (ECL_BARREL_CRATES < iCrate && iCrate <= ECL_BARREL_CRATES + ECL_FWD_CRATES)
    return 1;
  if (ECL_BARREL_CRATES + ECL_FWD_CRATES < iCrate && iCrate <= ECL_CRATES) return 2;
  return -1;
}

