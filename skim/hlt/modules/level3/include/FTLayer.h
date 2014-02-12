/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FTLayer_H
#define FTLayer_H

#include <cmath>

#include "skim/hlt/modules/level3/FTSuperLayer.h"

namespace Belle2 {

  class FTWire;
  class FTLayer {
  public:
    //! constructor
    FTLayer(const double radius, const double stereoAngle,
            const double zf, const double zb, const double offset,
            const int layerID, const int localLayerID, const int nWire,
            const FTSuperLayer& super, const int smallCell, FTWire* first);

    //! destructor
    ~FTLayer() {};

  public: // Selectors
    //! returns local-layer ID
    int localLayerId(void) const;

    //! returns layer ID
    int layerId(void) const;

    //! returns number of wires
    int nWire(void) const;

    //! returns local max ID
    int localMaxId(void) const;

    //! returns tangent of slant angle
    double tanSlant(void) const;

    //! returns r form origin
    double r(void) const;

    //! returns z of forward end-plate
    double zf(void) const;

    //! returns z of backward end-plate
    double zb(void) const;

    //! returns offset of numbering(local ID)
    double offset(void) const;

    //! returns if the layer is small cell or not
    int smallCell(void) const;

    //! returns super-layer
    const FTSuperLayer& superLayer(void) const;

    //! returns limit of "d" for stereo layer
    double limit(void) const;

    //! returns z for "d" in r-phi plane
    double z(const double d) const;

    //! returns cell size
    double csize(void) const;

    FTWire* firstWire() const { return m_firstWire; }
  private: // private data members
    const double m_radius;
    const double m_tanSlant;
    const double m_zf;        // z of forward endplate
    const double m_zb;        // z of backward endplate
    const double m_offset;
    const int m_layerId;
    const int m_localLayerId;
    const int m_Nwire;
    const FTSuperLayer& m_superLayer;
    const int m_smallCell;
    FTWire* const m_firstWire;
  };

  //----------------------------------------------
#ifdef FTLayer_NO_INLINE
#define inline
#else
#undef inline
#define FTLayer_INLINE_DEFINE_HERE
#endif

#ifdef FTLayer_INLINE_DEFINE_HERE

  inline
  FTLayer::FTLayer(const double radius, const double stereoAngle,
                   const double zf, const double zb, const double offset,
                   const int layerID, const int localLayerID, const int nWire,
                   const FTSuperLayer& super, const int smallCell, FTWire* first)
    : m_radius(radius),
      m_tanSlant(0 == stereoAngle ? 0.0 : 1. / std::tan(stereoAngle)),
      m_zf(zf),
      m_zb(zb),
      m_offset(offset),
      m_layerId(layerID),
      m_localLayerId(localLayerID),
      m_Nwire(nWire),
      m_superLayer(super),
      m_smallCell(smallCell),
      m_firstWire(first)
  {
  }

  inline
  int
  FTLayer::layerId(void) const
  {
    return m_layerId;
  }

  inline
  int
  FTLayer::localLayerId(void) const
  {
    return m_localLayerId;
  }

  inline
  int
  FTLayer::nWire(void) const
  {
    return m_Nwire;
  }

  inline
  int
  FTLayer::localMaxId(void) const
  {
    return (m_Nwire - 1);
  }

  inline
  double
  FTLayer::tanSlant(void) const
  {
    return m_tanSlant;
  }


  inline
  double
  FTLayer::r(void) const
  {
    return m_radius;
  }

  inline
  double
  FTLayer::zf(void) const
  {
    return m_zf;
  }

  inline
  double
  FTLayer::zb(void) const
  {
    return m_zb;
  }

  inline
  double
  FTLayer::limit(void) const
  {
    return (m_zf - m_zb) / m_tanSlant;
  }

  inline
  double
  FTLayer::z(const double d) const
  {
    return m_zb + d * m_tanSlant;
  }

  inline
  double
  FTLayer::offset(void) const
  {
    return m_offset;
  }

  inline
  int
  FTLayer::smallCell(void) const
  {
    return m_smallCell;
  }

  inline
  const FTSuperLayer&
  FTLayer::superLayer(void) const
  {
    return m_superLayer;
  }

  inline
  double
  FTLayer::csize(void) const
  {
    return 2 * M_PI * m_radius / m_Nwire;
  }

#endif

#undef inline

}

#endif /* FTLayer_FLAG_ */

