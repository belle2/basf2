#include <display/VisualRepMap.h>

using namespace Belle2;

const TObject* VisualRepMap::getDataStoreObject(TEveElement* elem) const
{
  const auto& it = m_dataStoreEveElementMap.right.find(elem);
  if (it != m_dataStoreEveElementMap.right.end())
    return it->second;
  return nullptr;
}

TEveElement* VisualRepMap::getEveElement(const TObject* obj) const
{
  const auto& it = m_dataStoreEveElementMap.left.find(obj);
  if (it != m_dataStoreEveElementMap.left.end())
    return it->second;
  return nullptr;
}
