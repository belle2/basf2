/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTWire_H
#define FTWire_H

//...Defs...
#define FTWireHit 1
#define FTWireHitInvalid 2
#define FTWireHitAppended 4
#define FTWireHitAppendedOrInvalid 6
#define FTWireNeighbor0 8
#define FTWireNeighbor1 16
#define FTWireNeighbor2 32
#define FTWireNeighbor3 64
#define FTWireNeighbor4 128
#define FTWireNeighbor5 256
#define FTWireHitSegment 16384
#define FTWireHitLinked 131072
#define FTWireHitRight 32768
#define FTWireHitLeft 65536

#define FTWireFittingInvalid 0x10000000
#define FTWireInvalid 0x20000000
#define FTWireStateMask 0xf0000000

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

#include "cdc/geometry/CDCGeometryPar.h"
//#include "mdst/dataobjects/MCParticle.h"
#include "skim/hlt/modules/level3/FTLayer.h"
#include "skim/hlt/modules/level3/FTSuperLayer.h"
#include "skim/hlt/modules/level3/FTList.h"
#include "skim/hlt/modules/level3/Lpav.h"
#include "TVectorD.h"

namespace Belle2 {

  class FTWire {
  public:
    //! constructors
    FTWire(const double x, const double y, const double dx, const double dy,
           const FTLayer& layer, const int localID);

    FTWire();
    //! destructor
    ~FTWire();

  public: //Selectors
    //! returns position x
    double x(void) const;

    //! returns position y
    double y(void) const;

    //! returns phi
    double phi(void) const;

    //! returns local ID
    int localId(void) const;

    //! returns layer
    const FTLayer& layer(void) const;

    //! returns pointer of neighbor array
    FTWire** neighborPtr(void);

    //! returns drift distance
    double distance(void) const;

    //! returns z_distance from the center of wire by drift distance
    double distance_z(void) const;

    //! returns t0 read at begin run
    double t0(void) const;

    //! returns pedestal read at begin run
    double pedestal(void) const;

    //! returns state
    unsigned state(void) const;

    //! returns state bit
    unsigned stateAND(const unsigned mask) const;

    //! returns z for track la
    int z(const L3::Lpav& la, double&   z) const;

    //! rerurns TDC time(after t0 subtraction)
    double time(void) const;

  public: // Modifires
    //! clear
    void clear(void);

    //! set drift distance
    double distance(const double distance);

    //! set t0 at begin-run
    double t0(const double t0);

    //! set pedestal at begin-run
    double pedestal(const double ped);

    //! set state
    unsigned state(const unsigned state);

    //! set state bit
    void stateOR(const unsigned mask);

    //! invert state bit
    void stateXOR(const unsigned mask);

    //! reset state bit
    void stateORXOR(const unsigned mask);

    //! check neighbors of phi-side and raise invalid flag if both hits
    void chkLeftRight(void);

    //! reset time
    double time(double t);

    //! set neighbours
    void set_neighbors(FTWire* const vt);

  public:
#ifdef FZISAN_DEBUG
    MCParticle* mcPart(MCParticle* src);
    MCParticle* mcPart(void) const;
#endif

  private: //private member functions
    //! returns left wire;
    const FTWire* left(void) const;

    //! returns left wire;
    const FTWire* right(void) const;

    //! returns innerleft wire;
    const FTWire* innerLeft(FTWire* const vtWire) const;

    //! returns inner-right wire;
    const FTWire* innerRight(FTWire* const vtWire) const;

    //! returns outer-left wire;
    const FTWire* outerLeft(FTWire* const vtWire) const;

    //! returns outer-right wire;
    const FTWire* outerRight(FTWire* const vtWire) const;

  private: //private data members
    const double m_x;
    const double m_y;
    const double m_dx;
    const double m_dy;
    const FTLayer& m_layer;
    const int m_localId;
#ifdef FZISAN_DEBUG
    MCParticle* m_mcPart;
#endif
    double m_distance;
    double m_t0;
    double m_time;
    double m_pedestal;
    unsigned int m_state;
    FTWire* m_neighbor[6];
  };

  //----------------------------------------------
#ifdef FTWire_NO_INLINE
#define inline
#else
#undef inline
#define FTWire_INLINE_DEFINE_HERE
#endif

#ifdef FTWire_INLINE_DEFINE_HERE

  inline
  FTWire::FTWire(const double x, const double y,
                 const double dx, const double dy,
                 const FTLayer& layer, const int localID)
    : m_x(x),     // x position in z = 0;
      m_y(y),     // y position in z = 0;
      m_dx(dx),
      m_dy(dy),
      m_layer(layer),
      m_localId(localID),
#ifdef FZISAN_DEBUG
      m_mcPart(NULL),
#endif
      m_distance(0),
      m_t0(0),
      m_time(0),
      m_pedestal(0),
      m_state(FTWireHitInvalid)
  {
    m_neighbor[0] = NULL;
    m_neighbor[1] = NULL;
    m_neighbor[2] = NULL;
    m_neighbor[3] = NULL;
    m_neighbor[4] = NULL;
    m_neighbor[5] = NULL;
  }

  inline
  FTWire::FTWire()
    : m_x(0),
      m_y(0),
      m_dx(0),
      m_dy(0),
      m_layer(*(FTLayer*)NULL),
      m_localId(0),
#ifdef FZISAN_DEBUG
      m_mcPart(NULL),
#endif
      m_distance(0),
      m_t0(0),
      m_time(0),
      m_pedestal(0),
      m_state(FTWireHitInvalid)
  {
    m_neighbor[0] = NULL;
    m_neighbor[1] = NULL;
    m_neighbor[2] = NULL;
    m_neighbor[3] = NULL;
    m_neighbor[4] = NULL;
    m_neighbor[5] = NULL;
  }

  inline
  FTWire::~FTWire()
  {
  }


  inline
  void
  FTWire::set_neighbors(FTWire* const vt)
  {
    m_neighbor[0] = (FTWire*) innerLeft(vt);
    m_neighbor[1] = (FTWire*) innerRight(vt);
    m_neighbor[2] = (FTWire*) left();
    m_neighbor[3] = (FTWire*) right();
    m_neighbor[4] = (FTWire*) outerLeft(vt);
    m_neighbor[5] = (FTWire*) outerRight(vt);
  }

  inline
  void
  FTWire::clear(void)
  {
    m_distance = 0.;
    m_time = 0.;
    m_t0 = 0.;
    m_pedestal = 0.;
    m_state = FTWireHitInvalid;
  }

  inline
  const FTLayer&
  FTWire::layer(void) const
  {
    return m_layer;
  }

  inline
  double
  FTWire::x(void) const
  {
    return m_x;
  }

  inline
  double
  FTWire::y(void) const
  {
    return m_y;
  }

  inline
  int
  FTWire::localId(void) const
  {
    return m_localId;
  }

  inline
  FTWire**
  FTWire::neighborPtr(void)
  {
    return m_neighbor;
  }

  inline
  double
  FTWire::phi(void) const
  {
    return 2.*M_PI * (m_layer.offset() + (double)m_localId) / (double)m_layer.nWire();
  }

  inline
  double
  FTWire::distance(void) const
  {
    return m_distance;
  }

  inline
  double
  FTWire::distance_z(void) const
  {
    return m_distance * std::fabs(m_layer.tanSlant());
  }

  inline
  double
  FTWire::distance(const double distance)
  {
    return m_distance = distance;
  }

  inline
  double
  FTWire::t0(void) const
  {
    return m_t0;
  }

  inline
  double
  FTWire::t0(const double t0)
  {
    return m_t0 = t0;
  }

  inline
  double
  FTWire::pedestal(void) const
  {
    return m_pedestal;
  }

  inline
  double
  FTWire::pedestal(const double ped)
  {
    return m_pedestal = ped;
  }

  inline
  double
  FTWire::time(double t)
  {
    return m_time = t;
  }

  inline
  double
  FTWire::time(void) const
  {
    return m_time;
  }

#ifdef FZISAN_DEBUG
  inline
  MCParticle*
  FTWire::mcPart(MCParticle* src)
  {
    return m_mcPart = src;
  }

  inline
  MCParticle*
  FTWire::mcPart(void) const
  {
    return m_mcPart;
  }
#endif

  inline
  unsigned
  FTWire::state(void) const
  {
    return m_state;
  }

  inline
  unsigned
  FTWire::state(const unsigned state)
  {
    return m_state = (m_state & FTWireStateMask) | state;
  }

  inline
  unsigned
  FTWire::stateAND(const unsigned mask) const
  {
    return m_state & mask;
  }

  inline
  void
  FTWire::stateOR(const unsigned mask)
  {
    m_state |= mask;
  }

  inline
  void
  FTWire::stateXOR(const unsigned mask)
  {
    m_state ^= mask;
  }

  inline
  void
  FTWire::stateORXOR(const unsigned mask)
  {
    m_state = (m_state | mask)^mask;
  }

  inline
  int
  FTWire::z(const L3::Lpav& la, double&   z) const
  {
    double rho = la.radius();
    double dx2 = la.xc() - m_x;
    double dy2 = la.yc() - m_y;
    double par1 = m_dx * m_dx + m_dy * m_dy;
    double par2 = (m_dx * dx2 + m_dy * dy2) / par1;
    double par3 = m_dx * dy2 - m_dy * dx2;
    double par4 = rho * rho * par1 -  par3 * par3;
    if (par4 < 0.) return 0;
    par4 = std::sqrt(par4) / par1;
    double delta = par2 + par4;
    if (delta >= 0. && delta < 1.) {
      z = m_layer.zb() + delta * (m_layer.zf() - m_layer.zb());
      return 1;
    } else {
      delta = par2 - par4;
      if (delta >= 0. && delta < 1.) {
        z = m_layer.zb() + delta * (m_layer.zf() - m_layer.zb());
        return 1;
      }
    }
    return 0;
  }

  inline
  void
  FTWire::chkLeftRight(void)
  {
    if (((**(m_neighbor + 2)).m_state & FTWireHit) &&
        ((**(m_neighbor + 3)).m_state & FTWireHit)) {
      m_state |= FTWireHitInvalid;
      (**(m_neighbor + 2)).m_state |= FTWireHitInvalid;
      (**(m_neighbor + 3)).m_state |= FTWireHitInvalid;
    }
  }

  inline
  const FTWire*
  FTWire::innerLeft(FTWire* const vtWire) const
  {
    if (!m_layer.localLayerId()) return vtWire;
    // do not relate cells between the normal ones and small ones
    //if (m_layer.smallCell() ^ (&m_layer-1)->smallCell()) return vtWire;
    const double diff = m_layer.offset() - (&m_layer - 1)->offset() - m_layer.nWire() - 0.5;
    int i_diff = (int)diff;
    if (m_localId + i_diff < -(double)m_layer.nWire()) i_diff += m_layer.nWire();
    return this + i_diff;
  }

  inline
  const FTWire*
  FTWire::innerRight(FTWire* const vtWire) const
  {
    if (!m_layer.localLayerId()) return vtWire;
    // do not relate cells between the normal ones and small ones
    //if (m_layer.smallCell() ^ (&m_layer-1)->smallCell()) return vtWire;
    const double diff = m_layer.offset() - (&m_layer - 1)->offset() - m_layer.nWire() + 0.5;
    int i_diff = (int)diff;
    if (m_localId + diff >= 0.) i_diff -= m_layer.nWire();
    return this + i_diff;
  }

  inline
  const FTWire*
  FTWire::left(void) const
  {
    const FTWire* tmp = this;
    if (!m_localId) tmp += m_layer.nWire();
    return --tmp;
  }

  inline
  const FTWire*
  FTWire::right(void) const
  {
    const FTWire* tmp = this;
    if (m_localId == m_layer.localMaxId()) {
      tmp -= m_layer.nWire();
    }
    return tmp + 1;
  }

  inline
  const FTWire*
  FTWire::outerLeft(FTWire* const vtWire) const
  {
    if (m_layer.localLayerId() == m_layer.superLayer().layerMaxId()) return vtWire;
    // do not relate between the nomal cells and small cells
    //if (m_layer.smallCell() ^ (&m_layer+1)->smallCell()) return vtWire;
    const double diff = m_layer.offset() - (&m_layer + 1)->offset() + m_layer.nWire() - 0.5;
    int i_diff = (int)diff;
    if (m_localId + diff < (double)m_layer.nWire()) i_diff += m_layer.nWire();
    return this + i_diff;
  }

  inline
  const FTWire*
  FTWire::outerRight(FTWire* const vtWire) const
  {
    if (m_layer.localLayerId() == m_layer.superLayer().layerMaxId()) return vtWire;
    // do not relate between the nomal cells and small cells
    //if (m_layer.smallCell() ^ (&m_layer+1)->smallCell()) return vtWire;
    const double diff = m_layer.offset() - (&m_layer + 1)->offset() + m_layer.nWire() + 0.5;
    int i_diff = (int)diff;
    if (m_localId + diff >= 2.*(double)m_layer.nWire()) i_diff -= m_layer.nWire();
    return this + i_diff;
  }


#endif

#undef inline

}

#endif /* FTWire_FLAG_ */
