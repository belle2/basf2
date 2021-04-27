/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *               Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <b2bii/modules/B2BIIMdstInput/B2BIIConvertBeamParamsModule.h>
#include <belle_legacy/benergy/BeamEnergy.h>
#include <belle_legacy/ip/IpProfile.h>
#include <belle_legacy/panther/panther.h>
#include <belle_legacy/tables/belletdf.h>

#include <framework/database/Database.h>
#include <framework/database/EventDependency.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/dbobjects/BeamParameters.h>
#include <framework/utilities/FileSystem.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>

#include <list>

namespace Belle {
  /** This is needed by some utility function from belle so let's provide a
   * dummy implementation.
   */
  int check_beginrun(bool) { return 0; }
}

namespace {
  /** Convert CLHEP point to ROOT TVector3 */
  TVector3 CLHEPtoROOT(const HepPoint3D& point)
  {
    return TVector3(point.x(), point.y(), point.z());
  }
  /** Convert CLHP SymMatrix to ROOT TMatrixDSym */
  TMatrixDSym CLHEPtoROOT(const HepSymMatrix& matrix)
  {
    TMatrixDSym result(matrix.num_col());
    for (int i = 0; i < matrix.num_row(); ++i) {
      for (int j = 0; j < matrix.num_col(); ++j) {
        result(i, j) = matrix(i + 1, j + 1);
      }
    }
    return result;
  }
}

namespace Belle2 {
  /** Register the module */
  REG_MODULE(B2BIIConvertBeamParams);

  B2BIIConvertBeamParamsModule::B2BIIConvertBeamParamsModule()
  {
    addParam("mcFlag", m_mcFlag, "which mc flag to use", m_mcFlag);
    addParam("missingBenergy", m_missingBenergy, "where to store information about runs with missing database info", m_missingBenergy);
    addParam("missingIp", m_missingIp, "where to store information about runs with missing IP profile info", m_missingIp);
  }

  void B2BIIConvertBeamParamsModule::initialize()
  {
    BsInit(0);
    m_event.isRequired();
  }

  void B2BIIConvertBeamParamsModule::beginRun()
  {
    // Fudge a runhead record with all the info we need to be able to get the
    // Beamenergy/IpProfile from the database
    BsClrTab(BBS_CLEAR);
    Belle::Belle_runhead_Manager& runMgr = Belle::Belle_runhead_Manager::get_manager();
    runMgr.add();
    Belle::Belle_runhead& runhead = runMgr[0];
    runhead.ExpMC(m_mcFlag);
    runhead.ExpNo(m_event->getExperiment());
    runhead.RunNo(m_event->getRun());
    B2INFO("Obtaining values for exp " << m_event->getExperiment() << ", run " << m_event->getRun());
    // and then get the values from the database
    Belle::BeamEnergy::flag_database(0);
    Belle::IpProfile::begin_run();
    Belle::Ip_profile_general_Manager& ipMgr = Belle::Ip_profile_general_Manager::get_manager();

    if (!Belle::BeamEnergy::usable()) {
      FileSystem::Lock lock(m_missingBenergy);
      if (!lock.lock()) {
        B2ERROR("No BeamEnergy for exp " << m_event->getExperiment() << ", run " << m_event->getRun());
      } else {
        std::ofstream file(m_missingBenergy.c_str(), std::ios::app);
        file << m_event->getExperiment() << "," << m_event->getRun() << std::endl;
      }
      return;
    }

    const double Eher = Belle::BeamEnergy::E_HER();
    const double Eler = Belle::BeamEnergy::E_LER();
    const double crossingAngle = Belle::BeamEnergy::Cross_angle();
    const double angleLer = M_PI; //parallel to negative z axis (different from Belle II!)
    const double angleHer = crossingAngle; //in positive z and x direction, verified to be consistent with Upsilon(4S) momentum

    // Beam energy spread taken from the hard-coded values in the Belle evtgen:
    // /sw/belle/belle/b20090127_0910/src/util/evtgenutil/basf_if/evtgen.cc
    // The beam energy spread seems not have been logged in the belle DB at all.
    // If you are reading this and you know how to access the run-dependent beam energy spread,
    // please fix this (April 2021)!
    std::vector<double> covarianceHer = {0.00513 * 0.00513}; // Energy spread only. No idea about the direction spread
    std::vector<double> covarianceLer = {0.002375 * 0.002375}; // Energy spread only. No idea about the direction spread
    std::vector<double> covariance; //this is used later on

    IntervalOfValidity iov(m_event->getExperiment(), m_event->getRun(), m_event->getExperiment(), m_event->getRun());

    BeamParameters beamParams;
    beamParams.setLER(Eler, angleLer, covarianceHer);
    beamParams.setHER(Eher, angleHer, covarianceLer);

    CollisionBoostVector collisionBoostVector;
    CollisionInvariantMass collisionInvM;
    TLorentzVector cms = beamParams.getLER() + beamParams.getHER();
    collisionBoostVector.setBoost(cms.BoostVector(), TMatrixTSym<double>(3));
    //note: maybe we could use Belle::BeamEnergy::E_beam_corr(), Belle::BeamEnergy::E_beam_err()
    collisionInvM.setMass(cms.M(), 0.0 , 0.0);

    // Boost vector and invariant mass are not intra-run dependent, store now
    Database::Instance().storeData("CollisionBoostVector", &collisionBoostVector, iov);
    Database::Instance().storeData("CollisionInvariantMass", &collisionInvM, iov);

    // and now we continue with the vertex
    if (!Belle::IpProfile::usable()) {
      FileSystem::Lock lock(m_missingIp);
      if (!lock.lock()) {
        B2ERROR("No IpProfile for exp " << m_event->getExperiment() << ", run " << m_event->getRun());
      } else {
        std::ofstream file(m_missingIp.c_str(), std::ios::app);
        file << m_event->getExperiment() << "," << m_event->getRun() << std::endl;
      }
      beamParams.setVertex(TVector3(
                             std::numeric_limits<double>::quiet_NaN(),
                             std::numeric_limits<double>::quiet_NaN(),
                             std::numeric_limits<double>::quiet_NaN()), covariance);
      Database::Instance().storeData("BeamParameters", &beamParams, iov);

      BeamSpot beamSpot;
      beamSpot.setIP(
        TVector3(std::numeric_limits<double>::quiet_NaN(),
                 std::numeric_limits<double>::quiet_NaN(),
                 std::numeric_limits<double>::quiet_NaN()
                ), TMatrixTSym<double>(3)
      );
      Database::Instance().storeData("BeamSpot", &beamSpot, iov);
      B2INFO("No IpProfile, created BeamEnergy Payload");
      return;
    }

    int nbins = std::max(ipMgr.count() - 1, 1);
    B2INFO("exp " << m_event->getExperiment() << ", run " << m_event->getRun() << ": " << nbins << " IpProfile bins");

    // need to keep the objects alive until we created the payloads
    std::list<BeamParameters> beamparamsList;
    std::list<BeamSpot> beamspotList;
    // and we want them to be intra-run dependent
    std::unique_ptr<EventDependency> beamparams;
    std::unique_ptr<EventDependency> beamspots;

    Belle::Belle_event_Manager& evtMgr = Belle::Belle_event_Manager::get_manager();
    if (!evtMgr.count()) evtMgr.add();
    for (int i = 0; i < nbins; ++i) {
      int evtNr = i * BIN_EVENTS;
      // fudge belle event record to get the correct run dependent ip profile
      Belle::Belle_event& evt = evtMgr[0];
      evt.ExpMC(m_mcFlag);
      evt.ExpNo(m_event->getExperiment());
      evt.RunNo(m_event->getRun());
      evt.EvtNo(evtNr);
      Belle::IpProfile::set_evtbin_number();
      B2ASSERT("something wrong: " << Belle::IpProfile::EvtBinNo() << "!=" << i, Belle::IpProfile::EvtBinNo() == i);

      // and convert
      HepPoint3D ip = Belle::IpProfile::e_position();
      HepSymMatrix ipErr = Belle::IpProfile::e_position_err();

      beamParams.setVertex(CLHEPtoROOT(ip));
      beamParams.setCovVertex(CLHEPtoROOT(ipErr));
      beamparamsList.emplace_back(beamParams);

      BeamSpot beamSpot;
      beamSpot.setIP(CLHEPtoROOT(ip), CLHEPtoROOT(ipErr));
      beamspotList.emplace_back(beamSpot);

      if (!beamparams) {
        beamparams = std::make_unique<EventDependency>(&beamparamsList.back(), false);
        beamspots = std::make_unique<EventDependency>(&beamspotList.back(), false);
      } else {
        beamparams->add(evtNr, &beamparamsList.back());
        beamspots->add(evtNr, &beamspotList.back());
      }
    }
    if (beamparamsList.size() < 1) {
      B2ERROR("Something is wrong with exp " << m_event->getExperiment() << ", run " << m_event->getRun() << ": no bins found");
      return;
    }
    if (beamparamsList.size() == 1) {
      // just one bin? no need to store event dependency
      Database::Instance().storeData("BeamParameters", &beamparamsList.back(), iov);
      Database::Instance().storeData("BeamSpot", &beamspotList.back(), iov);
      B2INFO("Created event independent payload");
    } else {
      // otherwise store full information
      Database::Instance().storeData("BeamParameters", beamparams.get(), iov);
      Database::Instance().storeData("BeamSpot", beamspots.get(), iov);
      B2INFO("Created event dependent payload with " << beamparamsList.size() << " entries");
    }
  }
} //Belle2 namespace
