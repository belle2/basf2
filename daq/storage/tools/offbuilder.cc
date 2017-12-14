#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <framework/logging/Logger.h>

#include <daq/storage/BinData.h>
#include <daq/storage/EventBuffer.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/File.h>
#include <daq/slc/system/FileReader.h>
#include <daq/slc/system/FileWriter.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <arpa/inet.h>

using namespace Belle2;

BinData data(new int[10000000]);
BinData data_hlt(new int[1000000]);
BinData data_onsen(new int[10000000]);

unsigned int readHLT(Reader& reader)
{
  reader.read(data_hlt.getBuffer(), sizeof(int));
  unsigned int nbyte = data_hlt.getByteSize() - sizeof(int);
  reader.read((data_hlt.getBuffer() + 1), nbyte);
  return data_hlt.getWordSize();
}

unsigned int readONSEN(Reader& reader)
{
  unsigned int nbyte_body =
    reader.read(data_onsen.getBody(), sizeof(int) * 2);
  unsigned int* buf = (unsigned int*)data_onsen.getBody();
  unsigned int magic = ntohl(buf[0]);
  if (magic != 0xcafebabe) {
    LogFile::error("bad magic = %.8x", magic);
  }
  unsigned int framenr = ntohl(buf[1]);
  unsigned int* length_of_frame =
    (unsigned int*)(data_onsen.getBody() + 2);
  nbyte_body += reader.read(length_of_frame, sizeof(int) * framenr);
  unsigned int nbyte = 0;
  for (int i = 0; i < framenr; i++) {
    nbyte += ((ntohl(length_of_frame[i]) + 3) & ~3);
  }
  nbyte_body += reader.read(length_of_frame + 1 * framenr, nbyte);
  data_onsen.setBodyWordSize(nbyte_body / sizeof(int));
  return data_onsen.getWordSize();
}

int main()
{
  FileReader<File> reader_hlt(File("/rawdata/disk01/storage/hlt.2014122401.dat"));
  FileReader<File> reader_onsen(File("/rawdata/disk01/storage/onsen.2014122401.dat"));
  FileWriter<File> writer(File("/rawdata/disk01/storage/hlt_onsen.2014122401.dat", "w"));
  while (true) {
    unsigned int nword = readHLT(reader_hlt);
    nword += readONSEN(reader_onsen);
    memcpy(data.getBuffer(), data_hlt.getHeader(), data_hlt.getHeaderByteSize());
    data.setWordSize(nword);
    data.setNBoard(2);
    writer.write(data.getHeader(), data_hlt.getHeaderByteSize());
    writer.write(data_hlt.getBuffer(), data_hlt.getByteSize());
    writer.write(data_onsen.getBuffer(), data_onsen.getByteSize());
    writer.write(data_hlt.getTrailer(), sizeof(BinTrailer));
  }
  return 0;
}

