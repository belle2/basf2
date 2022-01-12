/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <G4VSolid.hh>
#include <G4Transform3D.hh>
#include "ecl/dbobjects/ECLCrystalsShapeAndPosition.h"

namespace Belle2 {
  namespace ECL {
    double cosd(double);
    double sind(double);
    double tand(double);

#define UNUSED __attribute__((unused))

    /** shape */
    struct shape_t {
      int nshape; /**< shapes */
      shape_t(): nshape(0) {}
      virtual ~shape_t() {}

      /** get solid */
      G4VSolid* get_solid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const;
      /** get tesselated solid */
      virtual G4VSolid* get_tesselatedsolid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      /** get trapezoid */
      virtual G4VSolid* get_trapezoid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      /** get extruded solid */
      virtual G4VSolid* get_extrudedsolid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      /** get Belle crystal */
      virtual G4VSolid* get_bellecrystal(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      /** is trapped */
      virtual bool istrap() const = 0;
    };

    /** placement struct */
    struct cplacement_t {
      int nshape; /**< shapes */
      double Rphi1; /**< Eulers' angle phi 1 */
      double Rtheta; /**< Eulers' angle theta */
      double Rphi2; /**< Eulers' angle phi 2*/
      double Pr; /**< radius of origin position */
      double Ptheta; /**< polar angle of origin position */
      double Pphi; /**< azimuthal angle of origin position */
    };

    std::vector<shape_t*> load_shapes(const std::string& fname);
    std::vector<cplacement_t> load_placements(const std::string& fname);

    struct Point_t;
    Point_t centerofgravity(Point_t*, Point_t*);
    G4Transform3D get_transform(const cplacement_t& t);

    inline std::string suf(const std::string& s, int indx)
    {
      std::string r(s);
      r += "_" + std::to_string(indx);
      return r;
    }

    enum ECLParts { forward, barrel, backward };
    Belle2::ECLCrystalsShapeAndPosition loadCrystalsShapeAndPosition();

    std::vector<shape_t*> load_shapes(const Belle2::ECLCrystalsShapeAndPosition*, enum ECLParts);
    std::vector<cplacement_t> load_placements(const Belle2::ECLCrystalsShapeAndPosition*, enum ECLParts);

  }
}
