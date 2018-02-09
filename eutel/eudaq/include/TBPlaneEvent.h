#ifndef TBPLANEEVENT_H
#define TBPLANEEVENT_H

#include <vector>
#include <memory>

class TBPlaneEvent {
public:
  TBPlaneEvent() : m_PixelData_Col(), m_PixelData_Row(), m_PixelData_Signal()
  {
    m_PixelData_Col.reset(new std::vector<short int>());
    m_PixelData_Row.reset(new std::vector<short int>());
    m_PixelData_Signal.reset(new std::vector<double>());
  }
  virtual ~TBPlaneEvent()
  {
  }
  std::shared_ptr<const std::vector<short int> > getDataCol() const { return m_PixelData_Col; }
  std::shared_ptr<const std::vector<short int> > getDataRow() const { return m_PixelData_Row; }
  std::shared_ptr<const std::vector<double> > getDataSignal() const { return m_PixelData_Signal; }

  void addPixel(short int col, short int row, double charge)
  {
    m_PixelData_Col->push_back(col);
    m_PixelData_Row->push_back(row);
    m_PixelData_Signal->push_back(charge);
  }
  size_t getNumPixels() const { return m_PixelData_Col->size(); }
private:
  std::shared_ptr<std::vector<short int> > m_PixelData_Col;
  std::shared_ptr<std::vector<short int> > m_PixelData_Row;
  std::shared_ptr<std::vector<double> > m_PixelData_Signal;
};

#endif
