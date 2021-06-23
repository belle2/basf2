/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <display/HtmlClassInspector.h>

#include <framework/utilities/HTML.h>

#include <TClass.h>
#include <TClassRef.h>
#include <TDataType.h>
#include <TDataMember.h>
#include <TDatime.h>
#include <TROOT.h>
#include <TStreamerElement.h>
#include <TVirtualStreamerInfo.h>

#include <string>

using namespace Belle2;

TString HtmlClassInspector::getMemberData(const TObject* obj)
{
  HtmlClassInspector dm;
  const_cast<TObject*>(obj)->ShowMembers(dm);
  return dm.getTable();
}

TString HtmlClassInspector::getClassInfo(const TClass* cl)
{
  if (!cl)
    return "";
  TString info;
  info += "<b>";
  info += HTML::htmlToPlainText(cl->GetName());
  info += "</b> (";

  TString title(HTML::htmlToPlainText(stripTitle(cl->GetTitle()).Data()));
  return info + title + ")<br> <br>";
}

TString HtmlClassInspector::stripTitle(TString title)
{
  title = title.Strip(TString::kBoth, '/');
  title = title.Strip(TString::kBoth, '*');
  title = title.Strip(TString::kLeading, '!');
  title = title.Strip(TString::kLeading, '<');
  title = title.Strip(TString::kBoth, ' ');
  return title;
}

TString HtmlClassInspector::getTable() const
{
  TString tmp;
  tmp += "<table width=100% cellpadding=2 bgcolor=eeeeee>";
  tmp += m_info;
  tmp += "</table>";
  return tmp;
}
void HtmlClassInspector::Inspect(TClass* cl, const char* pname, const char* mname, const void* add)
{
  // Print value of member mname.
  //
  // This method is called by the ShowMembers() method for each
  // data member when object.Dump() is invoked.
  //
  //    cl    is the pointer to the current class
  //    pname is the parent name (in case of composed objects)
  //    mname is the data member name
  //    add   is the data member address

  const Int_t kvalue = 30;
  const Int_t kline  = 1024;
  Int_t cdate = 0;
  Int_t ctime = 0;
  UInt_t* cdatime;
  char line[kline];

  TDataType* membertype;
  TString memberTypeName;
  TString memberName;
  const char* memberFullTypeName;
  TString memberTitle;
  Bool_t isapointer;
  Bool_t isbasic;

  if (TDataMember* member = cl->GetDataMember(mname)) {
    memberTypeName = member->GetTypeName();
    memberName = member->GetName();
    memberFullTypeName = member->GetFullTypeName();
    memberTitle = member->GetTitle();
    isapointer = member->IsaPointer();
    isbasic = member->IsBasic();
    membertype = member->GetDataType();
  } else if (!cl->IsLoaded()) {
    // The class is not loaded, hence it is 'emulated' and the main source of
    // information is the StreamerInfo.
    TVirtualStreamerInfo* info = cl->GetStreamerInfo();
    if (!info) return;
    const char* cursor = mname;
    while ((*cursor) == '*') ++cursor;
    TString elname(cursor);
    Ssiz_t pos = elname.Index("[");
    if (pos != kNPOS) {
      elname.Remove(pos);
    }
    TStreamerElement* element = (TStreamerElement*)info->GetElements()->FindObject(elname.Data());
    if (!element) return;
    memberFullTypeName = element->GetTypeName();

    memberTypeName = memberFullTypeName;
    memberTypeName = memberTypeName.Strip(TString::kTrailing, '*');
    if (memberTypeName.Index("const ") == 0) memberTypeName.Remove(0, 6);

    memberName = element->GetName();
    memberTitle = element->GetTitle();
    isapointer = element->IsaPointer() || element->GetType() == TVirtualStreamerInfo::kCharStar;
    membertype = gROOT->GetType(memberFullTypeName);

    isbasic = membertype != 0;
  } else {
    return;
  }


  Bool_t isdate = kFALSE;
  if (strcmp(memberName, "fDatime") == 0 && strcmp(memberTypeName, "UInt_t") == 0) {
    isdate = kTRUE;
  }
  Bool_t isbits = kFALSE;
  if (strcmp(memberName, "fBits") == 0 && strcmp(memberTypeName, "UInt_t") == 0) {
    isbits = kTRUE;
  }
  TClass* dataClass = TClass::GetClass(memberFullTypeName);
  Bool_t isTString = (dataClass == TString::Class());
  static TClassRef stdClass("std::string");
  Bool_t isStdString = (dataClass == stdClass);

  Int_t i;
  for (i = 0; i < kline; i++) line[i] = ' ';
  line[kline - 1] = 0;
  //snprintf(line,kline,"%s%s ",pname,mname);
  //i = strlen(line); line[i] = ' ';

  // Encode data value or pointer value
  char* pointer = (char*)add;
  char** ppointer = (char**)(pointer);

  if (isapointer) {
    char** p3pointer = (char**)(*ppointer);
    if (!p3pointer)
      snprintf(&line[kvalue], kline - kvalue, "->0");
    else if (!isbasic)
      snprintf(&line[kvalue], kline - kvalue, "->%lx ", (Long_t)p3pointer);
    else if (membertype) {
      if (!strcmp(membertype->GetTypeName(), "char")) {
        i = strlen(*ppointer);
        if (kvalue + i > kline) i = kline - 1 - kvalue;
        Bool_t isPrintable = kTRUE;
        for (Int_t j = 0; j < i; j++) {
          if (!std::isprint((*ppointer)[j])) {
            isPrintable = kFALSE;
            break;
          }
        }
        if (isPrintable) {
          //strncpy(line + kvalue, *ppointer, i);
          std::string out(*ppointer);
          out.copy(line + kvalue, i);
          line[kvalue + i] = 0;
        } else {
          line[kvalue] = 0;
        }
      } else {
        strncpy(&line[kvalue], membertype->AsString(p3pointer), TMath::Min(kline - 1 - kvalue,
                (int)strlen(membertype->AsString(p3pointer))));
      }
    } else if (!strcmp(memberFullTypeName, "char*") ||
               !strcmp(memberFullTypeName, "const char*")) {
      i = strlen(*ppointer);
      if (kvalue + i >= kline) i = kline - 1 - kvalue;
      Bool_t isPrintable = kTRUE;
      for (Int_t j = 0; j < i; j++) {
        if (!std::isprint((*ppointer)[j])) {
          isPrintable = kFALSE;
          break;
        }
      }
      if (isPrintable) {
        std::string out(*ppointer);
        out.copy(line + kvalue, i);
        //strncpy(line + kvalue, *ppointer, i); //

        line[kvalue + i] = 0;
      } else {
        line[kvalue] = 0;
      }
    } else {
      snprintf(&line[kvalue], kline - kvalue, "->%lx ", (Long_t)p3pointer);
    }
  } else if (membertype) {
    if (isdate) {
      cdatime = (UInt_t*)pointer;
      TDatime::GetDateTime(cdatime[0], cdate, ctime);
      snprintf(&line[kvalue], kline - kvalue, "%d/%d", cdate, ctime);
    } else if (isbits) {
      snprintf(&line[kvalue], kline - kvalue, "0x%08x", *(UInt_t*)pointer);
    } else {
      strncpy(&line[kvalue], membertype->AsString(pointer), TMath::Min(kline - 1 - kvalue, (int)strlen(membertype->AsString(pointer))));
    }
  } else {
    if (isStdString) {
      std::string* str = (std::string*)pointer;
      snprintf(&line[kvalue], kline - kvalue, "%s", str->c_str());
    } else if (isTString) {
      TString* str = (TString*)pointer;
      snprintf(&line[kvalue], kline - kvalue, "%s", str->Data());
    } else {
      snprintf(&line[kvalue], kline - kvalue, "->%lx ", (Long_t)pointer);
    }
  }


  m_info += "<tr>";
  TString indent;
  if (TString(pname) != "") //indent nested members
    indent = "&nbsp;&nbsp;&nbsp;";
  m_info += "<td><b>" + indent + HTML::htmlToPlainText(memberName.Data()) + "</b><br>";
  m_info += indent + "<small>" + HTML::htmlToPlainText(stripTitle(memberTitle).Data()) + "</small>";
  m_info += "</td>";

  TString memberValue(HTML::htmlToPlainText(line));
  m_info += "<td align=right>" + memberValue + "</td>";
  m_info += "</tr>";
}
