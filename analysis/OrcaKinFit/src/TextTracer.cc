/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * Forked from https://github.com/iLCSoft/MarlinKinfit                    *
 *                                                                        *
 * Further information about the fit engine and the user interface        *
 * provided in MarlinKinfit can be found at                               *
 * https://www.desy.de/~blist/kinfit/doc/html/                            *
 * and in the LCNotes LC-TOOL-2009-001 and LC-TOOL-2009-004 available     *
 * from http://www-flc.desy.de/lcnotes/                                   *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "analysis/OrcaKinFit/TextTracer.h"
#include "analysis/OrcaKinFit/BaseFitter.h"
#include "analysis/OrcaKinFit/BaseFitObject.h"
#include "analysis/OrcaKinFit/BaseConstraint.h"
#include "analysis/OrcaKinFit/BaseHardConstraint.h"
#include "analysis/OrcaKinFit/BaseSoftConstraint.h"
#undef NDEBUG
#include <cassert>
#include <cstring>
#include <cmath>

namespace Belle2 {
  namespace OrcaKinFit {

    TextTracer::TextTracer(std::ostream& os_)
      : os(os_),
        istep(0), isubstep(0), chi2fo(0), chi2sc(0), sumhc(0), sumhcscal(0)
    {}

    TextTracer::~TextTracer() = default;


    void TextTracer::initialize(BaseFitter& fitter)
    {
      os << "=============== Starting fit ======================\n";

      printFitObjects(fitter);
      printConstraints(fitter);
      printTraceValues(fitter);
      printSums(fitter);

      istep = 1;
      isubstep = 0;

      BaseTracer::initialize(fitter);
    }

    void TextTracer::step(BaseFitter& fitter)
    {
      isubstep = 1;
      os << "--------------- Step " << istep << " --------------------\n";

      printFitObjects(fitter);
      printConstraints(fitter);
      printTraceValues(fitter);
      printSums(fitter);

      ++istep;
      BaseTracer::step(fitter);
    }

    void TextTracer::substep(BaseFitter& fitter, int flag)
    {
      os << "---- Substep " << istep << "." << isubstep << " ----\n";

      printFitObjects(fitter);
      printConstraints(fitter);
      printTraceValues(fitter);
      printSums(fitter);

      ++isubstep;
      BaseTracer::substep(fitter, flag);
    }

    void TextTracer::finish(BaseFitter& fitter)
    {

      os << "=============== Final result ======================\n";
      printFitObjects(fitter);
      printConstraints(fitter);
      printTraceValues(fitter);
      printSums(fitter);

      os << "=============== Finished fit ======================\n";


      BaseTracer::finish(fitter);
    }

    void TextTracer::printFitObjects(BaseFitter& fitter)
    {
      chi2fo = 0;
      FitObjectContainer* fitobjects = fitter.getFitObjects();
      if (!fitobjects) return;
      os << "Fit objects:\n";
      for (auto fo : *fitobjects) {
        assert(fo);
        os << fo->getName() << ": " << *fo << ", chi2=" << fo->getChi2() << std::endl;
        chi2fo += fo->getChi2();
      }
    }
    void TextTracer::printConstraints(BaseFitter& fitter)
    {
      chi2sc = 0;
      sumhc = 0;
      sumhcscal = 0;
      ConstraintContainer* constraints = fitter.getConstraints();
      if (constraints && constraints->size() > 0) {
        os << "Hard Constraints:\n";
        for (auto i = constraints->begin(); i != constraints->end(); ++i) {
          BaseConstraint* c = *i;
          assert(c);
          os << i - constraints->begin() << " " << c->getName() << ": " << c->getValue() << "+-" << c->getError() << std::endl;
          sumhc += std::fabs(c->getValue());
          sumhcscal += std::fabs(c->getValue() / c->getError());
        }
      }
      SoftConstraintContainer* softConstraints = fitter.getSoftConstraints();
      if (softConstraints && softConstraints->size() > 0) {
        os << "Soft Constraints:\n";
        for (auto i = softConstraints->begin(); i != softConstraints->end(); ++i) {
          BaseConstraint* c = *i;
          auto* sc = dynamic_cast<BaseSoftConstraint*>(c);
          assert(c);
          assert(sc);
          os << i - softConstraints->begin() << " " << c->getName() << ": " << c->getValue() << "+-" << c->getError()
             << ", chi2=" << sc->getChi2() << std::endl;
          chi2sc +=  sc->getChi2();
        }
      }
    }

    void TextTracer::printTraceValues(const BaseFitter& fitter)
    {
      for (auto& traceValue : fitter.traceValues) {
        std::string name = traceValue.first;
        double value = traceValue.second;
        os << "Value of " << name << ": " << value << std::endl;;
      }
    }
    void TextTracer::printSums(BaseFitter& fitter)
    {
      os << "Total chi2: " << fitter.getChi2()
         << " = " << chi2fo + chi2sc << " = " << chi2fo << "(fo) + " << chi2sc << "(sc)"
         << std::endl;
      os << "Hard constraints: " << sumhc << ", scaled: " << sumhcscal << std::endl;
      auto i = fitter.traceValues.find("mu");
      if (i != fitter.traceValues.end()) {
        double mu = i->second;
        os << "Contribution to merit function: " << sumhc* mu << ", scaled: " << sumhcscal* mu << std::endl;
        os << "Merit function: " << chi2fo + chi2sc + sumhc* mu << ", scaled: " << chi2fo + chi2sc + sumhcscal* mu << std::endl;
      }

    }
  }// end OrcaKinFit namespace
} // end Belle2 namespace
