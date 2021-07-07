/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cstdio>
#include <trg/gdl/TrgBit.h>
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
    nconf_psnm(0),
    nconf_ftdl(0),
    nconf_input(0),
    _isFiredFilled(false)
{

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  // set nconf_ftdl, nconf_psnm, nconf_input, n_input, n_output
  mapNumber(_exp, _run);

  StoreObjPtr<TRGSummary> trgsum;
  if (trgsum.isValid()) {
    for (unsigned i = 0; i < 6; i++) {
      unsigned i32b = trgsum->getInputBits(i);
      unsigned f32b = trgsum->getFtdlBits(i);
      unsigned p32b = trgsum->getPsnmBits(i);
      _input.set(i32b, i);
      _ftdl.set(f32b, i);
      _psnm.set(p32b, i);
      for (unsigned j = 0; j < 32; j++) {
        if (j + i * 32 < n_input)
          _itdVector.push_back((i32b & (1 << j)) ? 1 : 0);
        if (j + i * 32 < n_output) {
          _ftdVector.push_back((f32b & (1 << j)) ? 1 : 0);
          _psnVector.push_back((p32b & (1 << j)) ? 1 : 0);
        }
      }
    }
    timtype = trgsum->getTimType();
    _isFiredFilled = true;
  }
}

TrgBit::~TrgBit()
{
}

bool
TrgBit::isFired(const char* bitname) const
{
  for (unsigned i = 0; i < N_BITS_RESERVED; i++) {
    if (! strcmp(_inputBitNames[i], bitname)) {
      return isFired((input) i);
    }
  }
  for (unsigned i = 0; i < N_BITS_RESERVED; i++) {
    if (! strcmp(_outputBitNames[i], bitname)) {
      return isFired((output) i);
    }
  }
  return false;
}

bool
TrgBit::isFired(input a) const
{
  unsigned bit = _inputMap[nconf_input][a];
  if (bit == 999) return false;
  return _input.isFired(bit);
}

bool
TrgBit::isFiredInput(input a) const
{
  unsigned bit = _inputMap[nconf_input][a];
  if (bit == 999) return false;
  return _input.isFired(bit);
}

bool
TrgBit::isFired(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return _ftdl.isFired(bit);
}

bool
TrgBit::isFiredFtdl(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return _ftdl.isFired(bit);
}

bool
TrgBit::isFiredInput(unsigned ith_bit) const
{
  return _input.isFired(ith_bit);
}

bool
TrgBit::isFiredInput(const char* bitname) const
{
  unsigned bit = getInputBitNum(bitname);
  if (bit == 999) return false;
  return _input.isFired(bit);
}

bool
TrgBit::isFiredFtdl(unsigned ith_bit) const
{
  return _ftdl.isFired(ith_bit);
}

bool
TrgBit::isFiredFtdl(const char* bitname) const
{
  unsigned bit = getOutputBitNum(bitname);
  if (bit == 999) return false;
  return _ftdl.isFired(bit);
}

bool
TrgBit::isFiredPsnm(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return _psnm.isFired(bit);
}

bool
TrgBit::isFiredPsnm(const char* bitname) const
{
  unsigned bit = getOutputBitNum(bitname);
  if (bit == 999) return false;
  return _psnm.isFired(bit);
}

bool
TrgBit::isFiredPsnm(unsigned ith_bit) const
{
  return _psnm.isFired(ith_bit);
}

TRGSummary::ETimingType
TrgBit::getTimingSource(void) const
{
  return timtype;
}

unsigned
TrgBit::preScaleValue(output a) const
{
  if (_outputMap[nconf_ftdl][a] == 999) return 0;
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
TrgBit::InputBitPattern::isFired(unsigned bit) const
{
  unsigned wd = bit / 32;
  unsigned position = bit % 32;
  return (_pattern[wd] >> position) & 1;
}

bool
TrgBit::OutputBitPattern::isFired(unsigned bit) const
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
  for (unsigned i = 0; i < N_BITS_RESERVED; i++) {
    unsigned j = _inputMap[nconf_input][i];
    if (ith_bit == j) return _inputBitNames[i];
  }
  return "N/A";
}

const char*
TrgBit::getOutputBitName(unsigned ith_bit) const
{
  for (unsigned i = 0; i < N_BITS_RESERVED; i++) {
    unsigned j = _outputMap[nconf_ftdl][i];
    if (ith_bit == j) return _outputBitNames[i];
  }
  return "N/A";
}

void
TrgBit::printPreScaleValues(void) const
{
  for (unsigned i = 0; i < n_output; i++) {
    for (unsigned j = 0; j < N_BITS_RESERVED; j++) {
      unsigned k = _outputMap[nconf_ftdl][j];
      if (i == k) {
        printf("%3u %10s %d\n", i, _outputBitNames[j], _psnmValues[nconf_psnm][i]);
        break;
      }
    }
  }
}

void
TrgBit::printConf(void) const
{
  printf("TrgBit class: exp(%u), run(%u), nconf_psnm(%u), nconf_ftdl(%u), nconf_input(%u), n_input(%u), n_output(%u)\n", _exp, _run,
         nconf_psnm, nconf_ftdl, nconf_input, n_input, n_output);
}

int
TrgBit::getBitNum(const char* bitname) const
{
  int inum = getInputBitNum(bitname);
  if (inum == 999) {
    return -getOutputBitNum(bitname) - 1;
  } else {
    return inum;
  }
}

unsigned
TrgBit::getInputBitNum(const char* bitname) const
{
  for (unsigned i = 0; i < N_BITS_RESERVED; i++) {
    if (! strcmp(_inputBitNames[i], bitname)) {
      return _inputMap[nconf_input][i];
    }
  }
  return 999;
}

unsigned
TrgBit::getOutputBitNum(const char* bitname) const
{
  for (unsigned i = 0; i < N_BITS_RESERVED; i++) {
    if (! strcmp(_outputBitNames[i], bitname)) {
      return _outputMap[nconf_ftdl][i];
    }
  }
  return 999;
}

bool
TrgBit::isUsed(input a) const
{
  unsigned bit = _inputMap[nconf_input][a];
  if (bit == 999) return false;
  return true;
}

bool
TrgBit::isUsed(output a) const
{
  unsigned bit = _outputMap[nconf_ftdl][a];
  if (bit == 999) return false;
  return true;
}

bool
TrgBit::isUsed(const char* bitname) const
{
  if (getInputBitNum(bitname) != 999) {
    return true;
  } else if (getOutputBitNum(bitname) != 999) {
    return true;
  }
  return false;
}

