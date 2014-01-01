//+
// File : HLTTag.cc
// Description : Manage HLT tag info
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Dec - 2013
//-

#include "rawdata/dataobjects/HLTTag.h"

using namespace Belle2;
using namespace std;

#define CHECK_BIT(var,pos) (bool)((var) & (1<<(pos)))
#define SET_BIT(var,pos) ((var) | (1<<(pos)))
#define CLR_BIT(var,pos) ((var) & ~(1<<(pos)))

HLTTag::HLTTag()
{
  m_HLTSummary = 0;
  m_HLTEventID = 0;
  m_HLTUnitID = 0;
  m_HLTtime = (time_t) 0;
  for (int i = 0; i < MAXALGO; i++) {
    m_HLTAlgoInfo[i] = 0;
  }
}

HLTTag::~HLTTag()
{
}

int HLTTag::HLTEventID()
{
  return m_HLTEventID;
}

void HLTTag::HLTEventID(int id)
{
  m_HLTEventID = id;
}

int HLTTag::HLTUnitID()
{
  return m_HLTUnitID;
}

void HLTTag::HLTUnitID(int id)
{
  m_HLTUnitID = id;
}

time_t HLTTag::HLTTime()
{
  return m_HLTtime;
}

void HLTTag::HLTTime(time_t tm)
{
  m_HLTtime = tm;
}

void HLTTag::HLTSetCurrentTime()
{
  m_HLTtime = time(NULL);
}

bool HLTTag::Accepted(HLTTrigAlgo algo)
{
  return (CHECK_BIT(m_HLTSummary, algo));
}

bool HLTTag::Discarded(HLTTrigAlgo algo)
{
  return (!CHECK_BIT(m_HLTSummary, algo));
}

void HLTTag::SetSummaryWord(int sum)
{
  m_HLTSummary = sum;
}

void HLTTag::Accept(HLTTrigAlgo algo)
{
  m_HLTSummary = SET_BIT(m_HLTSummary, (int)algo);
  //  m_HLTSummary = m_HLTSummary | (1<<(int)algo);
  //  printf ( "algo = %d, m_HLTSummary = %8.8x\n", (int)algo, m_HLTSummary );
}

void HLTTag::Discard(HLTTrigAlgo algo)
{
  m_HLTSummary = CLR_BIT(m_HLTSummary, (int)algo);
}

int HLTTag::GetSummaryWord()
{
  return m_HLTSummary;
}

int HLTTag::GetAlgoInfo(HLTTrigAlgo algo)
{
  return m_HLTAlgoInfo[(int)algo];
}

void HLTTag::SetAlgoInfo(HLTTrigAlgo algo, int infobits)
{
  m_HLTAlgoInfo[(int)algo] = infobits;
}


