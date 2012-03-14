/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <root/TUrl.h>
#include <root/TString.h>
#include <root/TMessage.h>
#include <root/TSQLServer.h>
#include <root/TSQLResult.h>
#include <root/TSQLRow.h>
#include <root/TSQLStatement.h>

#ifdef HAS_ROOTPGSQL
#include <root/TPgSQLStatement.h>
#endif

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/gearbox/SqlHandler.h>
#include <framework/gearbox/Gearbox.h>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;
namespace io = boost::iostreams;

namespace Belle2 {
  namespace gearbox {

    enum { kSQLHANDLER_TMessage = 1, kSQLHANDLER_String = 0 };
    enum { kMySQL, kPgSQL, kOracle };

    class SqlMessage : public TMessage {
    private:
      Int_t kHdrlen;
    public:
      SqlMessage(void* buf, Int_t bufsize): TMessage(buf, bufsize) { kHdrlen = 2 * sizeof(UInt_t); };
      char* Data() { return Buffer() + kHdrlen; }
      Int_t  DataSize() { return BufferSize() - kHdrlen; }
    };

    std::string SqlHandler::GetQuery(TSQLServer* sql,  TUrl* url)
    {
      TString sqlstring;
      TString valquote;
      TString keyquote;
      if (strcmp(sql->ClassName(), "TMySQLServer") == 0) {
        valquote = "\"";
        keyquote = "`";
      } else {
        valquote = "\"";
        keyquote = "";
      }

      sqlstring.Form("SELECT %skey%s, %skeyid%s, %stype%s, %sdata%s FROM GEOMETRY ", keyquote.Data(), keyquote.Data(), keyquote.Data(),  keyquote.Data(), keyquote.Data(),
                     keyquote.Data(), keyquote.Data(), keyquote.Data());

      TString urloptions = url->GetOptions();
      TObjArray* objOptions = urloptions.Tokenize("&");
      for (Int_t n = 0; n < objOptions->GetEntries(); n++) {
        TString loption = ((TObjString*) objOptions->At(n))->GetName();


        TString optoperator[] = {"=", ">", "<", ">=", "<=", "!="};
        TString Operator = optoperator[0];
        for (int i = 0; i < 6; i++) if (loption.Contains(optoperator[i])) Operator = optoperator[i];

        TObjArray* objTags = loption.Tokenize(Operator);
        if (objTags->GetEntries() == 2) {
          TString key = ((TObjString*) objTags->At(0))->GetName();
          TString value = ((TObjString*) objTags->At(1))->GetName();

          if (n) sqlstring.Append(" AND ");
          else sqlstring.Append(" WHERE ");
          TString condition;
          condition.Append(keyquote);
          condition.Append(key);
          condition.Append(keyquote);

          condition.Append(Operator);

          condition.Append(valquote);
          condition.Append(value);
          condition.Append(valquote);

          sqlstring.Append(condition);
        }
        delete objTags;
      }
      delete objOptions;
      return std::string(sqlstring.Data());
    }
    //______________________________________________________________________________

    SqlContext::SqlContext(TSQLStatement* statement)
    {
      int PgSqlType = 0;
      if (strcmp(statement->ClassName(), "TPgSQLStatement") == 0) {
#ifdef HAS_ROOTPGSQL
        ((TPgSQLStatement*) statement)->SetResultFormat(1);
#endif
        PgSqlType = 1;
      }
      if (statement->Process()) {
        statement->StoreResult();
        statement->NextResultRow(); // take the first result row
        //  Int_t key        = statement->GetInt(0);
        //      Int_t keyid      = statement->GetInt(1);
        Int_t type       = statement->GetInt(2);
        char* data;
        Long_t bsize = 1;
        if (PgSqlType) data = new char[bsize];
        Bool_t err = statement->GetBinary(3, (void *&) data, bsize);
        if (err == kTRUE && bsize > 0) {
          switch (type) {
            case kSQLHANDLER_TMessage: {
              SqlMessage* message = new SqlMessage(data, bsize); // TMessage constructor is protected
              switch (message->What()) {
                case  kMESS_STRING:
                  if (message->DataSize() > 0) m_stream.push(io::array_source(message->Data(), message->Data() + message->DataSize()));
                  break;
                case kMESS_OBJECT:
                  //    GetObjects(message);
                  break;
              }

              break;
            }
            case kSQLHANDLER_String: {
              int compressed = 0;
              if (compressed) m_stream.push(io::gzip_decompressor());
              m_stream.push(io::array_source(data, data + strlen(data)));
              break;
            }

          }
        } else {
          B2ERROR("SqlHandler no Data");
        }
      }
    }

    SqlContext::~SqlContext()
    {

    }



    SqlHandler::SqlHandler(const string& uri): InputHandler(uri), m_database(0), m_statement(0)
    {

    }

    InputContext* SqlHandler::open(const string& path)
    {

      TUrl* url = new TUrl(path.c_str());
      url->Print();
      B2INFO(m_uri << "SqlHandler::Turl protocol=" << url->GetProtocol());
      B2INFO(m_uri << "SqlHandler::Turl host=" << url->GetHost());
      B2INFO(m_uri << "SqlHandler::Turl port=" << url->GetPort());
      B2INFO(m_uri << "SqlHandler::Turl dbname=" << url->GetFile());
      B2INFO(m_uri << "SqlHandler::Turl user=" << url->GetUser());
      B2INFO(m_uri << "SqlHandler::Turl pwd=" << url->GetPasswd());
      TString protocol(url->GetProtocol());
      if (!protocol.EqualTo("mysql") && !protocol.EqualTo("pgsql")  && !protocol.EqualTo("oracle")) {
        B2ERROR("Exception in "  << __FILE__  << "(" << __FUNCTION__ << ") protocol in db argument should be mysql, pgsql or oracle");
        return 0;
      }
      char dbstring[1024] = "";
      if (url->GetPort())
        sprintf(dbstring, "%s://%s:%d%s", url->GetProtocol(), url->GetHost(), url->GetPort(), url->GetFile());
      else
        sprintf(dbstring, "%s://%s%s", url->GetProtocol(), url->GetHost(), url->GetFile());

      B2INFO("dbstring " << dbstring);
      try {
        m_database  = TSQLServer::Connect(dbstring , url->GetUser() , url->GetPasswd());
        if (!m_database)   B2FATAL("Cannot connect to Sql DB " << dbstring);
        std::string query = GetQuery(m_database , url);
        B2INFO("DB Query " << query);
        m_statement = m_database->Statement(query.c_str(), 1);
      } catch (std::exception& e) {
        B2ERROR("SqlException in " << __FILE__ << "(" << __FUNCTION__ << ") on line "    << __LINE__);
        B2ERROR("code " << e.what());
      }

      delete url;

      if (!m_statement) return 0;
      /*
          StoreObjPtr<EventMetaData> eventMetaDataPtr;
          int exp = eventMetaDataPtr->getExperiment();
          int run = eventMetaDataPtr->getRun();
      */


      B2INFO(m_uri << "SqlHandler::open uri=" << path);
      return new SqlContext(m_statement);
    }

    SqlHandler::~SqlHandler()
    {
      if (m_statement) delete m_statement;
    }

    B2_GEARBOX_REGISTER_INPUTHANDLER(SqlHandler, "sql");
  }
}

