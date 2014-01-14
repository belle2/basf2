#include "framework/gearbox/types.h"

#include <sys/time.h>
#include <stdio.h>


namespace Belle2 {
  namespace gearbox {


    void randomSeed()
    {
      // random seed
      struct timeval t;
      gettimeofday(&t, NULL);
      srand((t.tv_sec * 1000) + (t.tv_usec / 1000));

    }

    std::string genRandomString(size_t length)
    {

      // TODO: switch to c++11 random functionality

      auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
      };

      std::string str(length, 0);
      std::generate_n(str.begin(), length, randchar);
      return str;

    }

  }
}
