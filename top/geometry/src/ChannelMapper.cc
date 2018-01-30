/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/geometry/ChannelMapper.h>
#include <framework/database/DBImportArray.h>
#include <unordered_set>
#include <framework/logging/LogSystem.h>
#include <iostream>

using namespace std;

namespace Belle2 {
  namespace TOP {

    ChannelMapper::ChannelMapper()
    {
      for (auto& channels : m_channels) {
        for (auto& channel : channels) channel = 0;
      }
      for (auto& pixels : m_pixels) {
        for (auto& pixel : pixels) pixel = 0;
      }
      if (c_numRows * c_numColumns != c_numAsics * c_numChannels)
        B2FATAL("TOP::ChannelMapper: bug in coding (enum) - "
                "number of channels and number of pixels disagree");
    }


    ChannelMapper::~ChannelMapper()
    {
      if (m_mappingDB) delete m_mappingDB;
    }

    void ChannelMapper::initialize(const GearDir& channelMapping)
    {

      clear();

      string path = channelMapping.getPath();
      auto i1 = path.rfind("type='") + 6;
      auto i2 = path.rfind("']");
      m_typeName = path.substr(i1, i2 - i1);

      if (m_typeName == "IRS3B") {
        m_type = c_IRS3B;
      } else if (m_typeName == "IRSX") {
        m_type = c_IRSX;
      } else {
        B2ERROR("TOP::ChannelMapper: unknown electronic type '" << m_typeName
                << "'");
      }

      // get parameters from Gearbox
      for (const GearDir& map : channelMapping.getNodes("map")) {
        std::vector<double> data = map.getArray("");
        unsigned row = int(data[0]) - 1;
        unsigned col = int(data[1]) - 1;
        unsigned asic = int(data[2]);
        unsigned chan = int(data[3]);
        m_mapping.push_back(TOPChannelMap(row, col, asic, chan));
      }
      if (m_mapping.empty()) {
        B2ERROR("TOP::ChannelMapper: mapping is not available in Gearbox");
        return;
      }

      // check the size of the mapping
      if (m_mapping.size() != c_numAsics * c_numChannels)
        B2FATAL("TOP::ChannelMapper: got incorrect map size from xml file for '"
                << m_typeName << "' - expect " <<  c_numAsics * c_numChannels
                << ", got " << m_mapping.size());

      // check the mapping for consistency
      bool ok = true;
      unordered_set<unsigned> pixels;
      unordered_set<unsigned> channels;
      for (unsigned ii = 0; ii < m_mapping.size(); ii++) {
        const auto& map = m_mapping[ii];
        unsigned row = map.getRow();
        unsigned col = map.getColumn();
        unsigned asic = map.getASICNumber();
        unsigned chan = map.getASICChannel();
        if (row >= c_numRows) {
          B2ERROR("TOP::ChannelMapper: pixel row out of range, node=" << ii
                  << ": " << row << " " << col << " " << asic << " " << chan);
          ok = false;
        }
        if (col >= c_numColumns) {
          B2ERROR("TOP::ChannelMapper: pixel column out of range, node=" << ii
                  << ": " << row << " " << col << " " << asic << " " << chan);
          ok = false;
        }
        if (asic >= c_numAsics) {
          B2ERROR("TOP::ChannelMapper: asic number out of range, node=" << ii
                  << ": " << row << " " << col << " " << asic << " " << chan);
          ok = false;
        }
        if (chan >= c_numChannels) {
          B2ERROR("TOP::ChannelMapper: asic channel number out of range, node=" << ii
                  << ": " << row << " " << col << " " << asic << " " << chan);
          ok = false;
        }
        if (!pixels.insert(col + row * c_numColumns).second) {
          B2ERROR("TOP::ChannelMapper: pixel already mapped, node=" << ii
                  << ": " << row << " " << col << " " << asic << " " << chan);
          ok = false;
        }
        if (!channels.insert(chan + asic * c_numChannels).second) {
          B2ERROR("TOP::ChannelMapper: channel already mapped, node=" << ii
                  << ": " << row << " " << col << " " << asic << " " << chan);
          ok = false;
        }
      }
      if (!ok) {
        B2FATAL("TOP::ChannelMapper: errors detected in xml file for '"
                << m_typeName << "'");
        return;
      }

      // prepare conversion arrays
      for (const auto& map : m_mapping) {
        m_channels[map.getRow()][map.getColumn()] = &map;
        m_pixels[map.getASICNumber()][map.getASICChannel()] = &map;
      }
      m_valid = true;

      B2INFO("TOP::ChannelMapper: " << m_mapping.size() <<
             " channels of carrier board of type '" << m_typeName
             << "' mapped to pixels.");

      // print mappings if debug level for package 'top' is set to 100 or larger
      const auto& logSystem = LogSystem::Instance();
      if (logSystem.isLevelEnabled(LogConfig::c_Debug, 100, "top")) {
        print();
      }

    }


    void ChannelMapper::initialize()
    {
      m_type = c_default;

      if (m_mappingDB) delete m_mappingDB;
      m_mappingDB = new DBArray<TOPChannelMap>();

      if (!m_mappingDB->isValid()) {
        clear();
        return;
      }
      update();

      m_mappingDB->addCallback(this, &ChannelMapper::update);

      const auto& logSystem = LogSystem::Instance();
      if (logSystem.isLevelEnabled(LogConfig::c_Debug, 100, "top")) {
        print();
      }

    }


    void ChannelMapper::import(const IntervalOfValidity& iov) const
    {
      DBImportArray<TOPChannelMap> array;
      for (const auto& map : m_mapping) {
        array.appendNew(map);
      }
      array.import(iov);
    }


    unsigned ChannelMapper::getChannel(int pixel) const
    {
      if (!isPixelIDValid(pixel)) return c_invalidChannel;

      unsigned pix = pixel - 1;
      unsigned pixRow = pix / c_numPixelColumns;
      unsigned pixCol = pix % c_numPixelColumns;

      unsigned carrier = pixRow / c_numRows;
      unsigned row = pixRow % c_numRows;
      unsigned boardstack = pixCol / c_numColumns;
      unsigned col =  pixCol % c_numColumns;

      const auto& map = m_channels[row][col];
      if (!map) {
        B2WARNING("TOP::ChannelMapper: no channel mapped to pixel " << pixel);
        return c_invalidChannel;
      }
      unsigned asic = map->getASICNumber();
      unsigned chan = map->getASICChannel();

      return getChannel(boardstack, carrier, asic, chan);
    }


    void ChannelMapper::splitChannelNumber(unsigned channel,
                                           unsigned& boardstack,
                                           unsigned& carrier,
                                           unsigned& asic,
                                           unsigned& chan) const
    {
      chan = channel % c_numChannels;
      channel /= c_numChannels;
      asic = channel % c_numAsics;
      channel /= c_numAsics;
      carrier = channel % c_numCarrierBoards;
      boardstack = channel / c_numCarrierBoards;
    }


    int ChannelMapper::getPixelID(unsigned channel) const
    {

      if (!isChannelValid(channel)) return c_invalidPixelID;

      unsigned boardstack = 0;
      unsigned carrier = 0;
      unsigned asic = 0;
      unsigned chan = 0;
      splitChannelNumber(channel, boardstack, carrier, asic, chan);

      const auto& map = m_pixels[asic][chan];
      if (!map) {
        B2ERROR("TOP::ChannelMapper: no pixel mapped to channel " << channel);
        return c_invalidPixelID;
      }
      unsigned row = map->getRow();
      unsigned col = map->getColumn();
      unsigned pixRow = row + carrier * c_numRows;
      unsigned pixCol = col + boardstack * c_numColumns;

      return pixCol + pixRow * c_numPixelColumns + 1;

    }


    void ChannelMapper::print() const
    {
      std::vector<std::string> what;
      what.push_back(string("Boardstack numbers (view from the back):"));
      what.push_back(string("Carrier board numbers (view from the back):"));
      what.push_back(string("ASIC numbers (view from the back):"));
      what.push_back(string("ASIC channel numbers (view from the back):"));
      unsigned value[4] = {0, 0, 0, 0};

      std::string xaxis("+------phi--------->");
      std::string yaxis("|ohr|||^");

      cout << endl;
      cout << "           Mapping of TOP electronic channels to pixels";
      if (!m_typeName.empty()) cout << " for " << m_typeName;
      cout << endl << endl;

      for (int i = 0; i < 4; i++) {
        cout << " " << what[i] << endl << endl;
        for (int row = c_numPixelRows - 1; row >= 0; row--) {
          cout << "  " << yaxis[row] << " ";
          for (int col = 0; col < c_numPixelColumns; col++) {
            int pixel = col + c_numPixelColumns * row + 1;
            auto channel = getChannel(pixel);
            if (channel != c_invalidChannel) {
              splitChannelNumber(channel, value[0], value[1], value[2], value[3]);
              cout << value[i];
            } else {
              cout << "?";
            }
          }
          cout << endl;
        }
        cout << "  " << xaxis << endl << endl;
      }

    }

    void ChannelMapper::test() const
    {
      for (int pixel = 1; pixel <= c_numPixels; pixel++)
        if (pixel != getPixelID(getChannel(pixel)))
          B2ERROR("TOP::ChannelMapper: bug, getPixelID is not inverse of getChannel");

      for (unsigned channel = 0; channel < c_numPixels; channel++)
        if (channel != getChannel(getPixelID(channel)))
          B2ERROR("TOP::ChannelMapper: bug, getChannel is not inverse of getPixelID");
    }


    void ChannelMapper::clear()
    {
      m_mapping.clear();
      for (auto& channels : m_channels) {
        for (auto& channel : channels) channel = 0;
      }
      for (auto& pixels : m_pixels) {
        for (auto& pixel : pixels) pixel = 0;
      }
      m_valid = false;
      m_fromDB = false;
    }


    void ChannelMapper::update()
    {
      clear();
      if (!m_mappingDB->isValid()) return;

      for (const auto& map : *m_mappingDB) {
        m_channels[map.getRow()][map.getColumn()] = &map;
        m_pixels[map.getASICNumber()][map.getASICChannel()] = &map;
      }
      m_valid = true;
      m_fromDB = true;
    }



  } // TOP namespace
} // Belle2 namespace
