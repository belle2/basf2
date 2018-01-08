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
#include <trg/cdc/CDCTrigger.h>
#include <trg/cdc/Cosim.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <trg/cdc/dataobjects/Bitstream.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
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
using TSF = CDCTriggerTSFFirmwareModule;

REG_MODULE(CDCTriggerTSFFirmware)

constexpr std::array<int, TSF::m_nSubModules> TSF::nAxialMergers;

TSF::CDCTriggerTSFFirmwareModule() :
  Module(), m_cdcHits{""}

{
  for (unsigned iAx = 0; iAx < m_nSubModules; ++iAx) {
    inputToTSF[iAx].resize(nAxialMergers[iAx]);
    for (auto& clock : m_priorityHit) {
      clock.insert({2 * iAx, priorityHitStructInSL(nAxialMergers[iAx])});
    }
    dataAcrossClocks.insert({2 * iAx, mergerStruct<5> (nAxialMergers[iAx])});
  }
  //Set module properties
  setDescription("Firmware simulation of the Track Segment Finder for CDC trigger.");

  // Define module parameters
  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCHits.",
           string("CDCHits"));
  addParam("outputCollectionName", m_outputCollectionName,
           "Name of the StoreArray holding the found TS hits.",
           string("CDCTriggerFirmwareSegmentHits"));
  addParam("outputBitstreamNameTo2D", m_outputBitstreamNameTo2D,
           "Name of the StoreArray holding the raw bit content to 2D trackers",
           string("BitstreamTSFto2D"));
  addParam("mergerOnly", m_mergerOnly,
           "Flag to only simulate merger and not TSF",
           false);
  addParam("simulateCC", m_simulateCC,
           "Flag to run the front-end clock counter",
           false);
  std::vector<bool> defaultStub(m_nSubModules, false);
  addParam("stubLUT", m_stubLUT,
           "list of flags to run each TSF firmware simulation with dummy L/R LUT (to speed up loading)",
           defaultStub);
}

TSF::~CDCTriggerTSFFirmwareModule()
{}

Priority TSF::priority(int index)
{
  CDCHit* hit = m_cdcHits[index];
  int offset = (hit->getISuperLayer() == 0) ? 1 : 0;
  switch (hit->getILayer() - 2 - offset) {
    case 0: return Priority::first;
    case 1: return Priority::second;
    default: return Priority::nothing;
  }
}

void TSF::write(const char* message, FILE* outstream)
{
  // write input to TSF firmware
  fprintf(outstream, "%s\n" , message);
  fflush(outstream);
}

TSF::outputArray TSF::read(FILE* instream)
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
  for (auto i2d = 0; i2d < 4; ++i2d) {
    B2DEBUG(50, display_value(buffer.data() + width_out * i2d, width_out));
  }
  auto bufferItr = buffer.cbegin();
  // for (int iTracker = 0; iTracker < nTrackers; ++iTracker) {
  for (int iTracker = nTrackers - 1; iTracker >= 0; --iTracker) {
    copy(bufferItr, bufferItr + width_out, output[iTracker].begin());
    bufferItr += width_out;
  }
  return output;
}

void TSF::initialize()
{
  m_cdcHits.isRequired(m_hitCollectionName);
  m_debugLevel = getLogConfig().getDebugLevel();
  if (m_mergerOnly) {
    computeEdges();
    return;
  }
  m_bitsTo2D.registerInDataStore(m_outputBitstreamNameTo2D);
  m_tsHits.registerInDataStore(m_outputCollectionName);
  for (unsigned i = 0; i < m_nSubModules; ++i) {
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
      if (m_stubLUT[i]) {
        design_libname_post = "_stub" + design_libname_post;
      }
      string design = design_libname_pre + to_string(i * 2) + design_libname_post;
      string waveform = wdbName_pre + to_string(i * 2) + wdbName_post;
      // execute the standalone worker program
      execlp("CDCTriggerTSFFirmwareWorker", "CDCTriggerTSFFirmwareWorker",
             str_fd[0].c_str(), str_fd[1].c_str(), design.c_str(), waveform.c_str(),
             to_string(nAxialMergers[i]).c_str(), nullptr);
      B2FATAL("The firmware simulation program didn't launch!");
    } else {
      /* Parent process (BASF2) */
      B2DEBUG(100, "parent " << i);
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
  B2INFO("It can take a while for TSF0 to load the LUT.");
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    // wait for worker initialization
    read(stream[i][1]);
  }
}

void TSF::terminate()
{
  B2DEBUG(10, "Waiting for TSF firmware termination...");
  wait();
  for (unsigned i = 0; i < m_nSubModules; ++i) {
    close(inputFileDescriptor[i][1]);
    close(outputFileDescriptor[i][0]);
  }
}

void TSF::computeEdges()
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
char* TSF::getData(inputToTSFArray input)
{
  static array<char, mergerWidth* nAxialMergers[iSL]> data;
  // 2-D array in XSI is totally in LSB, like this
  // ((012), (345), (678))
  auto itr = data.end() - mergerWidth;
  for (const auto& merger : input[iSL]) {
    copy(merger.begin(), merger.end(), itr);
    itr -= mergerWidth;
  }
  return data.data();
}

unsigned short TSF::trgTime(int index, int iFirstHit)
{
  if (m_allPositiveTime) {
    return (m_cdcHits[iFirstHit]->getTDCCount() / 2 - m_cdcHits[index]->getTDCCount() / 2);
  } else {
    short time = (m_TDCCountForT0 / 2 - m_cdcHits[index]->getTDCCount() / 2);
    return (time < 0) ? time + (1 << 9) : time;
  }
}

std::bitset<4> TSF::timeStamp(int index, int iFirstHit)
{
  return std::bitset<4> (trgTime(index, iFirstHit) % clockPeriod);
}

unsigned short TSF::mergerCellID(int index)
{
  const CDCHit& hit = (*m_cdcHits[index]);
  const unsigned offset = (hit.getISuperLayer() == 0) ? 3 : 0;
  return (hit.getILayer() - offset) * nSegmentsInMerger +
         hit.getIWire() % nSegmentsInMerger;
}

unsigned short TSF::mergerNumber(int index)
{
  const CDCHit& hit = (*m_cdcHits[index]);
  return hit.getIWire() / nSegmentsInMerger;
}

using WireSet = std::vector<unsigned short>;
using TSMap = std::unordered_map<int, WireSet>;

TSF::WireSet TSF::segmentID(int iHit)
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

void TSF::initializeMerger()
{
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
    B2DEBUG(20, "sorted TDC count: " << m_cdcHits[ihit]->getTDCCount() <<
            ", SL" << m_cdcHits[ihit]->getISuperLayer() << ", layer" << m_cdcHits[ihit]->getILayer() <<
            ", wire " << m_cdcHits[ihit]->getIWire() << ", ihit: " << ihit);
    if (trgTime(ihit, iAxialHit.front()) >= clockEdge * clockPeriod) {
      ++clockEdge;
      ++itr;
    }
    itr->push_back(ihit);
  }
  m_iFirstHit = iAxialHit.front();

  // In addition, clear the list holding priority hit indices
  for (auto& clock : m_priorityHit) {
    for (auto& sl : clock) {
      for (auto& merger : sl.second) {
        merger.clear();
      }
    }
  }
}

bool TSF::notHit(MergerOut field, unsigned iTS, TSF::registeredStructElement& reg)
{
  return ! reg[field][iTS];
}

void TSF::registerHit(MergerOut field, unsigned iTS, TSF::registeredStructElement& reg)
{
  reg[field].set(iTS);
}

void TSF::setSecondPriority(unsigned priTS,
                            unsigned iHit,
                            timeVec hitTime,
                            unsigned lr,
                            mergerStructElement<5>& mergerData,
                            registeredStructElement& registeredCell,
                            priorityHitInMerger& priorityHit)
{
  timeVec& priorityTime = (get<MergerOut::priorityTime>(mergerData))[priTS];
  // when there is not already a (1st or 2nd priority) hit
  if (notHit(MergerOut::priorityTime, priTS, registeredCell)) {
    priorityTime = hitTime;
    registerHit(MergerOut::priorityTime, priTS, registeredCell);
    get<MergerOut::secondPriorityHit>(mergerData)[0].set(priTS, ! lr);
    priorityHit.insert({priTS, iHit});
    // set the 2nd pri bit to right when T_left == T_right
  } else if (priority(priorityHit[priTS]) == Priority::second &&
             hitTime.to_ulong() == priorityTime.to_ulong() && lr == 1) {
    get<MergerOut::secondPriorityHit>(mergerData)[0].reset(priTS);
    priorityHit[priTS] = iHit;
  }
}

void TSF::simulateMerger(unsigned iClock)
{
  // clean up TSF input signals
  for (auto& sl : inputToTSF) {
    for (auto& merger : sl) {
      merger.fill(zero_val);
    }
  }
  // don't do simulation if there are no more CDC hits
  if (iClock >= iAxialHitInClock.size()) {
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
    // register the priority hit in the corresponding merger
    auto& priorityHit = m_priorityHit[iClock][iSL][iMerger];
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
        registerHit(MergerOut::fastestTime, iTS, registeredCell);
      }
    }
    // get edge hit timing or local fastest time
    if (m_edge[outer].find(iCell) != m_edge[outer].end()) {
      for (auto& iEdgeTime : m_edge[outer][iCell]) {
        timeVec& edgeTime = (get<MergerOut::edgeTime>(mergerData))[iEdgeTime];
        if (notHit(MergerOut::edgeTime, iEdgeTime, registeredCell) ||
            hitTime.to_ulong() < edgeTime.to_ulong()) {
          edgeTime = hitTime;
          registerHit(MergerOut::edgeTime, iEdgeTime, registeredCell);
        }
      }
    }
    switch (priority(iHit)) {
      case Priority::first: {
        // update priority time
        unsigned priTS = iCell % nSegmentsInMerger;
        timeVec& priorityTime = (get<MergerOut::priorityTime>(mergerData))[priTS];
        // when there is not already a (first priority) hit
        if (notHit(MergerOut::priorityTime, priTS, registeredCell)) {
          priorityTime = hitTime;
          registerHit(MergerOut::priorityTime, priTS, registeredCell);
          get<MergerOut::secondPriorityHit>(mergerData)[0].reset(priTS);
          priorityHit.insert({priTS, iHit});
        }
        break;
      }
      case Priority::second: {
        // update the 2 priority times and the sc bits
        for (unsigned i = 0; i < 2; ++i) {
          unsigned priTS = iCell % nSegmentsInMerger + i;
          if (priTS == nSegmentsInMerger) {
            // crossing the left edge
            // set the 2nd priority left of the first TS in the next merger
            priTS = 0;
            const unsigned short iMergerPlus1 = (iMerger == nMergers[iSL] - 1) ? 0 : iMerger + 1;
            auto& nextMergerData = dataInClock[iSL][iMergerPlus1];
            auto& nextRegisteredCell = registered[iSL][iMergerPlus1];
            auto& nextPriorityHit = m_priorityHit[iClock][iSL][iMergerPlus1];
            setSecondPriority(priTS, iHit, hitTime, i, nextMergerData, nextRegisteredCell, nextPriorityHit);
          } else {
            setSecondPriority(priTS, iHit, hitTime, i, mergerData, registeredCell, priorityHit);
          }
        }
        break;
      }
      default:
        break;
    }
  }
  // pack the output from merger into input to TSF
  for (auto iAx = 0; iAx < m_nSubModules; ++iAx) {
    unsigned nEdges = (iAx == 0) ? 5 : 3;
    for (unsigned iMerger = 0; iMerger < inputToTSF[iAx].size(); ++iMerger) {
      auto input = inputToTSF[iAx][iMerger].rbegin();
      auto& output = dataInClock[2 * iAx][iMerger];
      pack<MergerOut::hitmap, 1> (input, nWiresInMerger, output);
      pack<MergerOut::priorityTime, timeWidth> (input, nCellsInLayer, output);
      pack<MergerOut::fastestTime, timeWidth> (input, nCellsInLayer, output);
      pack<MergerOut::secondPriorityHit, 1> (input, nCellsInLayer, output);
      pack<MergerOut::edgeTime, timeWidth> (input, nEdges, output);

      auto& outputAcrossClocks = dataAcrossClocks[2 * iAx][iMerger];
      if (get<MergerOut::hitmap>(output)[0].any()) {
        string priTime, fasTime, edgTime;
        for (int i = 0; i < nSegmentsInMerger; ++i) {
          priTime += get<MergerOut::priorityTime>(output)[i].to_string() + ",";
          fasTime += get<MergerOut::fastestTime>(output)[i].to_string() + ",";
          edgTime += get<MergerOut::edgeTime>(output)[i].to_string() + ",";
          get<MergerOut::priorityTime>(outputAcrossClocks)[i] |=
            get<MergerOut::priorityTime>(output)[i];
          get<MergerOut::fastestTime>(outputAcrossClocks)[i] |=
            get<MergerOut::fastestTime>(output)[i];
        }
        B2DEBUG(150, "merger " << iAx * 2 << "-" << iMerger <<
                "\nhitmap:       " << get<MergerOut::hitmap>(output)[0] <<
                "\nsecond hit:   " << get<MergerOut::secondPriorityHit>(output)[0] <<
                "\nprioiry Time: " << priTime <<
                "\nfastest Time: " << fasTime <<
                "\nedge    Time: " << edgTime);
        get<MergerOut::hitmap>(outputAcrossClocks)[0] |= get<MergerOut::hitmap>(output)[0];
      }
      // simulate clock counter
      if (m_simulateCC) {
        bitset<9> cc(iClock);
        generate(inputToTSF[iAx][iMerger].rend() - 9,
        inputToTSF[iAx][iMerger].rend(), [&cc, i = 0]() mutable {
          return (cc[i++]) ? one_val : zero_val;});
      }
    }
  }
}

template<MergerOut field, size_t width>
void TSF::pack(inputVector::reverse_iterator& input, unsigned number,
               mergerStructElement<5>& output)
{
  std::generate(input, input + number * width, [&, n = 0]() mutable {
    int i = n++;
    if (width == 1)
    {
      return (get<field>(output)[0][i]) ? one_val : zero_val;
    } else {
      return (get<field>(output)[i / width][i % width]) ? one_val : zero_val;}});
  input += number * width;
}

void TSF::saveFirmwareOutput()
{
  m_bitsTo2D.appendNew(outputToTracker);
}

constexpr std::array<int, 9> TSF::nMergers;

void TSF::saveFastOutput(short iclock)
{
  const int ccWidth = 9;
  // number of widended clocks in TSF
  const int widenedClocks = 16;
  // TSF latency in unit of data clocks
  const int latency = 3;
  // N.B. widenedClocks and latency might be inaccurate

  int iAx = 0;
  for (const auto& sl : outputToTracker) {
    // keep a list of found TS hit in the same SL so that outputs to different
    // trackers won't be recorded twice
    std::map<unsigned, int> foundTS;
    unsigned short iTracker = 0;
    for (auto& tracker : sl) {
      string output = slv_to_bin_string(tracker);
      // loop through all TS hits in the output
      for (int i = ccWidth; i < width_out; i += tsInfoWidth) {
        string ts = output.substr(i, tsInfoWidth);
        tsOut decoded = decodeTSHit(ts);
        // finish if no more TS hit is present
        if (decoded[3] == 0) {
          break;
        }
        const unsigned nCellsInSL = nAxialMergers[iAx] * nCellsInLayer;
        // get global TS ID
        unsigned iTS = decoded[0] + nCellsInSL * iTracker / nTrackers;
        // periodic ID overflow when phi0 > 0 for the 4th tracker
        if (iTS >= nCellsInSL) {
          iTS -= nCellsInSL;
        }
        // ID in SL8 is shifted by 16
        if (iAx == 4) {
          if (iTS < 16) {
            iTS += nCellsInSL;
          }
          iTS -= 16;
        }
        int iHit = -1;
        int firstClock = iclock - widenedClocks - latency;
        if (firstClock < 0) {
          firstClock = 0;
        }
        int lastClock = iclock - latency;
        B2DEBUG(10, "iAx:" << iAx << ", iTS:" << iTS << ", iTracker: " << iTracker);
        // scan through all CDC hits to get the priority hit
        for (int iclkPri = firstClock; iclkPri < lastClock; ++iclkPri) {
          // get the map storing the hit index in the corresponding merger
          auto priMap = m_priorityHit[iclkPri][2 * iAx][iTS / nSegmentsInMerger];
          unsigned itsInMerger = iTS % nSegmentsInMerger;
          // Pick up the first CDCHit which agrees to the priority position
          // of firmware sim output
          if (priMap.find(itsInMerger) != priMap.end() &&
              toPriority(decoded[3]) == priority(priMap[itsInMerger])) {
            iHit = priMap[itsInMerger];
            B2DEBUG(10, "iHit:" << iHit);
            B2DEBUG(10, "TDC: " << m_cdcHits[iHit]->getTDCCount() << ", TSF: " << decoded[1]);
            break;
          }
        }
        // check if the same TS hit to another tracker is already there
        // TODO: some duplicates are from different clocks,
        // so they won't be caught here
        // This mostly likely means the wrong phase of TSF firmware output is used
        if (foundTS.find(iTS) != foundTS.end()) {
          if (iHit != foundTS[iTS]) {
            B2WARNING("Same TS ID exists, but they point to different CDC hit");
          }
          continue;
        }
        foundTS.insert({iTS, iHit});
        if (iHit < 0) {
          B2WARNING("No corresponding priority CDC hit can be found.");
          B2WARNING("Maybe the latency and number of widened clocks are wrong.");
          B2WARNING("In event " << StoreObjPtr<EventMetaData>()->getEvent());
          B2DEBUG(20, "priority " << decoded[3]);
          for (int iclkPri = 0; iclkPri < m_nClockPerEvent; ++iclkPri) {
            auto priMap = m_priorityHit[iclkPri][2 * iAx][iTS / 16];
            for (auto& m : priMap) {
              B2DEBUG(20, "iWire: " << m.first << ", iLayer: " <<
                      m_cdcHits[m.second]->getILayer() << ", iClock: " << iclkPri);
            }
          }
          if (iclock < 15) {
            B2WARNING("It could be the left over TS hits from the last event.\n" <<
                      "Try increasing m_nClockPerEvent");
          }
        }
        CDCHit cdchit = (iHit < 0) ? CDCHit() : *m_cdcHits[iHit];
        m_tsHits.appendNew(cdchit,
                           globalSegmentID(iTS, 2 * iAx),
                           decoded[3], // priority position
                           decoded[2], // L/R
                           // TODO: pri time should be the same as the CDCHit in 1st arg
                           // when the proper clock counter is given to firmware TSF
                           decoded[1],
                           // TODO: fastest time from ETF output
                           0,
                           iclock);
      }
      ++iTracker;
    }
    ++iAx;
  }
}

void TSF::event()
{
  // TODO: what if there is 0 CDC hit?
  int status = 0;
  initializeMerger();
  try {
    // clear accumulative merger output
    for (unsigned iAx = 0; iAx < m_nSubModules; ++iAx) {
      dataAcrossClocks[2 * iAx] = mergerStruct<5> (nAxialMergers[iAx]);
    }
    for (int iClock = 0; iClock < m_nClockPerEvent; iClock++) {
      B2INFO(string(15, '=') << " clock #" << iClock << " " << string(15, '='));
      if (iClock == m_nClockPerEvent - 1) {
        cout << "Accumulative hitmap:" << "\n";
      }
      simulateMerger(iClock);
      if (m_mergerOnly) {
        cout << std::hex;
        int iSL = 0;
        for (const auto& sl : inputToTSF) {
          int iMerger = 0;
          for (const auto& mergerOut : sl) {
            if (std::any_of(mergerOut.begin(), mergerOut.end(), [](char i) {
            return i != zero_val;
          })) {
              cout << "Merger " << iSL * 2 << "-" << iMerger / 2 << " u" << iMerger % 2 << ": ";
              display_hex(mergerOut);
            }
            // print accumulative hitmap
            if (iClock == m_nClockPerEvent - 1) {
              auto accuOut = dataAcrossClocks[2 * iSL][iMerger];
              if (get<MergerOut::hitmap>(accuOut)[0].any()) {
                cout << "Merger " << iSL * 2 << "-" << iMerger / 2 << " u" << iMerger % 2 << ": ";
                cout << get<MergerOut::hitmap>(accuOut)[0] << "\n";
                string priTime, fasTime;
                for (int i = 0; i < 16; ++i) {
                  priTime += get<MergerOut::priorityTime>(accuOut)[i].to_string() + ",";
                  fasTime += get<MergerOut::fastestTime>(accuOut)[i].to_string() + ",";
                }
                cout << "pritime: " << priTime << "\n";
                cout << "fastime: " << fasTime << "\n";
              }
            }
            iMerger++;
          }
          iSL++;
        }
        cout << std::dec;
        continue;
      }

      auto TSF0 = getData<0>(inputToTSF);
      auto TSF2 = getData<1>(inputToTSF);
      auto TSF4 = getData<2>(inputToTSF);
      auto TSF6 = getData<3>(inputToTSF);
      auto TSF8 = getData<4>(inputToTSF);
      array<char*, m_nSubModules> rawInputToTSF = {TSF0, TSF2, TSF4, TSF6, TSF8};

      for (unsigned iSL = 0; iSL < m_nSubModules; ++iSL) {
        B2DEBUG(100, "input to TSF" << iSL * 2 << ": \n" <<
                display_value(rawInputToTSF[iSL], nAxialMergers[iSL] * mergerWidth));
        write(rawInputToTSF[iSL], stream[iSL][0]);
      }
      // don't mess up stdout order with child processes
      if (m_debugLevel >= 50) {
        usleep(2000 * m_debugLevel);
      }
      for (unsigned iSL = 0; iSL < m_nSubModules; ++iSL) {
        B2DEBUG(50, "Reading buffer from TSF " << iSL * 2 << ":");
        outputToTracker[iSL] = read(stream[iSL][1]);
        B2DEBUG(30, "received TSF " << iSL * 2 << ":");
        if (m_debugLevel >= 30) {
          for (const auto& out : outputToTracker[iSL]) {
            display_hex(out);
          }
        }
      }
      saveFirmwareOutput();
      saveFastOutput(iClock);
    }
  } catch (std::exception& e) {
    B2ERROR("ERROR: An exception occurred: " << e.what());
    status = 2;
  } catch (...) {
    B2ERROR("ERROR: An unknown exception occurred.");
    status = 3;
  }
  B2DEBUG(50, "status code:" << status);
}
