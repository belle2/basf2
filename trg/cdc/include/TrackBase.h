/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to represent a track object in TRGCDC.
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackBase_FLAG_
#define TRGCDCTrackBase_FLAG_

#include <vector>
#include "CLHEP/Vector/LorentzVector.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTBase TRGCDCTrackBase
#endif

#define TRGCDCTrackBaseType            1
#define TRGCDCCircleType               2
#define TRGCDCLineType                 4
#define TRGCDCTrackType                8
#define TRGCDCSegmentType             16

namespace Belle2 {

  class TRGCDCLink;
  class TRGCDCFitter;
  class TRGCDCRelation;

/// A class to represent a track object in TRGCDC.
  class TRGCDCTrackBase {

  public:

    /// Copy constructor
    TRGCDCTrackBase(const TRGCDCTrackBase&);

    /// Constructor
    TRGCDCTrackBase(const std::string& name, double charge);

    /// Destructor
    virtual ~TRGCDCTrackBase();

    /// Assignment operator, deleted
    TRGCDCTrackBase& operator=(TRGCDCTrackBase&) = delete;

  public:

    /// returns name.
    std::string name(void) const;

    /// returns status.
    int status(void) const;

    /// returns charge.
    double charge(void) const;

    /// returns momentum vector.
    virtual const CLHEP::Hep3Vector& p(void) const;

    /// returns Pt.
    virtual double pt(void) const;

    /// returns position vector.
    virtual const CLHEP::Hep3Vector& x(void) const;

    /// dumps debug information.
    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const;

    /// returns a vector to track segments.
    const std::vector<TRGCDCLink*>& links(void) const;

    /// returns a vector to track segments.
    const std::vector<TRGCDCLink*>& links(unsigned layerId) const;

    /// returns true if fitted.
    bool fitted(void) const;

    /// set fit status
    void setFitted(bool fitted);

    /// set track ID.
    void setTrackID(int trackID);

    /// get track ID.
    int getTrackID();

    /// returns object type.
    virtual unsigned objectType(void) const;

    /// returns a pointer to a default fitter.
    const TRGCDCFitter* fitter(void) const;

    /// returns MC information. False will be returned if no MC
    /// info. available.
    const TRGCDCRelation relation(void) const;

    /// returns MC information for only axial layers.
    const TRGCDCRelation relation2D(void) const;

    /// returns MC information for only stereo layers.
    const TRGCDCRelation relation3D(void) const;

  public: // Modifiers

    /// sets and returns name.
    std::string name(const std::string& newName);

    /// appends a link.
    void append(TRGCDCLink*);

    /// appends links.
    void append(const std::vector<TRGCDCLink*>& links);

    /// sets and returns charge.
    double charge(double c);

    /// fits itself by a default fitter. Error was happened if return value is not zero.
    virtual int fit(void);

    /// sets a default fitter.
    const TRGCDCFitter* fitter(const TRGCDCFitter*);

  public: // Utility functions

    /// calculate closest approach. Error was happened if return value is not zero.
    virtual int approach2D(TRGCDCLink&) const;

  private:

    /// Name.
    std::string _name;

    /// Status.
    int _status;

    /// Charge.
    double _charge;

    /// Momentum.
    CLHEP::Hep3Vector _p;

    /// Position.
    CLHEP::Hep3Vector _x;

    /// Links for each super layer
    std::vector<TRGCDCLink*>* _ts;

    /// Links for all super layers
    std::vector<TRGCDCLink*> _tsAll;

    /// Size of _ts.
    const unsigned _nTs;

    /// Fitter.
    const TRGCDCFitter* _fitter;

    /// Fitting status.
    mutable bool _fitted;

    /// ID of tracks.
    int m_trackID;

    friend class TRGCDCFitter;
    friend class TRGCDCCircleFitter;
    friend class TRGCDCHelixFitter;
    friend class TRGCDCCircle;
    friend class TRGCDCTrack;
  };

//-----------------------------------------------------------------------------

  inline
  std::string
  TRGCDCTrackBase::name(void) const
  {
    return _name;
  }

  inline
  std::string
  TRGCDCTrackBase::name(const std::string& a)
  {
    return _name = a;
  }

  inline
  int
  TRGCDCTrackBase::status(void) const
  {
    return _status;
  }

  inline
  const CLHEP::Hep3Vector&
  TRGCDCTrackBase::p(void) const
  {
    return _p;
  }

  inline
  const CLHEP::Hep3Vector&
  TRGCDCTrackBase::x(void) const
  {
    return _x;
  }

  inline
  double
  TRGCDCTrackBase::pt(void) const
  {
    return _p.perp();
  }

  inline
  bool
  TRGCDCTrackBase::fitted(void) const
  {
    return _fitted;
  }

  inline
  void
  TRGCDCTrackBase::setFitted(bool fitted)
  {
    _fitted = fitted;
  }

  inline
  unsigned
  TRGCDCTrackBase::objectType(void) const
  {
    return TRGCDCTrackBaseType;
  }

  inline
  double
  TRGCDCTrackBase::charge(void) const
  {
    return _charge;
  }

  inline
  double
  TRGCDCTrackBase::charge(double a)
  {
    return _charge = a;
  }

  inline
  const TRGCDCFitter*
  TRGCDCTrackBase::fitter(void) const
  {
    return _fitter;
  }

  inline
  const TRGCDCFitter*
  TRGCDCTrackBase::fitter(const TRGCDCFitter* a)
  {
    _fitted = false;
    return _fitter = a;
  }

  inline
  void
  TRGCDCTrackBase::setTrackID(int trackID)
  {
    m_trackID = trackID;
  }

  inline
  int
  TRGCDCTrackBase::getTrackID()
  {
    return m_trackID;
  }

} // namespace Belle2

#endif /* TRGCDCTrackBase_FLAG_ */
