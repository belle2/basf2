#include <eutel/eudaq/DataConverterPlugin.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/RawDataEvent.h>
#include <eutel/eudaq/Logger.h>
#include <eutel/eudaq/Utils.h>
#include <eutel/eudaq/EUTELESCOPE.h>
#include <eutel/eudaq/DEPFETADCValues.h>
#include <eutel/eudaq/DEPFETEvent.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdint.h>

#define MAX_FRAME_LENGTH 131080

unsigned char DHPHybrid5Mapping[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                                        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
                                        56, 57, 58, 59, 60, 61, 62, 63, 64, 68, 72, 76, 80, 84, 88, 92, 100, 99, 101, 98,
                                        102, 97, 103, 96, 65, 69, 73, 77, 81, 85, 89, 93, 108, 107, 109, 106, 110, 105,
                                        111, 104, 66, 70, 74, 78, 82, 86, 90, 94, 116, 115, 117, 114, 118, 113, 119, 112,
                                        67, 71, 75, 79, 83, 87, 91, 95, 124, 123, 125, 122, 126, 121, 127, 120, 188, 184,
                                        180, 176, 172, 168, 164, 160, 132, 131, 133, 130, 134, 129, 135, 128, 189, 185,
                                        181, 177, 173, 169, 165, 161, 140, 139, 141, 138, 142, 137, 143, 136, 190, 186,
                                        182, 178, 174, 170, 166, 162, 148, 147, 149, 146, 150, 145, 151, 144, 191, 187,
                                        183, 179, 175, 171, 167, 163, 156, 155, 157, 154, 158, 153, 159, 152, 192, 193,
                                        194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
                                        210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225,
                                        226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
                                        242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
                                       };


struct EvtHeader {
  unsigned int    EventSize: 20;
  unsigned short   flag0: 1;
  unsigned short   flag1: 1;
  unsigned short  EventType: 2;
  unsigned short  ModuleNo: 4;
  unsigned short  DeviceType: 4;
  unsigned int    Triggernumber;
};

struct InfoWord  {
  unsigned int framecnt: 10; // number of Bits
  unsigned int startgate: 10; //jf new for 128x128
  unsigned int zerosupp: 1;
  unsigned int startgate_ver: 1;
  unsigned int temperature: 10;
};

#define ONSEN_MAGIC 0xCAFEBABE

struct DHH_Header_t {
  unsigned  flag: 1;
  unsigned  DataType: 3;
  unsigned  FrameNr: 4;
  unsigned  DHH_ID: 6;
  unsigned  Chip_ID: 2;
  unsigned short TriggerNr;
};

struct Onsen_Header_t {
  unsigned int magic;
  unsigned int size;
  unsigned int dummy1;
  unsigned int dummy2;
  unsigned int DHH_Header;
};


namespace eudaq {


  class DEPFETConverterBase {
  public:

    depfet::DEPFETADCValues ConvertDEPFETEvent(const std::vector<unsigned char>& data, unsigned id) const
    {


      //---------------------------------------------------------------------------------------------
      depfet::DEPFETADCValues m_event;

      bool debug = false; // false;
      long int rc = 0;

      if (debug) std::cout << "ConvertDEPFETEvent() of block id " << id << " of length " << data.size() << std::endl;
      /*
      // BENNI just print the event
      for (int i = 0; i<(int) data.size(); ++i  ){
      printf("next char 0x%02x\n",data[i]);
      }
      */


      //---------------------------------------------------------------------------------------------
      struct EvtHeader GROUP_Header = *(struct EvtHeader*) &data[rc];
      rc += sizeof(struct EvtHeader);

      if (debug) printf(" read Header: DevTyp=%2d   EvtType=%d  Trig=%#10X  Evt_size=%6d  ModID=%2d \n"
                          , GROUP_Header.DeviceType
                          , GROUP_Header.EventType
                          , GROUP_Header.Triggernumber
                          , GROUP_Header.EventSize
                          , GROUP_Header.ModuleNo
                         );

      int ModID = GROUP_Header.ModuleNo;
      m_event.setModuleNr(ModID);
      m_event.setTriggerNr(GROUP_Header.Triggernumber);


      //---------------------------------------------------------------------------------------------
      if (GROUP_Header.EventType == 0 && GROUP_Header.DeviceType == 14) { //-- INFO event
        if (debug)printf("     Info Event\n     Run Number=%d \n", GROUP_Header.Triggernumber);
      }

      //---------------------------------------------------------------------------------------------
      if (GROUP_Header.EventType == 2 && GROUP_Header.DeviceType == 13) { //-- TLU event
        if (debug)printf(" TLU Event found!! This will probably crash!!");
      }

      //---------------------------------------------------------------------------------------------
      //              check DATA event
      //---------------------------------------------------------------------------------------------
      if (GROUP_Header.EventType == 2
          && (GROUP_Header.DeviceType == 2  || GROUP_Header.DeviceType == 3 || GROUP_Header.DeviceType == 4 || GROUP_Header.DeviceType == 11)
          &&  ModID > 0) {
        if (debug)printf(" Data event from DevType=%d: this would be Hybrid 4 or 4.1 Geant, etc...\n", GROUP_Header.DeviceType);
      }



      if (GROUP_Header.EventType == 2
          && (GROUP_Header.DeviceType == 7) // This is the DHH/Onsen system; Should be DeviceType==7
          &&  ModID > 0) {

        // The InfoWord is a 32 bit frame header
        // behind the Bonn header
        struct InfoWord frameInfoData = *(struct InfoWord*) &data[rc];
        rc += sizeof(struct InfoWord);

        uint16_t tempZSFrameData[MAX_FRAME_LENGTH];

        // Npixel counts how many byte in payload
        // EventSize is size of payload + header in 32bit integers
        int Npixels = (GROUP_Header.EventSize - 3) * 4;

        //std::cout << "BENNI NPIXEL " << Npixels << std::endl;

        // Start looping over DHH/Onsen frames in the event

        unsigned int nextHeaderIndex = rc;
        unsigned int lastByte = Npixels + rc;
        bool finished = false;
        uint16_t lastWord = 0;

        while ((!finished) && (nextHeaderIndex < lastByte)) {

          char dhhType = (data[nextHeaderIndex + 17] & 0x70) >> 4;

          //printf("dhhType = %d\n", dhhType);

          /*
          if( !((dhhType == 3) || (dhhType == 4) || (dhhType == 2))){

          std::cout << "+++++++++++++++++++++++++++++++---------------" << std::endl;

          printf("dhhType = %d\n", dhhType);
            uint16_t DHPheader = (data[nextHeaderIndex+21] << 8) + (data[nextHeaderIndex+20]);
          if( DHPheader != 0xA000){
          //printf("Error header!!! = %04X\n", DHPheader);
          if(DHPheader == lastWord){
          //printf("Header corrected\n");
          DHPheader = 0xA000;
          dhhType = 5;
          }
            }
          }
          */



          // Extract ONSEN and DHH Header (==Onsen_Header_t )
          // DHH header is last integer in Onsen_Header_t
          // firstHeader.size is size of payload (without Onsen header but with DHH header) in byte
          struct Onsen_Header_t firstHeader = *(struct Onsen_Header_t*) &data[nextHeaderIndex];
          rc += sizeof(struct Onsen_Header_t);

          if (firstHeader.magic != ONSEN_MAGIC) {
            printf("\nERROR, no magic detected! got 0x%08x, expected 0x%08x \n\n", firstHeader.magic, ONSEN_MAGIC);
            return m_event;
          }

          unsigned int currentPayloadBeginIndex = nextHeaderIndex + sizeof(Onsen_Header_t); // in byte
          unsigned int payloadLength = firstHeader.size - sizeof(unsigned int); // in byte

          nextHeaderIndex += firstHeader.size + sizeof(Onsen_Header_t) - sizeof(unsigned int);
          if ((firstHeader.size + sizeof(Onsen_Header_t) - sizeof(unsigned int)) % 4 != 0) {
            nextHeaderIndex += 2;
          }

          //Correct DHH Header Endianess:
          unsigned int DHHHeaderNotCorrected = firstHeader.DHH_Header;

          DHH_Header_t  DHH_Header;
          DHH_Header.flag = (DHHHeaderNotCorrected & 0x8000) >> 15;
          DHH_Header.DataType = (DHHHeaderNotCorrected & 0x7800) >> 11;
          DHH_Header.FrameNr = (DHHHeaderNotCorrected & 0x0f00) >> 8;
          DHH_Header.DHH_ID = (DHHHeaderNotCorrected & 0x00fc) >> 2;
          DHH_Header.Chip_ID = (DHHHeaderNotCorrected & 0x0003);
          DHH_Header.TriggerNr = (DHHHeaderNotCorrected) >> 16;

          if (debug) printf("ErrorFlag %d, DataType %d, FrameNr %d, DHH_ID %d, Chip_ID %d, TriggerNr 0x%04x\n",
                              DHH_Header.flag,
                              DHH_Header.DataType,
                              DHH_Header.FrameNr,
                              DHH_Header.DHH_ID,
                              DHH_Header.Chip_ID,
                              DHH_Header.TriggerNr);

          if (false) {
            for (unsigned int index = 0; index < payloadLength; index++) {
              printf(" 0x%02x ", data[currentPayloadBeginIndex + index]);
            }
            printf("\n");
          }

          //if ( !(DHH_Header.DataType==2 || DHH_Header.DataType==3 || DHH_Header.DataType==4 || DHH_Header.DataType==5 ))
          //  printf("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGg\n");

          if (DHH_Header.DataType == 3) {
            if (debug)printf("StartOfFrame arrived\n");

            // This is data
            rc = currentPayloadBeginIndex;
            for (unsigned int index = 0; index < payloadLength / 2; index++) {
              tempZSFrameData[index] = *(uint16_t*) &data[rc];
              rc += sizeof(uint16_t);
              if (debug)printf(" 0x%02x ", tempZSFrameData[index]);
            }

            int delay_frsyc2trg = ((tempZSFrameData[3] & 0x3ff));
            int lastdhpframe = (tempZSFrameData[3] & 0xfc00) >> 10;

            if (debug) std::cout << " delay " << delay_frsyc2trg << std::endl;
            if (debug) std::cout << " last dhp frame id " << lastdhpframe << std::endl;

            continue;
          }
          if (DHH_Header.DataType == 4) {
            if (debug)printf("EndOfFrame arrived\n");
            finished = true;
            continue;
          }

          if (DHH_Header.DataType == 5) {

            // This is ZS data from DHH/Onsen
            rc = currentPayloadBeginIndex;
            for (unsigned int index = 0; index < payloadLength / 2; index++) {
              tempZSFrameData[index] = *(uint16_t*) &data[rc];
              rc += sizeof(uint16_t);
            }

            if (debug)printf("ZSData arrived\n");
            uint16_t current_word, current_row_base, current_row_bit, current_val, current_col, current_CM, current_row;
            uint16_t frame_type = ((tempZSFrameData[0]) >> 13);
            unsigned char mappedCol, mappedRowOffset, DCDChannel, mappedDrain, rowOffset;
            uint16_t mappedRow;

            // Here: word means 16bit integer
            // Here: any header can be skipped.
            //
            // In case last word is 0000, it means last
            // three words are footer (header+padding)
            // and should be skipped.
            // Otherwise, last two words are header.
            // Just skip it.
            // First two words are always header (skip
            // it). If a000 appears, new DHP reading
            // frame starts. Skip this word and the next
            // word.

            // Check for padding
            int lastword;
            if (tempZSFrameData[payloadLength / 2 - 1] == 0x0000)
              lastword = (payloadLength - 6) / 2; // skip last three words
            else
              lastword = (payloadLength - 4) / 2; // skip last two words

            if (debug) std::cout << "DHP frame id " << tempZSFrameData[1] << std::endl;


            lastWord = tempZSFrameData[payloadLength / 2 - 3] ;

            //printf("Drittes wort vom ende %04x\n",tempZSFrameData[payloadLength/2-3]);

            unsigned short mydhpid = tempZSFrameData[0];

            if ((mydhpid & 0xFFFC) != 0xA000) std::cout << "EEEERRRRROOOOOOOOOOR" << std::endl;

            mydhpid = mydhpid & 0x0003;

            for (int ipix = 2; ipix < lastword; ipix++) {
              if (debug) printf("ZSData arrived %04x\n", tempZSFrameData[ipix]);
              current_word = tempZSFrameData[ipix];

              // Detect start of new DHP reading frame
              if (current_word == 0xa000) {
                // Skip this word and the next
                ipix++;
                continue;
              }

              if (((current_word) >> 15) == 0) { //row header
                current_row_base = 2 * ((current_word) >> 6);
                current_CM = (current_word) & 0x3f;
                if (debug) printf("New row Header 0x%4x:  row base %2d,with common mode of:%d\n", current_word, current_row_base, current_CM);
              } else { //row content
                current_val = (current_word) & 0xFF;
                current_col = ((current_word) >> 8) & 0x3F;
                current_row_bit = (((current_word) >> 8) & 0x40) / 64;
                current_row = current_row_base + current_row_bit;
                if (debug) printf("New hit token 0x%4x: col: %3d val: %2d]\n", current_word, current_col, current_val);

                //mapping
                //rowOffset=current_row%4;
                //DCDChannel=4*current_col+rowOffset;
                //mappedDrain=DHPHybrid5Mapping[DCDChannel];
                mappedCol = (current_col ^ 0x3C) + 64 * mydhpid;  //mappedDrain>>2;
                //mappedRowOffset=3-mappedDrain%4;
                mappedRow = current_row; //(current_row>>2)*4+mappedRowOffset;
                if (debug)
                  printf("Translating coordinates DHPRow %4d%%4 = %d, DHPCol %3d -> DCDChannel %3d -> Drain %3d, Col %d, row %4d, Offset %d\n",
                         current_row, rowOffset, current_col, DCDChannel, mappedDrain, mappedCol, mappedRow, mappedRowOffset);
                if ((mappedRow < 480) && (mappedCol < 128)) {
                  m_event.at(0).push_back(mappedRow);
                  m_event.at(1).push_back(mappedCol);
                  m_event.at(2).push_back(current_val);
                  m_event.at(3).push_back(current_CM);
                } else printf("Warning: col/val: [%d|%d], in row  %d, out of range. Max Row=%d, max col=%d\n", mappedCol, current_val, mappedRow,
                                480, 128);
              }

            } //-- end dhp reading frame (hits)

          } //-- end zs data frame

        } //-- end loop over DHH Frames

      } //-- end DHH/Onsen DATA event


      return m_event;
    }

  protected:
    static size_t NumPlanes(const Event& event)
    {
      if (const RawDataEvent* ev = dynamic_cast<const RawDataEvent*>(&event)) {
        return ev->NumBlocks();
      }
      return 0;
    }

    static std::vector<unsigned char> GetPlane(const Event& event, size_t i)
    {
      if (const RawDataEvent* ev = dynamic_cast<const RawDataEvent*>(&event)) {
        return ev->GetBlock(i);
      }
      return std::vector<unsigned char>();
    }

    static size_t GetID(const Event& event, size_t i)
    {
      if (const RawDataEvent* ev = dynamic_cast<const RawDataEvent*>(&event)) {
        return ev->GetID(i);
      }
      return 0;
    }

  }; // end Base

  /********************************************/

  class DEPFETConverterPlugin : public DataConverterPlugin, public DEPFETConverterBase {
  public:
    //virtual lcio::LCEvent * GetLCIOEvent( eudaq::Event const * ee ) const;

    virtual bool GetStandardSubEvent(StandardEvent&, const eudaq::Event&) const;

    virtual unsigned GetTriggerID(Event const& ev) const
    {
      const RawDataEvent& rawev = dynamic_cast<const RawDataEvent&>(ev);
      if (rawev.NumBlocks() < 1) return (unsigned) - 1;
      return getlittleendian<unsigned>(&rawev.GetBlock(0)[4]);
    }

    virtual bool GetTBTelEventSubEvent(TBTelEvent& result, const Event& source) const;

  private:
    DEPFETConverterPlugin() : DataConverterPlugin("DEPFET") {}

    static DEPFETConverterPlugin const m_instance;
  };

  DEPFETConverterPlugin const DEPFETConverterPlugin::m_instance;

  bool DEPFETConverterPlugin::GetStandardSubEvent(StandardEvent& result, const Event& source) const
  {

    if (source.IsBORE()) {
      //FillInfo(source);
      return true;
    } else if (source.IsEORE()) {
      // nothing to do
      return true;
    }

    // If we get here it must be a data event

    //-----------------------------------------------
    // Decode event data to a DEPFETEvent format

    size_t numplanes = NumPlanes(source);

    depfet::DEPFETEvent m_event;

    for (size_t iPlane = 0; iPlane < numplanes; ++iPlane) {

      // Get read fully encoded subevent
      depfet::DEPFETADCValues m_subevent = ConvertDEPFETEvent(GetPlane(source, iPlane), GetID(source, iPlane));
      m_event.push_back(m_subevent);
    }

    //-----------------------------------------------
    // Decode event data to a eudaq::StandardEvent

    for (size_t iframe = 0; iframe < m_event.size(); iframe++) {

      // Get read fully decoded data
      const depfet::DEPFETADCValues& data = m_event[iframe];
      int nPixel = (int) data.at(0).size();

      StandardPlane plane(data.getModuleNr(), "DEPFET", "DEPFET");
      plane.SetSizeZS(64, 1024, nPixel);

      for (int iPixel = 0; iPixel < nPixel; iPixel++) {
        int val = data.at(2).at(iPixel);
        int col = data.at(1).at(iPixel);
        int row = data.at(0).at(iPixel);

        //cout << " col  " << col << endl;
        //cout << " row  " << row << endl;
        //cout << " val  " << val << endl;

        // Store pixel data
        plane.SetPixel(iPixel, col, row, val);
      }

      // Add plane to event
      result.AddPlane(plane);

    }

    return true;
  }


  bool DEPFETConverterPlugin::GetTBTelEventSubEvent(TBTelEvent& result, const Event& source) const
  {

    if (source.IsBORE()) {
      // shouldn't happen
      return true;
    } else if (source.IsEORE()) {
      // nothing to do
      return true;
    }

    // If we get here it must be a DEPFET data event

    //-----------------------------------------------
    // Decode event data to a DEPFETEvent format

    size_t numplanes = NumPlanes(source);

    depfet::DEPFETEvent m_event;

    for (size_t iPlane = 0; iPlane < numplanes; ++iPlane) {

      // Get read fully encoded subevent
      depfet::DEPFETADCValues m_subevent = ConvertDEPFETEvent(GetPlane(source, iPlane), GetID(source, iPlane));
      m_event.push_back(m_subevent);
    }

    //-----------------------------------------------
    // Decode event data to a TBTelEvent

    for (size_t iframe = 0; iframe < m_event.size(); iframe++) {

      // Get read fully decoded data
      const depfet::DEPFETADCValues& data = m_event[iframe];

      int sensorID = data.getModuleNr();

      size_t nPixel = (int) data.at(0).size();

      for (size_t iPixel = 0; iPixel < nPixel; iPixel++) {
        int val = data.at(2).at(iPixel);
        int col = data.at(1).at(iPixel);
        int row = data.at(0).at(iPixel);
        int cm = data.at(3).at(iPixel);

        //cout << " col  " << col << endl;
        //cout << " row  " << row << endl;
        //cout << " val  " << val << endl;
        //cout << " cm  " << cm << endl;

        // Store pixel data int EUTelescope format
        result.addPXDPixel(sensorID, col, row, val);
      }
    }
    return true;
  }

} //namespace eudaq
