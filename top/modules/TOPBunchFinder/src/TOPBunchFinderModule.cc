/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager
#include <framework/core/ModuleManager.h>

// Own include
#include <top/modules/TOPBunchFinder/TOPBunchFinderModule.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPBunchFinder)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPBunchFinderModule::TOPBunchFinderModule() : Module(),
    m_bunchTimeSep(0)

  {
    // set module description (e.g. insert text)
    setDescription("Bunch finder");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("bunchHalfRange", m_bunchHalfRange,
             "Half range of bunch numbers to be searched for", 5);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use default one)", 0.0);

  }

  TOPBunchFinderModule::~TOPBunchFinderModule()
  {
  }

  void TOPBunchFinderModule::initialize()
  {
    // input

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<DedxLikelihood> dedxLikelihoods;
    dedxLikelihoods.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();


    // output


    // Configure TOP detector

    TOPconfigure config;

    // bunch separation in time

    m_bunchTimeSep = 2.0; // TODO: get it from DB

    // other

    if (m_maxTime <= 0) m_maxTime = config.getTDCTimeRange();

    int n = 2 * m_bunchHalfRange + 1;
    for (int i = 0; i < n; i++) {
      m_xxx.push_back(0);
      for (int k = 0; k < 20; k++) m_yyy[k].push_back(0);
    }
  }

  void TOPBunchFinderModule::beginRun()
  {
  }

  void TOPBunchFinderModule::event()
  {

    int Nhyp = 1;
    double mass = Const::pion.getMass();
    TOPreco reco(Nhyp, &mass);
    reco.setPDFoption(TOPreco::c_Rough);
    reco.setTmax(m_maxTime + m_bunchTimeSep * m_bunchHalfRange);

    StoreArray<TOPDigit> topDigits;
    for (const auto & digit : topDigits) {
      if (digit.getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit.getBarID(), digit.getChannelID(), digit.getTDC());
    }

    int n = 2 * m_bunchHalfRange + 1;
    double t0[n];
    double logL[n];
    for (int i = 0; i < n; i++) {
      t0[i] = (i - m_bunchHalfRange) * m_bunchTimeSep;
      logL[i] = 0;
    }
    int numTrk = 0;

    StoreArray<Track> tracks;
    for (const auto & track : tracks) {
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      auto dedx = track.getRelated<DedxLikelihood>();
      if (dedx) {
        mass = getMostProbableMass(dedx);
      } else {
        mass = Const::pion.getMass();
        B2WARNING("TOPBunchFinder: no relation to DedxLikelihood - "
                  "pion mass used instead");
      }

      reco.setMass(mass);
      reco.reconstruct(trk);
      if (reco.getFlag() != 1) continue;
      numTrk++;

      //      cout<<trk.getP()<<"("<<trk.getHypID()<<") "<<mass<<" ";
      for (int i = 0; i < n; i++) {
        logL[i] += reco.getLogL(t0[i], m_maxTime);
      }
    }
    //    cout<<endl;

    int i0 = m_bunchHalfRange;
    for (int i = 0; i < n; i++) {
      if (logL[i] > logL[i0]) i0 = i;
    }

    m_xxx[i0]++;
    int k = numTrk < 20 ? numTrk : 19;
    m_yyy[k][i0]++;

    //    cout << numTrk <<" "<<i0<<" "<<t0[i0]<<endl;
    //    for (int i = 0; i < n; i++) cout << logL[i] - logL[i0]<<" ";
    //    cout<<endl;

  }


  void TOPBunchFinderModule::endRun()
  {
  }

  void TOPBunchFinderModule::terminate()
  {
    cout << "found bunches: " << endl;
    for (unsigned i = 0; i < m_xxx.size(); i++) cout << m_xxx[i] << " ";
    cout << endl;
    cout << endl;
    for (int k = 0; k < 20; k++) {
      int n = 0;
      cout << k << "  ";
      for (unsigned i = 0; i < m_yyy[k].size(); i++) {
        cout << m_yyy[k][i] << " ";
        n += m_yyy[k][i];
      }
      cout << " " << n << " ";
      if (n > 0) cout << float(m_yyy[k][5]) / float(n);
      cout << endl;
    }

  }


  double TOPBunchFinderModule::getMostProbableMass(const DedxLikelihood* dedx) const
  {
    if (!dedx) return 0;

    std::vector<double> logL;
    std::vector<double> mass;
    //    for(auto type: Const::chargedStableSet) {}
    for (auto type = Const::chargedStableSet.begin();
         type != Const::chargedStableSet.end(); ++type) {
      logL.push_back(dedx->getSVDLogLikelihood(type) + dedx->getCDCLogLikelihood(type));
      mass.push_back(type.getMass());
    }
    unsigned i0 = Const::pion.getIndex();
    for (unsigned i = 0; i < logL.size(); ++i) {
      if (logL[i] > logL[i0]) i0 = i;
    }
    return mass[i0];

  }


} // end Belle2 namespace

