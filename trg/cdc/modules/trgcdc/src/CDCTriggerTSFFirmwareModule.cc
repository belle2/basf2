/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/cdc/modules/trgcdc/CDCTriggerTSFFirmwareModule.h>
#include <trg/cdc/Cosim.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/logging/Logger.h>

#include <vector>
#include <array>
#include <bitset>
#include <string>
#include <algorithm>
#include <numeric>

#include <cstdio>
#include <unistd.h>

using namespace Belle2;
using namespace Cosim;
using namespace std;
using namespace CDCTrigger;

REG_MODULE(CDCTriggerTSFFirmware)

constexpr std::array<int, Belle2::CDCTriggerTSFFirmwareModule::m_nSubModules>
Belle2::CDCTriggerTSFFirmwareModule::nAxialMergers;

CDCTriggerTSFFirmwareModule::CDCTriggerTSFFirmwareModule() :
  Module(), m_cdcHits{""}

{
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    // loader.emplace_back(new Xsi::Loader(design_libname_pre + to_string(i * 2)
    //                                     + design_libname_post, simengine_libname));
    inputToTSF[i].resize(nAxialMergers[i]);
    array<char, 256> mer;
    mer.fill(2);
    fill(inputToTSF[i].begin(), inputToTSF[i].end(), mer);
  }
  //Set module properties
  setDescription("Firmware simulation of the Track Segment Finder for CDC trigger.");

  // Define module parameters
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCHits.",
           string("CDCHits"));

  // addParam("outputCollectionName", m_outputCollectionName,
  //          "Name of the StoreArray holding the tracks found in the Hough tracking.",
  //          string("TRGCDCTSFFirmwareTracks"));
}

CDCTriggerTSFFirmwareModule::~CDCTriggerTSFFirmwareModule()
{}

Priority CDCTriggerTSFFirmwareModule::priority(int index)
{
  CDCHit* hit = m_cdcHits[index];
  int offset = (hit->getISuperLayer() == 0) ? 0 : 1;
  switch (hit->getILayer() - 2 - offset) {
    case 0: return Priority::first;
    case 1: return Priority::second;
    default: return Priority::nothing;
  }
}

void CDCTriggerTSFFirmwareModule::write(const char* message, FILE* outstream)
{
  // write input to TSF firmware
  fprintf(outstream, "%s\n" , message);
  fflush(outstream);
}

CDCTriggerTSFFirmwareModule::outputArray
CDCTriggerTSFFirmwareModule::read(FILE* instream)
{
  // read output from TSF firmware
  array<char, 2048> buffer;
  outputArray output;
  buffer.fill(one_val);
  if (fgets(buffer.data(), buffer.size(), instream) == NULL) {
    B2ERROR("fgets reached end unexpectedly");
    return output;
  }
  // ins->getline(buffer.data(), buffer.size());
  B2DEBUG(50, "display received TSF output:");
  if (m_debugLevel >= 50) {
    for (auto i2d = 0; i2d < 4; ++i2d) {
      B2DEBUG(50, display_value(buffer.data() + width_out * i2d, width_out));
    }
  }
  auto bufferItr = buffer.cbegin();
  for (int iTracker = 0; iTracker < nTrackers; ++iTracker) {
    copy(bufferItr, bufferItr + width_out, output[iTracker].begin());
    bufferItr += width_out;
  }
  return output;
}

void CDCTriggerTSFFirmwareModule::initialize()
{
  m_cdcHits.isRequired(m_hitCollectionName);
  m_debugLevel = getLogConfig().getDebugLevel();
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    if (i == 1) continue;
    // i: input to worker (output from module)
    // o: output from worker (input to module)
    /* Create pipe and place the two-end pipe file descriptors*/
    pipe(inputFileDescriptor[i].data());
    pipe(outputFileDescriptor[i].data());
    string str_fd[] = {to_string(inputFileDescriptor[i][0]), to_string(outputFileDescriptor[i][1])};
    // spawn a child process
    pid_t pid = fork();
    // pid_t pid = 1;
    if (pid < 0) {
      B2FATAL("Fork failed!");
    } else if (pid == (pid_t) 0) {
      /* Child process (worker) */
      // close the unused ends of the file descriptors
      close(inputFileDescriptor[i][1]);
      close(outputFileDescriptor[i][0]);
      string design = design_libname_pre + to_string(i * 2) + design_libname_post;
      string waveform = wdbName_pre + to_string(i * 2) + wdbName_post;
      // execute the standalone worker program
      execlp("CDCTriggerTSFFirmwareWorker", "CDCTriggerTSFFirmwareWorker",
             str_fd[0].c_str(), str_fd[1].c_str(), design.c_str(), waveform.c_str(),
             to_string(nAxialMergers[i]).c_str(), nullptr);
      B2FATAL("The firmware simulation program didn't launch!");
    } else {
      /* Parent process (BASF2) */
      B2DEBUG(10, "parent " << i);
      m_pid[i] = pid;
      // Close the copy of the fds read/write end
      close(inputFileDescriptor[i][0]);
      close(outputFileDescriptor[i][1]);
      // open the fds
      stream[i][0] = fdopen(inputFileDescriptor[i][1], "w");
      stream[i][1] = fdopen(outputFileDescriptor[i][0], "r");
      // ins = createInStreamFromFD(outputFileDescriptor[i][0]);
      B2DEBUG(20, "pid for TSF" << i * 2 << ": " << m_pid[i]);
    }
  }
  computeEdges();
  if (m_debugLevel >= 50) usleep(1500000);
}

void CDCTriggerTSFFirmwareModule::terminate()
{
  B2DEBUG(10, "Waiting for TSF firmware termination...");
  wait();
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    if (i == 1) continue;
    close(inputFileDescriptor[i][1]);
    close(outputFileDescriptor[i][0]);
  }
}

void CDCTriggerTSFFirmwareModule::computeEdges()
{
  for (unsigned short iEdge = 0; iEdge < 5; ++iEdge) {
    for (const auto& cell : innerInvEdge[iEdge]) {
      m_edge[0][cell].push_back(iEdge);
    }
  }
  for (unsigned short iEdge = 0; iEdge < 3; ++iEdge) {
    for (const auto& cell : innerInvEdge[iEdge]) {
      m_edge[1][cell].push_back(iEdge);
    }
  }
}

template<int iSL>
char* CDCTriggerTSFFirmwareModule::getData(inputToTSFArray input)
{
  static array<char, mergerWidth* nAxialMergers[iSL]> data;
  auto itr = data.begin();
  for (const auto& merger : input[iSL]) {
    copy(merger.begin(), merger.end(), itr);
    itr += mergerWidth;
  }
  return data.data();
}

unsigned short CDCTriggerTSFFirmwareModule::trgTime(int index, int iFirstHit)
{
  if (m_allPositiveTime) {
    return (m_cdcHits[iFirstHit]->getTDCCount() - m_cdcHits[index]->getTDCCount()) / 2;
  } else {
    short time = (m_TDCCountForT0 - m_cdcHits[index]->getTDCCount()) / 2;
    return (time < 0) ? time + (1 << 9) : time;
  }
}

std::bitset<4> CDCTriggerTSFFirmwareModule::timeStamp(int index, int iFirstHit)
{
  return std::bitset<4> (trgTime(index, iFirstHit) % 16);
}

unsigned short CDCTriggerTSFFirmwareModule::mergerCellID(int index)
{
  const CDCHit& hit = (*m_cdcHits[index]);
  const unsigned offset = (hit.getISuperLayer() == 0) ? 3 : 0;
  return (hit.getILayer() - offset) * 16 + hit.getIWire() % 16;
}

unsigned short CDCTriggerTSFFirmwareModule::mergerNumber(int index)
{
  const CDCHit& hit = (*m_cdcHits[index]);
  return hit.getIWire() / 16;
}

using WireSet = std::vector<unsigned short>;
using TSMap = std::unordered_map<int, WireSet>;

Belle2::CDCTriggerTSFFirmwareModule::WireSet
CDCTriggerTSFFirmwareModule::segmentID(int iHit)
{
  const bool innerMost = (m_cdcHits[iHit]->getISuperLayer() == 0);
  const unsigned short iLayer = m_cdcHits[iHit]->getILayer();
  const unsigned short iCell = mergerCellID(iHit);
  TSMap& tsMap = m_tsMap[static_cast<int>(! innerMost)];
  tsMap.reserve(nWiresInMerger);
  if (tsMap.find(iCell) != tsMap.end()) {
    return tsMap[iCell];
  } else {
    WireSet id;
    // distance to the first priority layer: 0-4 (SL0) or 0-2 (outer)
    // The further it is, the more TS it would be involved in.
    const unsigned short distance = (innerMost) ? iLayer - 3 : abs(iLayer - 2);
    id.resize(distance + 1);
    std::iota(id.begin(), id.end(), (iCell - distance / 2) % nCellsInLayer);
    id.erase(std::remove_if(id.begin(), id.end(), [](short i) {
      return (i < 0 || i >= nCellsInLayer);
    }), id.end());
    tsMap.insert({iCell, id});
    return id;
  }
}

void CDCTriggerTSFFirmwareModule::initializeMerger()
{
  // data clock period (32ns) in unit of 2ns
  const int clockPeriod = 16;

  /* The CDCHits array in DataStore contains all the hits in an event,
   * regardless of when them at the trigger system. Their real timing behavior
   * is replayed/simulated by looking at the TDC count (timestamp) of each hit.
   */

  vector<int> iAxialHit;
  for (int i = 0; i < m_cdcHits.getEntries(); ++i) {
    // discard hits in the layers unused by trigger
    if ((m_cdcHits[i]->getISuperLayer() != 0 && m_cdcHits[i]->getILayer() == 5) ||
        m_cdcHits[i]->getICLayer() < 3) {
      continue;
    }
    if (m_cdcHits[i]->getISuperLayer() % 2 == 0) {
      iAxialHit.push_back(i);
    }
  }
  // sort by TDC count: larger count comes first
  std::sort(iAxialHit.begin(), iAxialHit.end(), [this](int i, int j) {
    return m_cdcHits[i]->getTDCCount() > m_cdcHits[j]->getTDCCount();
  });

  // assign the hits to clock edges based on TDC count
  iAxialHitInClock.clear();
  iAxialHitInClock.resize(1 + trgTime(iAxialHit.back(), iAxialHit.front()) / clockPeriod);
  auto itr = iAxialHitInClock.begin();
  int clockEdge = 1;
  for (const auto& ihit : iAxialHit) {
    B2DEBUG(50, "sorted TDC count: " << m_cdcHits[ihit]->getTDCCount());
    if (trgTime(ihit, iAxialHit.front()) >= clockEdge * clockPeriod) {
      ++clockEdge;
      ++itr;
    }
    itr->push_back(ihit);
  }
  m_iFirstHit = iAxialHit.front();
}

bool CDCTriggerTSFFirmwareModule::notHit(MergerOut field, unsigned iTS, CDCTriggerTSFFirmwareModule::registeredStructElement& reg)
{
  return ! reg[field][iTS];
}

void CDCTriggerTSFFirmwareModule::registerHit(MergerOut field, unsigned iTS,
                                              CDCTriggerTSFFirmwareModule::registeredStructElement& reg)
{
  reg[field].set(iTS);
}

void CDCTriggerTSFFirmwareModule::simulateMerger(unsigned iClock)
{
  if (iClock >= iAxialHitInClock.size()) {
    for (auto& sl : inputToTSF) {
      for (auto& merger : sl) {
        merger.fill(zero_val);
      }
    }
    return;
  }
  // mergerStruct<5> innerDataInClock(nAxialMergers[0]);
  // map<unsigned, mergerStruct<3> > outerDataInClock;
  // mergers in outer super layer has only 3 edge fields,
  // but for the sake of coding simplicity and consistency,
  // let's declare them as the same type of SL0,
  // at the cost of 2 wasted slots.
  map<unsigned, mergerStruct<5> > dataInClock;
  map<unsigned, registeredStruct> registered;
  for (int iAx = 0; iAx < m_nSubModules; ++iAx) {
    dataInClock.insert({2 * iAx, mergerStruct<5> (nAxialMergers[iAx])});
    registered.insert({2 * iAx, registeredStruct(nAxialMergers[iAx])});
  }
  auto clock = iAxialHitInClock[iClock];
  // move first priority hits forward, so that we can just use the 1st hit
  // but still preserve order in each group
  std::stable_partition(clock.begin(), clock.end(),
  [this](int hit) {return priority(hit) == Priority::first;});
  for (const auto& iHit : clock) {
    const CDCHit& hit = *m_cdcHits[iHit];
    const unsigned short iSL = hit.getISuperLayer();
    const short outer = (iSL == 0) ? 0 : 1;
    const unsigned short iMerger = mergerNumber(iHit);
    const unsigned short iCell = mergerCellID(iHit);
    const WireSet tsList = segmentID(iHit);
    const timeVec hitTime = timeStamp(iHit, m_iFirstHit);
    auto& mergerData = dataInClock[iSL][iMerger];
    auto& registeredCell = registered[iSL][iMerger];
    B2DEBUG(50, "iHit: " << iHit << ", Merger" << iSL << "-" << iMerger <<
            ", iCell: " << iCell);
    // update hit map
    get<MergerOut::hitmap>(mergerData)[0].set(iCell);
    // update fastest time
    for (const auto& iTS : tsList) {
      timeVec& fastestTime = (get<MergerOut::fastestTime>(mergerData))[iTS];
      // when there is not already a hit
      if (notHit(MergerOut::fastestTime, iTS, registeredCell) ||
          // or when this one is faster
          hitTime.to_ulong() < fastestTime.to_ulong()) {
        // note: if we move this up to before partition, we won't need to
        // compare timing, since they are already sorted. However,
        // leaving it here makes the code a bit cleaner.
        fastestTime = hitTime;
      }
    }
    switch (priority(iHit)) {
      case Priority::first: {
        // update priority time
        unsigned priTS = iCell % 16;
        timeVec& priorityTime = (get<MergerOut::priorityTime>(mergerData))[priTS];
        // when there is not already a (first priority) hit
        if (notHit(MergerOut::priorityTime, priTS, registeredCell)) {
          priorityTime = hitTime;
          registerHit(MergerOut::priorityTime, priTS, registeredCell);
          get<MergerOut::secondPriorityHit>(mergerData)[0].reset(priTS);
        }
        break;
      }
      case Priority::second: {
        // update the 2 priority times and the sc bits
        for (unsigned i = 0; i < 2; ++i) {
          unsigned priTS = iCell % 16 + i;
          if (priTS > 15) {
            continue;
          }
          timeVec& priorityTime = (get<MergerOut::priorityTime>(mergerData))[priTS];
          // when there is not already a (1st or 2nd priority) hit
          if (notHit(MergerOut::priorityTime, priTS, registeredCell)) {
            priorityTime = hitTime;
            registerHit(MergerOut::priorityTime, priTS, registeredCell);
            get<MergerOut::secondPriorityHit>(mergerData)[0].set(priTS, ! i);
            // set the 2nd pri bit to right when T_left == T_right
          } else if (hitTime.to_ulong() == priorityTime.to_ulong() && i == 1) {
            get<MergerOut::secondPriorityHit>(mergerData)[0].reset(priTS);
          }
        }
        break;
      }
      default:
        break;
    }
    // get edge hit timing or local fastest time
    if (m_edge[outer].find(iHit) != m_edge[outer].end()) {
      for (auto& iEdgeTime : m_edge[outer][iHit]) {
        timeVec& edgeTime = (get<MergerOut::edgeTime>(mergerData))[iEdgeTime];
        if (notHit(MergerOut::edgeTime, iEdgeTime, registeredCell) ||
            hitTime.to_ulong() < edgeTime.to_ulong()) {
          edgeTime = hitTime;
        }
      }
    }
  }
  // pack the output from merger into input to TSF
  for (auto iAx = 0; iAx < m_nSubModules; ++iAx) {
    unsigned nEdges = (iAx == 0) ? 5 : 3;
    for (unsigned iMerger = 0; iMerger < inputToTSF[iAx].size(); ++iMerger) {
      auto input = inputToTSF[iAx][iMerger].begin();
      auto& output = dataInClock[2 * iAx][iMerger];
      pack<MergerOut::hitmap, 1> (input, nWiresInMerger, output);
      pack<MergerOut::priorityTime, timeWidth> (input, nCellsInLayer, output);
      pack<MergerOut::fastestTime, timeWidth> (input, nCellsInLayer, output);
      pack<MergerOut::secondPriorityHit, 1> (input, nCellsInLayer, output);
      pack<MergerOut::edgeTime, timeWidth> (input, nEdges, output);

      if (get<MergerOut::hitmap>(output)[0].any()) {
        B2DEBUG(150, "merger " << iAx * 2 << "-" << iMerger << '\n' <<
                "hitmap:" << get<MergerOut::hitmap>(output)[0] <<
                "second hit:" << get<MergerOut::secondPriorityHit>(output)[0]);
        string priTime, fasTime, edgTime;
        for (int i = 0; i < 16; ++i) {
          priTime += get<MergerOut::priorityTime>(output)[i].to_string() + ",";
          fasTime += get<MergerOut::fastestTime>(output)[i].to_string() + ",";
          edgTime += get<MergerOut::edgeTime>(output)[i].to_string() + ",";
        }
        B2DEBUG(150, "prioiry Time:" << priTime);
        B2DEBUG(150, "fastest Time:" << fasTime);
        B2DEBUG(150, "edge    Time:" << edgTime);
      }
    }
  }
}

template<MergerOut field, size_t width>
void CDCTriggerTSFFirmwareModule::pack(Belle2::CDCTriggerTSFFirmwareModule::inputVector::iterator& input, unsigned number,
                                       mergerStructElement<5>& output)
{
  std::generate(input, input + number * width, [&, n = 0]() mutable {
    unsigned instance = (width == 1) ? 0 : n / width;
    char logic = (get<field>(output)[instance][n % width]) ? one_val : zero_val;
    ++n;
    return logic;});
  input += number * width;
}

void CDCTriggerTSFFirmwareModule::event()
{
  // TODO: what if there is 0 CDC hit?
  int status = 0;

  initializeMerger();
  try {
    for (int iClock = 0; iClock < m_nClockPerEvent; iClock++) {
      B2INFO(string(15, '=') << " clock #" << iClock << " " << string(15, '='));
      simulateMerger(iClock);

      auto TSF0 = getData<0>(inputToTSF);
      auto TSF2 = getData<1>(inputToTSF);
      auto TSF4 = getData<2>(inputToTSF);
      auto TSF6 = getData<3>(inputToTSF);
      auto TSF8 = getData<4>(inputToTSF);
      array<char*, m_nSubModules> rawInputToTSF = {TSF0, TSF2, TSF4, TSF6, TSF8};

      for (unsigned iSL = 0; iSL < m_nSubModules; ++iSL) {
        // skip problematic TSF2 simulation
        if (iSL == 1) continue;
        B2DEBUG(100, "input to TSF" << iSL * 2 << ": \n" <<
                display_value(rawInputToTSF[iSL], nAxialMergers[iSL] * mergerWidth));
        write(rawInputToTSF[iSL], stream[iSL][0]);
      }
      // don't mess up stdout order with child processes
      if (m_debugLevel >= 50) {
        usleep(2000 * m_debugLevel);
      }
      for (unsigned iSL = 0; iSL < m_nSubModules; ++iSL) {
        if (iSL == 1) continue;
        outputToTracker[iSL] = read(stream[iSL][1]);
        if (getLogConfig().getDebugLevel() >= 50) {
          for (const auto& out : outputToTracker[iSL]) {
            display_hex(out);
          }
        }
      }
    }
    B2DEBUG(50, "status code:" << status);
  } catch (std::exception& e) {
    B2ERROR("ERROR: An exception occurred: " << e.what());
    status = 2;
  } catch (...) {
    B2ERROR("ERROR: An unknown exception occurred.");
    status = 3;
  }
}
