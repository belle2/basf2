#include <topcaf/modules/PedestalModule/PedestalModule.h>
#include <topcaf/dataobjects/topFileMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
//#include <framework/conditions/ConditionsService.h>


#include <iostream>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TList.h>
#include <TFile.h>
#include <TClass.h>

using namespace Belle2;
using std::string;

REG_MODULE(Pedestal)

PedestalModule::PedestalModule() : Module()
{
  setDescription("This module is used to create itop pedestal file, see the parameters for various options");

  addParam("mode", m_mode, "Calculate Pedestals - 0 ; Apply Pedestals - 1");

  addParam("conditions", m_conditions,
           "Do not use Conditions Service - 0 ; Use Conditions Service - 1 (write to Conditions if mode==0) ", 0);
  addParam("iovInitialRunID", m_initial_run, "Initial run ID for the interval of validity for these pedestals", string("NULL"));
  addParam("iovFinalRunID", m_final_run, "Final run ID for the interval of validity for these pedestals", string("NULL"));

  addParam("inputFileName", m_in_ped_filename, "Input filename used if mode==1 and conditions==0", string());
  addParam("writeFile", m_writefile, "Do not write file - 0 ; Write Peds to local root file - 1 ", 0);
  addParam("outputFileName", m_out_ped_filename,
           "output filename written if mode==0 and writeFile==1, also used for conditions temporary output.",
           std::string("/tmp/temp_pedestal.root"));
  m_out_ped_file = nullptr;
  m_in_ped_file = nullptr;
}

PedestalModule::~PedestalModule() {}

void PedestalModule::initialize()
{
  if ((m_writefile == 1) || ((m_conditions == 1) && (m_mode == 0))) {
    m_out_ped_file = TFile::Open(m_out_ped_filename.c_str(), "recreate");
    if (!m_out_ped_file) {
      B2FATAL("Could not write output pedestal file.  Aborting.");
    } else {
      m_out_ped_file->Close();
    }
  }
  StoreObjPtr<topFileMetaData> metadata_ptr;
  metadata_ptr.isRequired();
  m_experiment = m_metadata_ptr->getExperiment();
  m_run = m_metadata_ptr->getRun();
}

void PedestalModule::beginRun()
{

  if (m_mode == 1) { // read pedestals
    TList* list;
    if (m_conditions == 1) { // read peds using conditions service
//      B2INFO("Retrieving pedestal data from service.");
      B2INFO("Should retrieve pedestal data from service. Code waiting modification!!!");

      /* FIXME      std::string filename = (ConditionsService::getInstance()->getPayloadFileURL(this));
            m_in_ped_file = TFile::Open(filename.c_str(), "READ");

            B2INFO("Retrieving pedestal data from service. done");
      */
    } else if (m_conditions == 0) { // read peds from local file
      m_in_ped_file = TFile::Open(m_in_ped_filename.c_str(), "READ");
    }

    if (not m_in_ped_file) {
      list = nullptr;
      B2ERROR("Couldn't open input itop pedestal file: " << m_in_ped_filename);
    }  else {
      list = m_in_ped_file->GetListOfKeys();
    }


    /// Now load up the pedestals
    TIter next(list);
    TKey* key;
    while ((key = (TKey*)next())) {
      TClass* cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TProfile")) continue;
      TProfile* profile = (TProfile*)key->ReadObj();
      std::string name_key = profile->GetName();
      topcaf_channel_id_t window_id = strtoull(name_key.c_str(), nullptr, 0);
      m_sample2ped[window_id] = profile;
    }

  }

}

void PedestalModule::event()
{
  //Get Waveform from datastore
  StoreObjPtr<EventHeaderPacket> evthead_ptr;
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();
  evthead_ptr.isRequired();

  if (evtwaves_ptr) {
    for (int c = 0; c < evtwaves_ptr.getEntries(); c++) {
      EventWaveformPacket* evtwave_ptr = evtwaves_ptr[c];

      topcaf_channel_id_t channel_name = evtwave_ptr->GetChannelID() + evtwave_ptr->GetASICWindow();
      if (m_mode == 0) { // Calculate pedestals from waveform
        const std::vector<double> v_samples = evtwave_ptr->GetSamples();
        size_t nsamples = v_samples.size();

        B2DEBUG(1, "PedestalModule::GetChannelID(): " << evtwave_ptr->GetChannelID() << " ASIC Window: " << evtwave_ptr->GetASICWindow());

        TProfile* channel_adc_h = m_sample2ped[channel_name];
        if (!channel_adc_h) {
          std::string s_channel_name = std::to_string(channel_name);
          channel_adc_h = new TProfile(s_channel_name.c_str(),
                                       s_channel_name.c_str(),
                                       nsamples, 0, nsamples);
          m_sample2ped[channel_name] = channel_adc_h;
        }
        for (size_t s = 0; s < nsamples; s++) {
          channel_adc_h->Fill(s, v_samples.at(s));
        }
      } else if (m_mode == 1) { // Apply pedestals to waveform
        //Look up reference pedestal and correct
        TProfile* ped_profile = m_sample2ped[channel_name];
        if (not ped_profile) {
          if (evthead_ptr->GetEventFlag() < 100) {
            B2WARNING("Problem retrieving itop pedestal data for channel " << channel_name << "!!!");
            evthead_ptr->SetFlag(405);
          }
        } else {
          std::vector<double> v_pedcorr_samples;
          std::vector<double> v_samples = evtwave_ptr->GetSamples();
          for (size_t s = 0; s < v_samples.size(); s++) {
            double this_ped = ped_profile->GetBinContent(s + 1);
            double this_sample = v_samples.at(s);
            double corr_sample = this_sample - this_ped;
            v_pedcorr_samples.push_back(corr_sample);
          }
          evtwave_ptr->SetSamples(v_pedcorr_samples);
        }
      }
    }
  }
}

void  PedestalModule::terminate()
{
  //If output requested then save pedestals
  if ((m_writefile == 1) || ((m_conditions == 1) && (m_mode == 0))) {

    //Save Channel sample adc info.
    if (m_conditions == 1) { // Use Conditions Service to save pedestals

      B2INFO("writing itop pedestals using Conditions Service - Payload Tag:" << GetPayloadTag()
             << "\tSubsystem Tag: " << getPackage() << "\tAlgorithm Name: " << getName()
             << "\tVersion: " << GetVersion() << "\tRun_i: " << GetInitialRun() << "\tRun_f: " << GetFinalRun());

      std::string tempFile = m_out_ped_filename;
      // test
      m_out_ped_file = TFile::Open(tempFile.c_str(), "recreate");

      std::string title = "Pedestal file generated ending with run " + m_run + " in experiment " + m_experiment;
      std::cout << "title: " << title << std::endl;

      m_out_ped_file->SetTitle(title.c_str());

      if (m_out_ped_file) {m_out_ped_file->cd();}

      std::map<topcaf_channel_id_t, TProfile*>::iterator it_ct = m_sample2ped.begin();

      for (; it_ct != m_sample2ped.end(); ++it_ct) {
        topcaf_channel_id_t key =  it_ct->first;
        m_sample2ped[key]->Write();
      }
      m_out_ped_file->Close();
//FIXME      ConditionsService::getInstance()->writePayloadFile(tempFile, this);
    }
    if (m_writefile == 1) {
      B2INFO("writing itop pedestal file manually to " << m_out_ped_filename);
      m_out_ped_file = TFile::Open(m_out_ped_filename.c_str(), "recreate");
      if (m_out_ped_file) {
        m_out_ped_file->cd();
      }

      std::map<topcaf_channel_id_t, TProfile*>::iterator it_ct = m_sample2ped.begin();
      for (; it_ct != m_sample2ped.end(); ++it_ct) {
        topcaf_channel_id_t key =  it_ct->first;
        m_sample2ped[key]->Write();
      }
    }
  }
  if (m_out_ped_file) {
    m_out_ped_file->Close();
    delete m_out_ped_file;
  }
  if (m_in_ped_file) {
    m_in_ped_file->Close();
    delete m_in_ped_file;
  }
}
