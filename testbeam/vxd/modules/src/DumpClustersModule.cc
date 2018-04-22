/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/DumpClustersModule.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DumpClusters)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DumpClustersModule::DumpClustersModule() : HistoModule()
{
  // Set module properties
  setDescription("Dumps information on SVD and PXD spacepoints into a root file which can be used for analysis outside of basf2");

  // Parameter definitions
  addParam("SVDClusters", m_svdclusters, "name of store array with svd clusters", std::string(""));
  addParam("PXDClusters", m_pxdclusters, "name of store array with pxd clusters", std::string(""));
  addParam("TelClusters", m_telclusters, "name of store array with tel clusters", std::string(""));
  addParam("PXDDigits", m_pxddigits, "name of store array with pxd digits", std::string(""));
  addParam("SVDDigits", m_svddigits , "name of store array with svd digits", std::string(""));
  addParam("TelDigits", m_teldigits , "name of store array with tel digits", std::string(""));


}

DumpClustersModule::~DumpClustersModule()
{
}

void DumpClustersModule::initialize()
{
  //calls the define histogram function
  REG_HISTOGRAM

  // FTSW optional for now as MC has no FTSW!
  StoreArray<RawFTSW> RawFTSW("");  RawFTSW.isOptional();
  // Cluster arrays
  StoreArray<SVDCluster> SVDCluster(m_svdclusters);  SVDCluster.isOptional();
  StoreArray<PXDCluster> PXDCluster(m_pxdclusters);  PXDCluster.isOptional();
  StoreArray<TelCluster> TelCluster(m_telclusters);  TelCluster.isOptional();

  // PXD Digit arrays
  //StoreArray<PXDDigit>::optional(m_pxddigits);
  StoreArray<PXDDigit> storePXDDigits(m_pxddigits);
  storePXDDigits.isRequired();

  // SVD Digit arrays
  StoreArray<SVDDigit> storeSVDDigits(m_svddigits);
  storeSVDDigits.isRequired();

  // Tel Digit arrays
  StoreArray<TelDigit> storeTelDigits(m_svddigits);
  storeTelDigits.isRequired();
}

void DumpClustersModule::beginRun()
{
}

void DumpClustersModule::event()
{
  //set the default values
  // Clusters
  m_posu.clear();
  m_posv.clear();
  m_erru.clear();
  m_errv.clear();
  m_rho.clear();
  m_id.clear();
  m_isU.clear();
  m_time = -999;
  m_runnumber = -999;
  m_eventnumber = -999;
  m_subrunnumber = -999;
  m_experimentnumber = -999;
  m_productionnumber = -999;
  m_tlunumber = -999;

  // PXD digits
  m_col.clear();
  m_row.clear();
  m_value.clear();
  m_id2.clear();

  // SVD digits
  m_IsUStrip.clear();
  m_cell_ID_SVD.clear();
  m_charge_SVD.clear();
  m_id3.clear();

  //currently not working as there is no rawFTSW in the MC!!
  StoreArray<RawFTSW> storeFTSW("");
  //CIRC::tag_type
  int currentTLUTagFromFTSW = static_cast<unsigned short>(storeFTSW[0]->Get15bitTLUTag(0)); //should be an int
  m_tlunumber = currentTLUTagFromFTSW; // this is the tlu tag

  //Print event meta data information
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (!eventMetaDataPtr) {
    B2WARNING("No MetaData found in the event!");
    return;
  }

  //WARNING: possible source of error (the return value is "long long int" which should be the same as "Long64_t")
  m_time = (Long64_t)eventMetaDataPtr->getTime();
  m_runnumber = eventMetaDataPtr->getRun();
  m_eventnumber = eventMetaDataPtr->getEvent();
  m_subrunnumber = eventMetaDataPtr->getSubrun();
  m_experimentnumber = eventMetaDataPtr->getExperiment();
  m_productionnumber = eventMetaDataPtr->getProduction();

  //loop the pxd digits
  StoreArray<PXDDigit> storePXDDigits(m_pxddigits);

  for (PXDDigit& adigit : storePXDDigits) {
    m_id2.push_back(adigit.getSensorID());
    m_col.push_back(adigit.getUCellID());
    m_row.push_back(adigit.getVCellID());
    m_value.push_back(adigit.getCharge());

    B2DEBUG(10, "PXD digit for sensor with ID " << adigit.getSensorID() << " or the corresponding integer: " <<
            (int)adigit.getSensorID());
  }

  //loop the tel digits
  StoreArray<TelDigit> storeTelDigits(m_teldigits);

  for (TelDigit& adigit : storeTelDigits) {
    m_id2.push_back(adigit.getSensorID());
    m_col.push_back(adigit.getUCellID());
    m_row.push_back(adigit.getVCellID());
    m_value.push_back(adigit.getCharge());

    B2DEBUG(10, "Tel digit for sensor with ID " << adigit.getSensorID() << " or the corresponding integer: " <<
            (int)adigit.getSensorID());
  }

  //loop the svd digits
  StoreArray<SVDDigit> storeSVDDigits(m_svddigits);

  for (SVDDigit& adigit : storeSVDDigits) {
    m_IsUStrip.push_back(adigit.isUStrip());
    m_cell_ID_SVD.push_back(adigit.getCellID());
    m_charge_SVD.push_back(adigit.getCharge());
    m_id3.push_back(adigit.getSensorID());

    B2DEBUG(10, "SVD digit for sensor with ID " << adigit.getSensorID() << " or the corresponding integer: " <<
            (int)adigit.getSensorID());
  }


  //loop the pxd clusters
  StoreArray<PXDCluster> pxdclusters(m_pxdclusters);

  for (PXDCluster& acluster : pxdclusters) {
    m_id.push_back(acluster.getSensorID());
    m_posu.push_back(acluster.getU());
    m_posv.push_back(acluster.getV());
    m_erru.push_back(acluster.getUSigma());
    m_errv.push_back(acluster.getVSigma());
    m_rho.push_back(acluster.getRho());
    m_isU.push_back(-1);

    B2DEBUG(10, "PXD cluster for sensor with ID " << acluster.getSensorID() << " or the corresponding integer: " <<
            (int)acluster.getSensorID());
  }

  //loop the svd clusters and do all posssible combinations
  StoreArray<SVDCluster> svdclusters(m_svdclusters);
  for (SVDCluster&   acluster : svdclusters) {
    m_id.push_back(acluster.getSensorID());
    m_rho.push_back(0.0);
    m_isU.push_back((int)acluster.isUCluster());

    if (acluster.isUCluster()) {
      m_posu.push_back(acluster.getPosition());
      m_erru.push_back(acluster.getPositionSigma());
      m_posv.push_back(-9999.);
      m_errv.push_back(-9999.);
    } else {
      m_posv.push_back(acluster.getPosition());
      m_errv.push_back(acluster.getPositionSigma());
      m_posu.push_back(-9999.);
      m_erru.push_back(-9999.);
    }
    B2DEBUG(10, "SVD cluster for sensor with ID " << acluster.getSensorID() << " or the corresponding integer: " <<
            (int)acluster.getSensorID());
    //getClsTime
    //getClsTimeSigma
  }

  //loop the tel clusters
  StoreArray<TelCluster> telclusters(m_telclusters);

  for (TelCluster& acluster : telclusters) {
    m_id.push_back(acluster.getSensorID());
    m_posu.push_back(acluster.getU());
    m_posv.push_back(acluster.getV());
    m_erru.push_back(acluster.getUSigma());
    m_errv.push_back(acluster.getVSigma());
    m_rho.push_back(acluster.getRho());
    m_isU.push_back(-1);

//    B2DEBUG(10,"Tel cluster for sensor with ID " << acluster.getSensorID() << " or the corresponding integer: " << (int)acluster.getSensorID());
  }

  m_tree->Fill();
}

void DumpClustersModule::endRun()
{
}

void DumpClustersModule::terminate()
{
}


void DumpClustersModule::defineHisto()
{
  m_tree = new TTree("SPtree", "tree with stored space points");
  m_tree->Branch("col", &m_col);
  m_tree->Branch("row", &m_row);
  m_tree->Branch("charge", &m_value);
  m_tree->Branch("sensorID_digits", &m_id2);
  m_tree->Branch("posu", &m_posu);
  m_tree->Branch("posv", &m_posv);
  m_tree->Branch("rho", &m_rho);
  m_tree->Branch("erru", &m_erru);
  m_tree->Branch("errv", &m_errv);
  m_tree->Branch("isU", &m_isU);
  m_tree->Branch("sensorID", &m_id);
  m_tree->Branch("TLUnumber", &m_tlunumber, "TLUnumber/I");
  m_tree->Branch("time", &m_time, "time/L");
  m_tree->Branch("runnumber", & m_runnumber, "runnumber/I");
  m_tree->Branch("eventnumber", & m_eventnumber, "eventnumber/I");
  m_tree->Branch("subrunnumber", & m_subrunnumber, "subrunnumber/I");
  m_tree->Branch("experimentnumber", & m_experimentnumber, "experimentnumber/I");
  m_tree->Branch("productionnumber", & m_productionnumber, "productionnumber/I");
  m_tree->Branch("isUStrip_svddigits", & m_IsUStrip);
  m_tree->Branch("cellID_svddigits", & m_cell_ID_SVD);
  m_tree->Branch("charge_svddigits", & m_charge_SVD);
  m_tree->Branch("SensorID_svddigits", & m_id3);
}


//void DumpClustersModule::fill(void * clusters)
//{

//  B2DEBUG(1, "Number of space points found in array: " << spacepoints.getEntries());
//  for( SpacePoint & asp : spacepoints ){
//    m_id.push_back( asp.getVxdID() );
//    m_posx.push_back( asp.x() );
//    m_posy.push_back( asp.y() );
//    m_posz.push_back( asp.z() );
//
//    // from the space point header file: /** return the hitErrors in sigma of the global position ????? */
//    TVector3 err = asp.getPositionError();
//    m_poserrx.push_back( err.X() );
//    m_poserry.push_back( err.Y() );
//    m_poserrz.push_back( err.Z() );
//  }
//}
