/* Yokogawa MW100 Readout program
   S.Uehara 22-APR-2014   */
#include "daq/slc/apps/mwreaderd/MWReaderImp.h"
#include "daq/slc/apps/mwreaderd/mwreader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MW100NUM 2
#define MW100IP0 0xc0a80164 /* 192.168.1.100 */
#define MW100IP1 0xc0a80165 /* 192.168.1.101 */
#define MWMAXITEM 600

#define DEB_WRITE 0

using namespace Belle2;

MWReader::MWReader()
{
  memset(m_buf, 0, sizeof(m_buf));
  m_index = 0;
  m_mw100ip = new int [MW100NUM];
  m_sock = new int [MW100NUM];
  m_reader = NULL;
}

void MWReader::set(mwreader* reader)
{
  m_reader = reader;
  memset(m_reader, 0, sizeof(mwreader));
}

void MWReader::init() throw(IOException)
{
  struct sockaddr_in sockad;

  // IPaddr list
  m_mw100ip[0] = MW100IP0;
  m_mw100ip[1] = MW100IP1;
  for (int ic = 0 ; ic < MW100NUM ; ic++) {
    if ((m_sock[ic] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
      throw (IOException("Failed to create socket"));
    }
    memset(&sockad, 0, sizeof(sockad));
    sockad.sin_family = AF_INET;
    sockad.sin_port = htons((short)34318);
    sockad.sin_addr.s_addr = htonl(m_mw100ip[ic]); /* 192.168.1.100 */
    if (connect(m_sock[ic], (struct sockaddr*)&sockad, sizeof(sockad)) < 0) {
      perror("connect");
      throw (IOException("Failed to connect"));
    }
    // Read the first-connection response
    memset(m_buf, 0, sizeof(m_buf));
    int ret = ::read(m_sock[ic], m_buf, sizeof(m_buf));
    if (ret < 0 && ret == EINTR) {}
    if (ret < 0) {
      perror("read");
      throw (IOException("Failed to read"));
    }
    //printf("receivelen: %d\n",ret);
    m_buf[ret] = '\0';
    if (DEB_WRITE) {
      printf("Connection response: %s \n", m_buf);
    }
  }
}

void MWReader::update() throw(IOException)
{
  m_index = 0;
  for (int ic = 0 ; ic < MW100NUM ; ic++) {
    sprintf(m_buf, "FD0,001,030\r\n");
    int len = strlen(m_buf);
    int ret = ::write(m_sock[ic], m_buf, len);
    if (ret < 0) {
      perror("write");
      throw (IOException("Failed to write"));
    }
    //  printf("%d \n", sizeof(buf));
    ret = ::read(m_sock[ic], m_buf, sizeof(m_buf));
    if (ret < 0 && ret == EINTR) { }
    if (ret < 0) {
      perror("read");
      throw (IOException("Failed to read"));
    }
    if (DEB_WRITE)printf("receivelen: %d\n", ret);
    m_buf[ret] = '\0';
    if (DEB_WRITE)printf("%s", m_buf);
    read(ic, 1);
    //****
    sprintf(m_buf, "FD0,031,060\r\n");
    len = strlen(m_buf);
    ret = ::write(m_sock[ic], m_buf, len);
    if (ret < 0) {
      perror("write");
      throw (IOException("Failed to write"));
    }
    //printf("%d \n", sizeof(buf));
    ret = ::read(m_sock[ic], m_buf, sizeof(m_buf));
    if (ret < 0 && ret == EINTR) {}
    if (ret < 0) {
      perror("read");
      throw (IOException("Failed to read"));
    }
    if (DEB_WRITE)printf("receivelen: %d\n", ret);
    m_buf[ret] = '\0';
    if (DEB_WRITE)printf("%s", m_buf);
    read(ic, 1);
    //*****
    sprintf(m_buf, "FD0,A01,A30\r\n");
    len = strlen(m_buf);
    ret = ::write(m_sock[ic], m_buf, len);
    if (ret < 0) {
      perror("write");
      throw (IOException("Failed to write"));
    }
    ret = ::read(m_sock[ic], m_buf, sizeof(m_buf));
    if (ret < 0 && ret == EINTR) {}
    if (ret < 0) {
      perror("read");
      throw (IOException("Failed to read"));
    }
    if (DEB_WRITE)printf("receivelen: %d\n", ret);
    m_buf[ret] = '\0';
    if (DEB_WRITE)printf("%s", m_buf);
    read(ic, 2);
    //*****
    sprintf(m_buf, "VF0\r\n");
    len = strlen(m_buf);
    if (DEB_WRITE)printf("len: %d\n", len);
    ret = ::write(m_sock[ic], m_buf, len);
    if (ret < 0) {
      perror("write");
      throw (IOException("Failed to write"));
    }
    ret = ::read(m_sock[ic], m_buf, sizeof(m_buf));
    if (ret < 0 && ret == EINTR) {}
    if (ret < 0) {
      perror("read");
      throw (IOException("Failed to read"));
    }
    if (DEB_WRITE)printf("receivelen: %d\n", ret);
    m_buf[ret] = '\0';
    if (DEB_WRITE)printf("%s\n", m_buf);
    read(ic, 3);
  }
  m_reader->nitem = m_index;
  for (int i = 0 ; i < m_index; i++)
    printf("DATA:%s  %d *%s *%s %f \n", m_reader->data[i].cond,
           m_reader->data[i].chan, m_reader->data[i].alarm,
           m_reader->data[i].unit, m_reader->data[i].value);
  printf("DATE:%d  TIME:%d \n", m_reader->date, m_reader->time);
}

void MWReader::read(int ic, int mode) throw(IOException)
{
  // mode 1:Ascii meas, data 2:Ascii math data, 3:DO relays
  char* line;
  char sep[2];
  int len, chan;
  int yy, mm, dd;
  float value;
  char dums[8], cond[4], alarm[8], unit[8];
  sep[0] = (char)13;
  sep[1] = (char)0;
  line = strtok(m_buf, sep);
  len = strlen(line);
  //printf("%d %s %d \n", *ipoi, line, len);
  while (true) {
    line = strtok(NULL, sep);
    len = strlen(line);
    line[len] = '\0';
    //    printf("%d %s \n %d %d %d \n", *ipoi, line, len, ic, mode );
    // Date and Time
    if (strncmp(&line[1], "DA", 2) == 0 && ic == 0 && mode == 1) {
      sscanf(&line[1], "%4s %2d/%2d/%2d", dums, &yy, &mm, &dd);
      m_reader->date = (2000 + yy) * 10000 + mm * 100 + dd;
    }
    if (strncmp(&line[1], "TI", 2) == 0 && ic == 0 && mode == 1) {
      sscanf(&line[1], "%4s %2d:%2d:%2d", dums, &yy, &mm, &dd);
      m_reader->time = yy * 10000 + mm * 100 + dd;
    }
    // mode = 1
    if ((strncmp(&line[1], "O ", 2) == 0 ||
         strncmp(&line[1], "N ", 2) == 0) && mode == 1) {
      strncpy(cond, &line[1], 1);
      sscanf(&line[3], "%3d", &chan);
      strncpy(alarm, &line[7], 4);
      strncpy(unit, &line[11], 6);
      cond[1] = '\0';
      alarm[4] = '\0';
      unit[6] = '\0';
      sscanf(&line[17], "%13f", &value);
      chan = ic * 100 + chan;

      //printf("DATA:%s  %d *%s *%s %f %d \n", cond, chan,
      //       alarm, unit, value, m_index );
      memcpy(m_reader->data[m_index].cond, cond, 1);
      memcpy(m_reader->data[m_index].alarm, alarm, 4);
      memcpy(m_reader->data[m_index].unit, unit, 6);
      m_reader->data[m_index].chan = chan;
      m_reader->data[m_index].value = value;
      m_index++;
    }
    // mode = 2
    if ((strncmp(&line[1], "O ", 2) == 0 ||
         strncmp(&line[1], "N ", 2) == 0) && mode == 2) {
      strncpy(cond, &line[1], 2);
      sscanf(&line[4], "%3d", &chan);
      strncpy(alarm, &line[7], 5);
      strncpy(unit, &line[11], 7);
      cond[1] = '\0';
      alarm[4] = '\0';
      unit[6] = '\0';
      sscanf(&line[17], "%13f", &value);
      chan = ic * 100 + 1000 + chan;
      //printf("DATA:%s  %d *%s *%s %f \n", cond, chan,
      //       alarm, unit, value );
      memcpy(m_reader->data[m_index].cond, cond, 2);
      memcpy(m_reader->data[m_index].alarm, alarm, 5);
      memcpy(m_reader->data[m_index].unit, unit, 7);
      m_reader->data[m_index].chan = chan;
      m_reader->data[m_index].value = value;
      m_index++;
    }
    // mode = 3
    if (mode == 3) {
      unsigned int ival, ival2;
      sscanf(&line[1], "%1d", &chan);
      if (chan >= 0 && chan <= 5) {
        if (strncmp(&line[3], "0", 1) == 0 ||
            strncmp(&line[3], "1", 1) == 0) {
          strncpy(cond, " ", 2);
          strncpy(alarm, "    ", 4);
          strncpy(unit, "      ", 6);
          cond[1] = '\0';
          alarm[4] = '\0';
          unit[6] = '\0';
          sscanf(&line[3], "%10o", &ival);
          ival2 = 0 ;
          for (int k = 0; k < 10; k++)
            if (ival & (1 << k * 3)) ival2 += (1 << k);
          value = (float)ival2 + 0.0001; //Positive ped.
          chan = ic * 100 + 2000 + chan;
          //  printf("DATA:%s  %d *%s *%s %f %d\n", cond, chan,
          //  alarm, unit, value, ival2 );
          memcpy(m_reader->data[m_index].cond, cond, 2);
          memcpy(m_reader->data[m_index].alarm, alarm, 5);
          memcpy(m_reader->data[m_index].unit, unit, 7);
          m_reader->data[m_index].chan = chan;
          m_reader->data[m_index].value = value;
          m_index++;
        }
      }
    }
    if (strncmp(&line[1], "EN", 2) == 0) break;
  }
  printf(" Full-data processed: %d %d %d \n",
         m_reader->date, m_reader->time, m_reader->nitem);
}
