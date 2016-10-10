/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/GBLdiagnostics/GBLdiagnosticsModule.h>
#include <genfit/WireMeasurement.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <genfit/PlanarMeasurement.h>
#include <genfit/WireMeasurementNew.h>
#include <vxd/dataobjects/VxdID.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <testbeam/vxd/reconstruction/TelRecoHit.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <alignment/reconstruction/BKLMRecoHit.h>

using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(GBLdiagnostics)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GBLdiagnosticsModule::GBLdiagnosticsModule() : Module(), ntracks(0)
{
  // Set module properties
  setDescription("Loops over genfit::Tracks fitted by GBLFitter and produces ROOT file with tree(s) with fit result data.");

  // Parameter definitions
  addParam("tracks", m_tracksColName, "Name of collection with genfit::Track fitted by GBL", std::string(""));
  addParam("rootFile", m_rootFile, "Name of output ROOT file with trees with fit data", std::string("gbl_diag.root"));

}

void GBLdiagnosticsModule::initialize()
{


  m_file = new TFile(m_rootFile.c_str(), "RECREATE");
  m_tree = new TTree("points", "Point level data");
  m_trackTree = new TTree("tracks", "Track level data");

  //Due to laziness, define some macros to create branches with less typing
#define ADDBRANCH__(name,var,type)   m_tree->Branch(#name,&m_info.var,#type)
#define ADDBARRAY__(name,var,n,type) ADDBRANCH__(name,var,name[n]/type)
#define ADDBRANCH(x,type)            ADDBRANCH__(x,x,x/type)
#define ADDBARRAY(x,n,type)          ADDBARRAY__(x,x,n,type)

#define ADDTBRANCH__(name,var,type)   m_trackTree->Branch(#name,&m_tinfo.var,#type)
#define ADDTBARRAY__(name,var,n,type) ADDTBRANCH__(name,var,name[n]/type)
#define ADDTBRANCH(x,type)            ADDTBRANCH__(x,x,x/type)
#define ADDTBARRAY(x,n,type)          ADDTBARRAY__(x,x,n,type)

  //and create all branches
  ADDBRANCH(sensorID, I);
  ADDBRANCH(pdg, I);
  ADDBRANCH(layer, I);
  ADDBRANCH(ladder, I);
  ADDBRANCH(sensor, I);
  ADDBRANCH(wire, I);
  ADDBRANCH(thinScat, D);
  ADDBRANCH(clsSize, I);

  ADDBRANCH(clsSizeU, I);
  ADDBRANCH(clsSizeV, I);
  ADDBRANCH(id, I);
  ADDBRANCH(trackid, I);
  ADDBRANCH(run, I);
  ADDBRANCH(timestamp, L);

  ADDBRANCH(ndf, I);
  ADDBRANCH(chi2, D);
  ADDBRANCH(pval, D);
  ADDBRANCH(mom, D);
  ADDBRANCH(phi, D);
  ADDBRANCH(theta, D);

  ADDBARRAY(predB, 5, D);
  ADDBARRAY(predF, 5, D);
  ADDBARRAY(predBglo, 6, D);
  ADDBARRAY(predFglo, 6, D);
  //ADDBARRAY(covB, 25, D);
  //ADDBARRAY(covF, 25, D);
  //ADDBARRAY(covBglo, 36, D);
  //ADDBARRAY(covFglo, 36, D);
  ADDBARRAY(res, 2, D);
  ADDBARRAY(resErr, 2, D);
  ADDBARRAY(kink, 2, D);
  ADDBARRAY(kinkErr, 2, D);
  ADDBARRAY(helix, 5, D);

  ADDTBRANCH(pdg, I);
  ADDTBRANCH(ndf, I);
  ADDTBRANCH(chi2, D);
  ADDTBRANCH(pval, D);
  ADDTBRANCH(npoints, I);
  ADDTBRANCH(nscat, I);
  ADDTBRANCH(nmeas, I);
  ADDTBARRAY(pred, 6, D);
  ADDTBARRAY(helix, 5, D);

  ADDTBRANCH(id, I);
  ADDTBRANCH(run, I);
  ADDTBRANCH(timestamp, L);
}

void GBLdiagnosticsModule::event()
{
  StoreArray<genfit::Track> tracks(m_tracksColName);
  StoreObjPtr<EventMetaData> emd;
  using namespace genfit;

  for (auto& track : tracks) {
    FitStatus* fs = dynamic_cast<FitStatus*>(track.getFitStatus());
    if (!fs)
      continue;

    //Find the point of closest approach of the track to the origin
    TVector3 pos(0., 0., 0.); //origin
    TVector3 lineDirection(0, 0, 1); // beam axis
    TVector3 poca(0., 0., 0.); //point of closest approach
    TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach
    TMatrixDSym cov(6);

    genfit::MeasuredStateOnPlane mop = track.getFittedState();
    try {
      mop.extrapolateToLine(pos, lineDirection);
    } catch (...) {
      continue;
    }
    mop.getPosMomCov(poca, dirInPoca, cov);
    dirInPoca *= 1. / dirInPoca.Mag();
    //int charge = fs->getCharge();
    //double pVal = fs->getPVal();
    //float bField = 1.5; //TODO: get magnetic field from genfit

    //track.insertMeasurement(new genfit::FullMeasurement);

    //Now calculate the parameters for helix parametrisation to fill the Track objects

    //calculate transverse momentum
    double pt = sqrt(mop.getMom().x() * mop.getMom().x() + mop.getMom().y() * mop.getMom().y());

    //determine angle phi for perigee parametrisation, distributed from -pi to pi
    double phi = atan2(dirInPoca.y() , dirInPoca.x());

    //determine d0 sign for perigee parametrization
    //double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x() + poca.y() * dirInPoca.y());

    //coefficient to illiminate the B field and get the 'pure' curvature
    double alpha = 1 / (1.5 * 0.00299792458);

    m_tinfo.helix[0] = poca.x() * dirInPoca.y() - poca.y() *
                       dirInPoca.x();//d0Sign * sqrt(poca.x() * poca.x() + poca.y() * poca.y()); // d0
    m_tinfo.helix[1] = phi; //phi
    m_tinfo.helix[2] = (fs->getCharge() / (pt * alpha)); //omega
    m_tinfo.helix[3] = poca.z(); // z0
    m_tinfo.helix[4] = dirInPoca.z() / (sqrt(dirInPoca.x() * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())); //cot theta

    // named here d0, phi, omega, z0, cotTheta together with an angle
    m_tinfo.ndf = fs->getNdf();
    m_tinfo.pval = fs->getPVal();
    m_tinfo.nmeas = track.getNumPointsWithMeasurement();
    m_tinfo.npoints = track.getNumPoints();
    m_tinfo.chi2 = fs->getChi2();
    m_tinfo.pdg = track.getCardinalRep()->getPDG();

    for (int i = 0; i < 6; i++)
      m_tinfo.pred[i] = track.getFittedState().get6DState()(i);

    int nscat = 0;

    for (unsigned int ip = 0; ip < track.getNumPoints(); ip++) {
      TrackPoint* point = track.getPoint(ip);
      AbsFitterInfo* fi = dynamic_cast<AbsFitterInfo*>(point->getFitterInfo());
      if (!fi)
        continue;

      SVDRecoHit* svd = 0;
      PXDRecoHit* pxd = 0;
      TelRecoHit* tel = 0;
      SVDRecoHit2D* svd2D = 0;

      m_info.pdg = track.getCardinalRep()->getPDG();

      if (fi->hasMeasurements()) {
        svd2D = dynamic_cast<SVDRecoHit2D*>(point->getRawMeasurement(0));
        svd = dynamic_cast<SVDRecoHit*>(point->getRawMeasurement(0));
        pxd = dynamic_cast<PXDRecoHit*>(point->getRawMeasurement(0));
        tel = dynamic_cast<TelRecoHit*>(point->getRawMeasurement(0));
        CDCRecoHit* cdchit = dynamic_cast<CDCRecoHit*>(point->getRawMeasurement(0));
        BKLMRecoHit* bklmhit = dynamic_cast<BKLMRecoHit*>(point->getRawMeasurement(0));

        m_info.helix[0] = poca.x() * dirInPoca.y() - poca.y() *
                          dirInPoca.x();//d0Sign * sqrt(poca.x() * poca.x() + poca.y() * poca.y()); // d0
        m_info.helix[1] = phi; //phi
        m_info.helix[2] = (fs->getCharge() / (pt * alpha)); //omega
        m_info.helix[3] = poca.z(); // z0
        m_info.helix[4] = dirInPoca.z() / (sqrt(dirInPoca.x() * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())); //cot theta

        m_info.sensorID = 0;
        m_info.ladder = 0;
        m_info.layer = 0;
        m_info.sensor = 0;
        m_info.wire = 0.;

        if (pxd) {
          m_info.sensorID = (int) pxd->getSensorID();
          Belle2::VxdID id(m_info.sensorID);
          m_info.ladder = id.getLadderNumber();
          m_info.layer = id.getLayerNumber();
          m_info.sensor = id.getSensorNumber();

          if (pxd->getCluster()) {
            m_info.clsSizeU = pxd->getCluster()->getUSize();
            m_info.clsSizeV = pxd->getCluster()->getVSize();
            m_info.clsSize = pxd->getCluster()->getSize();
          }

          m_info.res[0] = fi->getResidual(0, false).getState()(0);
          m_info.res[1] = fi->getResidual(0, false).getState()(1);
          m_info.resErr[0] = fi->getResidual(0, false).getCov()(0, 0);
          m_info.resErr[1] = fi->getResidual(0, false).getCov()(1, 1);
        }

        if (tel) {
          m_info.sensorID = (int) tel->getSensorID();
          Belle2::VxdID id(m_info.sensorID);
          m_info.ladder = id.getLadderNumber();
          m_info.layer = id.getLayerNumber();
          m_info.sensor = id.getSensorNumber();

          if (tel->getCluster()) {
            m_info.clsSizeU = tel->getCluster()->getUSize();
            m_info.clsSizeV = tel->getCluster()->getVSize();
            m_info.clsSize = tel->getCluster()->getSize();
          }

          m_info.res[0] = fi->getResidual(0, false).getState()(0);
          m_info.res[1] = fi->getResidual(0, false).getState()(1);
          m_info.resErr[0] = fi->getResidual(0, false).getCov()(0, 0);
          m_info.resErr[1] = fi->getResidual(0, false).getCov()(1, 1);
        }

        if (svd) {
          m_info.sensorID = (int) svd->getSensorID();
          Belle2::VxdID id(m_info.sensorID);
          m_info.ladder = id.getLadderNumber();
          m_info.layer = id.getLayerNumber();
          m_info.sensor = id.getSensorNumber();
          if (svd->getCluster())
            m_info.clsSize = svd->getCluster()->getSize();

          m_info.res[0] = fi->getResidual(0, false).getState()(0);
          m_info.res[1] = 0.;
          m_info.resErr[0] = fi->getResidual(0, false).getCov()(0, 0);
          m_info.resErr[1] = 0.;

        }
        if (svd2D) {
          m_info.sensorID = (int) svd2D->getSensorID();
          Belle2::VxdID id(m_info.sensorID);
          m_info.ladder = id.getLadderNumber();
          m_info.layer = id.getLayerNumber();
          m_info.sensor = id.getSensorNumber();
          if (svd2D->getUCluster() && svd2D->getVCluster()) {
            m_info.clsSizeU = svd2D->getUCluster()->getSize();
            m_info.clsSizeV = svd2D->getVCluster()->getSize();
            m_info.clsSize = m_info.clsSizeU + m_info.clsSizeV;
          }

          m_info.res[0] = fi->getResidual(0, false).getState()(0);
          m_info.res[1] = fi->getResidual(0, false).getState()(1);
          m_info.resErr[0] = fi->getResidual(0, false).getCov()(0, 0);
          m_info.resErr[1] = fi->getResidual(0, false).getCov()(1, 1);
        }

        if (cdchit) {
          m_info.sensorID = 0;
          m_info.ladder = 0;
          m_info.layer = 0;
          m_info.sensor = 0;
          m_info.wire = cdchit->getWireID();

          m_info.res[1] = 0.;
          m_info.res[0] = fi->getResidual(0, false).getState()(0);
          m_info.resErr[1] = 0.;
          m_info.resErr[0] = fi->getResidual(0, false).getCov()(0, 0);
        }

        if (bklmhit) {
          m_info.sensorID = 0;
          //m_info.ladder = bklmhit->getBKLMHit2d()->getSector();
          //m_info.layer = 100 + bklmhit->getBKLMHit2d()->getLayer();
          //m_info.sensor = bklmhit->getBKLMHit2d()->isForward() ? 1 : 0;
          m_info.wire = 0;

          m_info.res[1] = fi->getResidual(0, false).getState()(1);
          m_info.res[0] = fi->getResidual(0, false).getState()(0);
          m_info.resErr[1] = fi->getResidual(0, false).getCov()(1, 1);
          m_info.resErr[0] = fi->getResidual(0, false).getCov()(0, 0);
        }

      } else {
        //pure scatterrer
        m_info.sensorID = - m_info.sensorID;
        m_info.ladder = - m_info.ladder;
        m_info.layer = - m_info.layer;
        m_info.sensor = - m_info.sensor;
        m_info.wire = - m_info.wire;
      }

      if (point->hasThinScatterer()) {

        nscat++;
        m_info.thinScat = point->getMaterialInfo()->getMaterial().getDensity();
        if (auto gblfi = dynamic_cast<GblFitterInfo*>(fi)) {
          m_info.kink[0] = gblfi->getKink().getState()(0);
          m_info.kink[1] = gblfi->getKink().getState()(1);
          m_info.kinkErr[0] = gblfi->getKink().getCov()(0, 0);
          m_info.kinkErr[1] = gblfi->getKink().getCov()(1, 1);
        } else {
          m_info.kink[0] = 0.;
          m_info.kink[1] = 0.;
          m_info.kinkErr[0] = 0.;
          m_info.kinkErr[1] = 0.;
        }


      } else {
        m_info.thinScat = 0.;
      }


      for (int i = 0; i < 5; i++) {
        m_info.predB[i] = fi->getFittedState(false).getState()(i);
        m_info.predF[i] = fi->getFittedState(true).getState()(i);
        /*
        for (int j = 0; j < 5; j++) {
          m_info.covB[5 * i + j] = fi->getFittedState(false).getCov()(i, j);
          m_info.covF[5 * i + j] = fi->getFittedState(true).getCov()(i, j);

        }
        */
      }
      for (int i = 0; i < 6; i++) {
        m_info.predBglo[i] = fi->getFittedState(false).get6DState()(i);
        m_info.predFglo[i] = fi->getFittedState(true).get6DState()(i);
        /*
         *
        for (int j = 0; j < 5; j++) {
          m_info.covBglo[5 * i + j] = fi->getFittedState(false).get6DCov()(i, j);
          m_info.covFglo[5 * i + j] = fi->getFittedState(true).get6DCov()(i, j);

        }
        */

      }
      auto mom = fi->getFittedState(true).getMom();
      double px = mom[0];
      double py = mom[1];
      double pz = mom[2];
      double phi = atan2(py, px) * TMath::RadToDeg();
      if (phi < 0.)
        phi += 360.;

      double pxy = sqrt(px * px + py * py);
      double theta = atan2(pxy, pz) * TMath::RadToDeg();
      if (theta < 0.)
        theta += 360.;

      m_info.phi = phi;
      m_info.theta = theta;
      m_info.mom = sqrt(pxy * pxy + pz * pz);
      m_info.run = emd->getRun();
      m_info.timestamp = emd->getTime();
      m_info.id = ip;
      m_info.trackid = ntracks;
      m_info.ndf = m_tinfo.ndf;
      m_info.chi2 = m_tinfo.chi2;
      m_info.pval = m_tinfo.pval;
      m_tree->Fill();
    }
    m_tinfo.nscat = nscat;
    m_tinfo.id = ntracks;
    m_tinfo.run = emd->getRun();
    m_tinfo.timestamp = emd->getTime();
    m_trackTree->Fill();

    ntracks++;
  }
}

void GBLdiagnosticsModule::terminate()
{
  m_file->cd();
  m_tree->Write();
  m_trackTree->Write();
  m_file->Close();
}


