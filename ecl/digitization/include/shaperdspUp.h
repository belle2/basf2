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

static const double realNaNUp = std::numeric_limits<double>::quiet_NaN();

namespace Belle2 {
  namespace ECL {
    typedef std::pair<double, double> dd_t;

    /** Class include function that calculate electronic response from energy deposit   */
    class ShaperDSPUp_t {
    public:

      /** struct to encapsulate the electronic response from energy deposit : Fm and Fi parts  */
      struct sfshift_t {
        /** time   */
        double t;
        /** sin of the first Bessel stage   */
        double sm0;
        /** sin of the second Bessel stage   */
        double sm1;
        /** cos of the first Bessel stage   */
        double cm0;
        /** cos of the second Bessel stage   */
        double cm1;
        /** sin of the first Bessel stage   */
        double si0;
        /** sin of the second Bessel stage   */
        double si1;
        /** cos of the first Bessel stage   */
        double ci0;
        /** cos of the second Bessel stage   */
        double ci1;
        /** sin of the first Bessel stage   */
        double sm01;
        /** sin of the second Bessel stage   */
        double sm11;
        /** cos of the first Bessel stage   */
        double cm01;
        /** cos of the second Bessel stage   */
        double cm11;
        /** sin of the first Bessel stage   */
        double si01;
        /** sin of the second Bessel stage   */
        double si11;
        /** cos of the first Bessel stage   */
        double ci01;
        /** cos of the second Bessel stage   */
        double ci11;
        /** sin of the first Bessel stage without phi* phase   */
        double s0;
        /** sin of the second Bessel stage without phi* phase   */
        double s1;
        /** cos of the first Bessel stage without phi* phase   */
        double c0;
        /** cos of the second Bessel stage without phi* phase   */
        double c1;
        /** exponent factor for first Bessel stage : 1/tau_0  */
        double e0;
        /** exponent factor for second  Bessel stage : 1/tau_1  */
        double e1;
        /** first exponent factor for tail part of the signal. Scintillation decay time  */
        double es;
        /** first exponent factor for tail part of the signal. Slow scintillation decay time  */
        double es1;
        /** second exponent factor for tail part of the signal. Time of the differential stage */
        double ed;
        /** third exponent factor for tail part of the signal. Time of the differential stage   */
        double ei;

        sfshift_t() {} /**< default constructor */
        sfshift_t(double _t, const ShaperDSPUp_t& _p) { init(_t, _p);}  /**< constructor from a ShaperDSP class */
        void init(double, const ShaperDSPUp_t&) __attribute__((noinline)); /**< initialise */
        sfshift_t& operator +=(const sfshift_t&);      /**< increment operator */
        sfshift_t operator +(const sfshift_t&) const;  /**< addition operator */
        bool isPedestal(double t_start) const {return t < t_start;} /**< check for a valid shift */
      };

    private:
      /** parameters of the response function that use as default    */
      static const double _defs[]; //
      /** time shift that include in response function for numerical calculation time convolutions.  Integration  with histogram
       * is used. Int g(s-t)f(t)dt=g(s)+a*(g(s+_filterdt)-g(s-_filterdt))/2   */
      //static constexpr double _filterdt = 0.2;

      /** normalization coefficient of Fm for the main scintillation decay component */
      double _an = realNaNUp;
      /** normalization coefficient of Fi for the main scintillation decay component */
      double _ai = realNaNUp;
      /** normalization coefficient of Fm for the second scintillation decay component */
      double _an1 = realNaNUp;
      /** normalization coefficient of Fi for the second scintillation decay component */
      double _ai1 = realNaNUp;

      /** phase of the first Bessel term in Fm for the main scintillation decay component */
      double _phi0 = realNaNUp;
      /** phase of the second Bessel term in Fm for the main scintillation decay component */
      double _phi1 = realNaNUp;
      /** phase of the first Bessel term in Fi for the main scintillation decay component */
      double _phi0i = realNaNUp;
      /** phase of the second Bessel term in Fi for the main scintillation decay component */
      double _phi1i = realNaNUp;

      /** phase of the first Bessel term in Fm for the second scintillation decay component */
      double _phi01 = realNaNUp;
      /** phase of the second Bessel term in Fm for the second scintillation decay component */
      double _phi11 = realNaNUp;
      /** phase of the first Bessel term in Fi for the second scintillation decay component */
      double _phi0i1 = realNaNUp;
      /** phase of the second Bessel term in Fi for the second scintillation decay component */
      double _phi1i1 = realNaNUp;

      /** coefficient of the first Bessel sine term in Fm */
      double _cs0 = realNaNUp;
      /** coefficient of the first Bessel cosine term in Fi */
      double _cc0 = realNaNUp;
      /** coefficient of the second Bessel sine term in Fm */
      double _cs1 = realNaNUp;
      /** coefficient of the second Bessel cosine term in Fi */
      double _cc1 = realNaNUp;

      /** coefficient of the scintillation-decay exponential term in Fm */
      double _cems = realNaNUp;
      /** coefficient of the differentiation-time exponential term in Fm */
      double _cemd = realNaNUp;
      /** coefficient of the scintillation-decay exponential term in Fi */
      double _ceis = realNaNUp;
      /** coefficient of the differentiation-time exponential term in Fi */
      double _ceid = realNaNUp;
      /** coefficient of the integration-time exponential term in Fi */
      double _ceii = realNaNUp;

      /** coefficient of the first Bessel sine term in Fm for the second scintillation decay component */
      double _cs01 = realNaNUp;
      /** coefficient of the second Bessel sine term in Fm for the second scintillation decay component */
      double _cs11 = realNaNUp;
      /** coefficient of the first Bessel cosine term in Fi for the second scintillation decay component */
      double _cc01 = realNaNUp;
      /** coefficient of the second Bessel cosine term in Fi for the second scintillation decay component */
      double _cc11 = realNaNUp;

      /** coefficient of the scintillation-decay exponential term in Fm for the second decay component */
      double _cems1 = realNaNUp;
      /** coefficient of the differentiation-time exponential term in Fm for the second decay component */
      double _cemd1 = realNaNUp;
      /** coefficient of the scintillation-decay exponential term in Fi for the second decay component */
      double _ceis1 = realNaNUp;
      /** coefficient of the integration-time exponential term in Fi for the second decay component */
      double _ceii1 = realNaNUp;
      /** coefficient of the differentiation-time exponential term in Fi for the second decay component */
      double _ceid1 = realNaNUp;

      /** circular frequency of the first Bessel stage */
      double _w0 = realNaNUp;
      /** circular frequency of the second Bessel stage */
      double _w1 = realNaNUp;
      /** inverse main scintillation decay time */
      double _ds = realNaNUp;
      /** inverse second scintillation decay time */
      double _ds1 = realNaNUp;
      /** inverse differentiation time */
      double _dd = realNaNUp;
      /** inverse decay time of the first Bessel stage */
      double _d0 = realNaNUp;
      /** inverse decay time of the second Bessel stage */
      double _d1 = realNaNUp;
      /** inverse integration time */
      double _di = realNaNUp;

      /** time offset */
      double _toff = realNaNUp;
      /** fraction of the inversion cascade */
      double _fi = realNaNUp;
      /** fraction of the second scintillation decay component */
      double _alpha = realNaNUp;
      /** delay applied to the subtraction term */
      double _tshift = realNaNUp;
      /** fraction of the delayed subtraction term */
      double _eta = realNaNUp;
      /** normalization coefficient corresponding to the maximum of the response function */
      double _anorm = realNaNUp;

      /** time step of the grid for response function calculation   */
      sfshift_t _tstride;

      /** time offset */
      sfshift_t  _toffset;
      /** initial time */
      sfshift_t  _tzero;

      /** calculate some values for Sf function */
      void Sf_init(double dd, double d0, double w0, double d1, double w1, double di, double ds, double ds1);
      /** calculate some values for Sf function */
      void init(const double*, double) __attribute__((noinline));
      /** calculate some values for Sf function */
      void init(const double*) __attribute__((noinline));

      /** calculate Sf function */
      double Sf(const sfshift_t& c) const;
      /** calculate derivative of Sf function */
      double ddSf(const sfshift_t& c) const;

      /** calculate response function */
      double ShaperDSPUp(const sfshift_t&) const;
      /** calculate derivative of the   response function */
      dd_t ddShaperDSPUp(const sfshift_t&) const;

    public:
      /** class constructor  */
      ShaperDSPUp_t() { init(_defs); }
      /** calculate derivative of the   response function */
      explicit ShaperDSPUp_t(const std::vector<double>& s) { init(s); }
      /** class constructor */
      ShaperDSPUp_t(const std::vector<double>& s, double u) { init(s, u); }

      ~ShaperDSPUp_t() {}
      /** Initialization of the parameters response function
       *  @param s   vector of shape parameters
       *  @param u   'unitscale' of the waveform template
       *             This sets the normalization for the template amplitude to be unity.
       *             If the waveform templates are modified this needs to be adjusted as well.
       *  If unitscale is set to negative value, it is calculated dynamically
       *  based on shape parameters.
       */
      void init(const std::vector<double>& s, double u = -1);

      /** wrapper of the function */
      //double operator()(double) const;
      /** TF1 ROOT interface */
      //double operator()(double*, double*);
      /** set grid step for function calculation */
      void settimestride(double);
      /** set timeoffset */
      void setseedoffset(double);
      /** set initial time */
      void settimeseed(double);
      /** substruct toffset to tzero */
      void nextseed();

      /** fill vector with response function values */
      void fillvector(std::vector<double>&) const;
      /** fill vector with response function values and its derivative */
      void fillvector(std::vector<dd_t>&) const;
      /** fill vector with response function values */
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
