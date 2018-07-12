#include "ecl/geometry/shapes.h"
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "G4QuadrangularFacet.hh"
#include <G4ExtrudedSolid.hh>
#include "G4Trap.hh"
#include "ecl/geometry/BelleCrystal.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/Matrix.h"
#include <G4TwoVector.hh>
#include <G4Vector3D.hh>
#include <G4Polycone.hh>
#include <G4LogicalVolume.hh>
#include "ecl/geometry/BelleLathe.h"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include <framework/utilities/FileSystem.h>

using namespace std;

namespace Belle2 {
  namespace ECL {
    double cosd(double x) {return cos(x * (M_PI / 180));}
    double sind(double x) {return sin(x * (M_PI / 180));}
    double tand(double x) {return tan(x * (M_PI / 180));}

    double volume(const G4ThreeVector& v0, const G4ThreeVector& v1, const G4ThreeVector& v2, const G4ThreeVector& v3)
    {
      G4ThreeVector x = v1 - v0, y = v2 - v0, z = v3 - v0;
      return x.cross(y) * z;
    }

    G4ThreeVector newvertex(double d, const G4ThreeVector& v0, const G4ThreeVector& v1, const G4ThreeVector& v2,
                            const G4ThreeVector& v3)
    {
      G4ThreeVector x = v1 - v0, y = v2 - v0, z = v3 - v0;
      G4ThreeVector nxy = x.cross(y).unit();
      G4ThreeVector nyz = y.cross(z).unit();
      G4ThreeVector nzx = z.cross(x).unit();

      CLHEP::HepMatrix A(3, 3);
      A[0][0] = nxy.x(), A[0][1] = nxy.y(), A[0][2] = nxy.z();
      A[1][0] = nyz.x(), A[1][1] = nyz.y(), A[1][2] = nyz.z();
      A[2][0] = nzx.x(), A[2][1] = nzx.y(), A[2][2] = nzx.z();

      CLHEP::HepVector B(3);
      B[0] = v0 * nxy - d;
      B[1] = v0 * nyz - d;
      B[2] = v0 * nzx - d;
      //  int ierr;
      //  cout<<A<<" "<<B<<" "<<A.inverse(ierr)<<endl;
      CLHEP::HepVector r = A.inverse() * B; //CLHEP::solve(A, B);

      return G4ThreeVector(r[0], r[1], r[2]);
    }

    G4ThreeVector moveto(double r, double phi)
    {
      return G4ThreeVector(r * cosd(phi), r * sind(phi), 0);
    };


    G4VSolid* shape_t::get_solid(const string& prefix, double wrapthick, G4Translate3D& shift) const
    {
      return get_bellecrystal(prefix, wrapthick, shift);
    }

    G4ThreeVector centerofgravity(const map<int, G4ThreeVector>& v, int i0, int n)
    {
      double cx = 0, cy = 0, A = 0;
      for (int j = 0; j < n; j++) {
        int j0 = j + i0, j1 = ((j + 1) % n) + i0;
        const G4ThreeVector& v0 = v.find(j0)->second, &v1 = v.find(j1)->second;
        double t = v0.x() * v1.y() - v0.y() * v1.x();
        //    cout<<j0<<" "<<j1<<" "<<t<<" "<<v0.x()<<" "<<v0.y()<<endl;
        A  += t;
        cx += (v0.x() + v1.x()) * t;
        cy += (v0.y() + v1.y()) * t;
      }
      cx /= 3 * A;
      cy /= 3 * A;
      return G4ThreeVector(cx, cy, v.find(i0)->second.z());
    }

    Point_t centerofgravity(Point_t* b, Point_t* e)
    {
      double cx = 0, cy = 0, A = 0;
      int n = e - b;
      for (int j = 0; j < n; j++) {
        int j0 = j, j1 = ((j + 1) % n);
        const Point_t& v0 = b[j0], &v1 = b[j1];
        double t = v0.x * v1.y - v0.y * v1.x;
        A  += t;
        cx += (v0.x + v1.x) * t;
        cy += (v0.y + v1.y) * t;
      }
      cx /= 3 * A;
      cy /= 3 * A;
      Point_t t = {cx, cy};
      return t;
    }


    struct quadrilateral_t: public shape_t {
      quadrilateral_t() {}
      virtual ~quadrilateral_t() {}
      virtual map<int, G4ThreeVector> make_verticies(double wrapthick) const = 0;

      G4VSolid* get_tesselatedsolid(const string& prefix, double wrapthick, G4Translate3D& shift UNUSED) const
      {
        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);
        G4TessellatedSolid* s = new G4TessellatedSolid(name.c_str());

        // Now add the facets to the solid
        // top plane
        s->AddFacet(new G4QuadrangularFacet(v[1], v[4], v[3], v[2], ABSOLUTE));
        // bottom plane
        s->AddFacet(new G4QuadrangularFacet(v[5], v[6], v[7], v[8], ABSOLUTE));
        // lateral sides
        s->AddFacet(new G4QuadrangularFacet(v[1], v[2], v[6], v[5], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[2], v[3], v[7], v[6], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[3], v[4], v[8], v[7], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[4], v[1], v[5], v[8], ABSOLUTE));

        //  Finally declare the solid is complete
        s->SetSolidClosed(true);
        return s;
      }

      G4VSolid* get_extrudedsolid(const string& prefix, double wrapthick, G4Translate3D& shift UNUSED) const
      {
        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);

        std::vector<G4TwoVector> p1, p2;
        p1.push_back(G4TwoVector(v[1].x(), v[1].y()));
        p1.push_back(G4TwoVector(v[2].x(), v[2].y()));
        p1.push_back(G4TwoVector(v[3].x(), v[3].y()));
        p1.push_back(G4TwoVector(v[4].x(), v[4].y()));

        p2.push_back(G4TwoVector(v[1 + 4].x(), v[1 + 4].y()));
        p2.push_back(G4TwoVector(v[2 + 4].x(), v[2 + 4].y()));
        p2.push_back(G4TwoVector(v[3 + 4].x(), v[3 + 4].y()));
        p2.push_back(G4TwoVector(v[4 + 4].x(), v[4 + 4].y()));

        double sum = 0, sum2 = 0,  smin = 1e9, smax = -1e9;
        for (int i = 0; i < 4; i++) {
          for (int j = i + 1; j < 4; j++) {
            double s2 = (p2[j] - p2[i]).mag2() / (p1[j] - p1[i]).mag2();
            double s = sqrt(s2);
            sum2 += s2;
            sum += s;
            if (s > smax) smax = s;
            if (s < smin) smin = s;
            //  cout<<i<<" "<<j<<" "<<s<<endl;
          }
        }
        double ave = sum / 6;
        //    double rms = sqrt(sum2/6 - ave*ave);
        //    cout<<sum<<" +- "<<rms<<" "<<rms/sum*100<<" "<<60*(smin-ave)<<" "<<60*(smax-ave)<<endl;

        double scale = ave;
        G4TwoVector off1(0, 0), off2(scale * p1[0].x() - p2[0].x(), scale * p1[0].y() - p2[0].y());

        return new G4ExtrudedSolid(name, p1, abs(v[1].z()), off1, 1, -off2, scale);
      }

      G4VSolid* get_trapezoid(const string& prefix, double wrapthick, G4Translate3D& shift) const
      {
        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        // make sides @ +-Y parallel
        G4ThreeVector b = v[4] - v[1];
        G4ThreeVector d12 = v[2] - v[1], d43 = v[3] - v[4];
        double b2 = b.mag2();
        double h1 = b.cross(d12).mag2();
        double h4 = b.cross(d43).mag2();
        //    if(abs(h1/h4-1)<0.02) cout<<"Shape = "<<nshape<<" "<<h1<<" "<<h4<<" "<<h1/h4-1<<endl;
        if (h1 < h4) {
          G4ThreeVector d24 = v[4] - v[2];
          double d43b = d43 * b, s = (b2 * (d24 * d43) - (d24 * b) * d43b) / (d43b * d43b - b2 * d43.mag2());
          v[3] = v[4] + s * d43;
          v[7] = v[8] + s * (v[7] - v[8]);
        } else {
          G4ThreeVector d31 = v[1] - v[3];
          double d12b = d12 * b, s = (b2 * (d31 * d12) - (d31 * b) * d12b) / (d12b * d12b - b2 * d12.mag2());
          v[2] = v[1] + s * d12;
          v[6] = v[5] + s * (v[6] - v[5]);
        }

        string name(prefix);
        name += to_string(nshape);

        G4ThreeVector pt[8];
        pt[0] = v[4];
        pt[1] = v[1];
        pt[2] = v[3];
        pt[3] = v[2];
        pt[4] = v[8];
        pt[5] = v[5];
        pt[6] = v[7];
        pt[7] = v[6];

        auto alignz = [&](int i, int j) {pt[j].setZ(pt[i].z());};
        auto aligny = [&](int i, int j) {pt[j].setY(pt[i].y());};

        alignz(0, 1);
        alignz(0, 2);
        alignz(0, 3);

        alignz(4, 1 + 4);
        alignz(4, 2 + 4);
        alignz(4, 3 + 4);

        aligny(0, 1);
        aligny(2, 3);

        aligny(4, 5);
        aligny(6, 7);

        double dx = pt[0].x() + pt[1].x() + pt[4].x() + pt[5].x() + pt[2].x() + pt[3].x() + pt[6].x() + pt[7].x();
        dx /= 8;
        double dy = pt[0].y() + pt[2].y() + pt[4].y() + pt[6].y();
        dy /= 4;
        for (int j = 0; j < 8; j++) {
          pt[j].setX(pt[j].x() - dx);
          pt[j].setY(pt[j].y() - dy);
        }

        // int oprec = cout.precision(17);
        // cout<<dx<<" "<<dy<<endl;
        // cout.precision(oprec);

        shift = G4Translate3D(dx, dy, 0);
        G4VSolid* shape = new G4Trap(name.c_str(), pt);
        //    cout<<name<<" "<<shape->GetCubicVolume()*1e-3<<" "<<Volume<<endl;
        //    G4VSolid *shape = new BelleCrystal(name.c_str(), 4, pt);
        return shape;
      }

      G4VSolid* get_bellecrystal(const string& prefix, double wrapthick, G4Translate3D& shift UNUSED) const
      {
        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);

        G4ThreeVector pt[8];
        pt[0] = v[4];
        pt[1] = v[1];
        pt[2] = v[3];
        pt[3] = v[2];
        pt[4] = v[8];
        pt[5] = v[5];
        pt[6] = v[7];
        pt[7] = v[6];

        for (int i = 0; i < 8; i++) pt[i] = v[i + 1];
        G4VSolid* shape = new BelleCrystal(name.c_str(), 4, pt);
        // cout<<name<<" "<<shape->GetCubicVolume()*1e-3<<" "<<Volume<<endl;
        // for(int i=0;i<100000;i++){
        //   G4ThreeVector a = shape->GetPointOnSurface();
        //   cout<<a.x()<<" "<<a.y()<<" "<<a.z()<<endl;
        // }
        // exit(0);
        return shape;
      }
    };

    struct quadrilateral_barrel_t: public quadrilateral_t {
      union {
        struct {
          double A, B, H, a, b, h, alpha, beta, betap, gamma, Volume, Weight;
        };
        double t[12];
      };
      quadrilateral_barrel_t() {}
      virtual ~quadrilateral_barrel_t() {}

      bool istrap() const
      {
        return true;
      }

      map<int, G4ThreeVector> make_verticies(double wrapthick) const
      {
        map<int, G4ThreeVector> v;
        v[1] = G4ThreeVector(-a / 2, -h / 2, 150);
        v[2] = G4ThreeVector(-b / 2, h / 2, 150);
        v[3] = G4ThreeVector(b / 2, h / 2, 150);
        v[4] = G4ThreeVector(a / 2, -h / 2, 150);
        v[5] = G4ThreeVector(-A / 2, -H / 2, -150);
        v[6] = G4ThreeVector(-B / 2, H / 2, -150);
        v[7] = G4ThreeVector(B / 2, H / 2, -150);
        v[8] = G4ThreeVector(A / 2, -H / 2, -150);

        for (int j = 1; j <= 8; j++) v[j] = G4ThreeVector(v[j].x(), -v[j].y(), -v[j].z());

        // G4ThreeVector c0 = centerofgravity(v, 1, 4);
        // G4ThreeVector c1 = centerofgravity(v, 5, 4);
        // G4ThreeVector cz = 0.5*(c0+c1);
        // cout<<c0<<" "<<c1<<" "<<cz<<endl;

        if (wrapthick != 0) {
          map<int, G4ThreeVector> nv;
          nv[1] = newvertex(wrapthick, v[1], v[5], v[2], v[4]);
          nv[2] = newvertex(wrapthick, v[2], v[6], v[3], v[1]);
          nv[3] = newvertex(wrapthick, v[3], v[7], v[4], v[2]);
          nv[4] = newvertex(wrapthick, v[4], v[8], v[1], v[3]);
          nv[5] = newvertex(wrapthick, v[5], v[1], v[8], v[6]);
          nv[6] = newvertex(wrapthick, v[6], v[2], v[5], v[7]);
          nv[7] = newvertex(wrapthick, v[7], v[3], v[6], v[8]);
          nv[8] = newvertex(wrapthick, v[8], v[4], v[7], v[5]);
          std::swap(nv, v);
        }

        //    if(nshape==1){ for(int j=1;j<=8;j++) cout<<v[j]<<" "; cout<<endl;}

        return v;
      }
    };

    struct quadrilateral_endcap_t: public quadrilateral_t {
      union {
        struct {
          double A, B, C, D, a, b, c, d, H_aA, H_dD, dg13, dg24, dg57, dg68, a1, a2, a3, a4, Volume, Weight;
        };
        double t[20];
      };
      quadrilateral_endcap_t() {}
      virtual ~quadrilateral_endcap_t() {}

      bool istrap() const
      {
        double h1 = sind(a1) * D, h4 = sind(a4) * C;
        return abs(h1 - h4) < 0.01 * h1;
      }

      map<int, G4ThreeVector> make_verticies(double wrapthick) const
      {
        double minh = std::min(sind(a1) * D, sind(a4) * C);
        double h2 = minh / 2;
        double db = (tand(a1 - 90) - tand(a4 - 90)) * h2 / 2;

        map<int, G4ThreeVector> v;
        v[5] = G4ThreeVector(-A / 2 + db, -h2, -150);
        v[6] = v[5] + moveto(D, a1);
        v[8] = G4ThreeVector(A / 2 + db, -h2, -150);
        v[7] = v[8] + moveto(C, 180 - a4);

        // adjust position of v[2],v[3],v[7],v[6] to have all 4 points in a plane, other combination already in a plane by the construction
        G4ThreeVector vB = v[7] - v[6], va(a, 0, 0), vd = moveto(d, a1), vc = moveto(c, 180 - a4);
        double delta = vB.cross(va + vc - vd).z() / vB.cross(vc + vd).z(); // delta should be very small ~10^-6 or less

        vd *= 1 + delta;
        vc *= 1 - delta;

        v[1] = v[5] + G4ThreeVector((H_aA * cosd(a1) + H_dD) / sind(a1), H_aA, 300);
        v[2] = v[1] + vd;
        v[4] = v[1] + va;
        v[3] = v[4] + vc;

        for (int j = 1; j <= 8; j++) v[j] = G4ThreeVector(v[j].x(), -v[j].y(), -v[j].z());

        // G4ThreeVector c0 = centerofgravity(v, 1, 4);
        // G4ThreeVector c1 = centerofgravity(v, 5, 4);
        // G4ThreeVector cz = 0.5*(c0+c1);
        // cout<<c0<<" "<<c1<<" "<<cz<<endl;

        if (wrapthick != 0) {
          map<int, G4ThreeVector> nv;
          nv[1] = newvertex(wrapthick, v[1], v[5], v[2], v[4]);
          nv[2] = newvertex(wrapthick, v[2], v[6], v[3], v[1]);
          nv[3] = newvertex(wrapthick, v[3], v[7], v[4], v[2]);
          nv[4] = newvertex(wrapthick, v[4], v[8], v[1], v[3]);
          nv[5] = newvertex(wrapthick, v[5], v[1], v[8], v[6]);
          nv[6] = newvertex(wrapthick, v[6], v[2], v[5], v[7]);
          nv[7] = newvertex(wrapthick, v[7], v[3], v[6], v[8]);
          nv[8] = newvertex(wrapthick, v[8], v[4], v[7], v[5]);
          std::swap(nv, v);
        }
        //    if(nshape==2){ for(int j=1;j<=8;j++) cout<<v[j]<<" "; cout<<endl;}
        return v;
      }
    };

    struct pent_t: public shape_t {
      union {
        struct {
          double A, C, D, a, c, d, B, b, H_aA, H_dD, dg13, dg24, dg57, dg68, a1, a4, a2, a3, a9, Volume, Weight;
        };
        double t[21];
      };
      bool _adjusted;
      pent_t(): _adjusted(false) {}
      virtual ~pent_t() {}

      void adjust()
      {
        if (!_adjusted) {
          // adjust sizes to have flat sides
          H_dD -= 0.00005551197484235;
          B += 0.0011245236213532729;
          b += -0.00044853029662963;
          _adjusted = true;
        }
      }

      bool istrap() const { return false;}

      map<int, G4ThreeVector> make_verticies(double wrapthick) const
      {
        assert(_adjusted);

        double h2 = cosd(a1 - 90) * D / 2;
        map<int, G4ThreeVector> v;
        v[5]  = G4ThreeVector(-A / 2, -h2, -150);
        v[6]  = v[5] + moveto(D, a1);
        v[10] = v[6] + moveto(B, a1 + a2 - 180);
        v[8]  = G4ThreeVector(A / 2, -h2, -150);
        v[7]  = v[8] + moveto(D, 180 - a1);

        v[1]  = v[5] + G4ThreeVector((H_aA * cosd(a1) + H_dD) / sind(a1), H_aA, 300);
        v[2]  = v[1] + moveto(d, a1);
        v[9]  = v[2] + moveto(b, a1 + a2 - 180);
        v[4]  = v[1] + moveto(a, 0);
        v[3]  = v[4] + moveto(d, 180 - a1);

        for (int j = 1; j <= 10; j++) v[j] = G4ThreeVector(v[j].x(), -v[j].y(), -v[j].z());
        if (wrapthick != 0) {
          map<int, G4ThreeVector> nv;
          nv[ 1] = newvertex(wrapthick, v[1], v[5], v[2], v[4]);
          nv[ 2] = newvertex(wrapthick, v[2], v[6], v[9], v[1]);
          nv[ 9] = newvertex(wrapthick, v[9], v[10], v[3], v[2]);
          nv[ 3] = newvertex(wrapthick, v[3], v[7], v[4], v[9]);
          nv[ 4] = newvertex(wrapthick, v[4], v[8], v[1], v[3]);
          nv[ 5] = newvertex(wrapthick, v[5], v[1], v[8], v[6]);
          nv[ 6] = newvertex(wrapthick, v[6], v[2], v[5], v[10]);
          nv[10] = newvertex(wrapthick, v[10], v[9], v[6], v[7]);
          nv[ 7] = newvertex(wrapthick, v[7], v[3], v[10], v[8]);
          nv[ 8] = newvertex(wrapthick, v[8], v[4], v[7], v[5]);
          std::swap(nv, v);
        }
        return v;
      }

      G4VSolid* get_tesselatedsolid(const string& prefix, double wrapthick, G4Translate3D& shift UNUSED) const
      {
        if (nshape != 36) return NULL; // only one crystal has pentagon shape

        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);
        G4TessellatedSolid* s = new G4TessellatedSolid(name.c_str());

        // Now add the facets to the solid
        // top plane
        s->AddFacet(new G4QuadrangularFacet(v[1], v[4], v[3], v[2], ABSOLUTE));
        s->AddFacet(new G4TriangularFacet(v[2], v[3], v[9], ABSOLUTE));

        //bottom plane
        s->AddFacet(new G4QuadrangularFacet(v[5], v[6], v[7], v[8], ABSOLUTE));
        s->AddFacet(new G4TriangularFacet(v[6], v[10], v[7], ABSOLUTE));

        //sides
        s->AddFacet(new G4QuadrangularFacet(v[1], v[2], v[6], v[5], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[2], v[9], v[10], v[6], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[9], v[3], v[7], v[10], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[3], v[4], v[8], v[7], ABSOLUTE));
        s->AddFacet(new G4QuadrangularFacet(v[4], v[1], v[5], v[8], ABSOLUTE));

        //  Finally declare the solid is complete
        s->SetSolidClosed(true);
        return s;
      }

      G4VSolid* get_extrudedsolid(const string& prefix, double wrapthick, G4Translate3D& shift UNUSED) const
      {
        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);

        std::vector<G4TwoVector> p1, p2;
        p1.push_back(G4TwoVector(v[1].x(), v[1].y()));
        p1.push_back(G4TwoVector(v[2].x(), v[2].y()));
        p1.push_back(G4TwoVector(v[9].x(), v[9].y()));
        p1.push_back(G4TwoVector(v[3].x(), v[3].y()));
        p1.push_back(G4TwoVector(v[4].x(), v[4].y()));

        p2.push_back(G4TwoVector(v[1 + 4].x(), v[1 + 4].y()));
        p2.push_back(G4TwoVector(v[2 + 4].x(), v[2 + 4].y()));
        p2.push_back(G4TwoVector(v[ 10].x(), v[ 10].y()));
        p2.push_back(G4TwoVector(v[3 + 4].x(), v[3 + 4].y()));
        p2.push_back(G4TwoVector(v[4 + 4].x(), v[4 + 4].y()));

        double sum = 0, sum2 = 0,  smin = 1e9, smax = -1e9;
        int count = 0;
        for (int i = 0; i < 5; i++) {
          for (int j = i + 1; j < 5; j++) {
            double s2 = (p2[j] - p2[i]).mag2() / (p1[j] - p1[i]).mag2();
            double s = sqrt(s2);
            sum2 += s2;
            sum += s;
            if (s > smax) smax = s;
            if (s < smin) smin = s;
            //  cout<<i<<" "<<j<<" "<<s<<endl;
            count++;
          }
        }
        double ave = sum / count;
        //    double rms = sqrt(sum2/count - ave*ave);
        //    cout<<sum<<" +- "<<rms<<" "<<rms/sum*100<<" "<<60*(smin-ave)<<" "<<60*(smax-ave)<<endl;

        double scale = ave;
        G4TwoVector off1(0, 0), off2(scale * p1[0].x() - p2[0].x(), scale * p1[0].y() - p2[0].y());

        return new G4ExtrudedSolid(name, p1, abs(v[1].z()), off1, 1, -off2, scale);
      }

      G4VSolid* get_trapezoid(const string& prefix, double wrapthick, G4Translate3D& shift) const
      {
        if (nshape != 36) return NULL; // only one crystal has pentagon shape

        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);

        G4ThreeVector pt[8];
        pt[0] = v[4];
        pt[1] = v[1];
        pt[2] = v[3];
        pt[3] = v[2];
        pt[4] = v[8];
        pt[5] = v[5];
        pt[6] = v[7];
        pt[7] = v[6];

        auto alignz = [&](int i, int j) {pt[j].setZ(pt[i].z());};
        auto aligny = [&](int i, int j) {pt[j].setY(pt[i].y());};

        alignz(0, 1);
        alignz(0, 2);
        alignz(0, 3);

        alignz(4, 1 + 4);
        alignz(4, 2 + 4);
        alignz(4, 3 + 4);

        aligny(0, 1);
        aligny(2, 3);

        aligny(4, 5);
        aligny(6, 7);


        double dx = pt[0].x() + pt[1].x() + pt[4].x() + pt[5].x() + pt[2].x() + pt[3].x() + pt[6].x() + pt[7].x();
        dx /= 8;
        double dy = pt[0].y() + pt[2].y() + pt[4].y() + pt[6].y();
        dy /= 4;
        for (int j = 0; j < 8; j++) {
          pt[j].setX(pt[j].x() - dx);
          pt[j].setY(pt[j].y() - dy);
        }
        shift = G4Translate3D(dx, dy, 0);

        //    cout<<name<<" "<<dx<<" "<<dy<<endl;

        G4VSolid* shape = new G4Trap(name.c_str(), pt);
        //    G4VSolid *shape = new BelleCrystal(name.c_str(), 4, pt);
        return shape;
      }

      G4VSolid* get_bellecrystal(const string& prefix, double wrapthick, G4Translate3D& shift UNUSED) const
      {
        map<int, G4ThreeVector> v = make_verticies(wrapthick);

        string name(prefix);
        name += to_string(nshape);

        G4ThreeVector pt[10];
        pt[0] = v[1];
        pt[1] = v[2];
        pt[2] = v[9];
        pt[3] = v[3];
        pt[4] = v[4];
        pt[5] = v[5];
        pt[6] = v[6];
        pt[7] = v[10];
        pt[8] = v[7];
        pt[9] = v[8];

        G4VSolid* shape = new BelleCrystal(name.c_str(), 5, pt);
        // cout<<name<<" "<<shape->GetCubicVolume()*1e-3<<" "<<Volume<<endl;
        // for(int i=0;i<100000;i++){
        //   G4ThreeVector a = shape->GetPointOnSurface();
        //   cout<<a.x()<<" "<<a.y()<<" "<<a.z()<<endl;
        // }
        // exit(0);
        // if(prefix.find("crystal")!=string::npos){
        //   pentspeed(shape);
        //   exit(0);
        // }
        // if(prefix.find("wrap")!=string::npos){
        //   pentspeed2(shape);
        //   exit(0);
        // }
        return shape;
      }
    };

    vector<shape_t*> load_shapes(const string& fname)
    {
      vector<shape_t*> shapes;
      std::string fnamef = Belle2::FileSystem::findFile(fname);

      ifstream IN(fnamef.c_str());
      string tmp;
      while (getline(IN, tmp)) {
        size_t ic = tmp.find("#");
        if (ic != string::npos) tmp.erase(ic);
        istringstream iss(tmp);
        vector<string> t;
        copy(istream_iterator<string>(iss),  istream_iterator<string>(),  back_inserter(t));
        if (t.size() > 0) {
          shape_t* shape = NULL;
          if (t.size() == 21) {
            shape = new quadrilateral_endcap_t();
            quadrilateral_endcap_t& trap = static_cast<quadrilateral_endcap_t&>(*shape);

            istringstream in(t[0]);
            in >> trap.nshape;

            for (size_t i = 1; i < t.size(); i++) {
              in.str(t[i]); in.seekg(0, ios_base::beg);
              in >> trap.t[i - 1];
            }
          } else if (t.size() == 13) {
            shape = new quadrilateral_barrel_t();
            quadrilateral_barrel_t& trap = static_cast<quadrilateral_barrel_t&>(*shape);

            istringstream in(t[0]);
            in >> trap.nshape;

            for (size_t i = 1; i < t.size(); i++) {
              in.str(t[i]); in.seekg(0, ios_base::beg);
              in >> trap.t[i - 1];
            }
          } else if (t.size() == 22) {
            shape = new pent_t();
            pent_t& pent = static_cast<pent_t&>(*shape);

            istringstream in(t[0]);
            in >> pent.nshape;

            for (size_t i = 1; i < t.size(); i++) {
              in.str(t[i]); in.seekg(0, ios_base::beg);
              in >> pent.t[i - 1];
            }
            pent.adjust();
          }
          shapes.push_back(shape);
        }
      }

      return shapes;

      // for(unsigned int i=0;i<shapes.size();i++){
      //   shape_t &t = *shapes[i];
      //   crystals[t.nshape] = t.get_solid(prefix, 0.250-0.006);
      // }

      // return crystals;
    }

    vector<cplacement_t> load_placements(const string& fname)
    {
      vector<cplacement_t> plcmnt;
      std::string fnamef = Belle2::FileSystem::findFile(fname);

      ifstream IN(fnamef.c_str());
      string tmp;
      while (getline(IN, tmp)) {
        size_t ic = tmp.find("#");
        if (ic != string::npos) tmp.erase(ic);
        istringstream iss(tmp);
        vector<string> t;
        copy(istream_iterator<string>(iss),  istream_iterator<string>(),  back_inserter(t));
        if (t.size() == 7) {
          cplacement_t p;
          istringstream in(t[0]);
          in >> p.nshape;
          in.str(t[1]); in.seekg(0, ios_base::beg);
          in >> p.Rphi1;
          in.str(t[2]); in.seekg(0, ios_base::beg);
          in >> p.Rtheta;
          in.str(t[3]); in.seekg(0, ios_base::beg);
          in >> p.Rphi2;
          in.str(t[4]); in.seekg(0, ios_base::beg);
          in >> p.Pr;
          in.str(t[5]); in.seekg(0, ios_base::beg);
          in >> p.Ptheta;
          in.str(t[6]); in.seekg(0, ios_base::beg);
          in >> p.Pphi;
          plcmnt.push_back(p);
        }
      }

      return plcmnt;
    }

    G4Transform3D get_transform(const cplacement_t& t)
    {
      G4Transform3D r = G4Rotate3D(t.Rphi1, G4Vector3D(sin(t.Rtheta) * cos(t.Rphi2), sin(t.Rtheta) * sin(t.Rphi2), cos(t.Rtheta)));
      G4Transform3D p = G4Translate3D(t.Pr * sin(t.Ptheta) * cos(t.Pphi), t.Pr * sin(t.Ptheta) * sin(t.Pphi), t.Pr * cos(t.Ptheta));
      return p * r;
    }

    Belle2::ECLCrystalsShapeAndPosition loadCrystalsShapeAndPosition()
    {
      stringstream buffer;
      auto fillbuffer = [&buffer](const string & fname) {
        string path = Belle2::FileSystem::findFile(fname);
        ifstream IN(path.c_str());
        buffer.clear(); buffer.str("");
        buffer << IN.rdbuf();
      };

      Belle2::ECLCrystalsShapeAndPosition a;
      fillbuffer("/ecl/data/crystal_shape_forward.dat"); a.setShapeForward(buffer.str());
      fillbuffer("/ecl/data/crystal_shape_barrel.dat"); a.setShapeBarrel(buffer.str());
      fillbuffer("/ecl/data/crystal_shape_backward.dat"); a.setShapeBackward(buffer.str());
      fillbuffer("/ecl/data/crystal_placement_forward.dat"); a.setPlacementForward(buffer.str());
      fillbuffer("/ecl/data/crystal_placement_barrel.dat"); a.setPlacementBarrel(buffer.str());
      fillbuffer("/ecl/data/crystal_placement_backward.dat"); a.setPlacementBackward(buffer.str());
      return a;
      // Belle2::IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments
      // Belle2::Database::Instance().storeData<Belle2::ECLCrystalsShapeAndPosition>(&a, iov);
    }

    vector<shape_t*> load_shapes(stringstream& IN)
    {
      vector<shape_t*> shapes;
      string tmp;
      while (getline(IN, tmp)) {
        size_t ic = tmp.find("#");
        if (ic != string::npos) tmp.erase(ic);
        istringstream iss(tmp);
        vector<string> t;
        copy(istream_iterator<string>(iss),  istream_iterator<string>(),  back_inserter(t));
        if (t.size() > 0) {
          shape_t* shape = NULL;
          if (t.size() == 21) {
            shape = new quadrilateral_endcap_t();
            quadrilateral_endcap_t& trap = static_cast<quadrilateral_endcap_t&>(*shape);

            istringstream in(t[0]);
            in >> trap.nshape;

            for (size_t i = 1; i < t.size(); i++) {
              in.str(t[i]); in.seekg(0, ios_base::beg);
              in >> trap.t[i - 1];
            }
          } else if (t.size() == 13) {
            shape = new quadrilateral_barrel_t();
            quadrilateral_barrel_t& trap = static_cast<quadrilateral_barrel_t&>(*shape);

            istringstream in(t[0]);
            in >> trap.nshape;

            for (size_t i = 1; i < t.size(); i++) {
              in.str(t[i]); in.seekg(0, ios_base::beg);
              in >> trap.t[i - 1];
            }
          } else if (t.size() == 22) {
            shape = new pent_t();
            pent_t& pent = static_cast<pent_t&>(*shape);

            istringstream in(t[0]);
            in >> pent.nshape;

            for (size_t i = 1; i < t.size(); i++) {
              in.str(t[i]); in.seekg(0, ios_base::beg);
              in >> pent.t[i - 1];
            }
            pent.adjust();
          }
          shapes.push_back(shape);
        }
      }

      return shapes;
    }

    vector<cplacement_t> load_placements(stringstream& IN)
    {
      vector<cplacement_t> plcmnt;
      string tmp;
      while (getline(IN, tmp)) {
        size_t ic = tmp.find("#");
        if (ic != string::npos) tmp.erase(ic);
        istringstream iss(tmp);
        vector<string> t;
        copy(istream_iterator<string>(iss),  istream_iterator<string>(),  back_inserter(t));
        if (t.size() == 7) {
          cplacement_t p;
          istringstream in(t[0]);
          in >> p.nshape;
          in.str(t[1]); in.seekg(0, ios_base::beg);
          in >> p.Rphi1;
          in.str(t[2]); in.seekg(0, ios_base::beg);
          in >> p.Rtheta;
          in.str(t[3]); in.seekg(0, ios_base::beg);
          in >> p.Rphi2;
          in.str(t[4]); in.seekg(0, ios_base::beg);
          in >> p.Pr;
          in.str(t[5]); in.seekg(0, ios_base::beg);
          in >> p.Ptheta;
          in.str(t[6]); in.seekg(0, ios_base::beg);
          in >> p.Pphi;
          plcmnt.push_back(p);
        }
      }

      return plcmnt;
    }

    vector<cplacement_t> load_placements(const Belle2::ECLCrystalsShapeAndPosition* crystals, enum ECLParts part)
    {
      // Belle2::DBObjPtr<Belle2::ECLCrystalsShapeAndPosition> crystals;
      // if (!crystals.isValid()) B2FATAL("No crystal's data in the database.");

      // stringstream buffer;
      // auto fillbuffer = [&buffer](const string &fname) {
      //   string path = Belle2::FileSystem::findFile(fname);
      //   ifstream IN(path.c_str());
      //   buffer.clear(); buffer.str("");
      //   buffer << IN.rdbuf();
      // };

      // Belle2::ECLCrystalsShapeAndPosition *crystals = new Belle2::ECLCrystalsShapeAndPosition();
      // fillbuffer("/ecl/data/crystal_shape_forward.dat"); crystals->setShapeForward(buffer.str());
      // fillbuffer("/ecl/data/crystal_shape_barrel.dat"); crystals->setShapeBarrel(buffer.str());
      // fillbuffer("/ecl/data/crystal_shape_backward.dat"); crystals->setShapeBackward(buffer.str());
      // fillbuffer("/ecl/data/crystal_placement_forward.dat"); crystals->setPlacementForward(buffer.str());
      // fillbuffer("/ecl/data/crystal_placement_barrel.dat"); crystals->setPlacementBarrel(buffer.str());
      // fillbuffer("/ecl/data/crystal_placement_backward.dat"); crystals->setPlacementBackward(buffer.str());

      stringstream IN;
      if (part == ECLParts::forward)
        IN.str(crystals->getPlacementForward());
      else if (part == ECLParts::barrel)
        IN.str(crystals->getPlacementBarrel());
      else if (part == ECLParts::backward)
        IN.str(crystals->getPlacementBackward());
      return load_placements(IN);
    }

    vector<shape_t*> load_shapes(const Belle2::ECLCrystalsShapeAndPosition* crystals, enum ECLParts part)
    {
      stringstream IN;
      if (part == ECLParts::forward)
        IN.str(crystals->getShapeForward());
      else if (part == ECLParts::barrel)
        IN.str(crystals->getShapeBarrel());
      else if (part == ECLParts::backward)
        IN.str(crystals->getShapeBackward());
      return load_shapes(IN);
    }

    void testtest()
    {
      Belle2::ECLCrystalsShapeAndPosition crystals = loadCrystalsShapeAndPosition();
      vector<cplacement_t> bp = load_placements(&crystals, ECLParts::forward);
      for (vector<cplacement_t>::const_iterator it = bp.begin(); it != bp.end(); ++it) {
        const cplacement_t& t = *it;
        cout << t.nshape << " " << t.Rphi1 << " " << t.Rtheta << " " << t.Rphi2 << " " << t.Pr << " " << t.Ptheta << " " << t.Pphi << endl;
      }
      exit(0);
    }
  }
}
