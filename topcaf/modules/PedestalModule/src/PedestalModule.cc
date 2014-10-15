#include <topcaf/modules/PedestalModule/inc/PedestalModule.h>
#include <framework/datastore/StoreObjPtr.h>

//#include "DatabaseTool.h"
//#include "TROOT.h"
//#include "TKey.h"
//#include "TClass.h"
//#include "TDirectory.h"
#include <iostream>
#include <cstdlib>

using namespace Belle2;

REG_MODULE(Pedestal)

PedestalModule::PedestalModule() : Module()
{
  setDescription("This module is used to create pedestal file");
  addParam("OutputFileName", m_out_ped_filename, "Output pedestal payload");
  m_out_ped_file = NULL;
  //m_in_ped_file=NULL;
  //m_dbout_request=false;
}

PedestalModule::~PedestalModule()
{
  /*
  //If output file requested then save files

  if(m_out_ped_file||m_dbout_request){
    if(m_out_ped_file) m_out_ped_file->cd();
    //Save Channel sample adc info.
    std::map<unsigned int,TProfile*>::iterator it_ct = m_out_sample2ped.begin();

    std::cout<<"writing pedestals"<<std::endl;
    for (; it_ct!= m_out_sample2ped.end(); ++it_ct){
      unsigned int key =  it_ct->first;
      if(m_dbout_request){
  DatabaseTool::GetInstance()->WritePayload(m_out_sample2ped[key],
              GetPayloadTag(),
              GetSubsystemTag(),
              GetAlgName(),
              GetVersion(),
              GetInitialRun(),
              GetFinalRun());
      }
      else{
  m_out_sample2ped[key]->Write();
      }
    }
    if(m_out_ped_file){
      m_out_ped_file->Close();
      delete m_out_ped_file;
    }
  }
  if(m_in_ped_file){
    m_in_ped_file->Close();
    delete m_in_ped_file;
  }
  */
}
void PedestalModule::initialize()
{
  CreatePedestalFile(m_out_ped_filename.c_str());
}

void PedestalModule::event()
{
  //Get Waveform from datastore
  StoreObjPtr<EventWaveformPacket> evtwave_ptr;
  evtwave_ptr.isRequired();

  if (evtwave_ptr) {
    std::cout << "PedestalModule::GetChannelID(): " <<   evtwave_ptr->GetChannelID() << std::endl;
    unsigned int channel_name = evtwave_ptr->GetChannelID() + evtwave_ptr->GetASICWindow();
    const std::vector<double> v_samples = evtwave_ptr->GetSamples();
    int nsamples = v_samples.size();

    TProfile* channel_adc_h = m_out_sample2ped[channel_name];
    if (!channel_adc_h) {
      std::string s_channel_name = NumberToString(channel_name);
      channel_adc_h = new TProfile(s_channel_name.c_str(),
                                   s_channel_name.c_str(),
                                   nsamples, 0, nsamples);
      m_out_sample2ped[channel_name] = channel_adc_h;
    }
    for (int s = 0; s < nsamples; s++) {
      double adc = (double) v_samples.at(s);
      channel_adc_h->Fill(s, adc);
    }
  }
}

void  PedestalModule::terminate()
{
  //If output file requested then save files
  if (m_out_ped_file) {
    m_out_ped_file->cd();
    //Save Channel sample adc info.
    std::map<unsigned int, TProfile*>::iterator it_ct = m_out_sample2ped.begin();

    std::cout << "writing pedestals" << std::endl;
    for (; it_ct != m_out_sample2ped.end(); ++it_ct) {
      unsigned int key =  it_ct->first;
      m_out_sample2ped[key]->Write();
    }

    m_out_ped_file->Close();
    delete m_out_ped_file;
  }
}
//Invokes the output pedestal file
void PedestalModule::CreatePedestalFile(const char* output_file_name)
{
  m_out_ped_file = TFile::Open(output_file_name, "recreate");
}


/*
//Method used to record adc valies from waveform
void PedestalModule::FillWavePacket(const EventWaveformPacket *in_wp){
  unsigned int channel_name = GetWindowID(in_wp);
  const std::vector<double> v_samples = in_wp->GetSamples();
  int nsamples = v_samples.size();

  TProfile *channel_adc_h = m_out_sample2ped[channel_name];
  if(!channel_adc_h){
    std::string s_channel_name = NumberToString(channel_name);
    //std::cout << "TProfile <" << channel_name << "> doesn't exits... creating." << std::endl;
    channel_adc_h = new TProfile(s_channel_name.c_str(),
                                 s_channel_name.c_str(),
                                 nsamples,0,nsamples);
    m_out_sample2ped[channel_name] = channel_adc_h;
  }
  for(int s=0;s<nsamples;s++){
    double adc = (double) v_samples.at(s);
    channel_adc_h->Fill(s,adc);
  }

}


//Load the pedestal file
int PedestalModule::LoadPedestalFile(const char *input_file_name){
  m_in_ped_file = TFile::Open(input_file_name,"READ");
  if( !m_in_ped_file){
    std::cout << "Couldn't open input pedestal file: "
              << input_file_name << std::endl;
    return -9;
  }  else {
    TIter next(m_in_ped_file->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TProfile")) continue;
      TProfile *profile = (TProfile*)key->ReadObj();
      std::string name_key=profile->GetName();
      unsigned int window_id = strtoul (name_key.c_str(), NULL, 0);
      m_in_sample2ped[window_id]=profile;

    }
    return 0;
  }
}

//Load the pedestal file
int PedestalModule::LoadPedestalDB(std::string global_tag,Int_t run_number){

  DatabaseTool::GetInstance()->OpenDB();

  TList *payload = DatabaseTool::GetInstance()->GetPayloadList(global_tag,
                     run_number,
                     GetAlgName(),
                     GetVersion());
  std::cout<<"reading all... "<<std::endl;
  gDirectory->ReadAll();
  std::cout<<"read done."<<std::endl;

  TIter next(payload);
  TKey *key;
  Int_t c=0;
  while ((key = (TKey*)next())) {
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TProfile")) continue;
      TProfile *profile = (TProfile*)key->ReadObj();
      std::string name_key=profile->GetName();
      unsigned int window_id = strtoul (name_key.c_str(), NULL, 0);
      m_in_sample2ped[window_id]=profile;
      c++;
      if(c%100==0) std::cout<<"name_key("<<c<<"): "<<name_key<<std::endl;
  }
  return 0;

}


//Applies pedestal correction
EventWaveformPacket* PedestalModule::PedCorrect(const EventWaveformPacket *in_wp)
{
  EventWaveformPacket *out_wp = new EventWaveformPacket(*in_wp);

  //Bad channel ID need to improve!!!
  unsigned int window_id = GetWindowID(in_wp);

  //Look up reference pedestal and correct
  TProfile *ped_profile = m_in_sample2ped[window_id];
  if(ped_profile==NULL){
    std::cout << "Problem retrieving pedestal data for channel "
              << window_id << "!!!" << std::endl;
  }
  else {
    std::vector<double> v_pedcorr_samples;
    std::vector<double> v_samples = in_wp->GetSamples();
    for(unsigned int s=0;s<v_samples.size();s++){
      double this_ped=ped_profile->GetBinContent(s+1);
      double this_sample=v_samples.at(s);
      double corr_sample=this_sample-this_ped;
      v_pedcorr_samples.push_back(corr_sample);
    }
    out_wp->SetSamples(v_pedcorr_samples);
  }

  return out_wp;
}


RawEvent* PedestalModule::PedCorrect(const RawEvent *raw_event){
  RawEvent *corrected_raw_event = new RawEvent(*raw_event);

  EventWaveformPackets waves =  raw_event->GetEventWaveformPackets();
  for(unsigned int w=0;w<waves.size();w++){
    EventWaveformPacket* corr_wave = PedCorrect(waves.at(w));
    corrected_raw_event->AddEventWaveformPacket(corr_wave);
  }

  return corrected_raw_event;
}

unsigned int PedestalModule::GetWindowID(const EventWaveformPacket *in_wp){
  //Channel ID
  unsigned int channel_id = in_wp->GetChannelID();

  //Add window id
  unsigned short asic_window = in_wp->GetASICWindow();

  //Bad channel ID need to improve!!!
  unsigned int window_id = channel_id + asic_window;

  //std::cout << "Window ID: " << window_id << std::endl;

  return window_id;
}


void PedestalModule::SetDBOutputRequest(void){
  m_dbout_request = true;
}

*/
