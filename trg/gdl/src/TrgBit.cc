#include <sys/stat.h>
#include <cstdio>
#include <iostream>
#include <trg/gdl/TrgBit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

using namespace Belle2;

std::string
TrgBit::version(void) const
{
  return "20180623";
}

std::string
TrgBit::versionFTDL(void) const
{
  return _ftdlVersion[n_ftdl];
}

TrgBit::TrgBit(void)
  : _exp(0),
    _run(0),
    n_input(0),
    n_ftdl(0),
    n_psnm(0)
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  // set n_ftdl, n_psnm, n_input
  mapNumber(_exp, _run);

  StoreObjPtr<TRGSummary> trgsum;
  for (unsigned i = 0; i < 6; i++) {
    _input.set(trgsum->getInputBits(i), i);
    _ftdl.set(trgsum->getFtdlBits(i), i);
    _psnm.set(trgsum->getPsnmBits(i), i);
  }
  timtype = trgsum->getTimType();
}

TrgBit::~TrgBit()
{
}

bool
TrgBit::get(input a) const
{
  unsigned bit = _inputMap[n_input][a];
  if (bit == 999) return false;
  return _input.get(bit);
}

bool
TrgBit::get(output a) const
{
  unsigned bit = _outputMap[n_ftdl][a];
  if (bit == 999) return false;
  return _ftdl.get(bit);
}

bool
TrgBit::getPSNM(output a) const
{
  unsigned bit = _outputMap[n_ftdl][a];
  if (bit == 999) return false;
  return _psnm.get(bit);
}

TRGSummary::ETimingType
TrgBit::getTimingSource(void) const
{
  return timtype;
}

unsigned
TrgBit::preScaleValue(output a) const
{
  return _psnmValues[n_psnm][_outputMap[n_ftdl][a]];
}

void
TrgBit::InputBitPattern::set(unsigned a, unsigned b)
{
  _pattern[b] = a;
}

void
TrgBit::OutputBitPattern::set(unsigned a, unsigned b)
{
  _pattern[b] = a;
}

bool
TrgBit::InputBitPattern::get(unsigned bit) const
{
  unsigned wd = bit / 32;
  unsigned position = bit % 32;
  return (_pattern[wd] >> position) & 1;
}

bool
TrgBit::OutputBitPattern::get(unsigned bit) const
{
  unsigned wd = bit / 32;
  unsigned position = bit % 32;
  return (_pattern[wd] >> position) & 1;
}
