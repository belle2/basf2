#include "daq/slc/copper/top/ErrorStats.h"

#include <algorithm>
#include <iostream>
#include <math.h>

#include <stdio.h>

using namespace std;

//sort map according to values
template <typename A, typename B>
multimap<B, A> flip_map(map<A, B>& source)
{
  multimap<B, A> destination;
  for (typename map<A, B>::const_iterator it = source.begin(); it != source.end(); ++it) {
    destination.insert(pair<B, A>(it->second, it->first));
  }
  return destination;
}

ErrorStats::ErrorStats()
{
  InitErrorStats();
  InitTransactionCount();
}

ErrorStats::~ErrorStats()
{
  PrintErrorSummary();
}

void ErrorStats::InitErrorStats()
{
  m_errorCount.clear();
}

void ErrorStats::InitTransactionCount()
{
  m_transactionCount.clear();
}

void ErrorStats::IncrementErrorCount(const string& key, const bool quite)
{
  map<string, int>::iterator it = m_errorCount.find(key);
  if (it != m_errorCount.end()) {
    m_errorCount[key] += 1;
  } else {
    m_errorCount[key] = 1;
  }
  if (!quite) {
    ShowErrorStat(key);
  }
}

void ErrorStats::IncrementTransactionCount(const string& key)
{
  map<string, int>::iterator it = m_transactionCount.find(key);
  if (it != m_transactionCount.end()) {
    m_transactionCount[key] += 1;
  } else {
    m_transactionCount[key] = 1;
  }
}

void ErrorStats::ShowErrorStat(const string& key)
{
  map<string, int>::iterator it = m_errorCount.find(key);
  if (it != m_errorCount.end()) {
    cout << "errorcount[ " << key << " ] = " << m_errorCount[key] << endl;
  } else {
    cout << key << " not found" << endl;
  }
}

void ErrorStats::ShowErrorStats()
{
  bool showErrorStats = false;
  for (map<string, int>::iterator it = m_errorCount.begin(); it != m_errorCount.end(); ++it) {
    if (it->second != 0) showErrorStats = true;
  }
  if (showErrorStats) {
    cout << "--------------- b2l error summary ----------------" << endl;
    multimap<int, string> sorted = flip_map(m_errorCount);
    for (multimap<int, string>::const_iterator it = sorted.begin(); it != sorted.end(); ++it) {
      ShowErrorStat(it->second);
    }
  } else {
    cout << "no errors observed" << endl;
  }
}

int ErrorStats::GetHslbTransactionCount(const string& key)
{
  map<string, int>::iterator it = m_transactionCount.find(key);
  if (it != m_transactionCount.end()) return it->second;
  else return 0;
}

void ErrorStats::PrintErrorSummary()
{
  cout << "--------- b2l transaction summary ------------" << endl;
  string hslblist[4] = {"a", "b", "c", "d"};
  string mode[2] = {"read", "write"};
  for (int i = 0; i < 4; ++i) {
    string hslb_string = "hslb " + hslblist[i];
    int totalTransactionCount = 0;
    for (int j = 0; j < 2; ++j) {
      string key = mode[j] + "; " + hslb_string;
      totalTransactionCount += GetHslbTransactionCount(key);
    }
    if (totalTransactionCount != 0) {
      int busy_errorcount = 0;
      int other_errorcount = 0;
      int recoverable_errorcount = 0;
      int unrecoverable_errorcount = 0;
      for (map<string, int>::iterator it = m_errorCount.begin(); it != m_errorCount.end();
           ++it) {
        size_t pos = it->first.find(hslb_string);
        if (pos == string::npos) continue;
        else {
          if (it->first.find("readback") != string::npos) {
            recoverable_errorcount += it->second;
          } else if (it->first.find("write") != string::npos) {
            if (pos != string::npos) unrecoverable_errorcount += it->second;
          } else if (it->first.find("read") != string::npos) {
            unrecoverable_errorcount += it->second;
          } else if (it->first.find("busy") != string::npos) {
            busy_errorcount += it->second;
          } else other_errorcount += it->second;
        }
      }
      if (other_errorcount || recoverable_errorcount || unrecoverable_errorcount) {
        int sig = ceil(log10(totalTransactionCount));
        printf("%s    totaltransaction count = %*d\n", hslb_string.c_str(), sig, totalTransactionCount);
        if (busy_errorcount) {
          float busyErrorRate = 100 * busy_errorcount / (float)totalTransactionCount;
          cout << hslb_string << " busy error rate " << busyErrorRate << endl;
        }
        if (other_errorcount) {
          float otherErrorRate = 100 * other_errorcount / (float)totalTransactionCount;
          cout << hslb_string << " other error rate " << otherErrorRate << endl;
        }
        if (recoverable_errorcount) {
          float recoverableErrorRate = 100 * recoverable_errorcount / (float)totalTransactionCount;
          cout << hslb_string << " recoverable error rate " << recoverableErrorRate << endl;
        }
        if (unrecoverable_errorcount) {
          float unrecoverableErrorRate = 100 * unrecoverable_errorcount / (float)totalTransactionCount;
          cout << hslb_string << " unrecoverable error rate " << unrecoverableErrorRate << endl;
        }
      }
    }
  }
}
