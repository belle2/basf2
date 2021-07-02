/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>

//number of channels per HAPD
#define N_XCHANNELS 12
#define N_YCHANNELS 12

namespace Belle2 {

  //! The Class for ARICH HAPD channel mapping
  /*! This class provides mapping from hapd channel x,y numbers (both 0-11) to channel asic numbers (0-143)
  */

  class ARICHChannelMapping : public TObject {

  public:

    //! Default constructor
    ARICHChannelMapping();

    /**
     * Get HAPD channel X,Y numbers from asic channel number
     * @param asicChn
     * @param xChn returned channel X number
     * @param yChn returned channel Y number
     */
    bool getXYFromAsic(unsigned asicChn, int& xChn, int& yChn) const;

    /**
     * Get asic channel number from HAPD channel X,Y numbers
     * @param xChn  channel X number
     * @param yChn  channel Y number
     * @return asic channel number
     */
    int getAsicFromXY(unsigned xChn, unsigned yChn) const;

    /**
     * Add entry to channel map
     * @param xChn  channel X number
     * @param yChn  channel Y number
     * @param asicChn  asic channel number
     */
    void mapXY2Asic(unsigned xChn, unsigned yChn, unsigned asicChn);

    /**
    * Print mapping parameters
    */
    void print() const;

  private:

    std::vector<uint8_t> m_xy2asic; /**< map of X,Y to asic channels numbers */
    std::vector<uint8_t> m_asic2xy; /**<  map of asic to X,Y channel numbers */

    ClassDef(ARICHChannelMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
