/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdDQM/SVDDQMHitTimeModule.h>
#include <framework/core/HistoModule.h>

using namespace Belle2;

REG_MODULE(SVDDQMHitTime)

//---------------------------------
SVDDQMHitTimeModule::SVDDQMHitTimeModule(): HistoModule()
{
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing
  setDescription("Make data quality monitoring plots for SVD Hit Time for bhabha, mu mu, and hadron samples seeded by different trigger times.(ECL, CDC)");
}

//---------------------------------
SVDDQMHitTimeModule::~SVDDQMHitTimeModule() { }


//---------------------------------
void SVDDQMHitTimeModule::defineHisto()
{

  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("SVDDQMHitTime")->cd();

  int nBins = 400 ;
  double minT0 = -80 ;
  double maxT0 =  120 ;

  m_l3v_bhabha_L1_ECLTRG = new TH1F("SVDTime_L3V_bhabha_ECLTRG",
                                    "SVD L3 V-Side Cluster Time : bhabhas, ECLTRG time",
                                    nBins, minT0, maxT0);
  m_l3vEvtT0_bhabha_L1_ECLTRG = new TH1F("SVDTimeEvtT0_L3V_bhabha_ECLTRG",
                                         "SVD L3 V-Side Cluster Time - EventT0 : bhabhas, ECLTRG time",
                                         nBins, minT0, maxT0);

  m_l3v_hadron_L1_ECLTRG = new TH1F("SVDTime_L3V_hadron_ECLTRG",
                                    "SVD L3 V-Side Cluster Time : hadrons, ECLTRG time",
                                    nBins, minT0, maxT0);
  m_l3vEvtT0_hadron_L1_ECLTRG = new TH1F("SVDTimeEvtT0_L3V_hadron_ECLTRG",
                                         "SVD L3 V-Side Cluster Time - EventT0 : hadrons, ECLTRG time",
                                         nBins, minT0, maxT0);


  m_l3v_mumu_L1_ECLTRG = new TH1F("SVDTime_L3V_mumu_ECLTRG",
                                  "SVD L3 V-Side Cluster Time : mumus, ECLTRG time",
                                  nBins, minT0, maxT0);
  m_l3vEvtT0_mumu_L1_ECLTRG = new TH1F("SVDTimeEvtT0_L3V_mumu_ECLTRG",
                                       "SVD L3 V-Side Cluster Time - EventT0 : mumus, ECLTRG time",
                                       nBins, minT0, maxT0);

  m_l3v_bhabha_L1_CDCTRG = new TH1F("SVDTime_L3V_bhabha_CDCTRG",
                                    "SVD L3 V-Side Cluster Time : bhabhas, CDCTRG time",
                                    nBins, minT0, maxT0);
  m_l3vEvtT0_bhabha_L1_CDCTRG = new TH1F("SVDTimeEvtT0_L3V_bhabha_CDCTRG",
                                         "SVD L3 V-Side Cluster Time - EventT0 : bhabhas, CDCTRG time",
                                         nBins, minT0, maxT0);

  m_l3v_hadron_L1_CDCTRG = new TH1F("SVDTime_L3V_hadron_CDCTRG",
                                    "SVD L3 V-Side Cluster Time : hadrons, CDCTRG time",
                                    nBins, minT0, maxT0);
  m_l3vEvtT0_hadron_L1_CDCTRG = new TH1F("SVDTimeEvtT0_L3V_hadron_CDCTRG",
                                         "SVD L3 V-Side Cluster Time - EventT0 : hadrons, CDCTRG time",
                                         nBins, minT0, maxT0);


  m_l3v_mumu_L1_CDCTRG = new TH1F("SVDTime_L3V_mumu_CDCTRG",
                                  "SVD L3 V-Side Cluster Time : mumus, CDCTRG time",
                                  nBins, minT0, maxT0);
  m_l3vEvtT0_mumu_L1_CDCTRG = new TH1F("SVDTimeEvtT0_L3V_mumu_CDCTRG",
                                       "SVD L3 V-Side Cluster Time - EventT0 : mumus, CDCTRG time",
                                       nBins, minT0, maxT0);


  oldDir->cd();

}


//---------------------------------
void SVDDQMHitTimeModule::initialize()
{

  if (!m_clusters.isOptional()) {
    B2WARNING("Missing SVDClusters, SVDDQMHitTime is skipped.");
    return;
  }

  if (!m_TrgResult.isOptional()) {
    B2WARNING("Missing TRGSummary, SVDDQMHitTime is skipped.");
    return;
  }
  m_TrgResult.isRequired();
  m_eventT0.isOptional();
  m_svdEventInfo.isOptional();
  m_clusters.isRequired();

  REG_HISTOGRAM

}



//---------------------------------
void SVDDQMHitTimeModule::beginRun()
{
  if (!m_clusters.isOptional()) {
    B2WARNING("Missing SVDClusters, SVDDQMHitTime is skipped.");
    return;
  }

  m_l3v_bhabha_L1_ECLTRG->Reset();
  m_l3vEvtT0_bhabha_L1_ECLTRG->Reset();
  m_l3v_hadron_L1_ECLTRG->Reset();
  m_l3vEvtT0_hadron_L1_ECLTRG->Reset();
  m_l3v_mumu_L1_ECLTRG->Reset();
  m_l3vEvtT0_mumu_L1_ECLTRG->Reset();

  m_l3v_bhabha_L1_CDCTRG->Reset();
  m_l3vEvtT0_bhabha_L1_CDCTRG->Reset();
  m_l3v_hadron_L1_CDCTRG->Reset();
  m_l3vEvtT0_hadron_L1_CDCTRG->Reset();
  m_l3v_mumu_L1_CDCTRG->Reset();
  m_l3vEvtT0_mumu_L1_CDCTRG->Reset();

}


//---------------------------------
void SVDDQMHitTimeModule::event()
{
  if (!m_objTrgSummary.isValid()) {
    B2WARNING("TRGSummary object not available but required to indicate which detector provided the L1 trigger time");
    return;
  } else {
    m_L1TimingSrc = m_objTrgSummary->getTimType();
  }

  bool Is_ECL_L1TriggerSource = false ;
  bool Is_CDC_L1TriggerSource = false ;
  if (m_L1TimingSrc == 0) {            // for L1 timing source is "ecl trigger"
    Is_ECL_L1TriggerSource = true ;
  } else if (m_L1TimingSrc == 3) {     // for L1 timing source is "cdc trigger"
    Is_CDC_L1TriggerSource = true ;
  }
  // else if(m_L1TimingSrc==5){  // for L1 timing source is "delayed Bhabha" }
  B2DEBUG(20, "Is_ECL_L1TriggerSource = " << Is_ECL_L1TriggerSource) ;
  B2DEBUG(20, "Is_CDC_L1TriggerSource= " << Is_CDC_L1TriggerSource) ;


  if (!m_TrgResult.isValid()) {
    B2WARNING("SoftwareTriggerResult object not available but require to select bhabha/mumu/hadron events for this module");
    return;
  }

  const std::map<std::string, int>& fresults = m_TrgResult->getResults();
  if ((fresults.find("software_trigger_cut&skim&accept_bhabha") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_mumu_2trk") == fresults.end())   ||
      (fresults.find("software_trigger_cut&skim&accept_hadron") == fresults.end())) {
    B2WARNING("SVDDQMHitTimeModule: Can't find required bhabha or mumu or hadron trigger identifier");
    return;
  }



  // determine if the event was part of the hadron skim or bhabha skim or mumu skim
  const bool IsEvtAcceptedBhabha = (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha") ==
                                    SoftwareTriggerCutResult::c_accept);
  const bool IsEvtAcceptedHadron = (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron") ==
                                    SoftwareTriggerCutResult::c_accept);
  const bool IsEvtAcceptedMumu = (m_TrgResult->getResult("software_trigger_cut&skim&accept_mumu_2trk") ==
                                  SoftwareTriggerCutResult::c_accept);


  B2DEBUG(20, "bhabha trigger result = " << static_cast<std::underlying_type<SoftwareTriggerCutResult>::type>
          (m_TrgResult->getResult("software_trigger_cut&skim&accept_bhabha"))) ;
  B2DEBUG(20, "hadron trigger result = " << static_cast<std::underlying_type<SoftwareTriggerCutResult>::type>
          (m_TrgResult->getResult("software_trigger_cut&skim&accept_hadron"))) ;
  B2DEBUG(20, "mu mu trigger result = "  << static_cast<std::underlying_type<SoftwareTriggerCutResult>::type>
          (m_TrgResult->getResult("software_trigger_cut&skim&accept_mumu_2trk"))) ;
  B2DEBUG(20, "bhabha trigger comparison bool = " << IsEvtAcceptedBhabha) ;
  B2DEBUG(20, "hadron trigger comparison bool = " << IsEvtAcceptedHadron) ;
  B2DEBUG(20, "mumu trigger comparison bool = "   << IsEvtAcceptedMumu) ;


  // get EventT0 if present and valid
  double eventT0 = -1000;
  if (m_eventT0.isOptional())
    if (m_eventT0.isValid())
      if (m_eventT0->hasEventT0())
        eventT0 = m_eventT0->getEventT0();


  // eventT0 is synchronized with SVD reference frame
  eventT0 = eventT0 - 7.8625 * (3 - m_svdEventInfo->getModeByte().getTriggerBin());

  //loop on clusters
  for (const SVDCluster& cluster : m_clusters) {

    //skip all non-L3 clusters
    if (cluster.getSensorID().getLayerNumber() != 3) continue;
    //skip all U-side clusters
    if (cluster.isUCluster()) continue;

    double time = cluster.getClsTime();

    // Fill the plots that used the ECL trigger as the L1 timing source
    if (Is_ECL_L1TriggerSource) {
      if (IsEvtAcceptedBhabha) {
        m_l3v_bhabha_L1_ECLTRG->Fill(time);
        m_l3vEvtT0_bhabha_L1_ECLTRG->Fill(time - eventT0);
      }
      if (IsEvtAcceptedHadron) {
        m_l3v_hadron_L1_ECLTRG->Fill(time);
        m_l3vEvtT0_hadron_L1_ECLTRG->Fill(time - eventT0);
      }
      if (IsEvtAcceptedMumu) {
        m_l3v_mumu_L1_ECLTRG->Fill(time);
        m_l3vEvtT0_mumu_L1_ECLTRG->Fill(time - eventT0);
      }
    }

    // Fill the plots that used the CDC trigger as the L1 timing source
    else if (Is_CDC_L1TriggerSource) {
      if (IsEvtAcceptedBhabha) {
        m_l3v_bhabha_L1_CDCTRG->Fill(time);
        m_l3vEvtT0_bhabha_L1_CDCTRG->Fill(time - eventT0);
      }
      if (IsEvtAcceptedHadron) {
        m_l3v_hadron_L1_CDCTRG->Fill(time);
        m_l3vEvtT0_hadron_L1_CDCTRG->Fill(time - eventT0);
      }
      if (IsEvtAcceptedMumu) {
        m_l3v_mumu_L1_CDCTRG->Fill(time);
        m_l3vEvtT0_mumu_L1_CDCTRG->Fill(time - eventT0);
      }
    }

    B2DEBUG(20, "eventT0 = " << eventT0 << " ns" << ", SVD HitTime = " << time << " ns") ;

  } //close loop on clusters
}


