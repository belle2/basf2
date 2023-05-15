/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclEdgeCollector/eclEdgeCollectorModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <TH2F.h>
#include <TMath.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclEdgeCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

eclEdgeCollectorModule::eclEdgeCollectorModule() : CalibrationCollectorModule(), m_ECLCrystalOffsetTheta("ECLCrystalOffsetTheta"),
  m_ECLCrystalOffsetPhi("ECLCrystalOffsetPhi")
{
  /** Set module properties */
  setDescription("Obtain location of crystal edges from geometry and ECLCrystalOffsetTheta and ECLCrystalOffsetPhi payloads.");
}

void eclEdgeCollectorModule::prepare()
{
  //..Define histograms
  auto eclCrystalX = new TH1F("eclCrystalX", "x of each crystal;cellID;x (cm)", ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalX", eclCrystalX);

  auto eclCrystalY = new TH1F("eclCrystalY", "y of each crystal;cellID;y(cm)", ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalY", eclCrystalY);

  auto eclCrystalZ = new TH1F("eclCrystalZ", "z of each crystal;cellID;z (cm)", ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalZ", eclCrystalZ);

  auto eclCrystalR = new TH1F("eclCrystalR", "R (3d) of each crystal;cellID;R (cm)", ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalR", eclCrystalR);

  auto eclCrystalTheta = new TH1F("eclCrystalTheta", "theta of each crystal location;cellID;theta (rad)",
                                  ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalTheta", eclCrystalTheta);

  auto eclCrystalPhi = new TH1F("eclCrystalPhi", "phi of each crystal location;cellID;phi (rad)", ECLElementNumbers::c_NCrystals, 1,
                                8737);
  registerObject<TH1F>("eclCrystalPhi", eclCrystalPhi);

  auto eclCrystalDirTheta = new TH1F("eclCrystalDirTheta", "theta of each crystal direction;cellID;theta (rad)",
                                     ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalDirTheta", eclCrystalDirTheta);

  auto eclCrystalDirPhi = new TH1F("eclCrystalDirPhi", "phi of each crystal direction;cellID;phi (rad)",
                                   ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalDirPhi", eclCrystalDirPhi);

  auto eclCrystalEdgeTheta = new TH1F("eclCrystalEdgeTheta", "lower edge of each crystal in theta;cellID;theta (rad)",
                                      ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalEdgeTheta", eclCrystalEdgeTheta);

  auto eclCrystalEdgePhi = new TH1F("eclCrystalEdgePhi", "lower edge of each crystal in phi;cellID;phi (rad)",
                                    ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclCrystalEdgePhi", eclCrystalEdgePhi);

  auto eclEdgeCounter = new TH1F("eclEdgeCounter", "dummy histogram to count collector calls;cellID;calls",
                                 ECLElementNumbers::c_NCrystals, 1, 8737);
  registerObject<TH1F>("eclEdgeCounter", eclEdgeCounter);

}

void eclEdgeCollectorModule::collect()
{
  //..First time through, store the ECL geometry.
  if (storeGeo) {
    storeGeo = false;

    //..Read in payloads of offsets between crystal centers and edges
    std::vector<float> offsetTheta;
    if (m_ECLCrystalOffsetTheta.hasChanged()) {offsetTheta = m_ECLCrystalOffsetTheta->getCalibVector();}
    std::vector<float> offsetPhi;
    if (m_ECLCrystalOffsetPhi.hasChanged()) {offsetPhi = m_ECLCrystalOffsetPhi->getCalibVector();}

    //..ECL geometry
    Belle2::ECL::ECLGeometryPar* eclGeometry = Belle2::ECL::ECLGeometryPar::Instance();
    for (int cellID = 1; cellID <= ECLElementNumbers::c_NCrystals; cellID++) {
      ROOT::Math::XYZVector crystalPos = eclGeometry->GetCrystalPos(cellID - 1);
      ROOT::Math::XYZVector crystalDirection = eclGeometry->GetCrystalVec(cellID - 1);
      getObjectPtr<TH1F>("eclCrystalX")->SetBinContent(cellID, crystalPos.X());
      getObjectPtr<TH1F>("eclCrystalY")->SetBinContent(cellID, crystalPos.Y());
      getObjectPtr<TH1F>("eclCrystalZ")->SetBinContent(cellID, crystalPos.Z());
      getObjectPtr<TH1F>("eclCrystalR")->SetBinContent(cellID, crystalPos.R());
      getObjectPtr<TH1F>("eclCrystalTheta")->SetBinContent(cellID, crystalPos.Theta());
      getObjectPtr<TH1F>("eclCrystalPhi")->SetBinContent(cellID, crystalPos.Phi());
      getObjectPtr<TH1F>("eclCrystalDirTheta")->SetBinContent(cellID, crystalDirection.Theta());
      getObjectPtr<TH1F>("eclCrystalDirPhi")->SetBinContent(cellID, crystalDirection.Phi());
      getObjectPtr<TH1F>("eclEdgeCounter")->SetBinContent(cellID, 1);

      //..Subtract the offset between crystal center and edge to get edge locations
      float thetaEdge = crystalPos.Theta() - offsetTheta[cellID - 1];
      getObjectPtr<TH1F>("eclCrystalEdgeTheta")->SetBinContent(cellID, thetaEdge);

      float phiEdge = crystalPos.Phi();
      if (!offsetPhi.empty()) phiEdge -= offsetPhi[cellID - 1];
      if (phiEdge < -TMath::Pi()) {phiEdge += 2 * TMath::Pi();}
      getObjectPtr<TH1F>("eclCrystalEdgePhi")->SetBinContent(cellID, phiEdge);

      //..Print out some for debugging purposes
      if (cellID % 1000 == 0) {
        B2INFO("cellID " << cellID << ": theta " << crystalPos.Theta() << " phi " <<
               crystalPos.Phi() << " R " << crystalPos.R() << " thetaEdge " << thetaEdge << " phiEdge " << phiEdge);
      }
    }
  }
}


