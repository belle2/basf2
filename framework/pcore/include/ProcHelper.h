#pragma once

namespace Belle2 {
  enum class ProcType {
    c_Input = 'i',
    c_Worker = 'w',
    c_Output = 'o',
    c_Proxy = 'p',
    c_Monitor = 'm',
    c_Init = '0',
    c_Stopped = 's'
  };
}