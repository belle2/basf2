/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FCCrystal_H
#define FCCrystal_H

namespace Belle2 {

  //...Defs...
#define FCCrystalEHit 0
#define FCCrystalCheckedOrNotHit 1
#define FCCrystalNeighbor 79
#define FCCrystalNeighbor1 2
#define FCCrystalNeighbor2 4
#define FCCrystalNeighbor3 8
#define FCCrystalNeighbor6 64
#define FCCrystalAppendedNeighbor1 5
#define FCCrystalAppendedNeighbor2 3
#define FCCrystalAppendedNeighbor3 65
#define FCCrystalAppendedNeighbor6 9

  class FCCrystal {
  public:
    //! constructors
    FCCrystal(const int thetaId, const int phiId, FCCrystal* const, FCCrystal* const);

    FCCrystal();

    //! destructor
    ~FCCrystal() {}

  public: //Selectors
    //! returns energy
    double energy(void) const;

    //! returns theta id
    int thetaId(void) const;

    //! returns phi id
    int phiId(void) const;

    //! returns neighbor of + phiId side
    FCCrystal* neighborPlusPhi(void) const;

    //! returns neighbor of - phiId side
    FCCrystal* neighborMinusPhi(void) const;

    //! returns neighbor of + thetaId side
    FCCrystal* neighborPlusTheta(void) const;

    //! returns neighbor of - thetaId side
    FCCrystal* neighborMinusTheta(void) const;

    //! returns pedestal of low range
    double ped_low(void) const;

    //! returns pedestal of middle range
    double ped_mid(void) const;

    //! returns pedestal of high range
    double ped_high(void) const;

    //! returns electronics gain of low range
    double gain_low(void) const;

    //! returns electronics gain of middle range
    double gain_mid(void) const;

    //! returns electronics gain of high range
    double gain_high(void) const;

    //! returns cid_status
    double status(void) const;

    //! returns state
    unsigned state(void) const;

    //! returns state bit
    unsigned stateAND(const unsigned mask) const;

  public: // Modifires

    //! clear information
    void clear(void);

    //! sets counter energy
    double energy(double src);

    //! sets pedestal of low range
    double ped_low(double src);

    //! sets pedestal of middle range
    double ped_mid(double src);

    //! sets pedestal of high range
    double ped_high(double src);

    //! sets electronics gain of low range
    double gain_low(double src);

    //! sets electronics gain of middle range
    double gain_mid(double src);

    //! sets electronics gain of high range
    double gain_high(double src);

    //! set state
    double status(double status);

    //! set state
    unsigned state(const unsigned state);

    //! set state bit
    void stateOR(const unsigned mask);

    //! check state bit and append to the list of clusters
    void checkAndAppend(FTList<FCCrystal*>& hits, const unsigned& mask);

  private: //private member functions
    //! returns max phi id of this phi-ring
    int phiMax(void) const;

  private: //static data members
    static const int m_phiMaxFE[13];
    static const int m_phiMaxBE[10];
  private: //private data members
    const int m_thetaId;
    const int m_phiId;
    FCCrystal* const m_neighborPlus;
    FCCrystal* const m_neighborMinus;
    double m_ped_low;
    double m_ped_mid;
    double m_ped_high;
    double m_gain_low;
    double m_gain_mid;
    double m_gain_high;
    double m_energy;
    double m_status;
    unsigned int m_state;
  };

  //----------------------------------------------
#ifdef FCCrystal_NO_INLINE
#define inline
#else
#undef inline
#define FCCrystal_INLINE_DEFINE_HERE
#endif

#ifdef FCCrystal_INLINE_DEFINE_HERE

  inline
  FCCrystal::FCCrystal(const int thetaId, const int phiId,
                       FCCrystal* const plus, FCCrystal* const minus)
    : m_thetaId(thetaId),
      m_phiId(phiId),
      m_neighborPlus(plus),
      m_neighborMinus(minus),
      m_ped_low(0.),
      m_ped_mid(0.),
      m_ped_high(0.),
      m_gain_low(0.),
      m_gain_mid(0.),
      m_gain_high(0.),
      m_energy(0.),
      m_status(1.),
      m_state(FCCrystalCheckedOrNotHit)
  {
  }

  inline
  FCCrystal::FCCrystal(void)
    : m_thetaId(-999),
      m_phiId(-999),
      m_neighborPlus(NULL),
      m_neighborMinus(NULL),
      m_ped_low(0.),
      m_ped_mid(0.),
      m_ped_high(0.),
      m_gain_low(0.),
      m_gain_mid(0.),
      m_gain_high(0.),
      m_energy(0.),
      m_status(1.),
      m_state(FCCrystalCheckedOrNotHit)
  {
  }

  inline
  void
  FCCrystal::clear(void)
  {
    m_energy = 0.;
    m_state = 1;
  }

  inline
  double
  FCCrystal::energy(void) const
  {
    return m_energy;
  }

  inline
  double
  FCCrystal::energy(double src)
  {
    return m_energy = src;
  }

  inline
  int
  FCCrystal::thetaId(void) const
  {
    return m_thetaId;
  }

  inline
  int
  FCCrystal::phiId(void) const
  {
    return m_phiId;
  }

  inline
  int
  FCCrystal::phiMax(void) const
  {
    return (m_thetaId < 13) ? *(m_phiMaxFE + m_thetaId) :
           ((m_thetaId < 59) ? 143 : * (m_phiMaxBE + (m_thetaId - 59)));
  }

  inline
  FCCrystal*
  FCCrystal::neighborPlusPhi(void) const
  {
    return (m_phiId == phiMax()) ? (FCCrystal*)this - phiMax() : (FCCrystal*)this + 1;
  }

  inline
  FCCrystal*
  FCCrystal::neighborMinusPhi(void) const
  {
    return (m_phiId) ? (FCCrystal*)this - 1 : (FCCrystal*)this + phiMax();
  }

  inline
  FCCrystal*
  FCCrystal::neighborPlusTheta(void) const
  {
    return m_neighborPlus;
  }

  inline
  FCCrystal*
  FCCrystal::neighborMinusTheta(void) const
  {
    return m_neighborMinus;
  }

  inline
  double
  FCCrystal::ped_low(void) const
  {
    return m_ped_low;
  }

  inline
  double
  FCCrystal::ped_low(double src)
  {
    return m_ped_low = src;
  }

  inline
  double
  FCCrystal::ped_mid(void) const
  {
    return m_ped_mid;
  }

  inline
  double
  FCCrystal::ped_mid(double src)
  {
    return m_ped_mid = src;
  }

  inline
  double
  FCCrystal::ped_high(void) const
  {
    return m_ped_high;
  }

  inline
  double
  FCCrystal::ped_high(double src)
  {
    return m_ped_high = src;
  }

  inline
  double
  FCCrystal::gain_low(void) const
  {
    return m_gain_low;
  }

  inline
  double
  FCCrystal::gain_low(double src)
  {
    return m_gain_low = src;
  }

  inline
  double
  FCCrystal::gain_mid(void) const
  {
    return m_gain_mid;
  }

  inline
  double
  FCCrystal::gain_mid(double src)
  {
    return m_gain_mid = src;
  }

  inline
  double
  FCCrystal::gain_high(void) const
  {
    return m_gain_high;
  }

  inline
  double
  FCCrystal::gain_high(double src)
  {
    return m_gain_high = src;
  }

  inline
  double
  FCCrystal::status(void) const
  {
    return m_status;
  }

  inline
  double
  FCCrystal::status(double status)
  {
    return m_status = status;
  }

  inline
  unsigned
  FCCrystal::state(void) const
  {
    return m_state;
  }

  inline
  unsigned
  FCCrystal::state(const unsigned state)
  {
    return m_state = state;
  }

  inline
  unsigned
  FCCrystal::stateAND(const unsigned mask) const
  {
    return m_state & mask;
  }

  inline
  void
  FCCrystal::stateOR(const unsigned mask)
  {
    m_state |= mask;
  }

  inline
  void
  FCCrystal::checkAndAppend(FTList<FCCrystal*>& hits, const unsigned& mask)
  {
    if (m_state ^ FCCrystalCheckedOrNotHit) {
      if (!m_state) hits.append(this);
      m_state |= mask;
    }
  }

#endif

#undef inline

}

#endif /* FCCrystal_FLAG_ */

