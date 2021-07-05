/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Layer.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell layer.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCLayer_FLAG_
#define TRGCDCLayer_FLAG_

#include <string>
#include <vector>

#ifdef TRGCDC_SHORT_NAMES
#define TCLayer TRGCDCLayer
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCCell;

/// A class to represent a cell layer.
  class TRGCDCLayer : public std::vector<TRGCDCCell*> {

  public:

    /// Constructor.
    TRGCDCLayer(unsigned id,
                unsigned superLayerId,
                unsigned localLayerId,
                unsigned axialStereoLayerId,
                unsigned axialStereoSuperLayerId,
                float offset,
                int nShifts,
                float cellSize,
                unsigned nCells,
                float innerRadius,
                float outerRadius);

    /// Constructor for track segments.
    TRGCDCLayer(unsigned id, const TRGCDCCell& w);

    /// Destructor
    virtual ~TRGCDCLayer();

  public:// Selectors

    /// return name.
    const std::string& name(void) const;

    /// returns id.
    unsigned id(void) const;

    /// returns super layer id.
    unsigned superLayerId(void) const;

    /// returns local layer id in a super layer.
    unsigned localLayerId(void) const;

    /// returns id of axial or stereo id.
    unsigned axialStereoLayerId(void) const;

    /// returns id of axial or stereo super layer id.
    unsigned axialStereoSuperLayerId(void) const;

    /// returns \# of cells.
    unsigned nCells(void) const;

    /// returns a pointer to a cell. 'id' can be negative or 'id' can be greater than 'nCells()'.
    const TRGCDCCell& cell(int id) const;

    /// returns true if this is an axial layer.
    bool axial(void) const;

    /// returns true if this is a stereo layer.
    bool stereo(void) const;

    /// returns offset.
    float offset(void) const;

    /// returns shifts. (non-zero for stereo layers)
    int nShifts(void) const;

    /// returns cell size.
    float cellSize(void) const;

    /// sets and returns inner radius.
    float innerRadius(void) const;

    /// sets and returns outer radius.
    float outerRadius(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns "A" or "U" or "V" depending on stereo type.
    const std::string stereoType(void) const;

  private:

    /// Layer name.
    const std::string _name;

    /// ID in whole CDC.
    const unsigned _id;

    /// Super layer ID.
    const unsigned _superLayerId;

    /// ID in a super layer.
    const unsigned _localLayerId;

    /// ID in whole CDC counting only axial or stereo.
    const unsigned _axialStereoLayerId;

    /// Super layer ID counting only axial or stereo.
    const unsigned _axialStereoSuperLayerId;

    /// Cell position offset from X axis in cell unit.
    const float _offset;

    /// Stereo angle in cell unit.
    const int _nShifts;

    /// Cell size.
    const float _cellSize;

    /// \# of cells
    const unsigned _nCells;

    /// Inner radius.
    float _innerRadius;

    /// Outer radius.
    float _outerRadius;

    /// TRGCDC has complete access.
    friend class TRGCDC;
  };

//-----------------------------------------------------------------------------

  inline
  unsigned
  TRGCDCLayer::id(void) const
  {
    return _id;
  }

  inline
  unsigned
  TRGCDCLayer::superLayerId(void) const
  {
    return _superLayerId;
  }

  inline
  unsigned
  TRGCDCLayer::localLayerId(void) const
  {
    return _localLayerId;
  }

  inline
  float
  TRGCDCLayer::offset(void) const
  {
    return _offset;
  }

  inline
  int
  TRGCDCLayer::nShifts(void) const
  {
    return _nShifts;
  }

  inline
  unsigned
  TRGCDCLayer::nCells(void) const
  {
    return _nCells;
  }

  inline
  bool
  TRGCDCLayer::axial(void) const
  {
    if (! _nShifts) return true;
    return false;
  }

  inline
  bool
  TRGCDCLayer::stereo(void) const
  {
    if (_nShifts) return true;
    return false;
  }

  inline
  unsigned
  TRGCDCLayer::axialStereoLayerId(void) const
  {
    return _axialStereoLayerId;
  }

  inline
  unsigned
  TRGCDCLayer::axialStereoSuperLayerId(void) const
  {
    return _axialStereoSuperLayerId;
  }

  inline
  float
  TRGCDCLayer::cellSize(void) const
  {
    return _cellSize;
  }

  inline
  const std::string
  TRGCDCLayer::stereoType(void) const
  {
    if (axial())
      return "A";
    else if (nShifts() > 0)
      return "U";
    else
      return "V";
  }

  inline
  float
  TRGCDCLayer::innerRadius(void) const
  {
    return _innerRadius;
  }

  inline
  float
  TRGCDCLayer::outerRadius(void) const
  {
    return _outerRadius;
  }

  inline
  const std::string&
  TRGCDCLayer::name(void) const
  {
    return _name;
  }

} // namespace Belle2

#endif /* TRGCDCLayer_FLAG_ */
