/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <iostream>
#include <vector>


namespace Belle2 {

  /** FEB header size in bytes. */
#define ARICHFEB_HEADER_SIZE 10

  /** Raw header size in bytes. */
#define ARICHRAW_HEADER_SIZE 12

  /**
   * ARICH raw-data header.
   */
  struct ARICHRawHeader {
    uint8_t type = 0; /**< type */
    uint8_t version = 0; /**< version */
    uint8_t mergerID = 0; /**< merger-board identifier */
    uint8_t FEBSlot = 0; /**< FEB slot */
    uint32_t length = 0; /**< data length */
    uint32_t trigger = 0; /**< trigger number */
    std::vector<bool> SEU_FEB = std::vector<bool>(6, false);  /**< vector of SEU FEBs */
    bool thscan_mode = false; /**< thscan mode */
    unsigned vth = 0; /**< vth */

    /**
     * Print information about ARICHRawHeader.
     */
    void print()
    {
      std::cout << "Type: " << unsigned(type) << " version: " <<  unsigned(version) << std::endl;
      std::cout << "mergerID: " <<  unsigned(mergerID) << " FEBSlot: " <<  unsigned(FEBSlot) << std::endl;
      std::cout << "data length: " << length << " trigger No.: " << trigger << std::endl;
      std::cout << "SEU of FEB: " << SEU_FEB[5] << " " << SEU_FEB[4] << " " << SEU_FEB[3] << " " << SEU_FEB[2] << " " << SEU_FEB[1] << " "
                << SEU_FEB[0] << std::endl;
      std::cout << "thscan_mode: " << thscan_mode << ", vth: "  << vth << std::endl;
    }

  };
}
