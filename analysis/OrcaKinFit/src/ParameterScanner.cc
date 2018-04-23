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

#include "analysis/OrcaKinFit/ParameterScanner.h"

#include "analysis/OrcaKinFit/BaseFitter.h"
#include "analysis/OrcaKinFit/NewFitterGSL.h"
#include "analysis/OrcaKinFit/BaseFitObject.h"
#include "analysis/OrcaKinFit/BaseHardConstraint.h"
#include <framework/logging/Logger.h>

#include <TString.h>
#include <TH2F.h>
#include <TMultiGraph.h>
#include <TGraph.h>

#include <gsl/gsl_vector.h>

#undef NDEBUG
#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;

namespace Belle2 {
  namespace OrcaKinFit {

    ParameterScanner::ParameterScanner(BaseFitter& fitter_)
      : fitter(fitter_)
    {}

    void ParameterScanner::doScan(int xglobal,
                                  int nx,
                                  double xstart,
                                  double xstop,
                                  int yglobal,
                                  int ny,
                                  double ystart,
                                  double ystop,
                                  const char* idprefix,
                                  const char* titleprefix,
                                  double mumerit)
    {

      NewFitterGSL* newfitter = dynamic_cast<NewFitterGSL*>(&fitter);

      FitObjectContainer* fitobjects = fitter.getFitObjects();
      if (fitobjects == 0) return;
      ConstraintContainer* constraints = fitter.getConstraints();
      if (constraints == 0) return;

      // Get largest global parameter number,
      // find parameter names
      TString xname("");
      TString yname("");

      int idim = 1;
      for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
        BaseFitObject* fo = *i;
        assert(fo);
        for (int ilocal = 0; ilocal < fo->getNPar(); ++ilocal) {
          int iglobal = fo->getGlobalParNum(ilocal);
          if (iglobal >= idim) idim = iglobal + 1;
          if (iglobal == xglobal) xname = fo->getParamName(ilocal);
          if (iglobal == yglobal) yname = fo->getParamName(ilocal);
        }
      }
      for (ConstraintIterator i = constraints->begin(); i != constraints->end(); ++i) {
        BaseHardConstraint* c = *i;
        assert(c);
        int iglobal = c->getGlobalNum();
        if (iglobal >= idim) idim = iglobal + 1;
      }

      if (xglobal >= idim) return;
      if (yglobal >= idim) return;

      double* parsave  = new double [idim];
      double* par  = new double [idim];

      // Get starting values
      for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
        BaseFitObject* fo = *i;
        assert(fo);
        for (int ilocal = 0; ilocal < fo->getNPar(); ++ilocal) {
          int iglobal = fo->getGlobalParNum(ilocal);
          assert(iglobal >= 0 && iglobal < idim);
          parsave[iglobal] = fo->getParam(ilocal);
        }
      }

      // Book Histograms

      TString idpostfix("");
      idpostfix += xglobal;
      idpostfix += "_";
      idpostfix += yglobal;

      TString titlepostfix(" vs ");
      titlepostfix += xname;
      titlepostfix += " and ";
      titlepostfix += yname;
      titlepostfix += ";";
      titlepostfix += xname;
      titlepostfix += ";";
      titlepostfix += yname;

      // Chi2 Histogram

      TString id(idprefix);
      id += "chi2_";
      id += idpostfix;

      TString title(titleprefix);
      title += "Chi2 ";
      title += titlepostfix;

      TH2F* hchi2 = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
      B2INFO("Booking Histo '" << id << "': '" << title << "'");

      TH2F* halpha = 0;
      TH2F* hlog2alpha = 0;
      TH2F* hmu = 0;
      TH2F* hphi1 = 0;

      if (newfitter) {
        id = idprefix;
        id += "alpha_";
        id += idpostfix;

        title = titleprefix;
        title += "#alpha ";
        title += titlepostfix;

        halpha = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
        B2INFO("Booking Histo '" << id << "': '" << title << "'");

        id = idprefix;
        id += "log2alpha_";
        id += idpostfix;

        title = titleprefix;
        title += "log_{2} (#alpha) ";
        title += titlepostfix;

        hlog2alpha = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
        B2INFO("Booking Histo '" << id << "': '" << title << "'");

        id = idprefix;
        id += "mu_";
        id += idpostfix;

        title = titleprefix;
        title += "#mu ";
        title += titlepostfix;

        hmu = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
        B2INFO("Booking Histo '" << id << "': '" << title << "'");

        id = idprefix;
        id += "phi1_";
        id += idpostfix;

        title = titleprefix;
        title += "Merit function #phi_{1} ";
        title += titlepostfix;

        hphi1 = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
        B2INFO("Booking Histo '" << id << "': '" << title << "'");


      }

      TMultiGraph* mgstepsfull = 0;
      TMultiGraph* mgsteps = 0;
      TGraph* gstep0 = 0;
      TGraph* gstep1 = 0;
      TGraph* gstep2 = 0;
      if (newfitter) {
        id = idprefix;
        id += "stepsfull_";
        id += idpostfix;

        title = titleprefix;
        title += "Full Steps ";
        title += titlepostfix;

        mgstepsfull = new TMultiGraph(id, title);
        B2INFO("Booking Multigraph '" << id << "': '" << title << "'");

        id = idprefix;
        id += "steps_";
        id += idpostfix;

        title = titleprefix;
        title += "Steps ";
        title += titlepostfix;

        mgsteps = new TMultiGraph(id, title);
        B2INFO("Booking Multigraph '" << id << "': '" << title << "'");

        id = idprefix;
        id += "stepstart_";
        id += idpostfix;

        title = titleprefix;
        title += "Step Start Points ";
        title += titlepostfix;

        gstep0 = new TGraph(nx * ny);
        gstep0 ->SetName(id);
        gstep0 ->SetTitle(title);
        gstep0->SetMarkerStyle(20);
        gstep0->SetMarkerColor(kBlack);
        gstep0->SetMarkerSize(0.5);

        id = idprefix;
        id += "stependfull_";
        id += idpostfix;

        title = titleprefix;
        title += "Full Step End Points ";
        title += titlepostfix;

        gstep1 = new TGraph(nx * ny);
        gstep1 ->SetName(id);
        gstep1 ->SetTitle(title);
        gstep1->SetMarkerStyle(20);
        gstep1->SetMarkerColor(kRed);
        gstep1->SetMarkerSize(0.5);

        id = idprefix;
        id += "stepend_";
        id += idpostfix;

        title = titleprefix;
        title += "Step End Points ";
        title += titlepostfix;

        gstep2 = new TGraph(nx * ny);
        gstep2 ->SetName(id);
        gstep2 ->SetTitle(title);
        gstep2->SetMarkerStyle(21);
        gstep2->SetMarkerColor(kGreen);
        gstep2->SetMarkerSize(0.5);


      }

      unsigned int ncon = constraints->size();
      if (ncon > NCONMAX) ncon = NCONMAX;
      TH2F* hcon[NCONMAX];
      TH2F* hlambda[NCONMAX];
      for (unsigned int icon = 0; icon < ncon; ++icon) {
        hcon[icon] = 0;
        hlambda[icon] = 0;
        BaseConstraint* c = (*constraints)[icon];
        if (c) {
          id = idprefix;
          id += "con";
          id += icon;
          id += "_";
          id += idpostfix;

          title = titleprefix;
          title += "Constraint ";
          title += icon;
          title += ": ";
          title += c->getName();
          title += titlepostfix;

          hcon[icon] = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
          B2INFO("Booking Histo '" << id << "': '" << title << "'");

          if (newfitter) {
            id = idprefix;
            id += "lambda";
            id += icon;
            id += "_";
            id += idpostfix;

            title = titleprefix;
            title += "Lambda ";
            title += icon;
            title += ": ";
            title += c->getName();
            title += titlepostfix;

            hlambda[icon] = new TH2F(id, title, nx, xstart, xstop, ny, ystart, ystop);
            B2INFO("Booking Histo '" << id << "': '" << title << "'");

          }


        }
      }


      // Do the scan

      for (int ix = 1; ix <= nx; ++ix) {
        double x = (ix - 0.5) * (xstop - xstart) / nx + xstart;
        for (int iy = 1; iy <= ny; ++iy) {
          double y = (iy - 0.5) * (ystop - ystart) / ny + ystart;

          // Set parameters
          for (int i = 0; i < idim; ++i) par[i] = parsave[i];
          par[xglobal] = x;
          par[yglobal] = y;
          for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
            BaseFitObject* fo = *i;
            assert(fo);
            fo->updateParams(par, idim);
          }


          // Calculate chi2
          double chi2 = 0;
          for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
            BaseFitObject* fo = *i;
            assert(fo);
            chi2 += fo->getChi2();
          }

          //B2INFO( "Chi2 for x=" << x << " and y=" << y << ": " << chi2);

          if (newfitter) {
            newfitter->fillx(newfitter->x);
            newfitter->assembleConstDer(newfitter->M);
            newfitter->determineLambdas(newfitter->x, newfitter->M, newfitter->x, newfitter->W, newfitter->v1);
          }

          // Fill Histos
          hchi2->SetBinContent(ix, iy, chi2);
          for (unsigned int icon = 0; icon < ncon; ++icon) {
            BaseHardConstraint* c = (*constraints)[icon];
            TH2F* h = hcon[icon];
            if (c && h) h->SetBinContent(ix, iy, c->getValue());
            if (c) B2INFO("x=" << x << ", y=" << y << ": Constraint " << c->getName() << ": " << c->getValue())
              h = hlambda[icon];
            if (c && h && newfitter) {
              int kglobal = c->getGlobalNum();
              h->SetBinContent(ix, iy, gsl_vector_get(newfitter->x, kglobal));;
            }

          }

          if (newfitter) {
            newfitter->setDebug((ix == 1) ? 4 : 0);

            double xval[2], yval[2];
            xval[0] = x;
            yval[0] = y;
            gstep0->SetPoint((ix - 1)*ny + (iy - 1), xval[0], yval[0]);
            newfitter->fillx(newfitter->x);
            newfitter->fillperr(newfitter->perr);

            newfitter->assembleConstDer(newfitter->M);
            newfitter->determineLambdas(newfitter->x, newfitter->M, newfitter->x, newfitter->W, newfitter->v1);

            double phi1 = newfitter->meritFunction(mumerit, newfitter->x,  newfitter->perr);
            if (hphi1)    hphi1->SetBinContent(ix, iy, phi1);

            newfitter->calcNewtonDx(newfitter->dx, newfitter->dxscal, newfitter->x,
                                    newfitter->perr, newfitter->M, newfitter->Mscal,
                                    newfitter->y, newfitter->yscal, newfitter->W, newfitter->W2,
                                    newfitter->permW, newfitter->v1);
            newfitter->add(newfitter->xnew, newfitter->x, 1, newfitter->dx);

            xval[1] = gsl_vector_get(newfitter->xnew, xglobal);
            yval[1] = gsl_vector_get(newfitter->xnew, yglobal);

            B2INFO("ParameterScanner::doScan: full step from (" << xval[0] << ", " << yval[0]
                   << ") -> (" << xval[1] << ", " << yval[1] << ")");


            gstep1->SetPoint((ix - 1)*ny + (iy - 1), xval[1], yval[1]);

            TGraph* g = new TGraph(2, xval, yval);
            g->SetLineColor(kRed);

            if (xval[1] > xstart - 10 * (xstop - xstart) &&
                xval[1] < xstop + 10 * (xstop - xstart) &&
                yval[1] > ystart - 10 * (ystop - ystart) &&
                yval[1] < ystop + 10 * (ystop - ystart)) {

              mgstepsfull->Add(g, "L");
              B2INFO(" -> added\n");
            }
            double alpha = 1;
            double mu = 0;
            int imode = 2;

            newfitter->calcLimitedDx(alpha, mu, newfitter->xnew, imode,
                                     newfitter->x, newfitter->v2, newfitter->dx, newfitter->dxscal,
                                     newfitter->perr, newfitter->M, newfitter->Mscal,
                                     newfitter->W, newfitter->v1);

            xval[1] = gsl_vector_get(newfitter->xnew, xglobal);
            yval[1] = gsl_vector_get(newfitter->xnew, yglobal);

            B2INFO("    limited step from (" << xval[0] << ", " << yval[0]
                   << ") -> (" << xval[1] << ", " << yval[1] << "), alpha=" << alpha);

            gstep2->SetPoint((ix - 1)*ny + (iy - 1), xval[1], yval[1]);

            g = new TGraph(2, xval, yval);
            g->SetLineColor(kGreen);

            if (xval[1] > xstart - 10 * (xstop - xstart) &&
                xval[1] < xstop + 10 * (xstop - xstart) &&
                yval[1] > ystart - 10 * (ystop - ystart) &&
                yval[1] < ystop + 10 * (ystop - ystart)) {

              mgsteps->Add(g, "L");
              B2INFO(" -> added\n");
            }
            if (halpha) halpha->SetBinContent(ix, iy, alpha);
            if (hlog2alpha) hlog2alpha->SetBinContent(ix, iy, std::log(alpha) / std::log(2.));
            if (hmu)    hmu->SetBinContent(ix, iy, mu);


          }

        }
      }

      // Write histos;
      hchi2->Write();
      for (unsigned int icon = 0; icon < ncon; ++icon) {
        if (hcon[icon]) hcon[icon]->Write();
        if (hlambda[icon]) hcon[icon]->Write();
      }

      for (FitObjectIterator i = fitobjects->begin(); i != fitobjects->end(); ++i) {
        BaseFitObject* fo = *i;
        assert(fo);
        fo->updateParams(parsave, idim);
      }

      if (mgstepsfull) mgstepsfull->Write();
      if (mgsteps)     mgsteps->Write();
      if (gstep0) gstep0->Write();
      if (gstep1) gstep1->Write();
      if (gstep2) gstep2->Write();
      if (halpha) halpha->Write();
      if (hlog2alpha) hlog2alpha->Write();
      if (hmu)    hmu->Write();
      if (hphi1)  hphi1->Write();

      delete[] par;
      delete[] parsave;
    }

  }// end OrcaKinFit namespace
} // end Belle2 namespace

#endif // MARLIN_USE_ROOT
