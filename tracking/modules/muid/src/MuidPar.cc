/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/muid/MuidPar.h>

#include <fstream>
#include <exception>

#include <framework/utilities/FileSystem.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  MuidPar::MuidPar() : m_IsValid(false)
  {
  }

  MuidPar::MuidPar(int expNo, const char hypothesisName[]) : m_IsValid(false)
  {
    fillPDFs(expNo, hypothesisName);
    if (!m_IsValid) { B2ERROR("MuidPar::constructor:  Failed to read " << hypothesisName << " PDFs for experiment " << expNo) }
  }

  MuidPar::~MuidPar()
  {
  }

  void MuidPar::fillPDFs(int expNo, const char hypothesisName[])
  {
    expNo = 0; // DIVOT ignore supplied value for now

    char line[128];
    sprintf(line, "/Detector/Tracking/MuidParameters/Experiment[@exp=\"%d\"]/%s/", expNo, hypothesisName);
    GearDir content(line);
    if (!content) {
      B2ERROR("muid:MuidPar::fillPDFs(): required XML content MuidParameters not found")
    }

    m_IsValid = true;
    for (int outcome = 1; outcome <= 4; ++outcome) {
      sprintf(line, "/Outcome[@outcome=\"%d\"]", outcome);
      GearDir outcomeContent(content);
      outcomeContent.append(line);
      for (int layer = 0; layer <= 25; ++layer) {
        sprintf(line, "LongitudinalPDF/LastLayer[@layer=\"%d\"]", layer);
        std::vector<double> rangePDF = outcomeContent.getArray(line);
        if (rangePDF.size() != MUID_MaxRange) {
          B2ERROR("muid::MuidPar::fillPDFs(): LongitudinalPDF vector for hypothesis " << hypothesisName << "  outcome " << outcome
                  << " layer=" << layer << " has " << rangePDF.size() << " entries; should be " << MUID_MaxRange)
          m_IsValid = false;
        } else {
          for (int i = 0; i < MUID_MaxRange; ++i) {
            m_RangePDF[outcome - 1][layer][i] = rangePDF[i];
          }
        }
      }
      for (int ndof = 2; ndof < 38; ndof += 2) {
        sprintf(line, "TransversePDF/DegreesOfFreedom[@ndof=\"%d\"]", ndof);
        std::vector<double> reducedChiSquaredPDF = outcomeContent.getArray(line);
        if (reducedChiSquaredPDF.size() != MUID_MaxReducedChiSquared) {
          B2ERROR("muid::MuidPar::fillPDFs(): TransversePDF vector for hypothesis " << hypothesisName << "  outcome " << outcome
                  << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MUID_MaxReducedChiSquared)
          m_IsValid = false;
        } else {
          for (int i = 0; i < MUID_MaxReducedChiSquared; ++i) {
            m_ReducedChiSquaredPDF[outcome - 1][ndof / 2][i] = reducedChiSquaredPDF[i];
          }
          double dx = MUID_ReducedChiSquaredLimit / MUID_MaxReducedChiSquared;   // bin size
          spline(MUID_MaxReducedChiSquared, dx, &m_ReducedChiSquaredPDF[outcome - 1][ndof / 2][0],
                 &m_ReducedChiSquaredD1[outcome - 1][ndof / 2][0],
                 &m_ReducedChiSquaredD2[outcome - 1][ndof / 2][0],
                 &m_ReducedChiSquaredD3[outcome - 1][ndof / 2][0]);
        }
      }
    }
    if (!m_IsValid) {
      B2FATAL("muid::MuidPar::fillPDFs():  failed to read PDFs")
    }
  }

  void MuidPar::spline(int n, double dx, double Y[], double B[], double C[], double D[])
  {

    // Generate the spline interpolation coefficients B, C, D to smooth out a
    // binned histogram. Restrictions:  equal-size bins. more than 3 bins.

    D[0] = dx;
    C[1] = (Y[1] - Y[0]) / dx;
    for (int i = 1; i < n - 1; i++) {
      D[i]   = dx;
      B[i]   = dx * 4.0;
      C[i + 1] = (Y[i + 1] - Y[i]) / dx;
      C[i]   = C[i + 1] - C[i];
    }
    B[0]   = -dx;
    B[n - 1] = -dx;
    C[0]   = (C[2]   - C[1]) / 6.0;
    C[n - 1] = -(C[n - 2] - C[n - 3]) / 6.0;
    for (int i = 1; i < n; i++) {
      double temp = dx / B[i - 1];
      B[i] -= temp * dx;
      C[i] -= temp * C[i - 1];
    }
    C[n - 1] /= B[n - 1];
    for (int i = n - 2; i >= 0; i--) {
      C[i] = (C[i] - D[i] * C[i + 1]) / B[i];
    }
    B[n - 1] = (Y[n - 1] - Y[n - 2]) / dx + (C[n - 2] + C[n - 1] * 2.0) * dx;
    for (int i = 0; i < n - 1; i++) {
      B[i] = (Y[i + 1] - Y[i]) / dx - (C[i + 1] + C[i] * 2.0) * dx;
      D[i] = (C[i + 1] - C[i]) / dx;
      C[i] = C[i] * 3.0;
    }
    C[n - 1] = C[n - 1] * 3.0;
    D[n - 1] = D[n - 2];
  }

  double MuidPar::getPDF(int outcome, int lastExtLayer,
                         int layerDifference, int ndof, double chiSquared) const
  {

    // outcome:  0=Not in KLM, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
    // lastExtLayer:  last layer that Ext track touched (zero-based)
    // layerDifference:  difference between last Ext layer and last hit layer
    // reducedChiSquared: reduced chi**2 of the transverse deviations of all associated
    //           hits from the corresponding Ext track crossings

    if (!m_IsValid) {
      B2ERROR("MuidPar::getPDF():  PDFs have not been filled yet")
      return 0.0;
    }

    if ((outcome <= 0) || (outcome > 4)) return 0.0;
    return getPDFRange(outcome, lastExtLayer, layerDifference) * getPDFRchisq(outcome, ndof, chiSquared);

  }

  double MuidPar::getPDFRange(int outcome, int lastExtLayer, int layerDifference) const
  {

    if ((lastExtLayer < 0) || (lastExtLayer  > 25)) return 0.0;

    // Evaluate the longitudinal-coordinate PDF for this particleID hypothesis

    if (layerDifference >= MUID_MaxRange) { layerDifference = MUID_MaxRange - 1; }
    return m_RangePDF[outcome - 1][lastExtLayer][layerDifference];

  }

  double MuidPar::getPDFRchisq(int outcome, int ndof, double chiSquared) const
  {

    // Evaluate the transverse-coordinate PDF for this particleID hypothesis
    // Use spline interpolation of the PDF to avoid binning artifacts.

    if (ndof <= 0) return 1.0;
    if (chiSquared < 0.0) return 0.0;
    double reducedChiSquared = chiSquared / ndof;
    if (reducedChiSquared >= MUID_ReducedChiSquaredLimit)
      return m_ReducedChiSquaredPDF[outcome - 1][ndof / 2][MUID_MaxReducedChiSquared - 1];

    int    i  = (int)(reducedChiSquared / (MUID_ReducedChiSquaredLimit / MUID_MaxReducedChiSquared));
    double dx = reducedChiSquared - i * (MUID_ReducedChiSquaredLimit / MUID_MaxReducedChiSquared);
    double pdf = m_ReducedChiSquaredPDF[outcome - 1][ndof / 2][i] +
                 dx * (m_ReducedChiSquaredD1[outcome - 1][ndof / 2][i] +
                       dx * (m_ReducedChiSquaredD2[outcome - 1][ndof / 2][i] +
                             dx * m_ReducedChiSquaredD3[outcome - 1][ndof / 2][i]));
    return max(0.0, pdf);
  }

} // end of namespace Belle2
