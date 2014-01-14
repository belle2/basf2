
#include "framework/gearbox/Backend.h"

namespace Belle2 {
  namespace gearbox {

    Backend::Backend(const BackendConfigParamSet& p) : params(p)
    {

    }

    Backend::~Backend()
    {

    }

    std::list<Backend::MultiOp>::iterator Backend::multiOp_begin(multiOp_opType_t opType, MultiOp::IData_t idata, int autocommit)
    {


      if (opType ==  multiOp_opType_t::ADD_NODES)
        this->checkPathExists(idata["path"]);

      this->multiOpInfos.emplace_front(this, opType, idata, autocommit);

      return this->multiOpInfos.begin();

    }


    bool Backend::multiOp_commit(MultiOpRef it)
    {

      it->flushData();

      return true;

    }

    void Backend::multiOp_remove(MultiOpRef it)
    {

      this->multiOpInfos.erase(it);

    }


    Backend::MultiOp::MultiOp(Backend* owner, multiOp_opType_t type, IData_t idata, int autoCommit) : owner(owner), opType(type), idata(idata), autoCommit(autoCommit)
    {


    }

    Backend::MultiOp::~MultiOp()
    {


    }

    void Backend::MultiOp::addData(Data_t& data)
    {

      this->data.push_back(data);

      if (this->autoCommit > 0 &&  this->autoCommit == this->data.size()) {

        this->commit();

        this->flushData();

      }

    }

    bool Backend::MultiOp::hasData()
    {

      if (this->data.size() > 0) return true;
      else return false;

    }

    Backend::multiOp_opType_t Backend::MultiOp::getType()
    {

      return this->opType;

    }

    void Backend::MultiOp::flushData()
    {

      return this->data.clear();

    }

    bool Backend::MultiOp::commit()
    {

      bool ret(false);

      if (this->hasData()) {

        switch (this->getType()) {

          case multiOp_opType_t::ADD_NODES:

            //ret = this->owner->addNodes(this->idata,this->data,this->cache);

            break;

          case multiOp_opType_t::LINK_NODES:

            //ret = this->linkNodes(this->idata);

            break;

        }

      }


      return ret;

    }

    queryParams_t Backend::parseQueryParams(std::string paramString)
    {

      queryParams_t queryParameters;
      paramString = "foo";
      // TODO

      //QueryParamsParser<std::string::iterator> p;

      //qi::parse(extraction[6].str().begin(), extraction[6].str().begin(), p, queryParameters);

      return queryParameters;

    }

  } // namespace gearbox
} // namespace Belle2
