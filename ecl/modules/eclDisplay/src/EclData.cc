/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/
//This module
#include <ecl/modules/eclDisplay/EclData.h>

//ROOT
#include <TLeaf.h>
#include <TFile.h>

//Framework
#include <framework/logging/Logger.h>

//ECL
#include <ecl/dataobjects/ECLCalDigit.h>

using namespace Belle2;

const int EclData::ring_start_id[70] = {
  // forward (0-12)
  1,    49,   97,   161,  225,  289,  385,  481,  577,  673,
  769,  865,  1009,
  // barrel (13-58)
  1153, 1297, 1441, 1585, 1729, 1873, 2017, 2161, 2305, 2449,
  2593, 2737, 2881, 3025, 3169, 3313, 3457, 3601, 3745, 3889,
  4033, 4177, 4321, 4465, 4609, 4753, 4897, 5041, 5185, 5329,
  5473, 5617, 5761, 5905, 6049, 6193, 6337, 6481, 6625, 6769,
  6913, 7057, 7201, 7345, 7489, 7633,
  // forward (59-68)
  7777, 7921, 8065, 8161, 8257, 8353, 8449, 8545, 8609, 8673,
  // last_crystal+1
  EclData::getCrystalCount() + 1
};

EclData::EclData()
{
  m_tree = new TTree("tree", "tree");
  m_tree->Branch("ch", &m_branch_ch, "ch/I");
  m_tree->Branch("energy", &m_branch_energy, "energy/D");
  m_tree->Branch("time", &m_branch_time, "time/D");
  m_tree->Branch("evtn", &m_branch_evtn, "evtn/I");

  initVariables();
  initEventRanges();
}

EclData::EclData(const EclData& data)
{
  initVariables();
  initEventRanges();

  cloneFrom(data);
}

EclData& EclData::operator=(const EclData& other)
{
  if (this != &other) { // self-assignment check
    initVariables();
    initEventRanges();

    cloneFrom(other);
  }
  return *this;
}

EclData::~EclData()
{
  delete m_event_counts;
  delete m_energy_sums;
  delete m_tree;
}

void EclData::cloneFrom(const EclData& other)
{
  m_tree = other.m_tree->CloneTree();

  m_tree = new TTree("tree", "tree");
  m_tree->Branch("ch", &m_branch_ch, "ch/I");
  m_tree->Branch("energy", &m_branch_energy, "energy/I");
  m_tree->Branch("time", &m_branch_time, "time/I");
  m_tree->Branch("evtn", &m_branch_evtn, "evtn/I");

  m_last_event_id = other.m_last_event_id;
  m_time_max = other.m_time_max;
  // m_excluded_ch is not copied.
  m_excluded_ch.clear();

  m_event_count_max = other.m_event_count_max;
  m_energy_total = other.m_energy_total;

  for (int i = 0; i < getCrystalCount() + 1; i++) {
    m_event_counts[i] = other.m_event_counts[i];
    m_energy_sums[i] = other.m_energy_sums[i];
  }

  // TODO: Vectors surely can be copied in more simple way.
  for (unsigned int i = 0; i < other.m_event_entry.size(); i++) {
    m_event_entry.push_back(other.m_event_entry[i]);
  }

  m_en_range_min = other.m_en_range_min;
  m_en_range_max = other.m_en_range_max;

  m_time_range_min = other.m_time_range_min;
  m_time_range_max = other.m_time_range_max;

  m_ev_range_min = other.m_ev_range_min;
  m_ev_range_max = other.m_ev_range_max;
}

void EclData::initVariables()
{
  // Setting
  m_branch_energy = m_branch_ch = m_branch_time = m_branch_evtn = 0;

  m_event_counts = new int[getCrystalCount() + 1];
  m_energy_sums = new float[getCrystalCount() + 1];

  m_event_count_max = 0;
  m_energy_sums_max = 0;
  m_energy_total = 0;

  m_last_event_id = -1;

  m_time_max = 0;

  m_excluded_ch.clear();
}

void EclData::initEventRanges()
{
  m_en_range_min = 0;
  m_en_range_max = -1;

  m_time_range_min = -2048;
  m_time_range_max = m_time_max;

  m_ev_range_min = 0;
  m_ev_range_max = m_last_event_id;
}

int EclData::getCrystalCount()
{
  return 8736;
}

TTree* EclData::getTree()
{
  return m_tree;
}

int* EclData::getEventCounts()
{
  return m_event_counts;
}
// Alias for getEventCounts()
int* EclData::getEventCountsPerCrystal()
{
  return getEventCounts();
}

int EclData::getEventCountsMax()
{
  return m_event_count_max;
}

float* EclData::getEnergySums()
{
  return m_energy_sums;
}
// Alias for getEnergySums()
float* EclData::getEnergySumPerCrystal()
{
  return getEnergySums();
}

float EclData::getEnergySumsMax()
{
  return m_energy_sums_max;
}

float EclData::getEnergyTotal()
{
  return m_energy_total;
}

bool EclData::isCrystalInSubsystem(int crystal, EclData::EclSubsystem subsys)
{
  switch (subsys) {
    case ALL:
      return true;
    case BARR:
      return crystal >= 1153 && crystal <= 7776;
    case FORW:
      return crystal < 1153;
    case BACKW:
      return crystal > 7776;
    default:
      return false;
  }
}

int EclData::getTimeRangeMin()
{
  return m_time_range_min;
}
int EclData::getTimeRangeMax()
{
  return m_time_range_max;
}

void EclData::setTimeRange(int time_min, int time_max, bool do_update)
{
  if (m_time_range_min == time_min && m_time_range_max == time_max)
    return;

  m_time_range_min = time_min;
  m_time_range_max = time_max;
  if (do_update)
    update();
}

int EclData::getEventRangeMin()
{
  return m_ev_range_min;
}
int EclData::getEventRangeMax()
{
  return m_ev_range_max;
}
void EclData::setEventRange(int ev_min, int ev_max, bool do_update)
{
  if (ev_min == m_ev_range_min && ev_max == m_ev_range_max)
    return;

  m_ev_range_min = ev_min;
  m_ev_range_max = ev_max;
  if (do_update)
    update();
}

void EclData::setEnergyThreshold(int en_min, int en_max, bool do_update)
{
  if (en_min == m_en_range_min && en_max == m_en_range_max)
    return;

  m_en_range_min = en_min;
  m_en_range_max = en_max;
  if (do_update)
    update();
}

int EclData::getTimeMin()
{
  return m_time_range_min;
}
int EclData::getTimeMax()
{
  return m_time_max;
}

int EclData::getLastEventId()
{
  return m_last_event_id;
}

int EclData::getChannel(int phi_id, int theta_id)
{
  return ring_start_id[theta_id] + phi_id;
}

int EclData::getPhiId(int _ch)
{
  for (int i = 0; i < 69; i++) {
    if (_ch < ring_start_id[i + 1])
      return _ch - ring_start_id[i];
  }

  return -1;
}

int EclData::getThetaId(int _ch)
{
  for (int i = 0; i < 69; i++) {
    if (_ch < ring_start_id[i + 1])
      return i;
  }

  return -1;
}

void EclData::excludeChannel(int _ch, bool do_update)
{
  m_excluded_ch.insert(_ch);
  if (do_update)
    update();
}

void EclData::includeChannel(int _ch, bool do_update)
{
  m_excluded_ch.erase(_ch);
  if (do_update)
    update();
}

void EclData::loadRootFile(const char* path)
{
  // TODO: Move this process to some new reset() method.
  m_excluded_ch.clear();
  m_event_entry.clear();
  m_tree->Reset();
  m_last_event_id = -1;
  m_en_range_max = -1;
  m_time_max = 0;

  TFile* file = new TFile(path, "READ");
  TTree* tree = (TTree*)file->Get("tree");
  long nentries = tree->GetEntries();

  tree->SetBranchAddress("ECLCalDigits.m_CellId", &m_branch_ch);
  tree->SetBranchAddress("ECLCalDigits.m_Energy", &m_branch_energy);
  tree->SetBranchAddress("ECLCalDigits.m_Time"  , &m_branch_time);

  TLeaf* leafCellId;
  TLeaf* leafEnergy;
  TLeaf* leafTimeFit;

  leafCellId  = tree->GetLeaf("ECLCalDigits.m_CellId");
  leafEnergy  = tree->GetLeaf("ECLCalDigits.m_Energy");
  leafTimeFit = tree->GetLeaf("ECLCalDigits.m_Time");

  for (long i = 0; i < nentries; i++) {
    tree->GetEntry(i);

    const int len = leafCellId->GetLen();

    if (len > 0) {
      m_last_event_id++;
      m_branch_evtn = m_last_event_id;

      m_event_entry.push_back(m_tree->GetEntries());
    }

    for (int j = 0; j < len; j++) {
      m_branch_ch     = leafCellId ->GetTypedValue<int>(j);
      m_branch_energy = leafEnergy ->GetTypedValue<double>(j) * 1e3;
      m_branch_time   = leafTimeFit->GetTypedValue<double>(j);

      if (m_time_max < m_branch_time) m_time_max = m_branch_time;

      B2DEBUG(10, "Added event ECLCalDigits for crystal " << m_branch_ch);

      // TODO: Add energy cut.
      m_tree->Fill();
    }
  }

  update(true);
}

void EclData::update(bool reset_event_ranges)
{
  int start, end;

  if (reset_event_ranges) initEventRanges();

  if (m_ev_range_min < 0)
    return;
  if (m_ev_range_min > m_last_event_id || m_ev_range_min > m_ev_range_max)
    return;

  start = m_event_entry[m_ev_range_min];
  if (m_ev_range_max < m_last_event_id)
    end = m_event_entry[m_ev_range_max + 1];
  else
    end = m_tree->GetEntries();

  for (int i = 1; i <= getCrystalCount(); i++) {
    m_event_counts[i] = 0;
    m_energy_sums[i] = 0;
  }
  m_event_count_max = 0;
  m_energy_sums_max = 0;
  m_energy_total = 0;

  for (int i = start; i < end; i++) {
    m_tree->GetEntry(i);

    if (m_excluded_ch.count(m_branch_ch) > 0) continue;

    if (m_branch_ch >= 1 && m_branch_ch <= getCrystalCount()) {
      // Check if current time belongs to time_range, set by user.
      if (m_time_range_max >= 0)
        if (m_branch_time < m_time_range_min || m_branch_time > m_time_range_max)
          continue;

      if (m_en_range_max >= 0)
        if (m_branch_energy < m_en_range_min || m_branch_energy > m_en_range_max)
          continue;

      m_event_counts[m_branch_ch]++;
      if (m_event_count_max < m_event_counts[m_branch_ch])
        m_event_count_max = m_event_counts[m_branch_ch];

      m_energy_sums[m_branch_ch] += m_branch_energy;
      m_energy_total += m_branch_energy;

      if (m_energy_sums_max < m_energy_sums[m_branch_ch])
        m_energy_sums_max = m_energy_sums[m_branch_ch];
    }
  }

  if (m_time_range_max < 0)
    m_time_range_max = m_time_max;
  if (m_ev_range_max < 0)
    m_ev_range_max = m_last_event_id;

  B2DEBUG(250, end - start << " events handled.");
}

int EclData::addEvent(ECLCalDigit* event, int _evtn)
{
  if (event->getEnergy() <= 0 || event->getCellId() <= 0) {
    return -1;
  }

  m_branch_ch     = event->getCellId();
  m_branch_energy = event->getEnergy() * 1e3; // To MeV
  m_branch_time   = event->getTime();
  m_branch_evtn   = _evtn;

  if (m_time_max < m_branch_time) m_time_max = m_branch_time;
  if (m_last_event_id < _evtn) {
    m_last_event_id = _evtn;
    m_event_entry.push_back(m_tree->GetEntries());
  }

  B2DEBUG(200, "Added event #" << _evtn << ", ch:" << m_branch_ch
          << ", energy:" << m_branch_energy << ", time:" << m_branch_time);

  m_tree->Fill();

  return 0;
}

void EclData::fillEnergyHistogram(TH1F* hist, int energy_min, int energy_max, EclSubsystem subsys)
{
  int start, end;

  if (m_ev_range_min < 0)
    return;
  if (m_ev_range_min > m_last_event_id || m_ev_range_min > m_ev_range_max)
    return;

  start = m_event_entry[m_ev_range_min];
  if (m_ev_range_max < m_last_event_id)
    end = m_event_entry[m_ev_range_max + 1];
  else
    end = m_tree->GetEntries();

  for (int i = start; i < end; i++) {
    m_tree->GetEntry(i);
    if (isCrystalInSubsystem(m_branch_ch, subsys)) {
      if (m_branch_energy >= energy_min && m_branch_energy <= energy_max)
        hist->Fill(m_branch_energy);
    }
  }
}

void EclData::fillEnergySumHistogram(TH1F* hist, int energy_min, int energy_max, EclData::EclSubsystem subsys)
{
  int start, end;

  if (m_ev_range_min < 0)
    return;
  if (m_ev_range_min > m_last_event_id || m_ev_range_min > m_ev_range_max)
    return;

  start = m_event_entry[m_ev_range_min];
  if (m_ev_range_max < m_last_event_id)
    end = m_event_entry[m_ev_range_max + 1];
  else
    end = m_tree->GetEntries();

  m_tree->GetEntry(start);
  int cur_evtn = m_branch_evtn;
  int energy_sum = 0;

  for (int i = start; i < end; i++) {
    m_tree->GetEntry(i);

    // After reading all of the data from event cur_evtn,
    // save the sum in histogram.
    if (m_branch_evtn > cur_evtn) {
      if (energy_sum > 0) hist->Fill(energy_sum);
      energy_sum = 0;
      cur_evtn = m_branch_evtn;
    }

    if (isCrystalInSubsystem(m_branch_ch, subsys)) {
      if (m_branch_energy >= energy_min && m_branch_energy <= energy_max)
        energy_sum += m_branch_energy;
    }
  }

  // Add last selected event.
  if (energy_sum > 0) hist->Fill(energy_sum);
}

void EclData::fillTimeHistogram(TH1F* hist, int time_min, int time_max, EclData::EclSubsystem subsys)
{
  int start, end;

  if (m_ev_range_min < 0)
    return;
  if (m_ev_range_min > m_last_event_id || m_ev_range_min > m_ev_range_max)
    return;

  start = m_event_entry[m_ev_range_min];
  if (m_ev_range_max < m_last_event_id)
    end = m_event_entry[m_ev_range_max + 1];
  else
    end = m_tree->GetEntries();

  for (int i = start; i < end; i++) {
    m_tree->GetEntry(i);

    if (m_en_range_max >= 0)
      if (m_branch_energy < m_en_range_min || m_branch_energy > m_en_range_max)
        continue;

    if (isCrystalInSubsystem(m_branch_ch, subsys)) {
      if (m_branch_time >= time_min && m_branch_time <= time_max)
        hist->Fill(m_branch_time);
    }
  }
}
