#include "ecl/utility/ECLChannelMapper.h"
#include <iostream>

#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <rawdata/dataobjects/RawCOPPERFormat.h>

using namespace Belle2;
using namespace std;
using namespace ECL;

ECLChannelMapper::ECLChannelMapper()
{
  int i = 0, j = 0;
  for (i = 0; i < ECL_BARREL_CRATES * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER; i++)
    convertArrayBarrel[i] = 0;
  for (i = 0; i < ECL_FWD_CRATES * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER; i++)
    convertArrayFWD[i] = 0;
  for (i = 0; i < ECL_BKW_CRATES * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER; i++)
    convertArrayBKW[i] = 0;
  for (i = 0; i < ECL_TOTAL_CHANNELS; i++)
    for (j = 0; j < 3; j++)
      convertArrayInv[i][j] = 0;

  isInitialized = false;

}

bool ECLChannelMapper::initFromFile(const char* eclMapFileName = "crpsch.dat")
{


  ifstream mapFile(eclMapFileName);
  if (mapFile.is_open()) {

    float iCrate, iShaper, iChannel, thetaID, phiID, cellID;
    int arrayIndex = 0;
    int arrayCount = 0;
    while (mapFile.good()) {

      // Ignoring commented lines
      char ch = mapFile.get();
      switch (ch) {
        case '#':
          mapFile.ignore(256, '\n');
          break;
        case '\n':
          B2DEBUG(100, "Ignored comment/empty line");
          continue;
          break;

        default:
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

      if (iCrate >= 1 && iCrate <= 36) {
        arrayIndex = arrayCount;
//        std::cout << arrayIndex << " " << cellID << std::endl;
        convertArrayBarrel[arrayIndex] = (int)cellID;
      }

      if (iCrate > 36 && iCrate < 45) {
        arrayIndex = arrayCount - 36 * 12 * 16;

//        std::cout << arrayIndex << " " << cellID << std::endl
        if (arrayIndex >= 0 && arrayIndex < ECL_FWD_CRATES * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER)
          convertArrayFWD[arrayIndex] = (int)cellID;
      }

      if (iCrate > 44) {
        arrayIndex = arrayCount - 36 * 12 * 16 - 8 * 10 * 16;
//        std::cout << arrayIndex << " " << cellID << std::endl;
        if (arrayIndex >= 0 && arrayIndex < ECL_BKW_CRATES * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER)
          convertArrayBKW[arrayIndex] = (int)cellID;
      }
      arrayCount++;
    }
  } else {
    B2ERROR("ERROR:: file " << eclMapFileName << " doesn't found");
    return false;
  }

//  std::cout << "ECL Channel mapper is initialized \n";

  isInitialized = true;

  return true;


}

bool ECLChannelMapper::initFromDB()
{
  // TODO
  return false;
}


int ECLChannelMapper::getCrateID(int iCOPPERNode, int iFINESSE)
{
  int iCrate;

  //  B2DEBUG(50, "iCOPPERNode = %x" << iCOPPERNode);
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
  int arrayIndex = 0;

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

int ECLChannelMapper::getCOPPERNode(int iCrate)
{
  int systemID, iNode, iCOPPERNode;

  if (iCrate < 1 || iCrate > ECL_CRATES) return -1;
  systemID = (iCrate <= ECL_BARREL_CRATES) * BECL_ID + (iCrate > ECL_BARREL_CRATES) * EECL_ID;
  iNode = (iCrate <= ECL_BARREL_CRATES) ? iCrate : iCrate - ECL_BARREL_CRATES;
  iCOPPERNode = systemID + iNode;

  return iCOPPERNode;
}

int ECLChannelMapper::getFINESSE(int iCrate)
{
  if (iCrate < 1 || iCrate > ECL_CRATES) return -1;

  return 1;
// not implemented yet TODO

}

int ECLChannelMapper::getSubSystem(int iCrate)
{
  if (iCrate <= ECL_BARREL_CRATES) return 0;
  if (ECL_BARREL_CRATES < iCrate && iCrate <= ECL_BARREL_CRATES + ECL_FWD_CRATES)
    return 1;
  if (ECL_BARREL_CRATES + ECL_FWD_CRATES < iCrate && iCrate <= ECL_CRATES) return 2;
  return -1;
}

