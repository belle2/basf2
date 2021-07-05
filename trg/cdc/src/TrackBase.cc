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
// Filename : TrackBase.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track object in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/TrackBase.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Fitter.h"
#include "trg/cdc/Relation.h"

using namespace std;

namespace Belle2 {

  TRGCDCTrackBase::TRGCDCTrackBase(const TRGCDCTrackBase& t)
    : _name("CopyOf" + t._name),
      _status(t._status),
      _charge(t._charge),
      _ts(0),
      _tsAll(t._tsAll),
      _nTs(t._nTs),
      _fitter(t._fitter),
      _fitted(t._fitted),
      m_trackID(t.m_trackID)
  {
    _ts = new vector<TCLink*>[_nTs];
    for (unsigned i = 0; i < _nTs; i++)
      _ts[i].assign(t._ts[i].begin(), t._ts[i].end());
  }

  TRGCDCTrackBase::TRGCDCTrackBase(const string& name, double charge)
    : _name(name),
      _status(0),
      _charge(charge),
      _ts(0),
      _nTs(TRGCDC::getTRGCDC()->nSuperLayers()),
      _fitter(0),
      _fitted(false),
      m_trackID(9999)
  {
    _ts = new vector<TCLink*>[_nTs];
  }

  TRGCDCTrackBase::~TRGCDCTrackBase()
  {
  }

  void
  TRGCDCTrackBase::dump(const string&, const string& pre) const
  {
//  bool detail = (cmd.find("detail") != string::npos);

    string tab = TRGDebug::tab() + pre;
    cout << tab << "Dump of " << name() << endl;
    tab += "    ";

    cout << tab << "status=" << status() << ":p=" << p() << ":x=" << x()
         << endl;
    cout << tab;
    for (unsigned i = 0; i < _nTs; i++) {
      cout << i << ":" << _ts[i].size();
      for (unsigned j = 0; j < _ts[i].size(); j++) {
        if (j == 0)
          cout << "(";
        else
          cout << ",";
        const TCLink& l = * _ts[i][j];
        cout << l.cell()->name();
      }
      if (_ts[i].size())
        cout << "),";
    }
//     if (detail) {

//     }
    cout << endl;
  }


  int
  TRGCDCTrackBase::fit(void)
  {
    if (_fitter) {
      return _fitter->fit(* this);
    } else {
      cout << "TRGCDCTrackBase !!! no fitter available" << endl;
      return -1;
    }
  }

  void
  TRGCDCTrackBase::append(TCLink* a)
  {
    _ts[a->cell()->superLayerId()].push_back(a);
    _tsAll.push_back(a);
  }

  void
  TRGCDCTrackBase::append(const vector<TCLink*>& links)
  {
    for (unsigned i = 0; i < links.size(); i++) {
      append(links[i]);
    }
  }

  const std::vector<TCLink*>&
  TRGCDCTrackBase::links(void) const
  {
    return _tsAll;
  }

  const std::vector<TCLink*>&
  TRGCDCTrackBase::links(unsigned layerId) const
  {
    return _ts[layerId];
  }

  int
  TRGCDCTrackBase::approach2D(TCLink&) const
  {
    cout << "TRGCDCTrackBase::approach2D !!! not implemented" << endl;
    return -1;
  }

  const TRGCDCRelation
  TRGCDCTrackBase::relation(void) const
  {

    TRGDebug::enterStage("MCInfo");

    map<unsigned, unsigned> relations;
    for (unsigned i = 0; i < _tsAll.size(); i++) {

      const TCCell& cell = * _tsAll[i]->cell();
      const TCCHit& hit = * cell.hit();
      const unsigned iMCParticle = hit.iMCParticle();

      map<unsigned, unsigned>::iterator it = relations.find(iMCParticle);
      if (it != relations.end())
        ++it->second;
      else
        relations[iMCParticle] = 1;

      if (TRGDebug::level()) {
        cout << TRGDebug::tab() << cell.name() << ",MCParticle="
             << iMCParticle << endl;
      }
    }

    if (TRGDebug::level()) {
      map<unsigned, unsigned>::const_iterator it = relations.begin();
      while (it != relations.end()) {
        cout << TRGDebug::tab()
             << it->first << ","
             << it->second << endl;
        ++it;
      }
    }

    TRGDebug::leaveStage("MCInfo");

    return TCRelation(* this, relations);
  }

  const TRGCDCRelation
  TRGCDCTrackBase::relation2D(void) const
  {

    TRGDebug::enterStage("MCInfo");

    map<unsigned, unsigned> relations;
    for (unsigned i = 0; i < _tsAll.size(); i++) {


      const TCCell& cell = * _tsAll[i]->cell();

      // Ignore stereo layers
      if (cell.superLayerId() % 2 == 1) continue;

      const TCCHit& hit = * cell.hit();
      const unsigned iMCParticle = hit.iMCParticle();

      map<unsigned, unsigned>::iterator it = relations.find(iMCParticle);
      if (it != relations.end())
        ++it->second;
      else
        relations[iMCParticle] = 1;

      if (TRGDebug::level()) {
        cout << TRGDebug::tab() << cell.name() << ",MCParticle="
             << iMCParticle << endl;
      }
    }

    if (TRGDebug::level()) {
      map<unsigned, unsigned>::const_iterator it = relations.begin();
      while (it != relations.end()) {
        cout << TRGDebug::tab()
             << it->first << ","
             << it->second << endl;
        ++it;
      }
    }

    TRGDebug::leaveStage("MCInfo");

    return TCRelation(* this, relations);
  }






  const TRGCDCRelation
  TRGCDCTrackBase::relation3D(void) const
  {

    TRGDebug::enterStage("MCInfo");

    map<unsigned, unsigned> relations;
    for (unsigned i = 0; i < _tsAll.size(); i++) {

      const TCCell& cell = * _tsAll[i]->cell();

      // Ignore axial layers
      if (cell.superLayerId() % 2 == 0) continue;

      const TCCHit& hit = * cell.hit();
      const unsigned iMCParticle = hit.iMCParticle();

      map<unsigned, unsigned>::iterator it = relations.find(iMCParticle);
      if (it != relations.end())
        ++it->second;
      else
        relations[iMCParticle] = 1;

      if (TRGDebug::level()) {
        cout << TRGDebug::tab() << cell.name() << ",MCParticle="
             << iMCParticle << endl;
      }
    }

    if (TRGDebug::level()) {
      map<unsigned, unsigned>::const_iterator it = relations.begin();
      while (it != relations.end()) {
        cout << TRGDebug::tab()
             << it->first << ","
             << it->second << endl;
        ++it;
      }
    }

    TRGDebug::leaveStage("MCInfo");

    return TCRelation(* this, relations);
  }

} // namespace Belle2
