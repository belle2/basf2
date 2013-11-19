#ifndef Belle2_HVCrateInfo_h
#define Belle2_HVCrateInfo_h

#include "HVChannelInfo.h"

#include "base/Serializable.h"

namespace Belle2 {

  class HVCrateInfo : public Serializable {

  public:
    HVCrateInfo(unsigned int id = 0, unsigned int nslot = 0, unsigned int nch = 0);
    virtual ~HVCrateInfo() throw();

  public:
    unsigned int getID() const { return _id; }
    unsigned int getNChannel() const {
      if (_ch_info_v_v.size() > 0) return _ch_info_v_v[0].size();
      else return 0;
    }
    unsigned int getNSlot() const {
      return _ch_info_v_v.size();
    }
    HVChannelInfo* getChannel(unsigned int slot, unsigned int ch) {
      return _ch_info_v_v[slot][ch];
    }
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  private:
    unsigned int _id;
    std::vector<std::vector<HVChannelInfo*> > _ch_info_v_v;

  };

};

#endif
