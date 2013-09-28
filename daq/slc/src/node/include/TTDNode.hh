#ifndef _B2DAQ_TTDNode_hh
#define _B2DAQ_TTDNode_hh

#include "NSMNode.hh"
#include "FTSW.hh"

namespace B2DAQ {

  class TTDNode : public NSMNode {

  public:
    static const size_t MAX_FTSWS = 16;

  public:
    typedef std::vector<FTSW*> FTSWList;

  public:
    TTDNode(const std::string& name = "")
      : NSMNode(name, "ttd_node"), 
	_ftsw_v(MAX_FTSWS), _ftsw_i(0) { clearFTSWs(); }
    virtual ~TTDNode() throw() {}

  public:
    FTSW* getFTSW(int i) throw() { return _ftsw_v[i]; }
    FTSWList& getFTSWs() throw() { return _ftsw_v; }
    const FTSWList& getFTSWs() const throw() { return _ftsw_v; }
    void addFTSW(FTSW* ftsw) throw() {
      if ( _ftsw_i < MAX_FTSWS ) {
	_ftsw_v[_ftsw_i] = ftsw;
	_ftsw_i++;
      }
    }
    void clearFTSWs() throw();
    virtual const std::string getSQLFields() const throw();
    virtual const std::string getSQLLabels() const throw();
    virtual const std::string getSQLValues() const throw();
    virtual int getParams(const Command& command, 
			  int* pars, std::string& datap);

  private:
    FTSWList _ftsw_v;
    size_t _ftsw_i;

  };

}

#endif
