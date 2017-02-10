#ifndef SHAPERDSP_T_H
#define SHAPERDSP_T_H

#include <vector>
#include <utility>

namespace Belle2 {
  namespace ECL {
    typedef std::pair<double, double> dd_t;

    class ShaperDSP_t {
    public:
      struct sv123shift_t {
        double t, s0, c0, s1, c1, e0, e1, es, ed;

        sv123shift_t() {}
        sv123shift_t(double _t, const ShaperDSP_t& _p) { init(_t, _p);}
        void init(double, const ShaperDSP_t&) __attribute__((noinline));
        sv123shift_t& operator +=(const sv123shift_t&);
        sv123shift_t operator +(const sv123shift_t&) const;
        bool validshift(const sv123shift_t& x) const {return t + x.t > 0;}
      };

      struct shaperdspshift_t: public sv123shift_t {
        double et0, et1;

        shaperdspshift_t() {}
        shaperdspshift_t(double _t, const ShaperDSP_t& _p) { init(_t, _p);}
        void init(double, const ShaperDSP_t&) __attribute__((noinline));
        shaperdspshift_t& operator +=(const shaperdspshift_t&);
        shaperdspshift_t operator +(const shaperdspshift_t&) const;
      };

    private:
      static const double _defs[]; // default parameters
      static constexpr double _filterdt = 0.2;

      double _cs0, _cc0, _cs1, _cc1, _ces, _ced;
      double _dw0, _dw1, _dks0, _dks1, _ds, _dd, _dt0, _dt1;
      double _toff, _w0, _w1, _ccc;
      sv123shift_t _tp, _tm;
      shaperdspshift_t _tstride, _toffset, _tzero;

      void Sv123_init(double t01, double tb1, double t02, double tb2, double td1, double ts1);
      void init(const double*, double) __attribute__((noinline));
      void init(const double*) __attribute__((noinline));

      double Sv123(const sv123shift_t&) const;
      dd_t ddSv123(const sv123shift_t&) const;

      double Sv123_filtered(const sv123shift_t&) const __attribute__((noinline));
      dd_t ddSv123_filtered(const sv123shift_t&) const __attribute__((noinline));

      double ShaperDSP(const shaperdspshift_t&) const;
      dd_t ddShaperDSP(const shaperdspshift_t&) const;

    public:
      ShaperDSP_t() { init(_defs); }
      ShaperDSP_t(const std::vector<double>& s) { init(s); }
      ShaperDSP_t(const std::vector<double>& s, double u) { init(s, u); }
      ~ShaperDSP_t() {}

      void init(const std::vector<double>& s, double u = 27.7221);
      double operator()(double) const;
      double operator()(double*, double*);   // TF1 ROOT interface
      void settimestride(double);
      void setseedoffset(double);
      void settimeseed(double);
      void nextseed();
      void fillvector(std::vector<double>&) const;
      void fillvector(std::vector<dd_t>&) const;
      void fillvector(double, std::vector<double>&) const;
      void fillvector(double, std::vector<dd_t>&) const;
      void fillarray(int, double*) const;
      void fillarray(int, dd_t*) const;
      void fillarray(double, int, double*) const;
      void fillarray(double, int, dd_t*) const;
    };
  }
}
#endif//SHAPERDSP_T_H
