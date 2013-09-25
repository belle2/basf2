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
  if (nftsw != ttd->getFTSWs().size()) {
    ttd->clearFTSWs();
    for (size_t i = 0; i < nftsw; i++) {
      ttd->addFTSW(new FTSW());
    }
  }
  for (size_t i = 0; i < nftsw; i++) {
    FTSW* ftsw = ttd->getFTSW(i);
    if (ftsw != NULL) {
      ftsw->setChannel(data->channel[i]);
      ftsw->setFirmware((const char*)(data->firmware + i * 64));
      std::cout << "TTDData::write firmware = " << ftsw->getFirmware() << std::endl;
      ftsw->setUsed((bool)data->used[i]);
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
      strncpy((char*)(data->firmware + i * 64), ftsw->getFirmware().c_str(), 64);
      data->used[i] = (byte8)ftsw->isUsed();
    }
  }
}

