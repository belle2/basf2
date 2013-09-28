#include "TTDData.hh"

#include <node/TTDNode.hh>

extern "C" {
#include <nsm2.h>
#include "ttd_data.h"
}

#include <cstring>
#include <iostream>

using namespace B2DAQ;

TTDData::TTDData(const std::string& data_name) throw()
  : NSMData(data_name, "ttd_data", ttd_data_revision)
{

}

void TTDData::read(NSMNode* node) throw(NSMHandlerException)
{
  ttd_data* data = (ttd_data*)get();
  const size_t nftsw = data->nftsw;
  TTDNode* ttd = (TTDNode*)node;
  ttd->clearFTSWs();
  for (size_t i = 0; i < nftsw; i++) {
    FTSW* ftsw = new FTSW();
    if (data->channel[i] > 0) {
      ftsw->setChannel(data->channel[i]);
      std::cout << __FILE__ << ":" << __LINE__ << " channel = " << ftsw->getChannel() << std::endl;
      ftsw->setFirmware((const char*)(data->firmware + i * 64));
      std::cout << "TTDData::write firmware = " << ftsw->getFirmware() << std::endl;
      ftsw->setUsed((bool)data->used[i]);
      ftsw->setTriggerMode(data->trigger_mode[i]);
      ttd->addFTSW(ftsw);
    }
  }
}

void TTDData::write(NSMNode* node) throw(NSMHandlerException)
{
  ttd_data* data = (ttd_data*)get();
  TTDNode* ttd = (TTDNode*)node;
  data->nftsw = (short)ttd->getFTSWs().size();
  for (size_t i = 0; i < ttd->getFTSWs().size(); i++) {
    FTSW* ftsw = ttd->getFTSW(i);
    if (ftsw != NULL) {
      data->channel[i] = ftsw->getChannel();
      std::cout << __FILE__ << ":" << __LINE__ << " channel = " << data->channel[i] << std::endl;
      strncpy((char*)(data->firmware + i * 64), ftsw->getFirmware().c_str(), 64);
      data->used[i] = (byte8)ftsw->isUsed();
      data->trigger_mode[i] = ftsw->getTriggerMode();
    }
  }
}

