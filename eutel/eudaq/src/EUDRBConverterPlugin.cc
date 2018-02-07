#include <eutel/eudaq/DataConverterPlugin.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/RawDataEvent.h>
#include <eutel/eudaq/EUDRBEvent.h>
#include <eutel/eudaq/Configuration.h>
#include <eutel/eudaq/Logger.h>
#include <eutel/eudaq/EUTELESCOPE.h>
#include <eutel/eudaq/EUTelMimosa26Detector.h>

// include BASF2 logger
#include <framework/logging/Logger.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <cassert>

namespace eudaq {

  void map_1x1(unsigned& x, unsigned& y, unsigned c, unsigned r, unsigned, unsigned, unsigned)
  {
    x = c;
    y = r;
  }

  void map_4x1(unsigned& x, unsigned& y, unsigned c, unsigned r, unsigned m, unsigned nc, unsigned)
  {
    unsigned mat = (m == 0 || m == 3) ? 3 - m : m;
    x = c + mat * nc;
    y = r;
  }

  void map_2x2(unsigned& x, unsigned& y, unsigned c, unsigned r, unsigned m, unsigned nc, unsigned nr)
  {
    x = (m < 2) ? c : 2 * nc - 1 - c;
    y = (m == 0 || m == 3) ? r : 2 * nr - 1 - r;
  }

  struct SensorInfo {
    typedef void (*mapfunc_t)(unsigned& x, unsigned& y, unsigned c, unsigned r, unsigned m, unsigned nc, unsigned nr);
    SensorInfo(const std::string& name, unsigned c, unsigned r, unsigned m, unsigned w, unsigned h, mapfunc_t mfunc = 0)
      : name(name), cols(c), rows(r), mats(m), width(w), height(h), mapfunc(mfunc)
    {}
    std::string name;
    unsigned cols, rows, mats, width, height;
    mapfunc_t mapfunc;
  };

  static const SensorInfo g_sensors[] = {
    SensorInfo("MIMOSTAR2",   0,   0, 0,  132,  128),
    SensorInfo("MIMOTEL",    66, 256, 4,  264,  256, map_4x1),
    SensorInfo("MIMOTEL",    66, 256, 4,  264,  256, map_4x1),
    SensorInfo("MIMOSA18",  256, 256, 4,  512,  512, map_2x2),
    SensorInfo("MIMOSA5",     0,   0, 0, 1024, 1024),
    SensorInfo("MIMOSA26", 1152, 576, 1, 1152,  576, map_1x1)
  };

  struct BoardInfo {
    enum E_DET  { DET_NONE = -1, DET_MIMOSTAR2, DET_MIMOTEL, DET_MIMOTEL_NEWORDER, DET_MIMOSA18, DET_MIMOSA5, DET_MIMOSA26 };
    enum E_MODE { MODE_NONE = -1, MODE_ZS, MODE_RAW1, MODE_RAW2, MODE_RAW3, MODE_ZS2 };
    BoardInfo() : m_version(0), m_det(DET_MIMOTEL), m_mode(MODE_RAW3) {}
    BoardInfo(const Event& ev, int brd)
      : m_version(0), m_det(DET_NONE), m_mode(MODE_NONE)
    {
      std::string det = ev.GetTag("DET" + to_string(brd));
      if (det == "") det = ev.GetTag("DET", "MIMOTEL");

      for (size_t i = 0; i < sizeof g_sensors / sizeof * g_sensors; ++i) {
        if (det == g_sensors[i].name) {
          m_det = (E_DET)i;
          break;
        }
      }
      if (m_det == DET_NONE) EUDAQ_THROW("Unknown detector in EUDRBConverterPlugin: " + det);

      std::string mode = ev.GetTag("MODE" + to_string(brd));
      if (mode == "") mode = ev.GetTag("MODE", "RAW3");

      if (mode == "ZS") m_mode = MODE_ZS;
      else if (mode == "ZS2") m_mode = MODE_ZS2;
      else if (mode == "RAW2") m_mode = MODE_RAW2;
      else if (mode == "RAW3") m_mode = MODE_RAW3;
      else EUDAQ_THROW("Unknown mode in EUDRBConverterPlugin: " + mode);

      m_version = ev.GetTag("VERSION", 0);
      if (m_version == 0) { // No VERSION tag, try to guess it
        if (det == "MIMOTEL" && mode != "RAW2") {
          m_version = 1;
        } else {
          m_version = 2;
        }
        EUDAQ_WARN("No EUDRB Version tag, guessing VERSION=" + to_string(m_version));
      }
    }
    const SensorInfo& Sensor() const
    {
      return g_sensors[m_det];
    }
    unsigned Frames() const
    {
      return m_mode > 0 ? m_mode : 1;
    }
    int m_version;
    E_DET m_det;
    E_MODE m_mode;
  };

  class EUDRBConverterBase {
  public:
    void FillInfo(const Event& bore, const Configuration&)
    {
      unsigned nboards = from_string(bore.GetTag("BOARDS"), 0);
      //std::cout << "FillInfo " << nboards << std::endl;
      for (unsigned i = 0; i < nboards; ++i) {
        unsigned id = from_string(bore.GetTag("ID" + to_string(i)), i);
        if (m_info.size() <= id) m_info.resize(id + 1);
        m_info[id] = BoardInfo(bore, i);
      }
    }
    const BoardInfo& GetInfo(unsigned id) const
    {
      if (id >= m_info.size()
          || m_info[id].m_version < 1) EUDAQ_THROW("Unrecognised ID (" + to_string(id) + ", num=" + to_string(
                                                       m_info.size()) + ") converting EUDRB event");
      return m_info[id];
    }
    static unsigned GetTLUEvent(const std::vector<unsigned char>& data)
    {
      const unsigned word = getbigendian<unsigned>(&data[data.size() - 8]);
      return word >> 8 & 0xffff;
    }
    bool ConvertStandard(StandardEvent& stdEvent, const Event& eudaqEvent) const;
    StandardPlane ConvertPlane(const std::vector<unsigned char>& data, unsigned id, StandardEvent& evt) const
    {
      const BoardInfo& info = GetInfo(id);
      StandardPlane plane(id, "EUDRB", info.Sensor().name);
      plane.SetXSize(info.Sensor().width);
      plane.SetYSize(info.Sensor().height);
      plane.SetTLUEvent(GetTLUEvent(data));
      if (info.m_mode == BoardInfo::MODE_ZS2) {
        unsigned numoverflows = ConvertZS2(plane, data, info);
        if (numoverflows) evt.SetTag("OVF" + to_string(id), numoverflows);
      } else if (info.m_mode == BoardInfo::MODE_ZS) {
        ConvertZS(plane, data, info);
      } else {
        ConvertRaw(plane, data, info);
      }
      return plane;
    }
    static unsigned ConvertZS2(StandardPlane& plane, const std::vector<unsigned char>& alldata, const BoardInfo& info);
    static void ConvertZS(StandardPlane& plane, const std::vector<unsigned char>& alldata, const BoardInfo& info);
    static void ConvertRaw(StandardPlane& plane, const std::vector<unsigned char>& data, const BoardInfo& info);
    bool ConvertTBTelEvent(TBTelEvent& tbEvent, const Event& eudaqEvent) const;
  protected:
    static size_t NumPlanes(const Event& event)
    {
      if (const RawDataEvent* ev = dynamic_cast<const RawDataEvent*>(&event)) {
        return ev->NumBlocks();
      } else if (const EUDRBEvent* ev = dynamic_cast<const EUDRBEvent*>(&event)) {
        return ev->NumBoards();
      }
      return 0;
    }
    static std::vector<unsigned char> GetPlane(const Event& event, size_t i)
    {
      if (const RawDataEvent* ev = dynamic_cast<const RawDataEvent*>(&event)) {
        return ev->GetBlock(i);
      } else if (const EUDRBEvent* ev = dynamic_cast<const EUDRBEvent*>(&event)) {
        return ev->GetBoard(i).GetDataVector();
      }
      return std::vector<unsigned char>();
    }
    static size_t GetID(const Event& event, size_t i)
    {
      if (const RawDataEvent* ev = dynamic_cast<const RawDataEvent*>(&event)) {
        return ev->GetID(i);
      } else if (const EUDRBEvent* ev = dynamic_cast<const EUDRBEvent*>(&event)) {
        return ev->GetBoard(i).GetID();
      }
      return 0;
    }
    std::vector<BoardInfo> m_info;
  };

  /********************************************/

  class EUDRBConverterPlugin : public DataConverterPlugin, public EUDRBConverterBase {
  public:
    virtual void Initialize(const Event& e, const Configuration& c)
    {
      FillInfo(e, c);
    }

    virtual unsigned GetTriggerID(Event const& ev) const
    {
      const RawDataEvent& rawev = dynamic_cast<const RawDataEvent&>(ev);
      if (rawev.NumBlocks() < 1) return (unsigned) - 1;
      const std::vector<unsigned char>& data = rawev.GetBlock(rawev.NumBlocks() - 1);
      return GetTLUEvent(data);
    }

    virtual bool GetStandardSubEvent(StandardEvent& result, const Event& source) const
    {
      return ConvertStandard(result, source);
    }

    virtual bool GetTBTelEventSubEvent(TBTelEvent& tbEvent, const Event& eudaqEvent) const
    {
      return ConvertTBTelEvent(tbEvent, eudaqEvent);
    }

  private:
    EUDRBConverterPlugin() : DataConverterPlugin("EUDRB") {}
    static EUDRBConverterPlugin const m_instance;
  };

  EUDRBConverterPlugin const EUDRBConverterPlugin::m_instance;

  /********************************************/

  class LegacyEUDRBConverterPlugin : public DataConverterPlugin, public EUDRBConverterBase {
    virtual void Initialize(const eudaq::Event& e, const eudaq::Configuration& c)
    {
      FillInfo(e, c);
    }

    virtual unsigned GetTriggerID(Event const& ev) const
    {
      const RawDataEvent& rawev = dynamic_cast<const RawDataEvent&>(ev);
      if (rawev.NumBlocks() < 1) return (unsigned) - 1;
      const std::vector<unsigned char>& data = rawev.GetBlock(0);
      return GetTLUEvent(data);
    }

    virtual bool GetStandardSubEvent(StandardEvent& result, const Event& source) const
    {
      return ConvertStandard(result, source);
    }

    virtual bool GetTBTelEventSubEvent(TBTelEvent& tbEvent, const Event& eudaqEvent) const
    {
      return ConvertTBTelEvent(tbEvent, eudaqEvent);
    }

  private:
    LegacyEUDRBConverterPlugin() : DataConverterPlugin(Event::str2id("_DRB")) {}
    static LegacyEUDRBConverterPlugin const m_instance;
  };

  LegacyEUDRBConverterPlugin const LegacyEUDRBConverterPlugin::m_instance;

  /********************************************/

  bool EUDRBConverterBase::ConvertStandard(StandardEvent& result, const Event& source) const
  {
    if (source.IsBORE()) {
      // shouldn't happen
      return true;
    } else if (source.IsEORE()) {
      // nothing to do
      return true;
    }
    // If we get here it must be a data event
    size_t numplanes = NumPlanes(source);
    for (size_t i = 0; i < numplanes; ++i) {
      result.AddPlane(ConvertPlane(GetPlane(source, i), GetID(source, i), result));
    }
    return true;
  }

#define GET(o) getbigendian<unsigned>(&alldata[(o)*4])
  unsigned EUDRBConverterBase::ConvertZS2(StandardPlane& plane, const std::vector<unsigned char>& alldata, const BoardInfo& info)
  {
    static const bool dbg  = false;
    static const bool dbg2 = false;
    if (dbg) std::cout << "DataSize = " << hexdec(alldata.size(), 0) << std::endl;
    if (alldata.size() < 64) EUDAQ_THROW("Bad data packet (only " + to_string(alldata.size()) + " bytes)");
    unsigned offset = 0, word = GET(offset);
    if (dbg) std::cout << "BaseAddress = 0x"
                         << to_hex((word & 0xff000000) | 0x00400000)
                         << std::endl;
    unsigned wordcount = word & 0xffffff;
    if (dbg) std::cout << "WordCount = " << hexdec(wordcount, 0) << std::endl;
    if (wordcount * 4 + 16 != alldata.size()) EUDAQ_THROW("Bad wordcount (" + to_string(wordcount) +
                                                            ", bytes=" + to_string(alldata.size()) + ")");
    word = GET(offset = 1);
    if (dbg) std::cout << "Unknown = " << hexdec(word >> 8 & 0xffff) << std::endl;
    // offset 2 is a repeat of offset 0
    word = GET(offset = 3);
    unsigned sof = word;
    if (dbg) std::cout << "StartOfFrame = " << hexdec(sof, 0) << std::endl;
    word = GET(offset = 4);
    if (dbg) std::cout << "LocalEventNumber = " << hexdec(word >> 8 & 0xffff, 0) << std::endl;
    if (dbg) std::cout << "FrameNumberAtTrigger = " << hexdec(word & 0xff, 0) << std::endl;
    word = GET(offset = 5);
    unsigned pixadd = word & 0x3ffff;
    plane.SetPivotPixel((9216 + pixadd - sof + 56) % 9216);
    if (dbg) std::cout << "PixelAddressAtTrigger = " << hexdec(pixadd, 0)
                         << ": pivot = " << hexdec(plane.PivotPixel(), 0) << std::endl;
    unsigned wordremain = wordcount - 12;
    unsigned totaloverflows = 0;
    plane.SetSizeZS(info.Sensor().width, info.Sensor().height, 0, 2, StandardPlane::FLAG_WITHPIVOT | StandardPlane::FLAG_DIFFCOORDS);
    try {
      for (int frame = 1; frame <= 2; ++frame) {
        unsigned numoverflows = 0;
        word = GET(offset += 2);
        if (dbg) std::cout << "M26FrameCounter_" << frame << " = " << hexdec(word, 0) << std::endl;
        word = GET(++offset);
        unsigned count = word & 0xffff;
        if (dbg) std::cout << "M26WordCount_" << frame << " = " << hexdec(count, 0) << ", " << hexdec(word >> 16, 0) << std::endl;
        if (count > wordremain) EUDAQ_THROW("Bad M26 word count (" + to_string(count) + ", remain=" +
                                              to_string(wordremain) + ", total=" + to_string(wordcount) + ")");
        wordremain -= count;
        std::vector<unsigned short> vec;
        // read pixel data
        for (size_t i = 0; i < count; ++i) {
          word = GET(++offset);
          vec.push_back(word & 0xffff);
          vec.push_back(word >> 16 & 0xffff);
        }
        unsigned npixels = 0;
        for (size_t i = 0; i < vec.size(); ++i) {
          //  std::cout << "  " << i << " : " << hexdec(vec[i]) << std::endl;
          if (i == vec.size() - 1) break;
          unsigned numstates = vec[i] & 0xf;
          unsigned row = vec[i] >> 4 & 0x7ff;
          if (numstates + 1 > vec.size() - i) {
            // Ignoring bad line
            //std::cout << "Ignoring bad line " << row << " (too many states)" << std::endl;
            break;
          }
          bool over = vec[i] & 0x8000;
          if (dbg) std::cout << "Hit line " << (over ? "* " : ". ") << row
                               << ", states " << numstates << ":";
          bool pivot = row >= (plane.PivotPixel() / 16);
          for (unsigned s = 0; s < numstates; ++s) {
            unsigned v = vec.at(++i);
            unsigned column = v >> 2 & 0x7ff;
            unsigned num = v & 3;
            if (dbg) std::cout << (s ? "," : " ") << column;
            if (dbg) if ((v & 3) > 0) std::cout << "-" << (column + num);
            for (unsigned j = 0; j < num + 1; ++j) {
              plane.PushPixel(column + j, row, 1, pivot, frame - 1);
            }
            npixels += num + 1;
          }
          if (dbg) std::cout << std::endl;
          if (over) {
            numoverflows++;
            if (dbg2) std::cout << "*** Overflow in plane " << plane.ID() << ", row " << row << std::endl;
          }
        }
        if (dbg) std::cout << "Frame pixels = " << npixels << std::endl;
        if (dbg) std::cout << "Frame overflows = " << numoverflows << std::endl;
        ++offset;
        totaloverflows += numoverflows;
      }
      if (dbg && totaloverflows) std::cout << "Total overflows = " << totaloverflows << std::endl;
    } catch (const std::out_of_range& e) {
      std::cout << "\n%%%% Oops: " << e.what() << " %%%%" << std::endl;
    }
    //     if (dbg) {
    //       std::cout << "Plane " << plane.m_pix.size();
    //       for (size_t i = 0; i < plane.m_pix.size(); ++i) {
    //         std::cout << ", " << plane.m_pix[i].size();
    //       }
    //       std::cout << std::endl;
    //     }
    // readjust offset to be sure it points to trailer:
    word = GET(offset = alldata.size() / 4 - 2);
    if (dbg) std::cout << "TLUEventNumber = " << hexdec(word >> 8 & 0xffff, 0) << std::endl;
    if (dbg) std::cout << "NumFramesAtTrigger = " << hexdec(word & 0xff, 0) << std::endl;
    word = GET(++offset);
    if (dbg) std::cout << "EventWordCount = " << hexdec(word & 0x7ffff, 0) << std::endl;

    if (dbg) std::cout << "****************" << std::endl;
    return totaloverflows;
  }
#undef GET

  void EUDRBConverterBase::ConvertZS(StandardPlane& plane, const std::vector<unsigned char>& alldata, const BoardInfo& info)
  {
    unsigned headersize = 8, trailersize = 8;
    if (info.m_version > 2) {
      headersize += 8;
      EUDAQ_THROW("EUDRB V3 decoding not yet implemented");
    }
    bool padding = (alldata[alldata.size() - trailersize - 4] == 0);
    unsigned npixels = (alldata.size() - headersize - trailersize) / 4 - padding;
    plane.SetSizeZS(info.Sensor().width, info.Sensor().height, npixels);
    //plane.m_mat.resize(plane.m_pix[0].size());
    const unsigned char* data = &alldata[headersize];
    plane.SetPivotPixel(((data[5] & 0x3) << 16) | (data[6] << 8) | data[7]);
    for (unsigned i = 0; i < npixels; ++i) {
      int mat = 3 - (data[4 * i] >> 6), col = 0, row = 0;
      if (info.m_version < 2) {
        row = ((data[4 * i] & 0x7) << 5) | (data[4 * i + 1] >> 3);
        col = ((data[4 * i + 1] & 0x7) << 4) | (data[4 * i + 2] >> 4);
      } else {
        row = ((data[4 * i] & 0x3F) << 3) | (data[4 * i + 1] >> 5);
        col = ((data[4 * i + 1] & 0x1F) << 4) | (data[4 * i + 2] >> 4);
      }
      unsigned x, y;
      info.Sensor().mapfunc(x, y, col, row, mat, info.Sensor().cols, info.Sensor().rows);
      unsigned pix = ((data[4 * i + 2] & 0x0F) << 8) | (data[4 * i + 3]);
      plane.SetPixel(i, x, y, pix);
      //plane.m_x[i] = x;
      //plane.m_y[i] = y;
      //plane.m_mat[i] = mat;
      //plane.m_pix[0][i] = ((data[4*i+2] & 0x0F) << 8) | (data[4*i+3]);
    }
  }

  void EUDRBConverterBase::ConvertRaw(StandardPlane& plane, const std::vector<unsigned char>& data, const BoardInfo& info)
  {
    unsigned headersize = 8, trailersize = 8;
    if (info.m_version > 2) {
      headersize += 8;
      EUDAQ_THROW("EUDRB V3 decoding not yet implemented");
    }
    plane.SetPivotPixel(((data[5] & 0x3) << 16) | (data[6] << 8) | data[7]);
    unsigned possible1 = 2 *  info.Sensor().cols * info.Sensor().rows      * info.Sensor().mats * info.Frames();
    unsigned possible2 = 2 * (info.Sensor().cols * info.Sensor().rows - 1) * info.Sensor().mats * info.Frames();
    bool missingpixel = false;
    if (data.size() - headersize - trailersize == possible1) {
      // OK
    } else if (data.size() - headersize - trailersize == possible2) {
      missingpixel = true;
    } else {
      EUDAQ_THROW("Bad raw data size (" + to_string(data.size() - headersize - trailersize) + ") expecting "
                  + to_string(possible1) + " or " + to_string(possible2));
    }
    //unsigned npixels = info.Sensor().cols * info.Sensor().rows * info.Sensor().mats;
    plane.SetSizeRaw(info.Sensor().width, info.Sensor().height, info.Frames(),
                     StandardPlane::FLAG_WITHPIVOT | StandardPlane::FLAG_NEEDCDS | StandardPlane::FLAG_NEGATIVE);
    //plane.m_mat.resize(plane.m_pix[0].size());
    const unsigned char* ptr = &data[headersize];
    for (unsigned row = 0; row < info.Sensor().rows; ++row) {
      for (unsigned col = 0; col < info.Sensor().cols; ++col) {
        if (missingpixel && row == info.Sensor().rows - 1 && col == info.Sensor().cols - 1) break; // last pixel is not transferred
        for (size_t frame = 0; frame < info.Frames(); ++frame) {
          for (size_t mat = 0; mat < info.Sensor().mats; ++mat) {
            unsigned x = 0, y = 0;
            info.Sensor().mapfunc(x, y, col, row, mat, info.Sensor().cols, info.Sensor().rows);
            size_t i = x + y * info.Sensor().width;
            // if (frame == 0) {
            //   plane.m_x[i] = x;
            //   plane.m_y[i] = y;
            //   plane.m_mat[i] = mat;
            //   if (info.m_version < 2) {
            //     plane.m_pivot[i] = (row << 7 | col) >= plane.m_pivotpixel;
            //   } else {
            //     plane.m_pivot[i] = (row << 9 | col) >= plane.m_pivotpixel;
            //   }
            // }
            short pix = *ptr++ << 8;
            pix |= *ptr++;
            pix &= 0xfff;
            //plane.m_pix[frame][i] = pix;
            bool pivot = (info.m_version < 2) ? (row << 7 | col) >= plane.PivotPixel() : (row << 9 | col) >= plane.PivotPixel();
            plane.SetPixel(i, x, y, pix, pivot, frame);
          }
        }
      }
    }
  }

  bool EUDRBConverterBase::ConvertTBTelEvent(TBTelEvent& result, const Event& source) const
  {

    if (source.IsBORE()) {
      // shouldn't happen
      return true;
    } else if (source.IsEORE()) {
      // nothing to do
      return true;
    }
    // If we get here it must be a data event

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FIXME
    // START: Don't know what to do here
    // FIXME

    // result.parameters().setValue( eutelescope::EUTELESCOPE::EVENTTYPE, eutelescope::kDE );

    // // prepare the collections for the rawdata and the zs ones
    // LCCollectionVec * rawDataCollection, * zsDataCollection, * zs2DataCollection;
    // bool rawDataCollectionExists = false, zsDataCollectionExists = false, zs2DataCollectionExists = false;

    // try {
    //     rawDataCollection = static_cast< LCCollectionVec* > ( result.getCollection( "rawdata" ) );
    //     rawDataCollectionExists = true;
    // } catch ( lcio::DataNotAvailableException& e ) {
    //     rawDataCollection = new LCCollectionVec( lcio::LCIO::TRACKERRAWDATA );
    // }

    // try {
    //     zsDataCollection = static_cast< LCCollectionVec* > ( result.getCollection( "zsdata" ) );
    //     zsDataCollectionExists = true;
    // } catch ( lcio::DataNotAvailableException& e ) {
    //     zsDataCollection = new LCCollectionVec( lcio::LCIO::TRACKERDATA );
    // }

    // try {
    //     zs2DataCollection = static_cast< LCCollectionVec* > ( result.getCollection( "zsdata_m26" ) );
    //     zs2DataCollectionExists = true;
    // } catch ( lcio::DataNotAvailableException& e ) {
    //     zs2DataCollection = new LCCollectionVec( lcio::LCIO::TRACKERDATA );
    // }

    // // set the proper cell encoder
    // CellIDEncoder< TrackerRawDataImpl > rawDataEncoder  ( eutelescope::EUTELESCOPE::MATRIXDEFAULTENCODING, rawDataCollection );
    // CellIDEncoder< TrackerDataImpl    > zsDataEncoder   ( eutelescope::EUTELESCOPE::ZSDATADEFAULTENCODING, zsDataCollection  );
    // CellIDEncoder< TrackerDataImpl    > zs2DataEncoder  ( eutelescope::EUTELESCOPE::ZSDATADEFAULTENCODING, zs2DataCollection  );

    // FIXME
    // END: Don't know what to do here
    // FIXME
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // to understand if we have problem with de-syncronisation, let
    // me prepare a Boolean switch and a vector of size_t to contain the
    // pivot pixel position
    bool outOfSyncFlag = false;
    std::vector<size_t > pivotPixelPosVec;

    //const RawDataEvent & rawDataEvent = dynamic_cast< const RawDataEvent & > ( source ) ;

    size_t numplanes = NumPlanes(source);
    for (size_t iPlane = 0; iPlane < numplanes; ++iPlane) {

      StandardEvent tmp_evt;
      StandardPlane plane = ConvertPlane(GetPlane(source, iPlane), GetID(source, iPlane), tmp_evt);

      // We have very specific requirements for this converter
      // the type of the sensor must be MIMOSA26 and the input
      // data must be zero suppressed => put an assert statement
      if (plane.Sensor() != "MIMOSA26") {
        EUDAQ_ERROR("Input sensor is not Mimosa26, thus not supported!");
        exit(-1);
      }
      if (! plane.GetFlags(StandardPlane::FLAG_ZS)) {
        EUDAQ_ERROR("Input data is not zero suppressed, thus not supported!");
        exit(-1);
      }

      // The current detector is then:
      eutelescope::EUTelPixelDetector* currentDetector = new eutelescope::EUTelMimosa26Detector;
      std::string mode = "ZS2";
      currentDetector->setMode(mode);

      std::vector<size_t > markerVec = currentDetector->getMarkerPosition();


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // FIXME
      // START-NEW: Replacement
      // FIXME
      int sensorID = plane.ID();
      // FIXME
      // END-NEW: Replacement
      // FIXME
      // FIXME
      // START-OLD: What to do with this?
      // FIXME
      // zsDataEncoder["sensorID"] = plane.ID();
      // zsDataEncoder["sparsePixelType"] = eutelescope::kEUTelSimpleSparsePixel;
      // FIXME
      // END-OLD: What to do with this?
      // FIXME
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      // get the total number of pixel. This is written in the
      // eudrbBoard and to get it in a easy way pass through the eudrbDecoder
      size_t nPixel = plane.HitPixels();

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // FIXME
      // START: Remove this?
      // FIXME
      // prepare a new TrackerData for the ZS data
      // std::auto_ptr<lcio::TrackerDataImpl > zsFrame( new lcio::TrackerDataImpl );
      // zsDataEncoder.setCellID( zsFrame.get() );
      // FIXME
      // END: Remove this?
      // FIXME
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      // prepare a sparse pixel to be added to the sparse data
      float XCoord, YCoord, Signal;

      for (size_t iPixel = 0; iPixel < nPixel; ++iPixel) {

        // the data contain also the markers, so we have to strip
        // them out. First I need to have the original position
        // (with markers in) and then calculate how many pixels I
        // have to remove
        size_t originalX = (size_t)plane.GetX(iPixel);

        if (find(markerVec.begin(), markerVec.end(), originalX) == markerVec.end()) {
          // the original X is not on a marker column, so I need
          // to remove a certain number of pixels depending on the
          // position

          // this counts the number of markers found on the left
          // of the original X
          short  diff = (short) count_if(markerVec.begin(), markerVec.end(), std::bind2nd(std::less<short> (), originalX));
          XCoord = originalX - diff ;

          // no problem instead with the Y coordinate
          YCoord = plane.GetY(iPixel) ;

          // last the pixel charge. The CDS is automatically
          // calculated by the EUDRB
          Signal = plane.GetPixel(iPixel);

          // Print detailed pixel summary
          //std::cout << "Sensor " << plane.ID() << " x:" << XCoord << ", y:" << YCoord << ", charge:" << Signal << std::endl;

          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          // FIXME
          // START-NEW: Replacement
          // FIXME
          result.addTelPixel(sensorID, XCoord, YCoord, Signal);
          // FIXME
          // End-NEW: Replacement
          // FIXME
          // FIXME
          // START-OLD: Replace this by own code
          // FIXME
          // now add this pixel to the sparse frame
          // zsFrame.get()->chargeValues().push_back( XCoord );
          // zsFrame.get()->chargeValues().push_back( YCoord );
          // zsFrame.get()->chargeValues().push_back( Signal );
          // FIXME
          // End-OLD: Replace this by own code
          // FIXME
          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        } else {
          // the original X was on a marker column, so we don't
          // need to process this pixel any further and of course
          // we don't have to add it to the sparse frame.

          /*
            streamlog_out ( DEBUG0 ) << "Found a sparse pixel ("<< iPixel
            <<")  on a marker column. Not adding it to the frame" << endl
            << (* (sparsePixel.get() ) ) << endl;
          */

        }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // FIXME
      // START-NEW: Obsolete
      // FIXME
      // FIXME
      // START-NEW: Obsolete
      // FIXME
      // FIXME
      // START-OLD: Replace this by own code
      // FIXME
      // // perfect! Now add the TrackerData to the collection
      // if ( plane.Sensor() == "MIMOSA26" ) {
      //     zs2DataCollection->push_back( zsFrame.release() );
      // } else {
      //     zsDataCollection->push_back( zsFrame.release() );
      // }
      // FIXME
      // End-OLD: Replace this by own code
      // FIXME
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      // for the debug of the synchronization
      pivotPixelPosVec.push_back(plane.PivotPixel());

      delete currentDetector;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FIXME
    // START: What to do with this?
    // FIXME

    // if ( result.getEventNumber() == 0 ) {

    //     // do this only in the first event

    //     LCCollectionVec * eudrbSetupCollection = NULL;
    //     bool eudrbSetupExists = false;
    //     try {
    //  eudrbSetupCollection = static_cast< LCCollectionVec* > ( result.getCollection( "eudrbSetup" ) ) ;
    //  eudrbSetupExists = true;
    //     } catch ( lcio::DataNotAvailableException& e) {
    //  eudrbSetupCollection = new LCCollectionVec( lcio::LCIO::LCGENERICOBJECT );
    //     }

    //     //for ( size_t iPlane = 0 ; iPlane < setupDescription.size() ; ++iPlane ) {
    //     //  eudrbSetupCollection->push_back( setupDescription.at( iPlane ) );      }

    //     if (!eudrbSetupExists) {
    //  result.addCollection( eudrbSetupCollection, "eudrbSetup" );
    //     }
    // }

    // FIXME
    // END: What to do with this?
    // FIXME
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FIXME
    // Disabled console output for Belle2 !!!
    // FIXME

    // check if all the boards where running in synchronous mode or
    // not. Remember that the last pivot pixel entry is the one of the
    // master board.
    std::vector<size_t >::iterator masterBoardPivotAddress = pivotPixelPosVec.end() - 1;
    std::vector<size_t >::iterator slaveBoardPivotAddress  = pivotPixelPosVec.begin();
    while (slaveBoardPivotAddress < masterBoardPivotAddress) {
      if (*slaveBoardPivotAddress - *masterBoardPivotAddress >= 2) {
        outOfSyncFlag = true;

        // we don't need to continue looping over all boards if one of
        // them is already out of sync
        break;
      }
      ++slaveBoardPivotAddress;
    }

    // INFO: DISABLED this part for Belle2
    // if ( outOfSyncFlag ) {
    //     if ( result.getEventNumber()  < 20 ) {
    //  // in this case we have the responsibility to tell the user that
    //  // the event was out of sync

    //  std::cout << "Event number " << result.getEventNumber() << " seems to be out of sync" << std::endl;

    //  std::vector<size_t >::iterator masterBoardPivotAddress = pivotPixelPosVec.end() - 1;
    //  std::vector<size_t >::iterator slaveBoardPivotAddress  = pivotPixelPosVec.begin();
    //  while ( slaveBoardPivotAddress < masterBoardPivotAddress ) {
    //      // print out all the slave boards first
    //      std::cout << " --> Board (S) " <<  std::setw(3) << setiosflags( std::ios::right )
    //          << slaveBoardPivotAddress - pivotPixelPosVec.begin() << resetiosflags( std::ios::right )
    //          << " = " << std::setw(15) << setiosflags( std::ios::right )
    //          << (*slaveBoardPivotAddress) << resetiosflags( std::ios::right )
    //          << " (" << std::setw(15) << setiosflags( std::ios::right )
    //          << (signed) (*masterBoardPivotAddress) - (signed) (*slaveBoardPivotAddress) << resetiosflags( std::ios::right)
    //          << ")" << std::endl;
    //      ++slaveBoardPivotAddress;
    //  }
    //  // print out also the master. It is impossible that the master
    //  // is out of sync with respect to itself, but for completeness...
    //  std::cout  << " --> Board (M) "  <<  std::setw(3) << setiosflags( std::ios::right )
    //       << slaveBoardPivotAddress - pivotPixelPosVec.begin() << resetiosflags( std::ios::right )
    //       << " = " << std::setw(15) << setiosflags( std::ios::right )
    //       << (*slaveBoardPivotAddress) << resetiosflags( std::ios::right )
    //       << " (" << std::setw(15)  << setiosflags( std::ios::right )
    //       << (signed) (*masterBoardPivotAddress) - (signed) (*slaveBoardPivotAddress) << resetiosflags( std::ios::right)
    //       << ")" << std::endl;

    //     } else if ( result.getEventNumber()  == 20 ) {
    //  // if the number of consecutive warnings is equal to the maximum
    //  // allowed, don't bother the user anymore with this message,
    //  // because it's very likely the run was taken unsynchronized on
    //  // purpose
    //  std::cout << "The maximum number of unsychronized events has been reached." << std::endl
    //      << "Assuming the run was taken in asynchronous mode" << std::endl;
    //     }
    // }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FIXME
    // START: OBSOLETE
    // FIXME

    // // add the collections to the event only if not empty and not yet there
    // if ( !rawDataCollectionExists && ( rawDataCollection->size() != 0 ) ){
    //     result.addCollection( rawDataCollection, "rawdata" );
    // }

    // if ( !zsDataCollectionExists && ( zsDataCollection->size() != 0 )) {
    //     result.addCollection( zsDataCollection, "zsdata" );
    // }

    // if ( !zs2DataCollectionExists && ( zs2DataCollection->size() != 0 )) {
    //     result.addCollection( zs2DataCollection, "zsdata_m26" );
    // }

    // FIXME
    // END: OBSOLETE
    // FIXME
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return true;
  }

} //namespace eudaq
