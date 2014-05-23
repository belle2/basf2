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

#include <TMath.h>

using namespace std;

namespace Belle2 {

  MuidPar::MuidPar() : m_ReducedChiSquaredDx(0.0)
  {
  }

  MuidPar::MuidPar(int expNo, const char hypothesisName[]) : m_ReducedChiSquaredDx(0.0)
  {
    fillPDFs(expNo, hypothesisName);
    if (m_ReducedChiSquaredDx == 0.0) { B2ERROR("Failed to read " << hypothesisName << " PDFs for experiment " << expNo) }
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
      B2ERROR("Required XML content MuidParameters not found")
    }

    m_ReducedChiSquaredDx = MUID_ReducedChiSquaredLimit / MUID_ReducedChiSquaredNbins;   // bin size
    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      sprintf(line, "Outcome[@outcome=\"%d\"]/", outcome);
      GearDir outcomeContent(content);
      outcomeContent.append(line);
      for (int layer = 0; layer <= 25; ++layer) {
        sprintf(line, "LongitudinalPDF/LastLayer[@layer=\"%d\"]", layer);
        std::vector<double> rangePDF = outcomeContent.getArray(line);
        if (rangePDF.size() != MUID_RangeNbins) {
          B2ERROR("LongitudinalPDF vector for hypothesis " << hypothesisName << "  outcome " << outcome
                  << " layer=" << layer << " has " << rangePDF.size() << " entries; should be " << MUID_RangeNbins)
          m_ReducedChiSquaredDx = 0.0; // invalidate the PDFs for this hypothesis
        } else {
          for (int i = 0; i < MUID_RangeNbins; ++i) {
            m_RangePDF[outcome][layer][i] = rangePDF[i];
          }
        }
      }
      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        sprintf(line, "TransversePDF/DegreesOfFreedom[@ndof=\"%d\"]", 2 * halfNdof);
        std::vector<double> reducedChiSquaredPDF = outcomeContent.getArray(line);
        if (reducedChiSquaredPDF.size() != MUID_ReducedChiSquaredNbins) {
          B2ERROR("TransversePDF vector for hypothesis " << hypothesisName << "  outcome " << outcome
                  << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MUID_ReducedChiSquaredNbins)
          m_ReducedChiSquaredDx = 0.0; // invalidate the PDFs for this hypothesis
        } else {
          double integral = 1.0E-30;
          for (int i = 0; i < MUID_ReducedChiSquaredNbins; ++i) {
            integral += reducedChiSquaredPDF[i];
          }
          for (int i = 0; i < MUID_ReducedChiSquaredNbins; ++i) {
            m_ReducedChiSquaredPDF[outcome][halfNdof][i] = reducedChiSquaredPDF[i] / integral;
          }
          spline(MUID_ReducedChiSquaredNbins - 1, m_ReducedChiSquaredDx,
                 &m_ReducedChiSquaredPDF[outcome][halfNdof][0],
                 &m_ReducedChiSquaredD1[outcome][halfNdof][0],
                 &m_ReducedChiSquaredD2[outcome][halfNdof][0],
                 &m_ReducedChiSquaredD3[outcome][halfNdof][0]);
          m_ReducedChiSquaredD1[outcome][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
          m_ReducedChiSquaredD2[outcome][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
          m_ReducedChiSquaredD3[outcome][halfNdof][MUID_ReducedChiSquaredNbins - 1] = 0.0;
        }
      }
    }
    if (m_ReducedChiSquaredDx == 0.0) {
      B2FATAL("Failed to read PDFs")
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

    if ((outcome <= 0) || (outcome > MUID_MaxOutcome)) return 0.0;
    return getPDFRange(outcome, lastExtLayer, layerDifference) * getPDFRchisq(outcome, ndof, chiSquared);

  }

  double MuidPar::getPDFRange(int outcome, int lastExtLayer, int layerDifference) const
  {

    if ((lastExtLayer < 0) || (lastExtLayer > MUID_MaxLastExtLayer)) return 0.0;

    // Evaluate the longitudinal-coordinate PDF for this particleID hypothesis

    if (layerDifference >= MUID_RangeNbins) layerDifference = MUID_RangeNbins - 1;
    return m_RangePDF[outcome][lastExtLayer][layerDifference];

  }

  double MuidPar::getPDFRchisq(int outcome, int ndof, double chiSquared) const
  {

    // Evaluate the transverse-coordinate PDF for this particleID hypothesis.
    // Use spline interpolation of the PDF to avoid binning artifacts.

    if (ndof <= 0) return 1.0;
    int halfNdof = (ndof >> 1);
    double pdf = 0.0;
    if ((chiSquared > 0.0) && (halfNdof <= MUID_MaxHalfNdof)) {
      double x = chiSquared / ndof - 0.5 * m_ReducedChiSquaredDx;
      if (x >= MUID_ReducedChiSquaredLimit - 0.5 * m_ReducedChiSquaredDx) {
        pdf = m_ReducedChiSquaredPDF[outcome][halfNdof][MUID_ReducedChiSquaredNbins - 1];
      } else {
        int i  = (int)(x / m_ReducedChiSquaredDx);
        pdf = m_ReducedChiSquaredPDF[outcome][halfNdof][i];
        double dx = x - i * m_ReducedChiSquaredDx;
        double corr = dx * (m_ReducedChiSquaredD1[outcome][halfNdof][i] +
                            dx * (m_ReducedChiSquaredD2[outcome][halfNdof][i] +
                                  dx * m_ReducedChiSquaredD3[outcome][halfNdof][i]));
        if (corr > 0.0) pdf += corr;
      }
    }
    return pdf;
  }

} // end of namespace Belle2
