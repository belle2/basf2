#ifndef _zlibstream_h

#include <zlib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define BUFFER_MAX 1024 * 1024 //1MB
#define WRITE_TH 1024 * 500 // 500KB

class zlibstream {

public:
  zlibstream();
  zlibstream(const char* filename, const char* mode);
  ~zlibstream() throw();

public:
  bool open(const char* filename, const char* mode);
  template <typename T> int read(T& data);
  template <typename T> int write(const T& data);
  void close();
  int flush();

private:
  FILE* m_file;
  unsigned char* m_cmpbuf;
  unsigned char* m_orgbuf;
  unsigned long int m_orgsize;
  bool m_wmode;
  int m_offset;
  unsigned long int m_count;

};

zlibstream::zlibstream()
  : m_file(NULL), m_cmpbuf(NULL), m_orgbuf(NULL), m_wmode(false)
{

}

zlibstream::zlibstream(const char* filename, const char* mode)
  : m_file(NULL), m_cmpbuf(NULL), m_orgbuf(NULL), m_wmode(false)
{
  open(filename, mode);
}

zlibstream::~zlibstream() throw()
{
  close();
}

bool zlibstream::open(const char* filename, const char* mode)
{
  if (!m_file && !m_cmpbuf) {
    m_file = fopen(filename, mode);
    if (m_file == NULL) {
      perror("open");
      return false;
    }
    m_cmpbuf = (unsigned char*)malloc(BUFFER_MAX);
    m_orgbuf = (unsigned char*)malloc(BUFFER_MAX);
    m_wmode = (strcmp(mode, "w") == 0);
    m_orgsize = 0;
    m_count = 0;
  } else {
    printf("zlibstream::open already opened\n");
  }
  return true;
}

void zlibstream::close()
{
  if (m_file) {
    flush();
    fclose(m_file);
    m_file = NULL;
    free(m_cmpbuf);
    if (m_wmode) {
      printf("write total size = %lu\n", m_count);
    }
  }
}

template <typename T>
int zlibstream::read(T& data)
{
  if (!m_file || !m_cmpbuf || m_wmode) {
    printf("zlibstream::read not ready\n");
    return 0;
  }
  unsigned long int size_in = sizeof(data);
  if (m_orgsize > 0 && m_offset <= m_orgsize - size_in) {
    memcpy(&data, (m_orgbuf + m_offset), size_in);
    m_offset += size_in;
    return size_in;
  }
  if (feof(m_file) != 0) return 0;
  int size = 0;
  fread(&size, sizeof(int), 1, m_file);
  if (size == 0) return 0;
  fread(m_cmpbuf, size, 1, m_file);
  m_orgsize = BUFFER_MAX;
  m_offset = 0;
  uncompress(m_orgbuf, &m_orgsize, m_cmpbuf, size);
  memcpy(&data, (m_orgbuf + m_offset), size_in);
  m_offset += size_in;
  return size_in;
}

template <typename T>
int zlibstream::write(const T& data)
{
  if (!m_file || !m_cmpbuf || !m_wmode) {
    printf("zlibstream::write not ready\n");
    return 0;
  }
  unsigned long int size_in = sizeof(data);
  memcpy(m_orgbuf + m_orgsize, &data, size_in);
  m_orgsize += size_in;
  m_count += size_in;
  if (m_orgsize >= WRITE_TH) flush();
  return size_in;
}

int zlibstream::flush()
{
  if (!m_file || !m_cmpbuf || !m_wmode || m_orgsize == 0) {
    return 0;
  }
  unsigned long int cmpsize = BUFFER_MAX;
  compress2(m_cmpbuf, &cmpsize, m_orgbuf, m_orgsize, 1);
  int size = (int)cmpsize;
  fwrite(&size, sizeof(int), 1, m_file);
  fwrite(m_cmpbuf, cmpsize, 1, m_file);
  fflush(m_file);
  m_orgsize = 0;
  return 1;
}

#endif
