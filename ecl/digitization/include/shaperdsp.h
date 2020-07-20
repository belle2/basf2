/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *               Alex Bobrov                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SHAPERDSP_T_H
#define SHAPERDSP_T_H

#include <vector>
#include <utility>

namespace Belle2 {
  namespace ECL {
    typedef std::pair<double, double> dd_t;

    /** Class include function that calculate electronic response from energy deposit   */
    class ShaperDSP_t {
    public:

      /** struct to encapsulate the electronic response from energy deposit   */
      struct sv123shift_t {
        /** time   */
        double t;
        /** sin of the first Bessel stage   */
        double s0;
        /** cos of the first Bessel stage   */
        double c0;
        /** sin of the second Bessel stage   */
        double s1;
        /** cos of the second Bessel stage   */
        double c1;
        /** exponent factor for first Bessel stage   */
        double e0;
        /** exponent factor for second  Bessel stage   */
        double e1;
        /** first exponent factor for tail part of the signal. Scintillation decay time   */
        double es;
        /** second exponent factor for tail part of the signal. Time of the differential stage   */
        double ed;

        sv123shift_t() {} /**< default constructor */
        sv123shift_t(double _t, const ShaperDSP_t& _p) { init(_t, _p);}  /**< constructor from a ShaperDSP class */
        void init(double, const ShaperDSP_t&) __attribute__((noinline)); /**< initialise */
        sv123shift_t& operator +=(const sv123shift_t&);      /**< increment operator */
        sv123shift_t operator +(const sv123shift_t&) const;  /**< addition operator */
        bool validshift(const sv123shift_t& x) const {return t + x.t > 0;} /**< check for a valid shift */
      };

      /** struct for a shift of the shaper dsp */
      struct shaperdspshift_t: public sv123shift_t {
        double et0; /**< . */
        double et1; /**< . */

        shaperdspshift_t() {} /**< default constructor */
        shaperdspshift_t(double _t, const ShaperDSP_t& _p) { init(_t, _p);} /**< constructor from a ShaperDSP class */
        void init(double, const ShaperDSP_t&) __attribute__((noinline)); /**< initialise */
        shaperdspshift_t& operator +=(const shaperdspshift_t&); /**< increment operator */
        shaperdspshift_t operator +(const shaperdspshift_t&) const; /**< addition operator */
      };

    private:
      /** parameters of the response function that use as default    */
      static const double _defs[]; //
      /** time shift that include in response function for numerical calculation time convolutions.  Integration  with histogram
       * is used. Int g(s-t)f(t)dt=g(s)+a*(g(s+_filterdt)-g(s-_filterdt))/2   */
      static constexpr double _filterdt = 0.2;

      /** linear coefficient before sin of the first Bessel stage    */
      double _cs0;
      /** linear coefficient before cos of the first Bessel stage    */
      double _cc0;
      /** linear coefficient before sin of the second Bessel stage    */
      double _cs1;
      /** linear coefficient before cos of the second Bessel stage    */
      double _cc1;
      /** linear coefficient before first part of tail section    */
      double _ces;
      /** linear coefficient before second part of tail section    */
      double _ced;
      /**  circular frequency of the first Bessel stage  */
      double  _dw0;
      /**  circular frequency of the second Bessel stage  */
      double  _dw1;
      /**  decrement of the first Bessel stage  */
      double _dks0;
      /**  decrement of the second Bessel stage  */
      double _dks1;
      /** inverse scintillation decay time */
      double   _ds;
      /** inverse time of the differential stage */
      double   _dd;
      /** coefficient for first exponent factor  */
      double  _dt0;
      /** coefficient for second exponent factor  */
      double  _dt1;

      /** time offset  */
      double _toff;
      /** weight coefficient at sv123(t)  = (1-a)  */
      double  _w0;
      /** weight coefficient at sv123(t+_filterdt) +sv123(t-_filterdt)  = a/2  */
      double  _w1;

      /**  exponent factor for tail part of the signal   */
      double _ccc;

      /**  _filterdt   */
      sv123shift_t _tp;

      /**  - _filterdt   */
      sv123shift_t _tm;

      /** time step of the grid for response function calculation   */
      shaperdspshift_t _tstride;

      /** time offset */
      shaperdspshift_t  _toffset;
      /** initial time */
      shaperdspshift_t  _tzero;

      /** calculate some values for Sv123 function */
      void Sv123_init(double t01, double tb1, double t02, double tb2, double td1, double ts1);
      /** calculate some values for Sv123 function */
      void init(const double*, double) __attribute__((noinline));
      /** calculate some values for Sv123 function */
      void init(const double*) __attribute__((noinline));

      /** calculate  Sv123 function */
      double Sv123(const sv123shift_t&) const;
      /** calculate derivative of the Sv123 function */
      dd_t ddSv123(const sv123shift_t&) const;

      /**  Numerical calculation of the time convolution.  Integration  with histogram
       *   is used. Int g(s-t)f(t)dt=g(s)+a*(g(s+_filterdt)-g(s-_filterdt))/2   */
      double Sv123_filtered(const sv123shift_t&) const __attribute__((noinline));
      /**  This is derivative of the confolution  */
      dd_t ddSv123_filtered(const sv123shift_t&) const __attribute__((noinline));

      /** calculate  response function */
      double ShaperDSP(const shaperdspshift_t&) const;
      /** calculate derivative of the   response function */
      dd_t ddShaperDSP(const shaperdspshift_t&) const;

    public:
      /** class constructor  */
      ShaperDSP_t() { init(_defs); }
      /** calculate derivative of the   response function */
      explicit ShaperDSP_t(const std::vector<double>& s) { init(s); }
      /** class constructor */
      ShaperDSP_t(const std::vector<double>& s, double u) { init(s, u); }

      ~ShaperDSP_t() {}
      /** initialization of the parameters response function */
      void init(const std::vector<double>& s, double u = 27.7221);
      /** wrapper of the function */
      double operator()(double) const;
      /** TF1 ROOT interface */
      double operator()(double*, double*);
      /** set grid step for function calculation */
      void settimestride(double);
      /** set timeoffset */
      void setseedoffset(double);
      /** set initial time */
      void settimeseed(double);
      /** substruct toffset to tzero */
      void nextseed();
      /** fill vector with response function values and its derivative */
      void fillvector(std::vector<double>&) const;
      /** fill vector with response function values and its derivative */
      void fillvector(std::vector<dd_t>&) const;
      /** fill vector with response function values and its derivative */
      void fillvector(double, std::vector<double>&) const;
      /** fill vector with response function values and its derivative */
      void fillvector(double, std::vector<dd_t>&) const;
      /** fill array for amplitude and time calculation */
      void fillarray(int, double*) const;
      /** fill array for amplitude and time calculation */
      void fillarray(int, dd_t*) const;
      /** fill array for amplitude and time calculation */
      void fillarray(double, int, double*) const;
      /** fill array for amplitude and time calculation */
      void fillarray(double, int, dd_t*) const;
    };
  }
}
#endif//SHAPERDSP_T_H
