/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#ifndef G4TRIANGULARPRISM_H_
#define G4TRIANGULARPRISM_H_

#include "G4IntersectionSolid.hh"
#include "G4Tubs.hh"
#include "G4Box.hh"

namespace Belle2 {

  /**
   * G4TriangularPrism - triangular prism
   */
  class G4TriangularPrism {

  public:

    /**
     * Constructor.
     * @r1: first point radius
     * @phi1: first point angle
     * @r2: second point radius
     * @phi2: second point angle
     * @halfZlen: half of Z length
     *
     * Coordinates of the third point are (0, 0)
     */
    G4TriangularPrism(const G4String &name, double r1, double phi1,
                      double r2, double phi2, G4double halfZlen);

    /**
     * Destructor.
     */
    ~G4TriangularPrism();

    /**
     * getSolid - get solid
     */
    G4VSolid *getSolid();

  private:

    /**
     * Box.
     */
    G4Box *m_box;

    /**
     * Tube.
     */
    G4Tubs *m_tube;

    /**
     * Intersection.
     */
    G4IntersectionSolid *m_is;

  };

}

#endif

