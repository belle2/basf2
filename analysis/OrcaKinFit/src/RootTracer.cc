/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (ferber@physics.ubc.ca) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef MARLIN_USE_ROOT

#include "analysis/OrcaKinFit/RootTracer.h"
#include "analysis/OrcaKinFit/BaseFitter.h"
#include "analysis/OrcaKinFit/BaseFitObject.h"
#include "analysis/OrcaKinFit/BaseHardConstraint.h"
#include "analysis/OrcaKinFit/BaseSoftConstraint.h"
#include <framework/logging/Logger.h>
#undef NDEBUG
#include <cassert>
#include <cstring>
#include <iostream>
using namespace std;

namespace Belle2 {

  namespace OrcaKinFit {

    RootTracer::RootTracer(const char* filename, const char* option)
      : file(0), tree(0), eventtree(0),
        istep(0), isubstep(0),
        eventnumber(0), stepnumber(0), substepnumber(0), chi2(0)
    {
      file = new TFile(filename, option);
      tree = new TTree("trace", "Fit Tracing");
      CreateBranches();
      SetBranchAddresses();
      eventnumber = 0;
    }

    RootTracer::~RootTracer()
    {
      file->Write();
      file->Close();
    }


    void RootTracer::initialize(BaseFitter& fitter)
    {
      B2INFO("=============== Starting fit ======================\n");

      printFitObjects(fitter);
      printConstraints(fitter);

      ++eventnumber;
      istep = 1;
      isubstep = 0;


      TString name("event");
      TString title("Event ");
      name += eventnumber;
      title += eventnumber;
      eventtree = new TTree(name, title);
      CreateEventBranches(fitter);


      BaseTracer::initialize(fitter);
    }

    void RootTracer::step(BaseFitter& fitter)
    {
      isubstep = 1;
      B2INFO("--------------- Step " << istep << " --------------------\n");

      printFitObjects(fitter);
      printConstraints(fitter);

      stepnumber = istep;
      substepnumber = isubstep;
      chi2 = fitter.getChi2();
      B2INFO("chi2=" << chi2);
      FillParameterValues(fitter);

      tree->Fill();
      eventtree->Fill();

      ++istep;
      BaseTracer::step(fitter);
    }

    void RootTracer::substep(BaseFitter& fitter, int flag)
    {
      B2INFO("---- Substep " << istep << "." << isubstep << " ----\n");

      printFitObjects(fitter);
      printConstraints(fitter);

      ++isubstep;
      BaseTracer::substep(fitter, flag);
    }

    void RootTracer::finish(BaseFitter& fitter)
    {

      B2INFO("=============== Final result ======================\n");
      printFitObjects(fitter);
      printConstraints(fitter);

      B2INFO("=============== Finished fit ======================\n");


      BaseTracer::finish(fitter);
    }

    void RootTracer::printFitObjects(BaseFitter& fitter)
    {
      FitObjectContainer* fitobjects = fitter.getFitObjects();
      if (!fitobjects) return;
      B2INFO("Fit objects:\n");
      for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
        BaseFitObject* fo = *i;
        assert(fo);
        B2INFO(fo->getName() << ": " << *fo << ", chi2=" << fo->getChi2());
      }
    }
    void RootTracer::printConstraints(BaseFitter& fitter)
    {
      ConstraintContainer* constraints = fitter.getConstraints();
      if (!constraints) return;
      B2INFO("(Hard) Constraints:\n");
      for (ConstraintIterator i = constraints->begin(); i != constraints->end(); ++i) {
        BaseHardConstraint* c = *i;
        assert(c);
        B2INFO(i - constraints->begin() << " " << c->getName() << ": " << c->getValue() << "+-" << c->getError());
      }
      SoftConstraintContainer* softconstraints = fitter.getSoftConstraints();
      if (!softconstraints) return;
      B2INFO("Soft Constraints:\n");
      for (SoftConstraintIterator i = softconstraints->begin(); i != softconstraints->end(); ++i) {
        BaseSoftConstraint* c = *i;
        assert(c);
        B2INFO(i - softconstraints->begin() << " " << c->getName() << ": " << c->getValue() << "+-" << c->getError());
      }
    }

    void RootTracer::SetBranchAddresses()
    {
      tree->SetBranchAddress("event", &eventnumber);
      tree->SetBranchAddress("step", &stepnumber);
      tree->SetBranchAddress("substep", &substepnumber);

    }

    void RootTracer::CreateBranches()
    {
// create branch with addresses for own datamembers
      tree->Branch("event",      &eventnumber,     "event/I");
      tree->Branch("step",       &stepnumber,      "step/I");
      tree->Branch("substep",    &substepnumber,   "substep/I");
    }
    void RootTracer::CreateEventBranches(BaseFitter& fitter)
    {
// create branch with addresses for own datamembers
      eventtree->Branch("step",       &stepnumber,      "step/I");
      eventtree->SetBranchAddress("step", &stepnumber);
      eventtree->Branch("substep",    &substepnumber,   "substep/I");
      eventtree->SetBranchAddress("substep", &substepnumber);
      eventtree->Branch("chi2",    &chi2,   "chi2/D");
      eventtree->SetBranchAddress("chi2", &chi2);
      FitObjectContainer* fitobjects = fitter.getFitObjects();
      if (!fitobjects) return;
      B2INFO("RootTracer: Fit objects:\n");
      for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
        BaseFitObject* fo = *i;
        assert(fo);
        TString foname = fo->getName();
        for (int ilocal = 0; ilocal < fo->getNPar(); ilocal++) {
          int iglobal = fo->getGlobalParNum(ilocal);
          if (iglobal >= 0 && iglobal < NPARMAX) {
            TString parname = "Par";
            parname += iglobal;
            parname += "_";
            parname += fo->getParamName(ilocal);
            B2INFO("Parameter " << iglobal << " '" << parname << "'\n");
            eventtree->Branch(parname,    parvalue + iglobal,   parname + "/D");
            eventtree->SetBranchAddress(parname, parvalue + iglobal);

          }
        }
      }
      B2INFO("RootTracer: (Hard) Constraints:\n");
      ConstraintContainer* constraints = fitter.getConstraints();
      for (ConstraintIterator i = constraints->begin(); i != constraints->end(); ++i) {
        BaseHardConstraint* c = *i;
        assert(c);
        int iglobal = c->getGlobalNum();
        if (iglobal >= 0 && iglobal < NPARMAX) {
          TString cname = "Const";
          cname += (i - constraints->begin());
          cname += "_";
          cname += c->getName();
          eventtree->Branch(cname,    parvalue + iglobal,   cname + "/D");
          eventtree->SetBranchAddress(cname, parvalue + iglobal);
          B2INFO("Constraint " << (i - constraints->begin()) << " '" << cname);
        }
      }
//   B2INFO("RootTracer: Soft Constraints:\n");
//   SoftConstraintContainer* softconstraints = fitter.getSoftConstraints();
//   for (SoftConstraintIterator i = softconstraints->begin(); i != softconstraints->end(); ++i) {
//     BaseSoftConstraint *c = *i;
//     assert (c);
//     int iglobal = c->getGlobalNum();
//     if (iglobal >= 0 && iglobal < NPARMAX) {
//       TString cname = "Const";
// //      cname += iglobal;
//       cname += (i-softconstraints->begin());
//       cname += "_";
//       cname += c->getName();
//       eventtree->Branch(cname,    parvalue+iglobal,   cname+"/D");
//       eventtree->SetBranchAddress(cname,parvalue+iglobal);
// //     B2INFO("Constraint " << iglobal << " '" << cname);
//       B2INFO("Constraint " << (i-softconstraints->begin()) << " '" << cname);
//     }
//   }
    }

    void RootTracer::FillParameterValues(BaseFitter& fitter)
    {
      FitObjectContainer* fitobjects = fitter.getFitObjects();
      if (!fitobjects) return;
      for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
        BaseFitObject* fo = *i;
        assert(fo);
        B2INFO("fo " << fo->getName());
        for (int ilocal = 0; ilocal < fo->getNPar(); ilocal++) {
          int iglobal = fo->getGlobalParNum(ilocal);
          if (iglobal >= 0 && iglobal < NPARMAX) {
            parvalue[iglobal] =  fo->getParam(ilocal);
            B2INFO("Par " << ilocal << ": global " << iglobal << " = " << parvalue[iglobal]);
          }
        }
      }
      ConstraintContainer* constraints = fitter.getConstraints();
      for (ConstraintIterator i = constraints->begin(); i != constraints->end(); ++i) {
        BaseHardConstraint* c = *i;
        assert(c);
        int iglobal = c->getGlobalNum();
        if (iglobal >= 0 && iglobal < NPARMAX) {
          parvalue[iglobal] =  c->getValue();
          B2INFO("Const " << c->getName() << ": global " << iglobal << " = " << parvalue[iglobal]);
        }
      }
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // MARLIN_USE_ROOT
