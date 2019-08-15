/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <G4IntersectionSolid.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>

namespace Belle2 {

  /**
   * Triangular prism.
   */
  class G4TriangularPrism {

  public:

    /**
     * Constructor.
     * @param[in] r1        First point radius.
     * @param[in] phi1      First point angle.
     * @param[in] r2        Second point radius.
     * @param[in] phi2      Second point angle.
     * @param[in] halfZlen  Half of Z length.
     * @details
     * Coordinates of the third point are (0, 0)
     */
    G4TriangularPrism(const G4String& name, double r1, double phi1,
                      double r2, double phi2, G4double halfZlen);

    /**
     * Copy constructor (disabled).
     */
    G4TriangularPrism(const G4TriangularPrism&) = delete;

    /**
     * Operator =  (disabled).
     */
    G4TriangularPrism& operator=(const G4TriangularPrism&) = delete;

    /**
     * Destructor.
     * @details
     * The resulting solid m_is (access by getSolid()) is not deleted here.
     * It is meant to be deleted by Geant.
     */
    ~G4TriangularPrism();

    /**
     * Get solid.
     * @return Solid corresponding to the prism.
     * @details
     * The resulting G4VSolid* is allocated via operator new and must be
     * deleted when is is not necessary. If it is used in Geant geometry,
     * do not delete it.
     */
    G4VSolid* getSolid();

  private:

    /**
     * Box.
     */
    G4Box* m_box;

    /**
     * Tube.
     */
    G4Tubs* m_tube;

    /**
     * Intersection.
     */
    G4IntersectionSolid* m_is;

  };

}
