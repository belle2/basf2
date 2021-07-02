/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <ecl/dbobjects/ECLChannelMap.h>
namespace Belle2 {
  namespace ECL {
    /**
     * This class provides access to ECL channel map that is either
     *  a) Loaded from the database (see ecl/dbobject/include/ECLChannelMap.h).
     *  b) (** NOT SUPPORTED **) Loaded from the text file (standard location
     *     is FileSystem::findFile("ecl/data/ecl_channels_map.txt"))
     *
     * Please note that (a) is recommended and (b) should be used only for
     * very specific rare cases (such as prepation of new ECLChannelMap payloads)
     */
    class ECLChannelMapper {

      /// initialization flag
      bool isInitialized;

      /// convert (iCrate, iShaper, iChannel) to CellId for Barrel ECL
      int convertArrayBarrel[ECL_BARREL_CRATES * ECL_BARREL_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER];
      /// convert (iCrate, iShaper, iChannel) to CellId for Forward endcap
      int convertArrayFWD[ECL_FWD_CRATES * ECL_FWD_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER];
      /// convert (iCrate, iShaper, iChannel) to CellId Backward endcap
      int convertArrayBKW[ECL_BKW_CRATES * ECL_BKW_SHAPERS_IN_CRATE * ECL_CHANNELS_IN_SHAPER];
      /// convert CellId to (iCrate, iShaper, iChannel)
      int convertArrayInv[ECL_TOTAL_CHANNELS][3]; // 0 -- icrate, 1 -- iboard, 2 -- ichannel

    public:
      /// Default constructor
      ECLChannelMapper();
      /// Default destructor
      ~ECLChannelMapper() {}
      /// Initialize channel mapper using data stored in default location
      bool initFromFile();
      /// Initialize channel mapper using data stored in the ASCII file
      bool initFromFile(const char* eclMapFile);
      /// Initialize channel mapper from the conditions database
      bool initFromDB();
      /// Convert internal data to ECLChannelMap database object
      ECLChannelMap getDBObject();

      /// get crate number by given COPPER node number and FINESSE number
      int getCrateID(int iCOPPERNode, int iFINESSE); // iFINNES = 0 (FINESSE A) or 1 (FINESSE B)
      /// get CellId by given crate number, shaper position in the crate and DSP channel number in the shaper
      int getCellId(int iCrate, int iShaper, int iChannel);
      /// get number of COPPER node by given crate number
      int getCOPPERNode(int iCrate);
      /// get number of FINESSE (0/1) in COPPER by given crate number
      int getFINESSE(int iCrate);
      /// get ECL subsystem ID by given crate number: 0 -- barrel, 1 -- forward. 2 -- backward endcap
      int getSubSystem(int iCrate);

      /// get crate number by given CellId
      int getCrateID(int cellID);
      /// get position of the shaper in the crate by given CellId
      int getShaperPosition(int cellID);
      /// get number of DSP channel in the shaper by given number of  CellId
      int getShaperChannel(int cellID);

      /// get number of ShaperDSP modules in the given VME crate number
      inline int getNShapersInCrate(int iCrate)
      {
        if (iCrate <= ECL_BARREL_CRATES) return ECL_BARREL_SHAPERS_IN_CRATE;
        if (ECL_BARREL_CRATES < iCrate && iCrate <= ECL_BARREL_CRATES + ECL_FWD_CRATES) return ECL_FWD_SHAPERS_IN_CRATE;
        if (ECL_BARREL_CRATES + ECL_FWD_CRATES < iCrate && iCrate <= ECL_CRATES) return ECL_BKW_SHAPERS_IN_CRATE;
        return 0;
      }
    };
  }
}
