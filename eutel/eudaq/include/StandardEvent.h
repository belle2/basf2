#ifndef EUDAQ_INCLUDED_StandardEvent
#define EUDAQ_INCLUDED_StandardEvent

#include <eutel/eudaq/Event.h>
#include <vector>
#include <string>

namespace eudaq {

  class StandardPlane : public Serializable {
  public:
    enum FLAGS { FLAG_ZS = 0x1, // Data are zero suppressed
                 FLAG_NEEDCDS = 0x2, // CDS needs to be calculated (data is 2 or 3 raw frames)
                 FLAG_NEGATIVE = 0x4, // Signal polarity is negative
                 FLAG_ACCUMULATE = 0x8, // Multiple frames should be accumulated for output

                 FLAG_WITHPIVOT = 0x10000, // Include before/after pivot boolean per pixel
                 FLAG_WITHSUBMAT = 0x20000, // Include Submatrix ID per pixel
                 FLAG_DIFFCOORDS = 0x40000 // Each frame can have different coordinates (in ZS mode)
               };
    typedef double pixel_t;
    typedef double coord_t;
    StandardPlane(unsigned id, const std::string& type,
                  const std::string& sensor = "");
    StandardPlane(Deserializer&);
    StandardPlane();
    void Serialize(Serializer&) const;
    void SetSizeRaw(unsigned w, unsigned h,
                    unsigned frames = 1, int flags = 0);
    void SetSizeZS(unsigned w, unsigned h, unsigned npix,
                   unsigned frames = 1, int flags = 0);

    template <typename T>
    void SetPixel(unsigned index, unsigned x, unsigned y,
                  T pix, bool pivot = false, unsigned frame = 0)
    {
      SetPixelHelper(index, x, y, (double)pix, pivot, frame);
    }
    template <typename T>
    void SetPixel(unsigned index, unsigned x, unsigned y,
                  T pix, unsigned frame)
    {
      SetPixelHelper(index, x, y, (double)pix, false, frame);
    }
    template <typename T>
    void PushPixel(unsigned x, unsigned y, T pix,
                   bool pivot = false, unsigned frame = 0)
    {
      PushPixelHelper(x, y, (double)pix, pivot, frame);
    }
    template <typename T>
    void PushPixel(unsigned x, unsigned y, T pix, unsigned frame)
    {
      PushPixelHelper(x, y, (double)pix, false, frame);
    }

    void SetPixelHelper(unsigned index, unsigned x, unsigned y,
                        double pix, bool pivot, unsigned frame);
    void PushPixelHelper(unsigned x, unsigned y,
                         double pix, bool pivot, unsigned frame);
    double GetPixel(unsigned index, unsigned frame) const;
    double GetPixel(unsigned index) const;
    double GetX(unsigned index, unsigned frame) const;
    double GetX(unsigned index) const;
    double GetY(unsigned index, unsigned frame) const;
    double GetY(unsigned index) const;
    bool GetPivot(unsigned index, unsigned frame = 0) const;
    void SetPivot(unsigned index, unsigned frame , bool PivotFlag);
    // defined for short, int, double
    template <typename T>
    std::vector<T> GetPixels() const;
    const std::vector<coord_t>& XVector(unsigned frame) const;
    const std::vector<coord_t>& XVector() const;
    const std::vector<coord_t>& YVector(unsigned frame) const;
    const std::vector<coord_t>& YVector() const;
    const std::vector<pixel_t>& PixVector(unsigned frame) const;
    const std::vector<pixel_t>& PixVector() const;

    void SetXSize(unsigned x);
    void SetYSize(unsigned y);
    void SetTLUEvent(unsigned ev);
    void SetPivotPixel(unsigned p);
    void SetFlags(FLAGS flags);

    unsigned ID() const;
    const std::string& Type() const;
    const std::string& Sensor() const;
    unsigned XSize() const;
    unsigned YSize() const;
    unsigned NumFrames() const;
    unsigned TotalPixels() const;
    unsigned HitPixels(unsigned frame) const;
    unsigned HitPixels() const;
    unsigned TLUEvent() const;
    unsigned PivotPixel() const;

    int GetFlags(int f) const;
    bool NeedsCDS() const;
    int  Polarity() const;

    void Print(std::ostream&) const;
  private:
    const std::vector<pixel_t>& GetFrame(const std::vector<std::vector<pixel_t> >& v, unsigned f) const;
    void SetupResult() const;

    std::string m_type, m_sensor;
    unsigned m_id, m_tluevent;
    unsigned m_xsize, m_ysize;
    unsigned m_flags, m_pivotpixel;
    std::vector<std::vector<pixel_t> > m_pix;
    std::vector<std::vector<coord_t> > m_x, m_y;
    std::vector<std::vector<bool> > m_pivot;
    std::vector<unsigned> m_mat;

    mutable const std::vector<pixel_t>* m_result_pix;
    mutable const std::vector<coord_t>* m_result_x, * m_result_y;

    mutable std::vector<pixel_t> m_temp_pix;
    mutable std::vector<coord_t> m_temp_x, m_temp_y;
  };

  class StandardEvent : public Event {
    EUDAQ_DECLARE_EVENT(StandardEvent);
  public:
    StandardEvent(unsigned run = 0, unsigned evnum = 0,
                  unsigned long long timestamp = NOTIMESTAMP);
    StandardEvent(const Event&);
    StandardEvent(Deserializer&);
    void SetTimestamp(unsigned long long);

    StandardPlane& AddPlane(const StandardPlane&);
    size_t NumPlanes() const;
    const StandardPlane& GetPlane(size_t i) const;
    StandardPlane& GetPlane(size_t i);
    virtual void Serialize(Serializer&) const;
    virtual void Print(std::ostream&) const;

  private:
    std::vector<StandardPlane> m_planes;
  };

  inline std::ostream& operator << (std::ostream& os,
                                    const StandardPlane& pl)
  {
    pl.Print(os);
    return os;
  }
  inline std::ostream& operator << (std::ostream& os,
                                    const StandardEvent& ev)
  {
    ev.Print(os);
    return os;
  }

} // namespace eudaq

#endif // EUDAQ_INCLUDED_StandardEvent
