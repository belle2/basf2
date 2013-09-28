#include "PackageSerializer.hh"

using namespace B2DQM;

bool PackageSerializer::allocate(HistoPackage* pack)
{
  _pack = pack;
  if (_pack != NULL) {
    _config.allocate(_pack);
    _contents.allocate(_pack);
    _contents_all.allocate(_pack);
  }
  return _pack != NULL;
}

int PackageSerializer::update()
{
  if (_pack != NULL) {
    _config.update();
    _contents.update();
    _contents_all.update();
    return _pack->getUpdateId();
  }
  return -1;
}

