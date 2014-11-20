#include <topcaf/modules/WaveMergingModule/WaveMergingModule.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <framework/datastore/StoreArray.h>

#include <cstdlib>
#include <iostream>
#include <map>

using namespace Belle2;

REG_MODULE(WaveMerging);

WaveMergingModule::WaveMergingModule() : Module()
{
  setDescription("This module merges waveform packets in itop raw waveform event data");
}

WaveMergingModule::~WaveMergingModule() {}

void WaveMergingModule::initialize()
{

}

void WaveMergingModule::event()
{

  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();



  if (evtwaves_ptr) {

    std::map<unsigned int, EventWaveformPackets > ch2wp;
    for (int w = 0; w < evtwaves_ptr.getEntries(); w++) {
      unsigned int channel_id = evtwaves_ptr[w]->GetChannelID();

      if (ch2wp[channel_id].size() == 0) {
        EventWaveformPackets ch_waves;
        ch_waves.push_back(evtwaves_ptr[w]);
        ch2wp[channel_id] = ch_waves;
        //B2INFO("Wave packets found with channel_id "<<channel_id);
      } else {
        EventWaveformPackets ch_waves = ch2wp[channel_id];
        ch_waves.push_back(evtwaves_ptr[w]);
        ch2wp[channel_id] = ch_waves;
      }
    }
    //Create new event with merged waveforms
    EventWaveformPackets corrected_waves;
    std::map<unsigned int, EventWaveformPackets>::iterator it = ch2wp.begin();
    for (; it != ch2wp.end(); ++it) {
      //Sort waveform (?)
      //Assumed it's already in the right order!!!
      EventWaveformPacket* merged_wave;
      std::vector< double > merged_samples;
      //std::cout << "ch_id/waves: " << it->first << "/" << it->second.size()  << std::endl;
      for (unsigned int p = 0; p < it->second.size(); p++) {
        if (p == 0) {
          merged_wave = new EventWaveformPacket(*it->second.at(p));
        }
        std::vector< double > samples = it->second.at(p)->GetSamples();
        for (unsigned int s = 0; s < samples.size(); s++) {
          merged_samples.push_back(samples.at(s));
        }
      }
      merged_wave->SetSamples(merged_samples);
      corrected_waves.push_back(merged_wave);
    }

    /// Now clear the old waveform partials and store the new merged waveforms.
    evtwaves_ptr.clear();
    EventWaveformPackets::iterator wave_it = corrected_waves.begin();
    for (; wave_it != corrected_waves.end(); ++wave_it) {
      evtwaves_ptr.appendNew(EventWaveformPacket(**wave_it));
    }

  }
}

void WaveMergingModule::terminate()
{

}



/*
RawEvent* WaveMergingModule::MergeWavePackets(const RawEvent *raw_event){
}

*/
