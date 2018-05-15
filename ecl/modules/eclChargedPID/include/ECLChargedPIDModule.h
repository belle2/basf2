/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Cate MacQueen (UniMelb)                                  *
 * Contact: cmq.centaurus@gmail.com                                       *
 * Last Updated: May 2018                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <TTree.h>
#include <TFile.h>

// ECL
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

// FRAMEWORK
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// GEOMETRY
#include <ecl/geometry/ECLNeighbours.h>
#include <ecl/geometry/ECLGeometryPar.h>

namespace Belle2 {

  class ECLChargedPIDModule : public Module {

  public:

    ECLChargedPIDModule();
    virtual ~ECLChargedPIDModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:

    TFile* m_rootFilePtr;
    std::string m_rootFileName;
    bool m_writeToRoot;

    StoreArray<ECLShower> m_eclShowers;

    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    StoreArray<MCParticle> m_mcParticles;

    // N1 Hypo
    TTree* n1_tree;

    // variables
    int n1_iExperiment;
    int n1_iRun;
    int n1_iEvent;

    int n1_eclShowerMultip;
    std::vector<double>* n1_eclShowerEnergy;
    std::vector<double>* n1_eclShowerTheta;
    std::vector<double>* n1_eclShowerPhi;
    std::vector<double>* n1_eclShowerR;
    std::vector<int>* n1_eclShowerHypothesisId;
    std::vector<double>* n1_eclShowerAbsZernike40;
    std::vector<double>* n1_eclShowerAbsZernike51;

    int n1_mcMultip;
    std::vector<int>* n1_mcPdg;
    std::vector<int>* n1_mcMothPdg;
    std::vector<double>* n1_mcEnergy;
    std::vector<double>* n1_mcP;
    std::vector<double>* n1_mcTheta;
    std::vector<double>* n1_mcPhi;

    int n1_trkMultip;
    std::vector<int>* n1_trkPdg;
    std::vector<int>* n1_trkCharge;
    std::vector<double>* n1_trkP;
    std::vector<double>* n1_trkTheta;
    std::vector<double>* n1_trkPhi;
    std::vector<double>* n1_trkR;

    std::vector<double>* n1_eclEoP;

    // N2 Hypo
    TTree* n2_tree;

    // variables
    int n2_iExperiment;
    int n2_iRun;
    int n2_iEvent;

    int n2_eclShowerMultip;
    std::vector<double>* n2_eclShowerEnergy;
    std::vector<double>* n2_eclShowerTheta;
    std::vector<double>* n2_eclShowerPhi;
    std::vector<double>* n2_eclShowerR;
    std::vector<int>* n2_eclShowerHypothesisId;
    std::vector<double>* n2_eclShowerAbsZernike40;
    std::vector<double>* n2_eclShowerAbsZernike51;

    int n2_mcMultip;
    std::vector<int>* n2_mcPdg;
    std::vector<int>* n2_mcMothPdg;
    std::vector<double>* n2_mcEnergy;
    std::vector<double>* n2_mcP;
    std::vector<double>* n2_mcTheta;
    std::vector<double>* n2_mcPhi;

    int n2_trkMultip;
    std::vector<int>* n2_trkPdg;
    std::vector<int>* n2_trkCharge;
    std::vector<double>* n2_trkP;
    std::vector<double>* n2_trkTheta;
    std::vector<double>* n2_trkPhi;
    std::vector<double>* n2_trkR;

    std::vector<double>* n2_eclEoP;

  };
}

