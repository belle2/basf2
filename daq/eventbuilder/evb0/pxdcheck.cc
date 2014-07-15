#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>

class {
public:
  struct {
    uint32_t nbyte;
    uint32_t magic;
  } header;
  uint32_t body[10 * 1024 * 1024];

  int nbyte() const {
    return header.nbyte;
  };

  int nword() const {
    return nbyte() / 4;
  };

  uint32_t magic() const {
    return header.magic;
  };

  uint32_t nbyte_body() const {
    return nbyte() - 4;
  };
} onsen_event;

int
main(int argc, char* argv[])
{
  for (int i = 0;; i++) {
    int ret;
    ret = fread(&onsen_event.header, sizeof(onsen_event.header), 1, stdin);
    // assert(sizeof(onsen_event.header) == 8);
    if (ret == 0 && feof(stdin))
      break;
    assert(ret == 1);

    assert(onsen_event.magic() == 0xCAFEBABE);

    int body_size = onsen_event.nbyte_body();

    ret = fread(&onsen_event.body, 1, body_size, stdin);
    assert(ret == body_size);
    printf("%d\n", i);
    for (int i = 0; i < body_size / 4; i++) {
      printf("%4d %08x %d\n", i, onsen_event.body[i], onsen_event.body[i]);
    }
  }
}
