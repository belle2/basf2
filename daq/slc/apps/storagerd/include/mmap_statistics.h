#ifndef MMAP_STATISTICS_H
#define MMAP_STATISTICS_H

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>

#ifndef O_NOATIME
#define O_NOATIME 0
#endif

struct stream_statistics {
  uint32_t addr;
  uint32_t port;
  uint64_t event;
  uint64_t byte;
  uint64_t total_byte;
};

class eb_statistics {
private:
  int m_n_u;
  int m_n_d;
  int m_fd_u;
  int m_fd_d;
  int m_len_u;
  int m_len_d;
  stream_statistics* m_stat_u;
  stream_statistics* m_stat_d;

public:
  eb_statistics(const char* file_up, int nup, const char* file_down, int ndown)
  {
    m_n_u = nup;
    m_n_d = ndown;

    m_len_u = sizeof(stream_statistics) * m_n_u;
    m_len_d = sizeof(stream_statistics) * m_n_d;

    m_fd_u = open(file_up,   O_RDWR | O_CREAT | O_NOATIME, 0644);
    assert(m_fd_u >= 0);
    m_fd_d = open(file_down, O_RDWR | O_CREAT | O_NOATIME, 0644);
    assert(m_fd_d >= 0);

    stream_statistics buf_u[m_n_u];
    stream_statistics buf_d[m_n_d];

    memset(buf_u, 0, m_len_u);
    memset(buf_d, 0, m_len_d);

    int ret;

    ret = write(m_fd_u, buf_u, m_len_u);
    assert(ret == m_len_u);
    ret = write(m_fd_d, buf_d, m_len_d);
    assert(ret == m_len_d);

    m_stat_u = (stream_statistics*)mmap(0, m_len_u, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd_u, 0);
    assert(m_stat_u != MAP_FAILED);
    m_stat_d = (stream_statistics*)mmap(0, m_len_d, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd_d, 0);
    assert(m_stat_d != MAP_FAILED);
  };

  ~eb_statistics()
  {
    close(m_fd_u);
    close(m_fd_d);
  };

  stream_statistics* up()
  {
    return m_stat_u;
  };

  stream_statistics& up(int i)
  {
    return m_stat_u[i];
  };

  stream_statistics* down()
  {
    return m_stat_d;
  };

  stream_statistics& down(int i)
  {
    return m_stat_d[i];
  };
};

#endif
