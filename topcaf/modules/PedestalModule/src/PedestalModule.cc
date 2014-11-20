#include <topcaf/modules/PedestalModule/PedestalModule.h>
#include <topcaf/dataobjects/topFileMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/conditions/ConditionsService.h>


#include <iostream>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TList.h>
#include <TFile.h>
#include <TClass.h>

using namespace Belle2;

REG_MODULE(Pedestal)

PedestalModule::PedestalModule() : Module()
{
  setDescription("This module is used to create itop pedestal file, see the parameters for various options");

  addParam("Mode", m_mode, "Calculate Pedestals - 0 ; Apply Pedestals - 1");

  addParam("Conditions", m_conditions, "Do not use Conditions Service - 0 ; Use Conditions Service - 1 (write to Conditions if Mode==0) ", 0);
  addParam("IOV_initialRun", m_initial_run, "Initial run for the interval of validity for these pedestals", std::string("NULL"));
  addParam("IOV_finalRun", m_final_run, "Final run for the interval of validity for these pedestals", std::string("NULL"));

  addParam("InputFileName", m_in_ped_filename, "Input filename used if Mode==1 and PedConditions==0", std::string());
  addParam("WriteFile", m_writefile, "Do not write file - 0 ; Write Peds to local root file - 1 ", 0);
  addParam("OutputFileName", m_out_ped_filename, "Output filename written if Mode==0 and WriteFile==1", std::string());


  m_out_ped_file = NULL;
  m_in_ped_file = NULL;

}

PedestalModule::~PedestalModule() {}

void PedestalModule::initialize()
{

}

void PedestalModule::beginRun()
{

  StoreObjPtr<topFileMetaData> metadata_ptr;
  metadata_ptr.isRequired();

  if (metadata_ptr) {
    m_experiment = metadata_ptr->getExperiment();
    m_run = metadata_ptr->getRun();
  }


  if (m_mode == 1) { // read pedestals
    TList* list;

    if (m_conditions == 1) { // read peds using conditions service

      B2INFO("Retrieving pedestal data from service.");

      list = ConditionsService::GetInstance()->GetPayloadList(GetPayloadTag(),
                                                              m_run,
                                                              getName(),
                                                              GetVersion());
      B2INFO("Retrieving pedestal data from service. done");
    } else if (m_conditions == 0) { // read peds from local file

      m_in_ped_file = TFile::Open(m_in_ped_filename.c_str(), "READ");
      if (!m_in_ped_file) {
        B2ERROR("Couldn't open input itop pedestal file: " << m_in_ped_filename);
      }  else {
        list = m_in_ped_file->GetListOfKeys();
      }

    }

    /// Now load up the pedestals
    TIter next(list);
    TKey* key;
    while ((key = (TKey*)next())) {
      TClass* cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TProfile")) continue;
      TProfile* profile = (TProfile*)key->ReadObj();
      std::string name_key = profile->GetName();
      unsigned int window_id = strtoul(name_key.c_str(), NULL, 0);
      m_sample2ped[window_id] = profile;
    }

  }

}

void PedestalModule::event()
{
  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();

  if (evtwaves_ptr) {
    for (int c = 0; c < evtwaves_ptr.getEntries(); c++) {

      EventWaveformPacket* evtwave_ptr = evtwaves_ptr[c];

      if ((m_mode == 0)) { // Calculate pedestals from waveform

        //      B2INFO("PedestalModule::GetChannelID(): " <<   evtwave_ptr->GetChannelID() );
        unsigned int channel_name = evtwave_ptr->GetChannelID() + evtwave_ptr->GetASICWindow();
        const std::vector<double> v_samples = evtwave_ptr->GetSamples();
        int nsamples = v_samples.size();

        TProfile* channel_adc_h = m_sample2ped[channel_name];
        if (!channel_adc_h) {
          std::string s_channel_name = std::to_string(channel_name);
          channel_adc_h = new TProfile(s_channel_name.c_str(),
                                       s_channel_name.c_str(),
                                       nsamples, 0, nsamples);
          m_sample2ped[channel_name] = channel_adc_h;
        }
        for (int s = 0; s < nsamples; s++) {
          double adc = (double) v_samples.at(s);
          channel_adc_h->Fill(s, adc);
        }
      }
      if (m_mode == 1) { // Apply pedestals to waveform

        //      EventWaveformPacket *out_wp = new EventWaveformPacket(*evtwave_ptr);

        //Bad channel ID need to improve!!!
        unsigned int channel_name = evtwave_ptr->GetChannelID() + evtwave_ptr->GetASICWindow();
        //      unsigned int window_id = GetWindowID(evtwave_ptr);

        //Look up reference pedestal and correct
        //      TProfile *ped_profile = m_sample2ped[window_id];
        TProfile* ped_profile = m_sample2ped[channel_name];
        if (ped_profile == NULL) {
          B2WARNING("Problem retrieving itop pedestal data for channel " << channel_name << "!!!");
        } else {
          std::vector<double> v_pedcorr_samples;
          std::vector<double> v_samples = evtwave_ptr->GetSamples();
          for (unsigned int s = 0; s < v_samples.size(); s++) {
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

    if ((m_conditions == 1)) { // Use Conditions Service to save pedestals

      m_payload_tag =  m_experiment + '_' + m_run + '_';
      m_payload_tag += GetAlgName();
      m_payload_tag += '_';
      m_payload_tag += GetVersion();


      B2INFO("writing itop pedestals using Conditions Service - Payload Tag:" << GetPayloadTag()
             << "\tSubsystem Tag: " << getPackage() << "\tAlgorithm Name: " << getName()
             << "\tVersion: " << GetVersion() << "\tRun_i: " << GetInitialRun() << "\tRun_f: " << GetFinalRun());

      ConditionsService::GetInstance()->StartPayload(GetPayloadTag(),
                                                     GetPayloadType(),
                                                     getPackage(),
                                                     getName(),
                                                     GetVersion(),
                                                     GetExperiment(),
                                                     GetInitialRun(),
                                                     GetFinalRun());


      std::map<unsigned int, TProfile*>::iterator it_ct = m_sample2ped.begin();
      for (; it_ct != m_sample2ped.end(); ++it_ct) {

        unsigned int key =  it_ct->first;

        ConditionsService::GetInstance()->WritePayloadObject(m_sample2ped[key],
                                                             GetPayloadTag(),
                                                             getPackage(),
                                                             getName(),
                                                             GetVersion(),
                                                             GetInitialRun(),
                                                             GetFinalRun());
      }

      ConditionsService::GetInstance()->CommitPayload(GetPayloadTag(),
                                                      getPackage(),
                                                      getName(),
                                                      GetVersion(),
                                                      GetInitialRun(),
                                                      GetFinalRun());


    }
    if (m_writefile == 1) {

      B2INFO("writing itop pedestal file manually to " << m_out_ped_filename)

      m_out_ped_file = TFile::Open(m_out_ped_filename.c_str(), "recreate");

      if (m_out_ped_file) {m_out_ped_file->cd();}

      std::map<unsigned int, TProfile*>::iterator it_ct = m_sample2ped.begin();

      for (; it_ct != m_sample2ped.end(); ++it_ct) {
        unsigned int key =  it_ct->first;
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


