/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/core/PrintBeamParametersModule.h>
#include <framework/logging/Logger.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PrintBeamParameters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PrintBeamParametersModule::PrintBeamParametersModule() : Module()
{
  // Set module properties
  setDescription("Print the BeamParameters everytime they change");

  // Parameter definitions
}

void PrintBeamParametersModule::event()
{
  if (!m_beamparams || !m_beamparams.hasChanged()) return;
  std::stringstream out;
  const TLorentzVector& her = m_beamparams->getHER();
  const TLorentzVector& ler = m_beamparams->getLER();
  const TLorentzVector& cms = her + ler;
  const TVector3& vtx = m_beamparams->getVertex();
  out << "BeamParameters: cms Energy=" << m_beamparams->getMass() << " GeV, flags="
      << m_beamparams->getGenerationFlagString() << std::endl
      << "   HER=(" << her.X() << ", " << her.Y() << ", " << her.Z() << ", " << her.E() << "), " << std::endl
      << "   LER=(" << ler.X() << ", " << ler.Y() << ", " << ler.Z() << ", " << ler.E() << "), " << std::endl
      << "   CMS=(" << cms.X() << ", " << cms.Y() << ", " << cms.Z() << ", " << cms.E() << "), " << std::endl
      << "   VTX=(" << vtx.X() << ", " << vtx.Y() << ", " << vtx.Z() << "), " << std::endl
      << "   CovHER=";
  printCovMatrix(out, m_beamparams->getCovHER());
  out << std::endl << "   CovLER=";
  printCovMatrix(out, m_beamparams->getCovLER());
  out << std::endl << "   CovVTX=";
  printCovMatrix(out, m_beamparams->getCovVertex());
  B2INFO(out.str());
}

void PrintBeamParametersModule::printCovMatrix(std::ostream& out, const TMatrixDSym& cov)
{
  // check for off diagonal elements
  bool offdiag = cov(0, 1) != 0 || cov(0, 2) != 0 || cov(1, 2) != 0;
  if (offdiag) {
    // if so print full matrix
    out << "[";
    for (int i = 0; i < 3; ++i) {
      out << (i > 0 ? ", " : "") << "(";
      for (int j = 0; j < 3; ++j) {
        out << cov(i, j) << (j < 2 ? ", " : ")");
      }
    }
    out << "]";
  } else {
    // otherwise just print the diagonal
    out << "diag(";
    for (int i = 0; i < 3; ++i) {
      out << cov(i, i) << (i < 2 ? ", " : ")");
    }
  }
}
