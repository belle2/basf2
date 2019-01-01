/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <arich/dbobjects/ARICHGeoBase.h>
#include <arich/dbobjects/ARICHPositionElement.h>
#include <TVector3.h>
#include <TRotation.h>

namespace Belle2 {

  /**
   * Mirror displacement parameters for ARICH.
   */
  class ARICHGeoMirrorDisplacement: public ARICHGeoBase {
  public:

    /**
     * Default constructor
     */
    ARICHGeoMirrorDisplacement()
    {}

    ~ARICHGeoMirrorDisplacement()
    {}

    /**
     * Sets displacement parameters (element) for mirror plate
     * @param id mirror id
     * @param element displacement element
     */
    void setDisplacementElement(int id, const ARICHPositionElement& pars)
    {
      m_elements[id - 1] = pars;
    }

    /**
     * Returns displacement parameters for given mirror plate
     * @return position element
     */
    const ARICHPositionElement& getDisplacementElement(int mirrorID) const
    {
      return m_elements[mirrorID - 1];
    }

    void print(const std::string& title = "Mirror Displacement Parameters") const
    {
      ARICHGeoBase::print(title);
      int i = 1;
      for (auto el : m_elements) { std::cout << "Mirror plate " << i++ << std::endl; el.print(); std::cout << std::endl;}
    }


  private:

    std::vector<ARICHPositionElement> m_elements{std::vector<ARICHPositionElement>(18)}; /**< vector of positions elements for displacement */
    ClassDef(ARICHGeoMirrorDisplacement, 1); /**< ClassDef */

  };

} // end namespace Belle2
