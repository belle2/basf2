/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Used with permission from ILC                                          *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * Adopted by: Torben Ferber (torben.ferber@desy.de) (TF)                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef MARLIN_USE_ROOT

#ifndef __ROOTTRACER_H
#define __ROOTTRACER_H

#include <iostream>
#include "analysis/OrcaKinFit/BaseTracer.h"

class BaseFitter;
class TFile;

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

//  Class RootTracer:
/// Class to produce text output during kinematic fits
/**
 *
 * Author: Benno List
 * Last update: $Date: 2010/05/25 13:23:56 $
 *          by: $Author: boehmej $
 *
 */

namespace Belle2 {

  namespace OrcaKinFit {
    class BaseFitter;



    class RootTracer: public BaseTracer {
    public:
      RootTracer(const char* filename = "trace.root", const char* option = "RECREATE");
      virtual ~RootTracer();

      /// Called at the start of a new fit (during initialization)
      virtual void initialize(BaseFitter& fitter);
      /// Called at the end of each step
      virtual void step(BaseFitter& fitter);
      /// Called at intermediate points during a step
      virtual void substep(BaseFitter& fitter,
                           int flag
                          );
      /// Called at the end of a fit
      virtual void finish(BaseFitter& fitter);

      void printFitObjects(BaseFitter& fitter);
      void printConstraints(BaseFitter& fitter);

    protected:
      void SetBranchAddresses();
      void CreateBranches();
      void CreateEventBranches(BaseFitter& fitter);
      void FillParameterValues(BaseFitter& fitter);

      TFile* file;
      TTree* tree;
      TTree* eventtree;

      int istep;
      int isubstep;

      Int_t eventnumber;
      Int_t stepnumber;
      Int_t substepnumber;
      Double_t chi2;

      enum {NPARMAX = 100};
      Double_t parvalue[NPARMAX];

    };

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // __ROOTTRACER_H

#endif // MARLIN_USE_ROOT
