/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mva/interface/Interface.h>
#include <mva/methods/FastBDT.h>
#include <mva/methods/TMVA.h>
#include <mva/methods/Python.h>
#include <mva/methods/FANN.h>
#include <mva/methods/PDF.h>
#include <mva/methods/Trivial.h>
#include <mva/methods/Reweighter.h>
#include <mva/methods/Combination.h>

namespace Belle2 {
  namespace MVA {

    AbstractInterface::AbstractInterface(std::string name) : m_name(name)
    {
      if (s_supported_interfaces.find(m_name) != s_supported_interfaces.end()) {
        B2WARNING("An interface with the name " << m_name << " already exists!");
      } else {
        s_supported_interfaces.insert(std::make_pair(name, this));
      }
    }

    AbstractInterface::~AbstractInterface()
    {
      s_supported_interfaces.erase(m_name);
    }

    std::string AbstractInterface::getName() const { return m_name; }

    std::map<std::string, AbstractInterface*> AbstractInterface::s_supported_interfaces;

    void AbstractInterface::initSupportedInterfaces()
    {
      static Interface<FastBDTOptions, FastBDTTeacher, FastBDTExpert> interface_FastBDT;
      static Interface<FANNOptions, FANNTeacher, FANNExpert> interface_FANN;
      static Interface<TMVAOptionsClassification, TMVATeacherClassification, TMVAExpertClassification>
      interface_TMVAClassification;
      static Interface<TMVAOptionsRegression, TMVATeacherRegression, TMVAExpertRegression> interface_TMVARegression;
      static Interface<PythonOptions, PythonTeacher, PythonExpert> interface_Python;
      static Interface<PDFOptions, PDFTeacher, PDFExpert> interface_PDF;
      static Interface<CombinationOptions, CombinationTeacher, CombinationExpert> interface_Combination;
      static Interface<ReweighterOptions, ReweighterTeacher, ReweighterExpert> interface_Reweighter;
      static Interface<TrivialOptions, TrivialTeacher, TrivialExpert> interface_Trivial;
    }

  }
}
