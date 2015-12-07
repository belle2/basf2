#ifndef ECLCONFIGURATION_H_
#define ECLCONFIGURATION_H_
namespace Belle2 {

  class  EclConfiguration {
  public:
    static EclConfiguration& get()
    {
      static EclConfiguration instance;
      return instance;
    }
    bool background() const { return m_background; }
    void setBackground(bool val) { m_background = val; }

    static constexpr int        m_nch = 8736; // total number of electronic channels (crystals) in calorimeter
    static constexpr double    m_tick = 24.*12. / 508.; // == 72/127 digitization clock tick (in microseconds ???)
    static constexpr int       m_ntrg = 144; // number of trigger counts per ADC clock tick

    static constexpr int       m_nsmp = 31; // number of ADC measurements for signal fitting
    static constexpr double    m_tmin = -15; // lower range of the signal fitting region in ADC clocks

    static constexpr int         m_nl = 48; // length of samples signal in number of ADC clocks
    static constexpr int         m_ns = 32; // number of samples per ADC clock

    static constexpr int        m_ndt = 96; // number of points per ADC tick where signal fit procedure parameters are evaluated

  private:
    EclConfiguration() {};
    bool m_background;
  public:

    struct signalsample_t {
      void InitSample(const float*);
      void InitSample(const double*);
      double Accumulate(const double, const double, double*) const;

      double m_sumscale; // energy deposit in fitting window scale factor
      double m_ft[m_nl * m_ns];
    };

    struct adccounts_t {
      void AddHit(const double a, const double t0, const signalsample_t& q);
      double total; // total deposition (sum of m_s array)
      double c[m_nsmp]; // flash ADC measurements
    };

    struct fitparams_t {
      typedef int int_array_192x16_t[2 * m_ndt][16];
      typedef int int_array_24x16_t[m_ndt / 4][16];
      int_array_192x16_t f, f1, fg31, fg32, fg33;
      int_array_24x16_t fg41, fg43;
    };

    struct algoparams_t {
      typedef short int shortint_array_16_t[16];
      typedef unsigned char uchar_array_32_t[32];
      union {
        shortint_array_16_t id;
        uchar_array_32_t    ic;
      };
    };

  };
}

#endif
