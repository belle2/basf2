#include <eutel/eudaq/FileReader.h>
#include <eutel/eudaq/FileNamer.h>
#include <eutel/eudaq/PluginManager.h>
#include <eutel/eudaq/Event.h>
#include <eutel/eudaq/Logger.h>

#include <list>

namespace eudaq {

  namespace {
    static const unsigned TLUID = Event::str2id("_TLU");
    static const unsigned IDMASK = 0x7fff;
  }

  std::ostream& operator << (std::ostream&, const FileReader::eventqueue_t&);

  struct FileReader::eventqueue_t {
    struct item_t {
      item_t(DetectorEvent* ev = 0) : event(ev)
      {
        if (ev) {
          for (size_t i = 0; i < ev->NumEvents(); ++i) {
            triggerids.push_back(PluginManager::GetTriggerID(*ev->GetEvent(i)));
          }
        }
      }
      eudaq::DetectorEvent* event;
      std::vector<unsigned>  triggerids;
    };
    eventqueue_t(unsigned numproducers = 0)
      : offsets(numproducers, items.end()), firstid((unsigned) - 1), lastid(0) {}
    bool isempty() const
    {
      for (size_t i = 0; i < offsets.size(); ++i) {
        if (events(i) == 0) {
          return true;
        }
      }
      return false;
    }
    size_t events(size_t producer) const
    {
      std::list<item_t>::const_iterator it = iter(producer, -1);
      if (it == items.begin()) return 0;
      return std::distance(items.begin(), it);
    }
    size_t fullevents() const
    {
      size_t min = events(0);
      for (size_t i = 1; i < offsets.size(); ++i) {
        size_t evts = events(i);
        if (evts < min) min = evts;
      }
      return min;
    }
    void push(eudaq::Event* ev)
    {
      DetectorEvent* dev = dynamic_cast<DetectorEvent*>(ev);
      items.push_front(item_t(dev));
    }
    void discardevent(size_t producer)
    {
      --offsets[producer];
    }
    int clean_back()
    {
      int result = 0;
      bool done = false;
      while (!done) {
        for (size_t i = 0; i < producers(); ++i) {
          std::list<item_t>::const_iterator it = offsets.at(i);
          if (it == items.end()) return result;
          ++it;
          if (it == items.end()) {
            done = true;
            offsets[i] = items.end();
          }
        }
        ++result;
        items.pop_back();
      }
      return result;
    }
    eudaq::DetectorEvent* popevent()
    {
      unsigned run = getevent(0).GetRunNumber();
      unsigned evt = getevent(0).GetEventNumber();
      unsigned long long ts = NOTIMESTAMP;
      for (size_t i = 0; i < producers(); ++i) {
        if (getevent(i).get_id() == TLUID) {
          run = getevent(i).GetRunNumber();
          evt = getevent(i).GetEventNumber();
          ts = getevent(i).GetTimestamp();
          break;
        }
      }
      DetectorEvent* dev = new DetectorEvent(run, evt, ts);
      for (size_t i = 0; i < producers(); ++i) {
        dev->AddEvent(iter(i)->event->GetEventPtr(i));
        --offsets[i];
      }
      bool more = true;
      do {
        for (size_t i = 0; i < producers(); ++i) {
          /*std::list<item_t>::const_iterator it =*/ iter(i, -1);
        }
        clean_back();
        for (size_t i = 0; i < producers(); ++i) {
          /*std::list<item_t>::const_iterator it =*/ iter(i, -1); //, true);
        }
        more = true;
        for (size_t i = 0; i < producers(); ++i) {
          if (iter(i, -1) == items.end()) {
            more = false;
            break;
          }
        }
      } while (more);
      return dev;
    }
    void debug(std::ostream& os) const
    {
      os << "empty=" << (isempty() ? "yes" : "no") << std::flush;
      os << " fullevents=" << fullevents() << std::flush
         << " events=" << events(0) << std::flush;
      for (size_t i = 1; i < producers(); ++i) {
        os << "," << events(i) << std::flush;
      }
    }
    std::list<item_t>::const_iterator iter(size_t producer, int offset = 0) const
    {
      std::list<item_t>::const_iterator it = offsets.at(producer);
      for (int i = 0; i <= offset; ++i) {
        if (it == items.begin()) EUDAQ_THROW("Bad offset in ResyncTLU routine");
        --it;
      }
      return it;
    }
    unsigned getid(size_t producer, size_t offset = 0) const
    {
      unsigned diff = 0;
      if (firstid != (unsigned) - 1 && getevent(producer).get_id() == TLUID) {
        diff = firstid;
      }
      return (iter(producer, offset)->triggerids[producer] + diff) & IDMASK;
    }
    const eudaq::Event& getevent(size_t producer, int offset = 0) const
    {
      return *iter(producer, offset)->event->GetEvent(producer);
    }
    unsigned producers() const
    {
      return offsets.size();
    }
    std::list<item_t> items;
    std::vector<std::list<item_t>::const_iterator> offsets;
    unsigned firstid, lastid;
  };

  std::ostream& operator << (std::ostream& os, const FileReader::eventqueue_t& q)
  {
    q.debug(os);
    return os;
  }

  namespace {

    static bool ReadEvent(FileDeserializer& des, int ver, eudaq::Event*& ev, size_t skip = 0)
    {
      if (!des.HasData()) {
        return false;
      }
      if (ver < 2) {
        for (size_t i = 0; i <= skip; ++i) {
          if (!des.HasData()) break;
          ev = EventFactory::Create(des);
        }
      } else {
        BufferSerializer buf;
        for (size_t i = 0; i <= skip; ++i) {
          if (!des.HasData()) break;
          des.read(buf);
        }
        ev = eudaq::EventFactory::Create(buf);
      }
      return true;
    }

    static bool SyncEvent(FileReader::eventqueue_t& queue, FileDeserializer& des, int ver, eudaq::Event*& ev)
    {

      static const int MAXTRIES = 3;
      unsigned eventnum = 0;
      static const bool dbg = false;
      for (int itry = 0; itry < MAXTRIES; ++itry) {
        // Make sure there is at least one whole event in the queue
        if (queue.isempty()) {
          eudaq::Event* evnt = 0;
          if (!ReadEvent(des, ver, evnt)) {
            return false;
          }
          queue.push(evnt);
        }
        if (queue.firstid == (unsigned) - 1 && !queue.getevent(0).IsBORE()) {
          for (size_t i = 0; i < queue.producers(); ++i) {
            if (queue.getevent(i).get_id() != TLUID) {
              queue.firstid = PluginManager::GetTriggerID(queue.getevent(i)) & IDMASK;
              if (queue.firstid <= 1) {
                EUDAQ_INFO("First TLU id detected as " + to_string(queue.firstid));
              } else {
                EUDAQ_WARN("First TLU id detected as " + to_string(queue.firstid) + " (should be 0 or 1)");
              }
              break;
            }
          }
        }
        bool isbore = queue.getevent(0).IsBORE();
        bool iseore = queue.getevent(0).IsEORE();
        bool hasother = false;
        bool hasrepeat = false;
        unsigned triggerid = queue.getid(0);
        bool haszero = triggerid == 0;
        eventnum = queue.getevent(0).GetEventNumber();
        for (size_t i = 1; i < queue.producers(); ++i) {
          if (eventnum == 0 || queue.getevent(i).get_id() == TLUID) {
            eventnum = queue.getevent(i).GetEventNumber();
          }
          unsigned tid = queue.getid(i);
          if (!queue.getevent(i).IsBORE()) isbore = false;
          if (queue.getevent(i).IsEORE()) iseore = true;
          if (tid == 0) haszero = true;
          if (triggerid == 0) triggerid = tid;
          if (tid != 0) {
            if (tid == queue.lastid) {
              hasrepeat = true;
            } else if (tid != triggerid) {
              hasother = true;
            }
          }
        }
        if (dbg) std::cout << "Event " << eventnum
                             << ", id=" << triggerid
                             << ", zero=" << (haszero ? "y" : "n")
                             << ", repeat=" << (hasrepeat ? "y" : "n")
                             << ", other=" << (hasother ? "y" : "n")
                             << ", last=" << queue.lastid
                             << std::flush;
        // If everything looks fine, return the next event
        if (isbore || iseore || (!haszero && !hasrepeat && !hasother) || triggerid == 0) {
          if (dbg) std::cout << ", ok" << std::endl;
          ev = queue.popevent();
          queue.lastid = triggerid;
          return true;
        }
        if (triggerid != 0 && triggerid == queue.lastid) {
          EUDAQ_WARN("Trigger ID " + to_string(triggerid) + " is repeated for whole event " + to_string(eventnum));
        }
        if (hasrepeat) {
          for (size_t i = 0; i < queue.producers(); ++i) {
            unsigned tid = queue.getid(i);
            if (tid == queue.lastid) {
              if (dbg) std::cout << std::endl;
              EUDAQ_INFO("Discarded repeated ID (" + to_string(triggerid) + ") in event " + to_string(eventnum));
              queue.discardevent(i);
            }
          }
          continue;
        }
        // If there is an unexpected id, look in more detail
        if (hasother) {
          bool istlumiss = true, isothermiss = true;
          std::vector<int> nums;
          for (size_t i = 0; i < queue.producers(); ++i) {
            unsigned tid = queue.getid(i);
            if (tid != ((queue.lastid + 1) & IDMASK) &&
                tid != ((queue.lastid + 2) & IDMASK)) {
              isothermiss = false;
            }
            if (queue.getevent(i).get_id() == TLUID) {
              if (tid != ((queue.lastid + 2) & IDMASK)) {
                istlumiss = false;
              }
            } else {
              if (tid != ((queue.lastid + 1) & IDMASK)) {
                istlumiss = false;
              }
            }
            nums.push_back(tid);
          }
          if (istlumiss) {
            if (dbg) std::cout << ", tlumiss" << std::endl;
            EUDAQ_INFO("Skipped TLU event detected, discarding rest of event " + to_string(eventnum));
            for (size_t i = 0; i < queue.producers(); ++i) {
              if (queue.getevent(i).get_id() != TLUID) {
                queue.discardevent(i);
              }
            }
            continue;
          }
          if (isothermiss) {
            if (dbg) std::cout << ", othermiss" << std::endl;
            EUDAQ_INFO("Skipped event detected, discarding rest of event " + to_string(eventnum));
            for (size_t i = 0; i < queue.producers(); ++i) {
              if (queue.getid(i) != ((queue.lastid + 2) & IDMASK)) {
                queue.discardevent(i);
              }
            }
            continue;
          }
          if (dbg) std::cout << ", " << to_string(nums) << std::endl;
          EUDAQ_WARN("Unexpected tid in event number " + to_string(eventnum) + ": " + to_string(nums));
        } else {
          if (dbg) std::cout << ", hmm" << std::endl;
        }
        // Make sure we have at least two full events in the queue
        if (queue.fullevents() < 2) {
          eudaq::Event* evnt = 0;
          if (!ReadEvent(des, ver, evnt)) {
            break;
          }
          queue.push(evnt);
        }
        bool doskip = false;
        unsigned triggerid1 = (triggerid + 1) & IDMASK;
        for (size_t i = 0; i < queue.producers(); ++i) {
          unsigned tid = queue.getid(i);
          if (tid == 0) {
            unsigned tid1 = queue.getid(i, 1);
            if (tid1 == triggerid) {
              EUDAQ_INFO("Discarded extra 'zero' in event " + to_string(eventnum));
              queue.discardevent(i);
            } else if (tid1 == triggerid1 || tid1 == 1) {
              if (tid1 == 1 && tid != triggerid + 1)
                EUDAQ_WARN("Performing wrap-around for event " + to_string(eventnum) +
                           ", trigger id " + to_string(tid1) + ", expected " + to_string(triggerid1));
              EUDAQ_DEBUG("Detected 'zero' in event " + to_string(eventnum));
            } else if (tid1 == 0) {
              // Make sure there are at least three full events in the queue
              if (queue.fullevents() < 3) {
                eudaq::Event* evnt = 0;
                if (!ReadEvent(des, ver, evnt)) {
                  break;
                }
                queue.push(evnt);
              }
              unsigned tid2 = queue.getid(i, 2);
              unsigned triggerid2 = (triggerid + 2) & IDMASK;
              if (tid2 == triggerid) {
                EUDAQ_INFO("Discarded two extra 'zero's in event " + to_string(eventnum));
                queue.discardevent(i);
                queue.discardevent(i);
              } else if (tid2 == triggerid2) {
                EUDAQ_DEBUG("Detected double 'zero' in event " + to_string(eventnum));
              } else if (tid2 == triggerid1) {
                EUDAQ_WARN("Ambiguous double zero in event " + to_string(eventnum) + ", discarding whole event plus zero");
                queue.discardevent(i);
                queue.popevent();
                doskip = true;
                break;
              } else if (tid2 == 0) {
                EUDAQ_WARN("Three consecutive 'zero's in event " + to_string(eventnum) + ", discarding one event.");
                queue.popevent();
                doskip = true;
                break;
              }
            } else {
              EUDAQ_THROW("Unable to synchronize at event " + to_string(eventnum) +
                          ", next = " + to_string(tid1) + ", expected = " + to_string(triggerid1));
            }
          }
        }
        if (!doskip) {
          ev = queue.popevent();
          queue.lastid = triggerid;
          return true;
        }
      }
      EUDAQ_WARN("Unable to synchronize after " + to_string(MAXTRIES) + " events at event " + to_string(eventnum));
      return false;
    }

  }

  FileReader::FileReader(const std::string& file, const std::string& filepattern, bool synctriggerid)
    : m_filename(FileNamer(filepattern).Set('X', ".raw").SetReplace('R', file)),
      m_des(m_filename),
      m_ev(EventFactory::Create(m_des)),
      m_ver(1),
      m_queue(0)
  {
    //unsigned versiontag = m_des.peek<unsigned>();
    //if (versiontag == Event::str2id("VER2")) {
    //  m_ver = 2;
    //  m_des.read(versiontag);
    //} else if (versiontag != Event::str2id("_DET")) {
    //  EUDAQ_WARN("Unrecognised native file (tag=" + Event::id2str(versiontag) + "), assuming version 1");
    //}
    //EUDAQ_INFO("FileReader, version = " + to_string(m_ver));
    //NextEvent();

    if (synctriggerid) {
      m_queue = new eventqueue_t(GetDetectorEvent().NumEvents());
    }
  }

  FileReader::~FileReader()
  {
    delete m_queue;
  }

  bool FileReader::NextEvent(size_t skip)
  {

    eudaq::Event* ev = 0;
    if (m_queue) {

      bool result = false;
      for (size_t i = 0; i <= skip; ++i) {
        if (!SyncEvent(*m_queue, m_des, m_ver, ev)) break;
        result = true;
      }
      if (ev) {
        m_ev = ev;
      }
      return result;
    }

    bool result = ReadEvent(m_des, m_ver, ev, skip);
    if (ev) m_ev = ev;
    return result;
  }

  unsigned FileReader::RunNumber() const
  {
    return m_ev->GetRunNumber();
  }

  const Event& FileReader::GetEvent() const
  {
    return *m_ev;
  }

  const DetectorEvent& FileReader::GetDetectorEvent() const
  {
    return dynamic_cast<const DetectorEvent&>(*m_ev);
  }

  const StandardEvent& FileReader::GetStandardEvent() const
  {
    return dynamic_cast<const StandardEvent&>(*m_ev);
  }

//   const StandardEvent & FileReader::GetStandardEvent() const {
//     if (!m_sev) {
//       counted_ptr<StandardEvent> sevent(new StandardEvent);
//       const DetectorEvent & dev = GetDetectorEvent();
//       for (size_t i = 0; i < dev.NumEvents(); ++i) {
//         const eudaq::Event * subevent = dev.GetEvent(i);

//         try {
//           const DataConverterPlugin * converterplugin = PluginManager::GetInstance().GetPlugin(subevent->GetType());
//           converterplugin->GetStandardSubEvent(*sevent, *subevent);
//           //std::fprintf(m_file, "Event %d %d\n", devent.GetEventNumber(), standardevent->m_x.size());
//         } catch(eudaq::Exception & e) {
//           //std::cout <<  e.what() << std::endl;
//           std::cout <<  "FileWriterText::WriteEvent(): Ignoring event type "
//                     <<  subevent->GetType() << std::endl;
//           continue;
//         }
//       }
//       m_sev = sevent;
//     }
//     return *m_sev;
//   }

}
