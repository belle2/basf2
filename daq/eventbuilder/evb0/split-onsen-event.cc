#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "onsen_tcp.h"

class onsen_file : public onsen_tcp {
public:
  void set_fp(FILE* fp) {
    m_fp = fp;
  };

  int write_event(FILE* dest) {
    int ret;
    ret = fwrite(&m_magic, sizeof(m_magic), 1, dest);
    if (ret != 1) {
      log("failed to write MAGIC: %s\n", strferror(dest, "w"));
      exit(1);
    }

    ret = fwrite(&m_framenr, sizeof(m_framenr), 1, dest);
    if (ret != 1) {
      log("failed to write framenr: %s\n", strferror(dest, "w"));
      exit(1);
    }

    const int nframe = framenr();

    ret = fwrite(m_length_of_frame,
                 nframe * sizeof(m_length_of_frame[0]), 1, dest);

    if (ret != 1) {
      log("failed to write length_of_frame: %s\n", strferror(dest, "w"));
      exit(1);
    }

    ret = fwrite(m_body, nbyte_of_frames(), 1, dest);
    if (ret != 1) {
      log("failed to write ONSEN frames: %s\n", strferror(dest, "w"));
      exit(1);
    }

    return nbyte();
  };
};

int
main(int argc, char** argv)
{
  int ret;
  char* filename = "stdin";
  FILE* in = stdin;

  if (argc > 1) {
    filename = argv[1];
    in = fopen(filename, "r");
    if (in == NULL) {
      perror("fopen:");
      exit(1);
    }
    assert(in);
    fprintf(stderr, "open %s\n", filename);
  }

  onsen_file* onsen = new onsen_file;

  onsen->set_fp(in);

  for (int event_number = 0;; event_number++) {
    onsen->fetch_event();

    char dumpfilename[1024];

    snprintf(dumpfilename, sizeof(dumpfilename),
             "%s.%d", filename, event_number);

    assert(access(dumpfilename, F_OK) == -1);

    FILE* out = fopen(dumpfilename, "w");

    onsen->write_event(out);

    fclose(out);
  }
}
