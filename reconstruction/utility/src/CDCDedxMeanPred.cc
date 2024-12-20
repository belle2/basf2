/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/utility/CDCDedxMeanPred.h>

using namespace Belle2;

void CDCDedxMeanPred::setParameters()
{

  // make sure the resolution parameters are reasonable
  if (!m_DBMeanPars || m_DBMeanPars->getSize() == 0)
    B2FATAL("No dE/dx mean parameters!");

  std::vector<double> meanpar;
  meanpar = m_DBMeanPars->getMeanPars();

  for (int i = 0; i < 15; ++i)
    m_meanpars[i] = meanpar[i];
}


void CDCDedxMeanPred::setParameters(std::string infile)
{

  B2INFO("\n\tWidgetParameterization: Using parameters from file --> " << infile);

  std::ifstream fin;
  fin.open(infile.c_str());

  if (!fin.good()) B2FATAL("\tWARNING: CANNOT FIND" << infile);

  int par;

  B2INFO("\t --> Curve parameters");
  for (int i = 0; i < 15; ++i) {
    fin >> par >> m_meanpars[i];
    B2INFO("\t\t (" << i << ")" << m_meanpars[i]);
  }

  fin.close();
}

void CDCDedxMeanPred::printParameters(std::string outfile)
{

  B2INFO("\tCDCDedxMeanPred: Printing parameters to file --> " << outfile.c_str());

  // write out the parameters to file
  std::ofstream fout(outfile.c_str());

  for (int i = 1; i < 16; ++i) fout << i << "\t" << m_meanpars[i - 1] << std::endl;
  fout.close();
}

double CDCDedxMeanPred::getMean(double bg)
{

  // define the section of the mean to use
  double A = 0, B = 0, C = 0;

  if (bg < 4.5) A = 1;
  else if (bg < 10) B = 1;
  else C = 1;

  double x[1]; x[0] = bg;
  double parsA[9];
  double parsB[5];
  double parsC[5];

  parsA[0] = 1; parsB[0] = 2; parsC[0] = 3;
  for (int i = 0; i < 15; ++i) {
    if (i < 7) parsA[i + 1] = m_meanpars[i];
    else if (i < 11) parsB[i % 7 + 1] = m_meanpars[i];
    else parsC[i % 11 + 1] = m_meanpars[i];
  }

  // calculate dE/dx from the Bethe-Bloch mean
  CDCDedxWidgetCurve gc;
  double partA = gc.meanCurve(x, parsA);
  double partB = gc.meanCurve(x, parsB);
  double partC = gc.meanCurve(x, parsC);

  return (A * partA + B * partB + C * partC);
}

