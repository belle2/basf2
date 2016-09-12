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

  //! CsI crystal class for the Level-3 Fast Cluster Finder
  class FCCrystal {

  public:
    //! definition of state bits and masks
    enum FCCrystalState {
      EHit = 0x0,
      CheckedOrNotHit = 0x1,
      Neighbor = 0x4F,
      Neighbor1 = 0x2,
      Neighbor2 = 0x4,
      Neighbor3 = 0x8,
      Neighbor6 = 0x40,
      AppendedNeighbor1 = 0x5,
      AppendedNeighbor2 = 0x3,
      AppendedNeighbor3 = 0x41,
      AppendedNeighbor6 = 0x9
    };

  public:
    //! constructor
    FCCrystal(const int thetaId, const int phiId, FCCrystal* const, FCCrystal* const);

    //! constructor
    FCCrystal();

    //! destructor
    ~FCCrystal() {}

  public:
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

    //! returns cid_status
    //double status(void) const;

    //! returns state
    unsigned state(void) const;

    //! returns state bit
    unsigned stateAND(const unsigned mask) const;

  public:
    //! clear information
    void clear(void);

    //! sets counter energy
    double energy(double src);

    //! set status
    //double status(double status);

    //! set state
    unsigned state(const unsigned state);

    //! set state bit
    void stateOR(const unsigned mask);

    //! check state bit and append to the list of clusters
    void checkAndAppend(FTList<FCCrystal*>& hits, const unsigned& mask);

  private: //private member functions
    //! returns max phi id of this phi-ring
    int phiMax(void) const;

  private:
    //! maximum phi ID for FE
    static const int m_phiMaxFE[13];

    //! maximum phi ID for BE
    static const int m_phiMaxBE[10];

  private:
    //! theta ID
    const int m_thetaId;

    //! phi ID
    const int m_phiId;

    //! pointer of neighbor channel
    FCCrystal* const m_neighborPlus;

    //! pointer of neighbor channel
    FCCrystal* const m_neighborMinus;

    //! energy deposit
    double m_energy;

    //! state bits
    unsigned int m_state;
  };

  inline
  FCCrystal::FCCrystal(const int thetaId, const int phiId,
                       FCCrystal* const plus, FCCrystal* const minus)
    : m_thetaId(thetaId),
      m_phiId(phiId),
      m_neighborPlus(plus),
      m_neighborMinus(minus),
      m_energy(0.),
      //m_status(1.),
      m_state(CheckedOrNotHit)
  {
  }

  inline
  FCCrystal::FCCrystal(void)
    : m_thetaId(-999),
      m_phiId(-999),
      m_neighborPlus(NULL),
      m_neighborMinus(NULL),
      m_energy(0.),
      //m_status(1.),
      m_state(CheckedOrNotHit)
  {
  }

  inline
  void
  FCCrystal::clear(void)
  {
    m_energy = 0.;
    m_state = CheckedOrNotHit;
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
    const int max(phiMax());
    return (m_phiId == max)
           ? const_cast<FCCrystal*>(this) + (-max)
           : const_cast<FCCrystal*>(this) + 1;
  }

  inline
  FCCrystal*
  FCCrystal::neighborMinusPhi(void) const
  {
    return (m_phiId)
           ? const_cast<FCCrystal*>(this) - 1
           : const_cast<FCCrystal*>(this) + phiMax();
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
  /*
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
  */
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
    if (m_state ^ CheckedOrNotHit) {
      if (!m_state) hits.append(this);
      m_state |= mask;
    }
  }
}

#endif /* FCCrystal_FLAG_ */

