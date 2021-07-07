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
   * Mirror alignment parameters for ARICH.
   */
  class ARICHMirrorAlignment: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHMirrorAlignment()
    {}

    ~ARICHMirrorAlignment()
    {}

    /**
     * Sets Alignment parameters (element) for mirror plate
     * @param id mirror id
     * @param element displacement element
     */
    void setAlignmentElement(int id, const ARICHPositionElement& pars)
    {
      m_elements[id - 1] = pars;
    }

    /**
     * Returns alignment parameters for given mirror plate
     * @return position element
     */
    const ARICHPositionElement& getAlignmentElement(int mirrorID) const
    {
      return m_elements[mirrorID - 1];
    }

    void print(const std::string& title = "Mirror Alignment Parameters") const
    {
      ARICHGeoBase::print(title);
      int i = 1;
      for (auto el : m_elements) { std::cout << "Mirror plate " << i++ << std::endl; el.print(); std::cout << std::endl;}
    }


  private:

    std::vector<ARICHPositionElement> m_elements{std::vector<ARICHPositionElement>(18)}; /**< vector of position elements for alignment */
    ClassDef(ARICHMirrorAlignment, 1); /**< ClassDef */

  };

} // end namespace Belle2
