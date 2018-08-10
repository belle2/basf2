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
  return _ftdlVersion[nconf_ftdl];
}

TrgBit::TrgBit(void)
  : _exp(0),
    _run(0),
    nconf_input(0),
    nconf_ftdl(0),
    nconf_psnm(0)
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  // set nconf_ftdl, nconf_psnm, nconf_input
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
  unsigned bit = _inputMap[nconf_input][a];
  if (bit == 999) return false;
  return _input.get(bit);
}

bool
TrgBit::getInput(input a) const
{
  unsigned bit = _inputMap[nconf_input][a];
  if (bit == 999) return false;
  return _input.get(bit);
}

bool
TrgBit::getInput(unsigned ith_bit) const
{
  return _input.get(ith_bit);
}

bool
TrgBit::get(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return _ftdl.get(bit);
}

bool
TrgBit::getOutput(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return _ftdl.get(bit);
}

bool
TrgBit::getOutput(unsigned ith_bit) const
{
  return _input.get(ith_bit);
}

bool
TrgBit::getPSNM(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return _psnm.get(bit);
}

bool
TrgBit::getPSNM(unsigned ith_bit) const
{
  return _psnm.get(ith_bit);
}

TRGSummary::ETimingType
TrgBit::getTimingSource(void) const
{
  return timtype;
}

unsigned
TrgBit::preScaleValue(output a) const
{
  return _psnmValues[nconf_psnm][_outputMap[nconf_ftdl][a]];
}

unsigned
TrgBit::preScaleValue(unsigned i) const
{
  return _psnmValues[nconf_psnm][i];
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

int
TrgBit::InputBitPattern::operator [](unsigned a)
{
  if (a >= 32 * N_INPUT_ARRAY) return 0;
  const unsigned i = a / 32;
  const unsigned j = a % 32;

  if ((_pattern[i] >> j) & 1) return 1;
  return 0;
}

int
TrgBit::OutputBitPattern::operator [](unsigned a)
{
  if (a >= 32 * N_OUTPUT_ARRAY) return 0;
  const unsigned i = a / 32;
  const unsigned j = a % 32;

  if ((_pattern[i] >> j) & 1) return 1;
  return 0;
}

const char*
TrgBit::getInputBitName(unsigned ith_bit) const
{
  for (unsigned i = 0; i < 192; i++) {
    unsigned j = _inputMap[nconf_input][i];
    if (ith_bit == j) return _inputBitNames[i];
  }
  return "N/A";
}

const char*
TrgBit::getOutputBitName(unsigned ith_bit) const
{
  for (unsigned i = 0; i < 192; i++) {
    unsigned j = _outputMap[nconf_ftdl][i];
    if (ith_bit == j) return _outputBitNames[i];
  }
  return "N/A";
}

void
TrgBit::printPreScaleValues(void) const
{
  for (unsigned i = 0; i < n_output; i++) {
    for (unsigned j = 0; j < 192; j++) {
      unsigned k = _outputMap[nconf_ftdl][j];
      if (i == k) {
        printf("%3d %10s %d\n", i, _outputBitNames[j], _psnmValues[nconf_psnm][i]);
        break;
      }
    }
  }
}
