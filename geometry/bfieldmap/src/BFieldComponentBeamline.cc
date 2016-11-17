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

  typedef short int tindex_t;

  struct triangledelaunay_t {
    tindex_t j0, j1, j2;
    tindex_t n0, n1, n2;
  };

  struct xy_t {
    double x, y;
  };

  union vector3_t {
    union {
      struct {
        double x, y, z;
      };
      double v[3];
    };
    vector3_t& operator +=(const vector3_t& u) {
      x += u.x;
      y += u.y;
      z += u.z;
      return *this;
    }
  };

  vector3_t operator +(const vector3_t& u, const vector3_t& v)
  {
    return {u.x + v.x, u.y + v.y, u.z + v.z};
  }

  vector3_t operator -(const vector3_t& u, const vector3_t& v)
  {
    return {u.x - v.x, u.y - v.y, u.z - v.z};
  }

  vector3_t operator *(const vector3_t& u, double a)
  {
    return {u.x * a, u.y * a, u.z * a};
  }

  vector3_t operator *(double a, const vector3_t& u)
  {
    return u * a;
  }

  class interpol_t {
    vector<triangledelaunay_t> _ts;
    vector<xy_t> _pc;
    vector<tindex_t> _indx;
    vector<xy_t> _c;
    vector<double> _id;
    double _xmin, _xmax;
    double _ymin, _ymax;
    unsigned int _nx, _ny;
    double _ixnorm, _iynorm;
  public:
    const vector<xy_t>& getpoints() const { return _pc;}
    const vector<triangledelaunay_t>& gettriangles() const { return _ts;}

    interpol_t() {}

    interpol_t(vector<xy_t> pc, vector<triangledelaunay_t> ts, double d)
    {
      init(pc, ts, d);
    }

    ~interpol_t() {}

    void init(vector<xy_t> pc, vector<triangledelaunay_t> ts, double d)
    {
      std::swap(pc, _pc);
      std::swap(ts, _ts);
      const double inf = numeric_limits<double>::infinity();
      double xmin = inf, xmax = -inf;
      double ymin = inf, ymax = -inf;
      auto limit = [&xmin, &xmax, &ymin, &ymax](const xy_t & p) -> void {
        xmin = std::min(xmin, p.x); xmax = std::max(xmax, p.x);
        ymin = std::min(ymin, p.y); ymax = std::max(ymax, p.y);
      };
      for (const auto& t : _ts) {
        limit(_pc[t.j0]);
        limit(_pc[t.j1]);
        limit(_pc[t.j2]);
      }
      double xw = xmax - xmin;
      double yw = ymax - ymin;

      xmin -= xw * (1. / 256), xmax += xw * (1. / 256);
      ymin -= yw * (1. / 256), ymax += yw * (1. / 256);
      int nx = lrint(xw * (1 + 1. / 128) / d), ny = lrint(yw * (1 + 1. / 128) / d);
      nx = std::max(nx, 2);
      ny = std::max(ny, 2);
      makeindex(nx, xmin, xmax, ny, ymin, ymax);
    }

    xy_t center(const triangledelaunay_t& p) const
    {
      const xy_t& p0 = _pc[p.j0], &p1 = _pc[p.j1], &p2 = _pc[p.j2];
      return {(p0.x + p1.x + p2.x)* (1. / 3), (p0.y + p1.y + p2.y)* (1. / 3)};
    }

    void makeindex(int nx, double xmin, double xmax, int ny, double ymin, double ymax)
    {
      _nx = nx, _ny = ny, _xmin = xmin, _xmax = xmax, _ymin = ymin, _ymax = ymax;
      _ixnorm = (nx - 1) / (xmax - xmin), _iynorm = (ny - 1) / (ymax - ymin);
      double dx = (xmax - xmin) / (nx - 1), dy = (ymax - ymin) / (ny - 1);
      //    cout<<nx<<" "<<xmin<<" "<<xmax<<" "<<ny<<" "<<ymin<<" "<<ymax<<endl;
      _indx.resize(nx * ny);
      _c.resize(_ts.size());
      for (unsigned int i = 0; i < _ts.size(); i++) _c[i] = center(_ts[i]);

      _id.resize(_ts.size());
      auto getid = [this](const triangledelaunay_t& p) ->double{
        const xy_t& p0 = _pc[p.j0], &p1 = _pc[p.j1], &p2 = _pc[p.j2];
        double d21x = p2.x - p1.x, d21y = p2.y - p1.y;
        double d01x = p0.x - p1.x, d01y = p0.y - p1.y;
        return 1 / (d21x * d01y - d21y * d01x);
      };
      for (unsigned int i = 0; i < _ts.size(); i++) _id[i] = getid(_ts[i]);


      for (int ix = 0; ix < nx; ix++) {
        double x = xmin + ix * dx;
        for (int iy = 0; iy < ny; iy++) {
          double y = ymin + iy * dy;
          int imin = -1; double dmin = 1e100;
          for (unsigned int i = 0; i < _c.size(); i++) {
            const xy_t& p = _c[i];
            double d = pow(p.x - x, 2) + pow(p.y - y, 2);
            if (d < dmin) {imin = i; dmin = d;}
          }
          int k = iy + ny * ix;
          _indx[k] = imin;
        }
      }
    }

    tindex_t sidecross(tindex_t prev, tindex_t curr, const xy_t& r, const xy_t& v0) const
    {
      const double vx = r.x - v0.x, vy = r.y - v0.y;
      auto iscrossed = [&vx, &vy, &v0](const xy_t & p1, const xy_t & p0) -> bool {
        double u0x = p0.x, u0y = p0.y;
        double ux = p1.x - u0x, uy = p1.y - u0y;
        double dx = u0x - v0.x, dy = u0y - v0.y;
        double D = uy * vx - ux * vy;
        double t = dx * vy - dy * vx;
        double s = dx * uy - dy * ux;
        return ((t < D) != (t < 0)) && ((s < D) != (s < 0));
      };

      const triangledelaunay_t& p = _ts[curr];
      const xy_t& p0 = _pc[p.j0], &p1 = _pc[p.j1], &p2 = _pc[p.j2];
      if (p.n0 != prev && iscrossed(p1, p2)) return p.n0;
      if (p.n1 != prev && iscrossed(p2, p0)) return p.n1;
      if (p.n2 != prev && iscrossed(p0, p1)) return p.n2;
      return _ts.size();
    }

    void weights(tindex_t i, const xy_t& r, double& w0, double& w1, double& w2) const
    {
      const triangledelaunay_t& p = _ts[i];
      const xy_t& p0 = _pc[p.j0], &p1 = _pc[p.j1], &p2 = _pc[p.j2];
      double dx2  = p2.x -  r.x, dy2  = p2.y -  r.y;
      double d21x = p2.x - p1.x, d21y = p2.y - p1.y;
      double d02x = p0.x - p2.x, d02y = p0.y - p2.y;
      w0 = (dx2 * d21y - dy2 * d21x) * _id[i];
      w1 = (dx2 * d02y - dy2 * d02x) * _id[i];
      w2 = 1 - (w0 + w1);
    }

    tindex_t find_triangle(const xy_t& r0) const
    {
      unsigned int ix = lrint((r0.x - _xmin) * _ixnorm), iy = lrint((r0.y - _ymin) * _iynorm);
      tindex_t curr = (ix < _nx && iy < _ny) ? _indx[iy + _ny * ix] : 0;
      xy_t r = _c[curr];
      const tindex_t end = _ts.size();
      tindex_t prev = end;
      do {
        tindex_t next = sidecross(prev, curr, r, r0);
        if (next == end) break;
        prev = curr;
        curr = next;
      } while (1);
      return curr;
    }
  };

  class interpol3d_t {
    vector<vector3_t> _B;
    interpol_t _I;
    int _nxy, _nz, _nz1, _nz2, _nr, _nphi;
    double _rj, _rj2, _zj, _dz0, _dz1, _idz0, _idz1, _idphi, _idr;
    double _rmax, _zmax;
  public:
    const interpol_t& getinterpol() const {return _I;}
    interpol3d_t() {}
    ~interpol3d_t() {}

    void init(const string& fieldmapname, const string& interpolname, double validradius)
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

      _rmax = validradius;

      B2INFO("Delaunay triangulation of the beamline field map: " << l_interpolname);
      B2INFO("Beamline field map: " << l_fieldmapname);

      // load interpolation triangles
      ifstream INd(l_interpolname);
      int nts; INd >> nts;
      B2INFO("Total number of triangles: " << nts);
      vector<triangledelaunay_t> ts;
      ts.reserve(nts);

      triangledelaunay_t p;
      while (INd >> nts >> p.j0 >> p.j1 >> p.j2 >> p.n0 >> p.n1 >> p.n2) ts.push_back(p);

      //Load the map file
      io::filtering_istream IN;
      IN.push(io::gzip_decompressor());
      IN.push(io::file_source(l_fieldmapname));

      int nrphi;
      IN >> nrphi >> _rj >> _nr >> _nphi;
      IN >> _nz >> _zj >> _dz0 >> _dz1;
      _idphi = _nphi / M_PI;
      _rj2 = _rj * _rj;
      _idz0 = 1 / _dz0;
      _idz1 = 1 / _dz1;
      int nz0 = 2 * int(_zj / _dz0);
      _zmax = (_nz - (nz0 + 1)) / 2 * _dz1 + _zj;
      _nz1 = (_nz - (nz0 + 1)) / 2;
      _nz2 = _nz1 + nz0;
      //    cout<<_zmax<<" "<<nz0<<" "<<_nz1<<" "<<_nz2<<endl;

      struct cs_t {double c, s;};
      vector<cs_t> cs(nrphi);
      vector<xy_t> pc;
      vector<vector3_t> tbc;
      pc.reserve(nrphi);
      char cbuf[256]; IN.getline(cbuf, 256);
      double r, phi, Br, Bphi, Bz;
      double rmax = 0;
      for (int j = 0; j < nrphi; j++) {
        IN.getline(cbuf, 256);
        char* next = cbuf;
        r    = strtod(next, &next);
        phi  = strtod(next, &next);
        strtod(next, &next);
        Br   = strtod(next, &next);
        Bphi = strtod(next, &next);
        Bz   = strtod(next, NULL);
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

      _idr = _nr / (rmax - _rj);
      _rmax = std::min(_rmax, rmax);
      bool reduce = _rj > _rmax;

      vector<bool> ip;
      if (reduce) {
        ip.resize(nrphi, false);
        vector<bool> it(ts.size(), false);
        auto inside = [this](const xy_t & p)->bool{
          return p.x * p.x + p.y * p.y < _rmax * _rmax;
        };

        for (int i = 0, imax = ts.size(); i < imax; i++) {
          const triangledelaunay_t& p = ts[i];
          const xy_t& p0 = pc[p.j0], &p1 = pc[p.j1], &p2 = pc[p.j2];
          if (inside(p0) || inside(p1) || inside(p2)) {
            it[i] = 1;
            ip[p.j0] = 1;
            ip[p.j1] = 1;
            ip[p.j2] = 1;
          }
        }

        vector<tindex_t> pindx(nrphi, -1);
        int rnp = 0;
        for (int i = 0, imax = ip.size(); i < imax; i++) {
          if (ip[i]) pindx[i] = rnp++;
        }
        vector<xy_t> rpc;
        rpc.reserve(rnp);
        for (int i = 0, imax = pc.size(); i < imax; i++) {
          if (ip[i]) rpc.push_back(pc[i]);
        }

        vector<tindex_t> tindx(ts.size(), -1);
        tindex_t rnt = 0;
        for (int i = 0, imax = it.size(); i < imax; i++) {
          if (it[i]) tindx[i] = rnt++;
        }
        vector<triangledelaunay_t> rts;
        rts.reserve(rnt);
        tindex_t nt = ts.size();
        auto newind = [&nt, &tindx, &rnt](tindex_t n) -> tindex_t {return (n < nt) ? tindx[n] : rnt;};
        for (int i = 0, imax = ts.size(); i < imax; i++) {
          if (it[i]) {
            const triangledelaunay_t& t = ts[i];
            rts.push_back({pindx[t.j0], pindx[t.j1], pindx[t.j2], newind(t.n0), newind(t.n1), newind(t.n2)});
          }
        }

        B2INFO("Reduce map size to cover only region R<" << _rmax << " cm: Ntriangles=" << rnt << " Nxypoints = " << rnp << " Nzslices=" <<
               _nz << " NBpoints = " << rnp * _nz);
        std::swap(rpc, pc);
        std::swap(rts, ts);
      } else {
        ip.resize(nrphi, true);
      }
      _nxy = pc.size();

      _I.init(pc, ts, 0.1);

      vector<vector3_t> bc(_nxy * _nz);
      unsigned int count = 0;
      for (int i = 0; i < nrphi; i++) {
        if (ip[i]) bc[count++] = tbc[i];
      }

      for (int i = 1; i < _nz; ++i) {
        for (int j = 0; j < nrphi; j++) {
          IN.getline(cbuf, 256);
          if (!ip[j]) continue;
          char* next = cbuf;
          next = strchr(next, ' ');
          next = strchr(next + 1, ' ');
          next = strchr(next + 1, ' ');
          Br   = strtod(next, &next);
          Bphi = strtod(next, &next);
          Bz   = strtod(next, NULL);
          if (cs[j].s == 0) Bphi = 0;
          double Bx = Br * cs[j].c - Bphi * cs[j].s;
          double By = Br * cs[j].s + Bphi * cs[j].c;
          bc[count++] = {Bx, By, Bz};
        }
      }
      assert(count == bc.size());
      swap(bc, _B);
    }

    int zindexweight(double z, double& w1) const __attribute__((noinline))
    {
      if (std::abs(z) > _zmax) return -1;
      double fz;
      int iz = 0;
      if (z < -_zj) {
        fz = (z + _zmax) * _idz1;
      } else if (z < _zj) {
        fz = (z + _zj) * _idz0;
        iz = _nz1;
      } else {
        fz = (z - _zj) * _idz1;
        iz = _nz2;
      }
      int jz = static_cast<int>(fz);
      w1 = fz - jz;
      iz += jz;
      if (iz == _nz) {
        --iz;
        w1 = 1;
      }
      return iz;
    }

    bool inrange(const vector3_t& v) const
    {
      if (std::abs(v.z) > _zmax) return false;
      double R2 = v.x * v.x + v.y * v.y;
      if (R2 > _rmax * _rmax) return false;
      return true;
    }

    vector3_t getfield(const vector3_t& v) const
    {
      vector3_t res = {0, 0, 0};
      double R2 = v.x * v.x + v.y * v.y;
      if (R2 > _rmax * _rmax) return res;
      double wz1;
      int iz = zindexweight(v.z, wz1);
      if (iz < 0) return res;
      double wz0 = 1 - wz1;

      if (R2 < _rj2) { // triangular interpolation
        xy_t xy = {v.x, std::abs(v.y)};
        double w0, w1, w2;
        tindex_t it = _I.find_triangle(xy);
        _I.weights(it, xy, w0, w1, w2);
        vector<triangledelaunay_t>::const_iterator t = _I.gettriangles().begin() + it;
        int j0 = t->j0, j1 = t->j1, j2 = t->j2;
        const vector3_t* B = _B.data() + _nxy * iz;
        vector3_t b = (B[j0] * w0 + B[j1] * w1 + B[j2] * w2) * wz0;
        B += _nxy; // next z-slice
        b += (B[j0] * w0 + B[j1] * w1 + B[j2] * w2) * wz1;
        res = b;
      } else {// r-phi grid
        double r = sqrt(R2), phi = atan2(std::abs(v.y), v.x);
        double fr = (r - _rj) * _idr;
        double fphi = phi * _idphi;

        int ir = static_cast<int>(fr);
        int iphi = static_cast<int>(fphi);

        ir -= (ir == _nr);
        iphi -= (iphi == _nphi);

        double wr1 = fr - ir, wr0 = 1 - wr1;
        double wphi1 = fphi - iphi, wphi0 = 1 - wphi1;
        const int nr1 = _nr + 1, nphi1 = _nphi + 1;
        int j00 = _nxy - nr1 * (nphi1 - iphi) + ir;
        int j01 = j00 + 1;
        int j10 = j00 + nr1;
        int j11 = j01 + nr1;

        const vector3_t* B = _B.data() + _nxy * iz;
        vector3_t b = ((B[j00] * wr0 + B[j01] * wr1) * wphi0 + (B[j10] * wr0 + B[j11] * wr1) * wphi1) * wz0;
        B += _nxy; // next z-slice
        b += ((B[j00] * wr0 + B[j01] * wr1) * wphi0 + (B[j10] * wr0 + B[j11] * wr1) * wphi1) * wz1;
        res = b;
      }
      if (v.y > 0) res.y = -res.y;
      return res;
    }
  };

  void BFieldComponentBeamline::initialize()
  {
    if (!m_ler) m_ler = new interpol3d_t;
    if (!m_her) m_her = new interpol3d_t;
    m_ler->init(m_mapFilename_ler, m_interFilename_ler, s_mapRegionR[1]);
    m_her->init(m_mapFilename_her, m_interFilename_her, s_mapRegionR[1]);
  }

  BFieldComponentBeamline::~BFieldComponentBeamline()
  {
    if (m_ler) delete m_ler;
    if (m_ler) delete m_ler;
  }

  bool BFieldComponentBeamline::isInRange(const TVector3& v) const
  {
    TVector3 res;
    double s = s_sinBeamCrossAngle, c = s_cosBeamCrossAngle;

    double xc = -v.x() * c, zs = -v.z() * s, zc = -v.z() * c, xs = -v.x() * s;
    vector3_t hv = {xc - zs, v.y(), zc + xs};
    vector3_t lv = {xc + zs, v.y(), zc - xs};
    return m_ler->inrange(lv) || m_her->inrange(hv);
  }

  TVector3 BFieldComponentBeamline::calculate(const TVector3& v) const
  {
    TVector3 res;
    double s = s_sinBeamCrossAngle, c = s_cosBeamCrossAngle;

    double xc = -v.x() * c, zs = -v.z() * s, zc = -v.z() * c, xs = -v.x() * s;
    vector3_t hv = {xc - zs, v.y(), zc + xs};
    vector3_t lv = {xc + zs, v.y(), zc - xs};
    vector3_t hb = m_her->getfield(hv);
    vector3_t lb = m_ler->getfield(lv);
    vector3_t rhb = {hb.x* c + hb.z * s, hb.y,  hb.z* c - hb.x * s};
    vector3_t rlb = {lb.x* c - lb.z * s, lb.y,  lb.z* c + lb.x * s};

    double mhb = std::abs(rhb.x) + std::abs(rhb.y) + std::abs(rhb.z);
    double mlb = std::abs(rlb.x) + std::abs(rlb.y) + std::abs(rlb.z);

    if (mhb < 1e-10) res.SetXYZ(rlb.x, rlb.y, rlb.z);
    else if (mlb < 1e-10) res.SetXYZ(rhb.x, rhb.y, rhb.z);
    else {
      vector3_t t = 0.5 * (rlb + rhb);
      res.SetXYZ(t.x, t.y, t.z);
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
