/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <arich/dbobjects/ARICHPositionElement.h>

namespace Belle2 {

  /**
   * Alignment parameters for aerogel tiles.
   */
  class ARICHAeroTilesAlignment: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHAeroTilesAlignment()
    {}

    /**
     * Sets Alignment parameters (element) for aerogel tile
     * @param id slot id
     * @param element alignment element
     */
    void setAlignmentElement(int id, const ARICHPositionElement& pars)
    {
      m_elements[id - 1] = pars;
    }

    /**
     * Returns alignment parameters for given aerogel tile
     * @return position element
     */
    const ARICHPositionElement& getAlignmentElement(int mirrorID) const
    {
      return m_elements[mirrorID - 1];
    }

    /**
     * Prints parameters of aerogel tiles alignment
     */
    void print(const std::string& title = "Aerogel tilesAlignment Parameters") const
    {
      ARICHGeoBase::print(title);
      int i = 1;
      for (auto el : m_elements) { std::cout << "Aerogel tile slot " << i++ << std::endl; el.print(); std::cout << std::endl;}
    }


  private:

    std::vector<ARICHPositionElement> m_elements{std::vector<ARICHPositionElement>(124)}; /**< vector of position elements for alignment */
    ClassDef(ARICHAeroTilesAlignment, 1); /**< ClassDef */

  };

} // end namespace Belle2
