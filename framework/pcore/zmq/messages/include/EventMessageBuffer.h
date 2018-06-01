//
// Created by abaur on 31.05.18.
//

#pragma once

namespace Belle2 {
  class EventMessageBuffer {
  public:
    EventMessageBuffer(char* data, int size) : m_data(data), m_size(size) {}
    char* getData()const {return m_data;}
    int getSize() const {return m_size;}
  private:
    char* m_data;
    int m_size;
  };
}