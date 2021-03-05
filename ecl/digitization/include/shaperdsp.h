/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <utility>
#include <limits>

static const double realNaN = std::numeric_limits<double>::quiet_NaN();

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
      double _cs0 = realNaN;
      /** linear coefficient before cos of the first Bessel stage    */
      double _cc0 = realNaN;
      /** linear coefficient before sin of the second Bessel stage    */
      double _cs1 = realNaN;
      /** linear coefficient before cos of the second Bessel stage    */
      double _cc1 = realNaN;
      /** linear coefficient before first part of tail section    */
      double _ces = realNaN;
      /** linear coefficient before second part of tail section    */
      double _ced = realNaN;
      /**  circular frequency of the first Bessel stage  */
      double  _dw0 = realNaN;
      /**  circular frequency of the second Bessel stage  */
      double  _dw1 = realNaN;
      /**  decrement of the first Bessel stage  */
      double _dks0 = realNaN;
      /**  decrement of the second Bessel stage  */
      double _dks1 = realNaN;
      /** inverse scintillation decay time */
      double   _ds = realNaN;
      /** inverse time of the differential stage */
      double   _dd = realNaN;
      /** coefficient for first exponent factor  */
      double  _dt0 = realNaN;
      /** coefficient for second exponent factor  */
      double  _dt1 = realNaN;

      /** time offset  */
      double _toff = realNaN;
      /** weight coefficient at sv123(t)  = (1-a)  */
      double  _w0 = realNaN;
      /** weight coefficient at sv123(t+_filterdt) +sv123(t-_filterdt)  = a/2  */
      double  _w1 = realNaN;

      /**  exponent factor for tail part of the signal   */
      double _ccc = realNaN;

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
      /** Initialization of the parameters response function
       *  @param u   'unitscale' of the waveform template
       *             This sets the normalization for the template amplitude to be unity.
       *             If the waveform templates are modified this needs to be adjusted as well.
       *  If unitscale is set to negative value, it is calculated dynamically
       *  based on shape parameters.
       */
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
