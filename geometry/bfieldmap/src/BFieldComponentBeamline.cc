/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Kazutaka Sumisawa, Alexei Sibidanov        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentBeamline.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>
#include <limits>

using namespace std;
namespace io = boost::iostreams;
namespace Belle2 {

  /** Triangle structure */
  struct triangle_t {
    /** 1st vertex index in a list of xy-points */
    short int j0;
    /** 2nd vertex index in a list of xy-points */
    short int j1;
    /** 3rd vertex index in a list of xy-points */
    short int j2;
    /** 1st adjacent triangle in a list of triangles */
    short int n0;
    /** 2nd adjacent triangle in a list of triangles */
    short int n1;
    /** 3rd adjacent triangle in a list of triangles */
    short int n2;
  };

  /** A simple 2d vector stucture */
  struct xy_t {
    double x; /**< x component */
    double y; /**< y component */
  };

  /**
   * The TriangularInterpolation class.
   *
   * This class travers triangular meshes which satisfies the Delaunay
   * condition. In other meshes it may give a wrong result.  The mesh
   * is represented by a list of triangles coupled to a list of
   * xy-points.  To speed up the traverse a sort of spatial index is
   * constructed where on a regular cartesian grid the closeses
   * triangle center is memorized.
   */
  class TriangularInterpolation {
  public:
    /** returns list of verticies */
    const vector<xy_t>& getPoints() const { return m_points;}
    /** returns list of triangles */
    const vector<triangle_t>& getTriangles() const { return m_triangles;}

    /** Default constructor */
    TriangularInterpolation() {}

    /** More complex constructor */
    TriangularInterpolation(vector<xy_t>& pc, vector<triangle_t>& ts, double d)
    {
      init(pc, ts, d);
    }

    /** Destructor */
    ~TriangularInterpolation() {}

    /**
     * Calculate extents of a triangular mesh and build spatial
     * index. Moves vector contents inside the class.
     *
     * @param points    List of verticies
     * @param triangles List of triangles
     * @param d         Hint how close spatial index should be built
     */
    void init(vector<xy_t>& points, vector<triangle_t>& triangles, double d)
    {
      std::swap(points, m_points);
      std::swap(triangles, m_triangles);
      const double inf = numeric_limits<double>::infinity();
      double xmin = inf, xmax = -inf;
      double ymin = inf, ymax = -inf;
      auto limit = [&xmin, &xmax, &ymin, &ymax](const xy_t & p) -> void {
        xmin = std::min(xmin, p.x); xmax = std::max(xmax, p.x);
        ymin = std::min(ymin, p.y); ymax = std::max(ymax, p.y);
      };
      for (const auto& t : m_triangles) {
        limit(m_points[t.j0]);
        limit(m_points[t.j1]);
        limit(m_points[t.j2]);
      }
      double xw = xmax - xmin;
      double yw = ymax - ymin;

      xmin -= xw * (1. / 256), xmax += xw * (1. / 256);
      ymin -= yw * (1. / 256), ymax += yw * (1. / 256);
      int nx = lrint(xw * (1 + 1. / 128) / d), ny = lrint(yw * (1 + 1. / 128) / d);
      nx = std::max(nx, 2);
      ny = std::max(ny, 2);
      makeIndex(nx, xmin, xmax, ny, ymin, ymax);
    }

    /**
     * Calculate triangle center
     *
     * @param p    Triangle
     */
    xy_t triangleCenter(const triangle_t& p) const
    {
      const xy_t& p0 = m_points[p.j0], &p1 = m_points[p.j1], &p2 = m_points[p.j2];
      return {(p0.x + p1.x + p2.x)* (1. / 3), (p0.y + p1.y + p2.y)* (1. / 3)};
    }

    /**
     * Make spatial index.
     *
     * @param nx    Grid size in X axis
     * @param xmin  Low border on in X
     * @param xmax  Upper border on in X
     * @param ny    Grid size in Y axis
     * @param ymin  Low border on in Y
     * @param ymax  Upper border on in Y
     */
    void makeIndex(int nx, double xmin, double xmax, int ny, double ymin, double ymax)
    {
      m_nx = nx, m_ny = ny, m_xmin = xmin, m_xmax = xmax, m_ymin = ymin, m_ymax = ymax;
      m_ixnorm = (nx - 1) / (xmax - xmin), m_iynorm = (ny - 1) / (ymax - ymin);
      double dx = (xmax - xmin) / (nx - 1), dy = (ymax - ymin) / (ny - 1);
      //    cout<<nx<<" "<<xmin<<" "<<xmax<<" "<<ny<<" "<<ymin<<" "<<ymax<<endl;
      m_spatialIndex.resize(nx * ny);
      m_triangleCenters.resize(m_triangles.size());
      for (unsigned int i = 0; i < m_triangles.size(); i++) m_triangleCenters[i] = triangleCenter(m_triangles[i]);

      m_triangleAreas.resize(m_triangles.size());
      auto getTriangleArea = [this](const triangle_t& p) ->double{
        const xy_t& p0 = m_points[p.j0], &p1 = m_points[p.j1], &p2 = m_points[p.j2];
        double d21x = p2.x - p1.x, d21y = p2.y - p1.y;
        double d01x = p0.x - p1.x, d01y = p0.y - p1.y;
        return 1 / (d21x * d01y - d21y * d01x);
      };
      for (unsigned int i = 0; i < m_triangles.size(); i++) m_triangleAreas[i] = getTriangleArea(m_triangles[i]);


      for (int ix = 0; ix < nx; ix++) {
        double x = xmin + ix * dx;
        for (int iy = 0; iy < ny; iy++) {
          double y = ymin + iy * dy;
          int imin = -1; double dmin = 1e100;
          for (unsigned int i = 0; i < m_triangleCenters.size(); i++) {
            const xy_t& p = m_triangleCenters[i];
            double d = pow(p.x - x, 2) + pow(p.y - y, 2);
            if (d < dmin) {imin = i; dmin = d;}
          }
          int k = iy + ny * ix;
          m_spatialIndex[k] = imin;
        }
      }
    }

    /**
     * Determine which triangle side is crossed by a line segment defined by r and v0 points
     *
     * @param prev  Triangle number which has been already checked
     * @param xmin  Triangle number which is being checked
     * @param r     Starting point of the line segment
     * @param ny    Ending point of the line segment
     * @return      Next triangle index in the list if nothing found returns the total number of triangles in the list
     */
    short int sideCross(short int prev, short int curr, const xy_t& r, const xy_t& v0) const
    {
      const double vx = r.x - v0.x, vy = r.y - v0.y;
      auto isCrossed = [&vx, &vy, &v0](const xy_t & p1, const xy_t & p0) -> bool {
        double u0x = p0.x, u0y = p0.y;
        double ux = p1.x - u0x, uy = p1.y - u0y;
        double dx = u0x - v0.x, dy = u0y - v0.y;
        double D = uy * vx - ux * vy;
        double t = dx * vy - dy * vx;
        double s = dx * uy - dy * ux;
        return ((t < D) != (t < 0)) && ((s < D) != (s < 0));
      };

      const triangle_t& p = m_triangles[curr];
      const xy_t& p0 = m_points[p.j0], &p1 = m_points[p.j1], &p2 = m_points[p.j2];
      if (p.n0 != prev && isCrossed(p1, p2)) return p.n0;
      if (p.n1 != prev && isCrossed(p2, p0)) return p.n1;
      if (p.n2 != prev && isCrossed(p0, p1)) return p.n2;
      return m_triangles.size();
    }

    /**
     * Calculate barycentric coordinates of a point inside triangle
     *
     * @param i     Triangle index in the list
     * @param r     2d cartesian point
     * @param w0    Weight of 0 vertex
     * @param w1    Weight of 1 vertex
     * @param w2    Weight of 2 vertex
     */
    void weights(short int i, const xy_t& r, double& w0, double& w1, double& w2) const
    {
      const triangle_t& p = m_triangles[i];
      const xy_t& p0 = m_points[p.j0], &p1 = m_points[p.j1], &p2 = m_points[p.j2];
      double dx2  = p2.x -  r.x, dy2  = p2.y -  r.y;
      double d21x = p2.x - p1.x, d21y = p2.y - p1.y;
      double d02x = p0.x - p2.x, d02y = p0.y - p2.y;
      w0 = (dx2 * d21y - dy2 * d21x) * m_triangleAreas[i];
      w1 = (dx2 * d02y - dy2 * d02x) * m_triangleAreas[i];
      w2 = 1 - (w0 + w1);
    }

    /**
     * Find the triangle which contain the point. If not returns the
     * closest one.  First using the spatial index locate triangle
     * close to the point and then traverse the mesh using Delaunay
     * triangulation properties.
     *
     * @param r0  2d cartesian point
     * @return    Triangle index in the list
     */
    short int findTriangle(const xy_t& r0) const
    {
      unsigned int ix = lrint((r0.x - m_xmin) * m_ixnorm), iy = lrint((r0.y - m_ymin) * m_iynorm);
      short int curr = (ix < m_nx && iy < m_ny) ? m_spatialIndex[iy + m_ny * ix] : 0;
      xy_t r = m_triangleCenters[curr];
      const short int end = m_triangles.size();
      short int prev = end;
      do {
        short int next = sideCross(prev, curr, r, r0);
        if (next == end) break;
        prev = curr;
        curr = next;
      } while (1);
      return curr;
    }
  protected:
    /** Triangle list */
    vector<triangle_t> m_triangles;
    /** Vertex list */
    vector<xy_t> m_points;
    /** Triangle centers */
    vector<xy_t> m_triangleCenters;
    /** Triangle areas */
    vector<double> m_triangleAreas;
    /** Spatial index */
    vector<short int> m_spatialIndex;
    /** Border of the region where the spatial index is constructed */
    double m_xmin;
    /** Border of the region where the spatial index is constructed */
    double m_xmax;
    /** Border of the region where the spatial index is constructed */
    double m_ymin;
    /** Border of the region where the spatial index is constructed */
    double m_ymax;
    /** Spatial index grid size */
    unsigned int m_nx;
    /** Spatial index grid size */
    unsigned int m_ny;
    /** Reciprocals to speedup the index calculation */
    double m_ixnorm{1};
    /** Reciprocals to speedup the index calculation */
    double m_iynorm{1};
  };

  /**
   * The BeamlineFieldMapInterpolation class.
   *
   * This class interpolates a magnetic field map around beamline.
   * The magnetic field map is stored as a grid in cylindrical
   * coordinates for outer radiuses and triangular mesh for inner part
   * It is defined by a maximum radius and a maximum z value, a pitch
   * size in both, r and z, and the number of grid points.
   */
  class BeamlineFieldMapInterpolation {
  public:
    /**
     * Expose the triangular interpolation to outside
     *
     * @return constant reference to the interpolation
     */
    const TriangularInterpolation& getTriangularInterpolation() const {return m_triInterpol;}
    /**
     * Default constructor
     */
    BeamlineFieldMapInterpolation() {}
    /**
     * Default destructor
     */
    ~BeamlineFieldMapInterpolation() {}

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file and triangular mesh.
     *
     * @param fieldmapname File name containing the field map
     * @param interpolname File name containing triangular mesh
     * @param validRadius Maximum radius up to which interpolation is valid
     */
    void init(const string& fieldmapname, const string& interpolname, double validRadius)
    {
      if (fieldmapname.empty()) {
        B2ERROR("The filename for the beamline magnetic field component is empty !");
        return;
      }
      std::string l_fieldmapname = FileSystem::findFile("/data/" + fieldmapname);

      if (interpolname.empty()) {
        B2ERROR("The filename for the triangulation of the beamline magnetic field component is empty !");
        return;
      }
      std::string l_interpolname = FileSystem::findFile("/data/" + interpolname);

      m_rmax = validRadius;

      B2DEBUG(50, "Delaunay triangulation of the beamline field map: " << l_interpolname);
      B2DEBUG(50, "Beamline field map: " << l_fieldmapname);

      // load interpolation triangles
      ifstream INd(l_interpolname);
      int nts; INd >> nts;
      B2DEBUG(50, "Total number of triangles: " << nts);
      vector<triangle_t> ts;
      ts.reserve(nts);
      // load triangle definitions from file
      {
        triangle_t p;
        while (INd >> nts >> p.j0 >> p.j1 >> p.j2 >> p.n0 >> p.n1 >> p.n2) ts.push_back(p);
      }

      //Load the map file
      io::filtering_istream IN;
      IN.push(io::gzip_decompressor());
      IN.push(io::file_source(l_fieldmapname));

      int nrphi;
      IN >> nrphi >> m_rj >> m_nr >> m_nphi;
      IN >> m_nz >> m_zj >> m_dz0 >> m_dz1;
      m_idphi = m_nphi / M_PI;
      m_rj2 = m_rj * m_rj;
      m_idz0 = 1 / m_dz0;
      m_idz1 = 1 / m_dz1;
      int nz0 = 2 * int(m_zj / m_dz0);
      m_zmax = (m_nz - (nz0 + 1)) / 2 * m_dz1 + m_zj;
      m_nz1 = (m_nz - (nz0 + 1)) / 2;
      m_nz2 = m_nz1 + nz0;
      //    cout<<_zmax<<" "<<nz0<<" "<<m_nz1<<" "<<m_nz2<<endl;

      struct cs_t {double c, s;};
      vector<cs_t> cs(nrphi);
      vector<xy_t> pc;
      vector<B2Vector3D> tbc;
      pc.reserve(nrphi);
      char cbuf[256]; IN.getline(cbuf, 256);
      double rmax = 0;
      for (int j = 0; j < nrphi; j++) {
        IN.getline(cbuf, 256);
        char* next = cbuf;
        double r    = strtod(next, &next);
        double phi  = strtod(next, &next);
        strtod(next, &next);
        double Br   = strtod(next, &next);
        double Bphi = strtod(next, &next);
        double Bz   = strtod(next, NULL);
        r *= 100;
        rmax = std::max(r, rmax);
        if (phi == 0) {
          cs[j] = { 1, 0};
        } else if (phi == 180) {
          cs[j] = { -1, 0};
        } else {
          phi *= M_PI / 180;
          cs[j] = {cos(phi), sin(phi)};
        }
        double x = r * cs[j].c, y = r * cs[j].s;
        pc.push_back({x, y});
        if (cs[j].s == 0) Bphi = 0;
        double Bx = Br * cs[j].c - Bphi * cs[j].s;
        double By = Br * cs[j].s + Bphi * cs[j].c;
        tbc.push_back({Bx, By, Bz});
      }
      //    cout<<"Field map has data points up to R="<<rmax<<" cm."<<endl;

      m_idr = m_nr / (rmax - m_rj);
      m_rmax = std::min(m_rmax, rmax);
      bool reduce = m_rj > m_rmax;

      // if valid radius is within the triangular mesh try to reduce
      // field map keeping only points which participate to the
      // interpolation
      vector<bool> ip;
      if (reduce) {
        ip.resize(nrphi, false);
        vector<bool> it(ts.size(), false);
        auto inside = [this](const xy_t & p)->bool{
          return p.x * p.x + p.y * p.y < m_rmax * m_rmax;
        };

        for (int i = 0, imax = ts.size(); i < imax; i++) {
          const triangle_t& p = ts[i];
          const xy_t& p0 = pc[p.j0], &p1 = pc[p.j1], &p2 = pc[p.j2];
          if (inside(p0) || inside(p1) || inside(p2)) {
            it[i] = 1;
            ip[p.j0] = 1;
            ip[p.j1] = 1;
            ip[p.j2] = 1;
          }
        }

        vector<short int> pindx(nrphi, -1);
        int rnp = 0;
        for (int i = 0, imax = ip.size(); i < imax; i++) {
          if (ip[i]) pindx[i] = rnp++;
        }
        vector<xy_t> rpc;
        rpc.reserve(rnp);
        for (int i = 0, imax = pc.size(); i < imax; i++) {
          if (ip[i]) rpc.push_back(pc[i]);
        }

        vector<short int> tindx(ts.size(), -1);
        short int rnt = 0;
        for (int i = 0, imax = it.size(); i < imax; i++) {
          if (it[i]) tindx[i] = rnt++;
        }
        vector<triangle_t> rts;
        rts.reserve(rnt);
        short int nt = ts.size();
        auto newind = [&nt, &tindx, &rnt](short int n) -> short int {return (n < nt) ? tindx[n] : rnt;};
        for (int i = 0, imax = ts.size(); i < imax; i++) {
          if (it[i]) {
            const triangle_t& t = ts[i];
            rts.push_back({pindx[t.j0], pindx[t.j1], pindx[t.j2], newind(t.n0), newind(t.n1), newind(t.n2)});
          }
        }

        B2DEBUG(50, "Reduce map size to cover only region R<" << m_rmax << " cm: Ntriangles=" << rnt << " Nxypoints = " << rnp <<
                " Nzslices=" << m_nz << " NBpoints = " << rnp * m_nz);
        std::swap(rpc, pc);
        std::swap(rts, ts);
      } else {
        ip.resize(nrphi, true);
      }
      m_nxy = pc.size();

      m_triInterpol.init(pc, ts, 0.1);

      vector<B2Vector3F> bc(m_nxy * m_nz);
      unsigned int count = 0;
      for (int i = 0; i < nrphi; i++) {
        if (ip[i]) bc[count++] = B2Vector3F(tbc[i]);
      }

      for (int i = 1; i < m_nz; ++i) {
        for (int j = 0; j < nrphi; j++) {
          IN.getline(cbuf, 256);
          if (!ip[j]) continue;
          char* next = cbuf;
          next = strchr(next, ' ');
          next = strchr(next + 1, ' ');
          next = strchr(next + 1, ' ');
          double Br   = strtod(next, &next);
          double Bphi = strtod(next, &next);
          double Bz   = strtod(next, NULL);
          if (cs[j].s == 0) Bphi = 0;
          double Bx = Br * cs[j].c - Bphi * cs[j].s;
          double By = Br * cs[j].s + Bphi * cs[j].c;
          bc[count++].SetXYZ(Bx, By, Bz);
        }
      }
      assert(count == bc.size());
      swap(bc, m_B);
    }

    /**
     * For a given Z coordinate calculate the index of Z slice and corresponding weight
     *
     * @param z  Z coordinate
     * @param w1  weight of the slice
     *
     * @return the index of Z slice. Returns -1 if Z coordinate is outside valid region
     * region.
     */
    int zIndexAndWeight(double z, double& w1) const
    {
      if (std::abs(z) > m_zmax) return -1;
      double fz;
      int iz = 0;
      if (z < - m_zj) {
        fz = (z + m_zmax) * m_idz1;
      } else if (z < m_zj) {
        fz = (z + m_zj) * m_idz0;
        iz = m_nz1;
      } else {
        fz = (z - m_zj) * m_idz1;
        iz = m_nz2;
      }
      int jz = static_cast<int>(fz);
      w1 = fz - jz;
      iz += jz;
      if (iz == m_nz) {
        --iz;
        w1 = 1;
      }
      return iz;
    }

    /**
     * Check the space point if the interpolation exists.
     *
     * @param v The space point in Cartesian coordinates (x,y,z) in
     * [cm] at which the interpolation exists.
     * @return The magnetic field vector at the given space point in
     * [T]. Returns false if the space point lies outside the valid
     * region.
     */
    bool inRange(const B2Vector3D& v) const
    {
      if (std::abs(v.z()) > m_zmax) return false;
      double R2 = v.x() * v.x() + v.y() * v.y();
      if (R2 > m_rmax * m_rmax) return false;
      return true;
    }

    /**
     * Interpolate the magnetic field vector at the specified space point.
     *
     * @param v The space point in Cartesian coordinates (x,y,z) in
     * [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in
     * [T]. Returns a zero vector (0,0,0) if the space point lies
     * outside the region described by the component.
     */
    B2Vector3D interpolateField(const B2Vector3D& v) const
    {
      B2Vector3D res = {0, 0, 0};
      double R2 = v.x() * v.x() + v.y() * v.y();
      if (R2 > m_rmax * m_rmax) return res;
      double wz1;
      int iz = zIndexAndWeight(v.z(), wz1);
      if (iz < 0) return res;
      double wz0 = 1 - wz1;

      if (R2 < m_rj2) { // triangular interpolation
        xy_t xy = {v.x(), std::abs(v.y())};
        double w0, w1, w2;
        short int it = m_triInterpol.findTriangle(xy);
        m_triInterpol.weights(it, xy, w0, w1, w2);
        vector<triangle_t>::const_iterator t = m_triInterpol.getTriangles().begin() + it;
        int j0 = t->j0, j1 = t->j1, j2 = t->j2;
        const B2Vector3F* B = m_B.data() + m_nxy * iz;
        B2Vector3D b = (B[j0] * w0 + B[j1] * w1 + B[j2] * w2) * wz0;
        B += m_nxy; // next z-slice
        b += (B[j0] * w0 + B[j1] * w1 + B[j2] * w2) * wz1;
        res = b;
      } else {// r-phi grid
        double r = sqrt(R2), phi = atan2(std::abs(v.y()), v.x());
        double fr = (r - m_rj) * m_idr;
        double fphi = phi * m_idphi;

        int ir = static_cast<int>(fr);
        int iphi = static_cast<int>(fphi);

        ir -= (ir == m_nr);
        iphi -= (iphi == m_nphi);

        double wr1 = fr - ir, wr0 = 1 - wr1;
        double wphi1 = fphi - iphi, wphi0 = 1 - wphi1;
        const int nr1 = m_nr + 1, nphi1 = m_nphi + 1;
        int j00 = m_nxy - nr1 * (nphi1 - iphi) + ir;
        int j01 = j00 + 1;
        int j10 = j00 + nr1;
        int j11 = j01 + nr1;

        double w00 = wr0 * wphi0, w01 = wphi0 * wr1, w10 = wphi1 * wr0, w11 = wphi1 * wr1;
        const B2Vector3F* B = m_B.data() + m_nxy * iz;
        B2Vector3D b = (B[j00] * w00 + B[j01] * w01 + B[j10] * w10 + B[j11] * w11) * wz0;
        B += m_nxy; // next z-slice
        b += (B[j00] * w00 + B[j01] * w01 + B[j10] * w10 + B[j11] * w11) * wz1;
        res = b;
      }
      if (v.y() < 0) res.SetY(-res.y());
      return res;
    }
  protected:
    /** Buffer for the magnetic field map */
    vector<B2Vector3F> m_B;
    /** Object to locate point in a triangular mesh */
    TriangularInterpolation m_triInterpol;
    /** Number of field points in XY plane */
    int m_nxy{0};
    /** Number of field slices in Z direction */
    int m_nz{0};
    /** Start Z slice number for the finer Z grid */
    int m_nz1{0};
    /** End Z slice number for the finer Z grid */
    int m_nz2{0};
    /** Number of grid points in R direction */
    int m_nr{0};
    /** Number of grid points in Phi direction */
    int m_nphi{0};
    /** Separation radius between triangular and cylindrical meshes */
    double m_rj{0};
    /** Square of the separation radius between triangular and cylindrical meshes */
    double m_rj2{0};
    /** Z border of finer Z grid */
    double m_zj{0};
    /** Coarse Z grid pitch */
    double m_dz0{0};
    /** Finer Z grid pitch */
    double m_dz1{0};
    /** Inverse of coarse Z grid pitch */
    double m_idz0{0};
    /** Inverse of finer Z grid pitch */
    double m_idz1{0};
    /** Repciprocal of Phi grid */
    double m_idphi{0};
    /** Repciprocal of R grid */
    double m_idr{0};
    /** Maximal radius where interpolation is still valid */
    double m_rmax{0};
    /** Maximal Z where interpolation is still valid */
    double m_zmax{0};
  };

  void BFieldComponentBeamline::initialize()
  {
    if (!m_ler) m_ler = new BeamlineFieldMapInterpolation;
    if (!m_her) m_her = new BeamlineFieldMapInterpolation;
    m_ler->init(m_mapFilename_ler, m_interFilename_ler, m_mapRegionR[1]);
    m_her->init(m_mapFilename_her, m_interFilename_her, m_mapRegionR[1]);
  }

  BFieldComponentBeamline::~BFieldComponentBeamline()
  {
    if (m_ler) delete m_ler;
    if (m_her) delete m_her;
  }

  bool BFieldComponentBeamline::isInRange(const B2Vector3D& p) const
  {
    if (!m_ler || !m_her) return false;
    double s = m_sinBeamCrossAngle, c = m_cosBeamCrossAngle;
    B2Vector3D v = -p; // invert coordinates to match ANSYS one
    double xc = v.x() * c, zs = v.z() * s, zc = v.z() * c, xs = v.x() * s;
    B2Vector3D hv{xc - zs, v.y(), zc + xs};
    B2Vector3D lv{xc + zs, v.y(), zc - xs};
    return m_ler->inRange(lv) || m_her->inRange(hv);
  }

  B2Vector3D BFieldComponentBeamline::calculate(const B2Vector3D& p) const
  {
    B2Vector3D res;
    double s = m_sinBeamCrossAngle, c = m_cosBeamCrossAngle;
    B2Vector3D v = -p; // invert coordinates to match ANSYS one
    double xc = v.x() * c, zs = v.z() * s, zc = v.z() * c, xs = v.x() * s;
    B2Vector3D hv{xc - zs, v.y(), zc + xs};
    B2Vector3D lv{xc + zs, v.y(), zc - xs};
    B2Vector3D hb = m_her->interpolateField(hv);
    B2Vector3D lb = m_ler->interpolateField(lv);
    B2Vector3D rhb{hb.x()* c + hb.z()* s, hb.y(),  hb.z()* c - hb.x()* s};
    B2Vector3D rlb{lb.x()* c - lb.z()* s, lb.y(),  lb.z()* c + lb.x()* s};

    double mhb = std::abs(rhb.x()) + std::abs(rhb.y()) + std::abs(rhb.z());
    double mlb = std::abs(rlb.x()) + std::abs(rlb.y()) + std::abs(rlb.z());

    if (mhb < 1e-10) res = rlb;
    else if (mlb < 1e-10) res = rhb;
    else {
      res = 0.5 * (rlb + rhb);
    }
    return res;
  }

  void BFieldComponentBeamline::terminate()
  {
  }

  /** Static function holding the instance.*/
  BFieldComponentBeamline** GetInstancePtr()
  {
    static BFieldComponentBeamline* gInstance = nullptr;
    return &gInstance;
  }

  ///static function
  BFieldComponentBeamline& BFieldComponentBeamline::Instance()
  {
    auto gInstance = GetInstancePtr();
    if (*gInstance == nullptr) {
      // Constructor creates a new instance, inits gInstance.
      new BFieldComponentBeamline();
    }
    return **gInstance;
  }

  BFieldComponentBeamline::BFieldComponentBeamline()
  {
    auto gInstance = GetInstancePtr();
    if (*gInstance != nullptr) {
      B2WARNING("BFieldComponentBeamline: object already instantiated");
    } else {
      *gInstance = this;
    }
  }

}
