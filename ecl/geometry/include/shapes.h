#ifndef __h_shapes
#define __h_shapes
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

    struct shape_t {
      int nshape;
      shape_t(): nshape(0) {}
      virtual ~shape_t() {}

      G4VSolid* get_solid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const;
      virtual G4VSolid* get_tesselatedsolid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      virtual G4VSolid* get_trapezoid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      virtual G4VSolid* get_extrudedsolid(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      virtual G4VSolid* get_bellecrystal(const std::string& prefix, double wrapthick, G4Translate3D& shift) const = 0;
      virtual bool istrap() const = 0;
    };

    struct cplacement_t {
      int nshape;
      double Rphi1, Rtheta, Rphi2; // Eulers' angles
      double Pr, Ptheta, Pphi; // origin position
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
#endif //__h_shapes
