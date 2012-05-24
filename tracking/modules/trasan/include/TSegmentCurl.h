#ifndef TSegmentCurl_FLAG_
#define TSegmentCurl_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TCurlFinderParameters.h"
#include "tracking/modules/trasan/AList.h"

namespace Belle {

  class TRGCDC;
  class TSegmentCurl {
  public:
    TSegmentCurl(const unsigned superLayerId = 9999,
                 const unsigned max = 9999);
    TSegmentCurl(TSegmentCurl&);
    TSegmentCurl(TSegmentCurl*);

    ~TSegmentCurl(void);

    const unsigned maxLocalLayerId(void) const;
    const unsigned superLayerId(void) const;
    const unsigned seqOfLayer(const unsigned);
    const unsigned sizeOfLayer(const unsigned);

    const unsigned maxSeq(void) const;
    const unsigned layerIdOfMaxSeq(void);
    const unsigned numOfSeqOneLayer(void);
    const unsigned numOfLargeSeqLayer(void);

    void setMaxSeq(const unsigned);
    void releaseMaxSeq(void);

    inline const unsigned size(void) { return m_list.length(); }

    const unsigned maxLocalLayerId(const unsigned);
    const unsigned superLayerId(const unsigned);

    void append(TLink&);
    void append(TLink*);
    void append(AList<TLink>&);

    void remove(TLink&);
    void remove(TLink*);
    void remove(AList<TLink>&);

    void removeAll(void);

    inline const AList<TLink> & list(void) { return m_list; }
    inline const AList<TLink> & list(unsigned i) { return m_li[i].m_layer; }

    TSegmentCurl& operator=(const TSegmentCurl&);

    void dump(void);

    unsigned wires(const unsigned) const;

    void update(void);
  private:
    static void set_smallcell(bool s) {
      ms_smallcell = s;
    }
    static void set_superb(bool s) {
      ms_superb = s;
    }
    friend class TRGCDC;

  private:
    struct LayerInfo {
      unsigned int m_seqOfLayer;
      unsigned int m_sizeOfLayer;
      AList<TLink> m_layer;
      LayerInfo() : m_seqOfLayer(0), m_sizeOfLayer(0) {
      }
      ~LayerInfo() {
        m_layer.removeAll();
      }
    };


    // main private members
    // flag for the update
    bool m_flagOfUpdate;

    // elements
    AList<TLink> m_list;
    // max local layer id = 0,1,2...
    unsigned m_MaxLocalLayerId;
    // super layer id = 0,1,2...
    unsigned m_superLayerId;
    // seq. num. of each layer
    //  unsigned *m_seqOfLayer;
    // size of each layer
    //  unsigned *m_sizeOfLayer;

    // sub private members
    //  AList<TLink> *m_layer;
    LayerInfo* m_li;


    void calcuSeq(unsigned);

    unsigned m_maxSeq;
    unsigned m_layerIdOfMaxSeq;
    unsigned m_numOfSeqOneLayer;
    unsigned m_numOfLargeSeqLayer;
    //
    // for quick dicision
    //
    static bool ms_smallcell;
    static bool ms_superb;
  };

} // namespace Belle

#endif /* TSegmentCurl_FLAG_ */
