#include <daq/storage/BinData.h>
#include <daq/storage/SharedEventBuffer.h>

#include <daq/slc/readout/RunInfoBuffer.h>

#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include <unistd.h>

const unsigned int NWORD_BUFFER = 2000000;//20kB
const unsigned int interval = 10000;

using namespace Belle2;

std::vector<SharedEventBuffer> obuf;
SharedEventBuffer dbuf;
std::vector<SharedEventBuffer> ibuf;
int ninput;
RunInfoBuffer info;

void help(int val, char** argv);
void set_arguments(int argc, char** argv);


int main(int argc, char** argv)
{
  if (argc < 2) help(1, argv);
  set_arguments(argc, argv);
  unsigned int offset = 0;
  unsigned int nword = 0;

  Time t0;
  double datasize = 0;
  int* evtbuf = new int[NWORD_BUFFER];
  BinData data;
  unsigned int count = 0;
  if (info.isAvailable()) {
    info.reportReady();
  }
  while (true) {
    nword = 0;
    offset = sizeof(BinHeader) / sizeof(int);
    data.setBuffer(evtbuf);
    data.setExpNumber(0);
    data.setRunNumber(0, 0);
    data.setEventNumber(0);
    if (info.isAvailable()) {
      //info.reportRunning();
    }
    int nobuf = obuf.size();
    for (size_t i = 0; i < ibuf.size(); i++) {
      BinData datain((evtbuf + offset));
      ibuf[i].read((evtbuf + offset), false, false);
      if (count < 10) {
        LogFile::debug("buf[%d]: %d-th event", i, count);
      }
      nword = evtbuf[offset];
      if (info.isAvailable()) {
        info.addInputNBytes(nword * sizeof(int));
      }
      if (datain.getTrailerMagic() != BinData::TRAILER_MAGIC) {
        LogFile::error("Bad trailer magic:%x (should be %x) nword=%u, offset=%u",
                       datain.getTrailerMagic(), BinData::TRAILER_MAGIC,
                       nword, offset);
        return 1;
      }
      offset += nword;
      if (data.getExpNumber() == 0) {
        data.setExpNumber(datain.getExpNumber());
      }
      if (data.getRunNumber() == 0) {
        data.setRunNumber(datain.getRunNumber(), 0);
      }
      if (data.getEventNumber() == 0) {
        data.setEventNumber(datain.getEventNumber());
      }
      if (data.getExpNumber() != datain.getExpNumber() ||
          data.getRunNumber() != datain.getRunNumber() ||
          data.getEventNumber() != datain.getEventNumber()) {
        LogFile::error("Event mismatch (runno.evtno : %u.%u.%u != %u.%u.%u).",
                       data.getExpNumber(), data.getRunNumber(),
                       data.getEventNumber(), datain.getExpNumber(),
                       datain.getRunNumber(), datain.getEventNumber());
        return 1;
      }
    }
    data.setBodyWordSize(offset - sizeof(BinHeader) / sizeof(int));
    data.setNBoard(ninput);
    nword = data.getWordSize();
    datasize += nword * sizeof(int);
    obuf[data.getEventNumber() % nobuf].write(data.getBuffer(), nword, true);
    if (info.isAvailable()) {
      info.addInputCount(1);
      info.addOutputCount(1);
      info.addOutputNBytes(nword * sizeof(int));
    }
    //if (count % 20 == 0 && dbuf.isWritable(nword)) {
    //  dbuf.write(data.getBuffer(), data.getWordSize(), false);
    //}
    if (count < 10) {
      LogFile::debug("%d-th event", count);
    }
    count++;
    if (count % interval == 0) {
      Time t;
      double dt = (t.get() - t0.get());
      double freq = interval / dt  / 1000.;
      double rate = datasize / dt / 1000. / 1000. / 1000. * 8;
      LogFile::debug("Serial = %u Freq = %f [kHz], Rate = %f [Gbps], DataSize = %f [kB/event]",
                     count, freq, rate, datasize / 1000. / interval);
      t0 = t;
      datasize = 0;
    }
  }
  return 0;
}

void help(int val, char** argv)
{
  printf("%s : -i [obname1,obname2,...]:size "
         "-o obname:size -d dbname:size "
         "-m memname:size\n", argv[0]);
  exit(val);
}

void set_arguments(int argc, char** argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "i:o:d:m:h::")) != -1) {
    switch (opt) {
      case 'i': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int bsize_org = atoi(s[1].c_str());
          unsigned int bsize = bsize_org * 1000000;
          StringList s1 = StringUtil::split(s[0], ',');
          ninput = s1.size();
          ibuf.resize(ninput);
          for (int i = 0; i < ninput; i++) {
            ibuf[i].open(s1[i], bsize);
            LogFile::debug("%s:%d", s1[i].c_str(), bsize_org);
          }
        } else {
          help(1, argv);
        }
      } break;
      case 'o': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int bsize_org = atoi(s[1].c_str());
          unsigned int bsize = bsize_org * 1000000;
          StringList s1 = StringUtil::split(s[0], ',');
          obuf.resize(s1.size());
          for (size_t i = 0; i < obuf.size(); i++) {
            obuf[i].open(s1[i], bsize);
            LogFile::debug("%s:%d", s1[i].c_str(), bsize_org);
          }
        } else {
          help(1, argv);
        }
      } break;
      case 'd': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          int bsize_org = atoi(s[1].c_str());
          unsigned int bsize = bsize_org * 1000000;
          dbuf.open(s[0], bsize);
          LogFile::debug("%s:%d", s[0].c_str(), bsize_org);
        } else {
          help(1, argv);
        }
      } break;
      case 'm': {
        StringList s = StringUtil::split(optarg, ':');
        if (s.size() > 1) {
          info.open(s[0], atoi(s[1].c_str()));
        }
      } break;
      case 'h':
        help(0, argv);
      default:
        printf("error! \'%c\' \'%c\'\n", opt, optopt);
        help(1, argv);
    }
  }
}
