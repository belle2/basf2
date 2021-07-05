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
// Filename : Relation.cc
// Section  : TrackBase
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent relations between TrackBase and MC track
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "framework/datastore/StoreArray.h"
#include "mdst/dataobjects/MCParticle.h"
#include "trg/cdc/Relation.h"

using namespace std;

namespace Belle2 {

  TRGCDCRelation::TRGCDCRelation(const TRGCDCTrackBase& track,
                                 const std::map<unsigned, unsigned>& relation)
    : _track(track),
      _relations(relation),
      _pairs(0)
  {
  }

  TRGCDCRelation::~TRGCDCRelation()
  {
    delete [] _pairs;
  }

  unsigned
  TRGCDCRelation::contributor(unsigned a) const
  {

    const unsigned n = _relations.size();

    if (n == 0) {
      return 99999;
    } else if (n == 1) {
      return _relations.begin()->first;
    }

    //...Preparation...
    if (_pairs == 0) {
      _pairs = (const pair<unsigned, unsigned>**)
               malloc(sizeof(pair<unsigned, unsigned>) * n);
      map<unsigned, unsigned>::const_iterator it = _relations.begin();
      unsigned i = 0;
      while (it != _relations.end()) {
        _pairs[i] =
          new const pair<unsigned, unsigned>(it->first, it->second);
        ++it;
        ++i;
      }

      //...Sorting...
      for (unsigned k = 0; k < n - 1; k++) {
        for (unsigned j = i + 1; j < n; j++) {
          if (_pairs[k]->second < _pairs[j]->second) {
            const pair<unsigned, unsigned>* tmp = _pairs[k];
            _pairs[k] = _pairs[j];
            _pairs[j] = tmp;
          }
        }
      }
    }

    return _pairs[a]->first;
  }

  const MCParticle&
  TRGCDCRelation::mcParticle(unsigned a) const
  {
    const unsigned id = contributor(a);
    StoreArray<MCParticle> mcParticles;
    const unsigned nMcParticles = mcParticles.getEntries();
    if (nMcParticles == 0) cout << "[Error] TRGCDCRelation::mcParticle() => There are no mc particles in MCParticle store array." <<
                                  endl;
    return * mcParticles[id];
  }

  float
  TRGCDCRelation::purity(unsigned a) const
  {

    //...Get target trkID...
    const unsigned trkID = contributor(a);

    //...Count # of hits...
    unsigned n = 0;
    unsigned na = 0;
    map<unsigned, unsigned>::const_iterator it = _relations.begin();
    while (it != _relations.end()) {
      n += it->second;
      if (it->first == trkID)
        na = it->second;
      ++it;
    }

    return float(na) / float(n);
  }

  float
  TRGCDCRelation::purity3D(unsigned trkID) const
  {

    //...Count # of hits...
    unsigned n = 0;
    unsigned na = 0;
    map<unsigned, unsigned>::const_iterator it = _relations.begin();
    while (it != _relations.end()) {
      n += it->second;
      if (it->first == trkID)
        na = it->second;
      ++it;
    }

    return float(na) / float(n);
  }

  float
  TRGCDCRelation::efficiency3D(unsigned trkID, std::map<unsigned, unsigned>& numTSsParticle) const
  {

    // # of found TSs/ # of true TSs
    unsigned nFoundTS = 0;
    unsigned nTrueTS = 0;

    // Find number of found TSs. One for each layer.
    for (unsigned iStereoSuperLayer = 0; iStereoSuperLayer < 4; iStereoSuperLayer++) {
      if (((this->track()).links(2 * iStereoSuperLayer + 1)).size() > 0) nFoundTS += 1;
    }

    // Find number of true TSs.
    map<unsigned, unsigned>::iterator itTSF = numTSsParticle.find(trkID);
    if (itTSF != numTSsParticle.end()) nTrueTS = itTSF->second;
    else {
      //cout<<"[Error] TRGCDCRelation::efficiency3D. No mc stereo TSs for track."<<endl;
      return -1;
    }

    //cout<<"[JB] Found TS: "<< nFoundTS <<" nTrue TS: " << nTrueTS << endl;

    return float(nFoundTS) / float(nTrueTS);

  }

  void
  TRGCDCRelation::dump(const std::string&,
                       const std::string& prefix) const
  {

    const unsigned n = nContributors();
    cout << prefix << "#contributions=" << n << endl;
    const string tab = prefix + "    ";
    for (unsigned i = 0; i < n; i++) {
      cout << tab << i << ":MCTrkID=" << contributor(i)
           << ",purity=" << purity(i) * 100 << "%"
           << " [PDG=" << mcParticle(i).getPDG() << "]" << endl;
    }
  }

} // namespace Belle2
