/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano de Marino, Tadeas Bilka                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/calibration/BeamSpotAlgorithm.h>

#include <mdst/dbobjects/BeamSpot.h>

#include <TH1F.h>

using namespace std;
using namespace Belle2;

BeamSpotAlgorithm::BeamSpotAlgorithm() : CalibrationAlgorithm("BeamSpotCollector")
{
  setDescription("BeamSpot calibration algorithm");
}

CalibrationAlgorithm::EResult BeamSpotAlgorithm::calibrate()
{
  int minVertices = 300;
  int nSigmacut = 6;

  auto hVertexX = getObjectPtr<TH1F>("Y4S_Vertex.X");
  hVertexX->GetXaxis()->UnZoom();

  int entries = hVertexX->GetEntries();
  if (entries < minVertices) {
    return c_NotEnoughData;
  }


  auto hVertexY = getObjectPtr<TH1F>("Y4S_Vertex.Y");
  hVertexY->GetXaxis()->UnZoom();
  auto hVertexZ = getObjectPtr<TH1F>("Y4S_Vertex.Z");
  hVertexZ->GetXaxis()->UnZoom();

  auto hVarX = getObjectPtr<TH1F>("Var.X");
  hVarX->GetXaxis()->UnZoom();
  auto hVarY = getObjectPtr<TH1F>("Var.Y");
  hVarY->GetXaxis()->UnZoom();
  auto hVarZ = getObjectPtr<TH1F>("Var.Z");
  hVarZ->GetXaxis()->UnZoom();

  Double_t medianX[1] = {0.};
  Double_t medianY[1] = {0.};
  Double_t medianZ[1] = {0.};

  Double_t q[1] = {0.5};

  hVertexX->GetQuantiles(1, medianX, q);
  hVertexY->GetQuantiles(1, medianY, q);
  hVertexZ->GetQuantiles(1, medianZ, q);

  auto vertexPos = TVector3(medianX[0], medianY[0], medianZ[0]);
  auto vertexSize = TMatrixDSym(3);
  auto vertexCov = TMatrixDSym(3);

  auto xRMS = hVertexX->GetRMS();
  auto yRMS = hVertexY->GetRMS();
  auto zRMS = hVertexZ->GetRMS();

  hVertexX->SetAxisRange(medianX[0] - nSigmacut * xRMS, medianX[0] + nSigmacut * xRMS, "X");
  hVertexY->SetAxisRange(medianY[0] - nSigmacut * yRMS, medianY[0] + nSigmacut * yRMS, "X");
  hVertexZ->SetAxisRange(medianZ[0] - nSigmacut * zRMS, medianZ[0] + nSigmacut * zRMS, "X");

  vertexCov[0][1] = vertexCov[1][0] = 0;
  vertexCov[0][2] = vertexCov[2][0] = 0;
  vertexCov[1][2] = vertexCov[2][1] = 0;
  vertexCov[0][0] = hVertexX->GetRMS() * hVertexX->GetRMS() / double(entries);
  vertexCov[1][1] = hVertexY->GetRMS() * hVertexY->GetRMS() / double(entries);
  vertexCov[2][2] = hVertexZ->GetRMS() * hVertexZ->GetRMS() / double(entries);

  vertexSize[0][1] = vertexSize[1][0] = 0;
  vertexSize[0][2] = vertexSize[2][0] = 0;
  vertexSize[1][2] = vertexSize[2][1] = 0;
  vertexSize[0][0] = hVertexX->GetRMS() * hVertexX->GetRMS() - hVarX->GetMean();
  vertexSize[1][1] = hVertexY->GetRMS() * hVertexY->GetRMS() - hVarY->GetMean();
  vertexSize[2][2] = hVertexZ->GetRMS() * hVertexZ->GetRMS() - hVarZ->GetMean();

  // if size y NAN, set it to expected size for beta_star_y = 1 mm:
  if (vertexSize[1][1] < 0) vertexSize[1][1] = 0.3 * 1e-4 * 0.3 * 1e-4;

  auto payload = new BeamSpot();

  payload->setIP(vertexPos, vertexCov);
  payload->setSizeCovMatrix(vertexSize);

  saveCalibration(payload);

  // probably not needed - would trigger re-doing the collection
  //if ( ... too large corrections ... ) return c_Iterate;

  return c_OK;
}
