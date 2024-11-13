#include <svd/modules/svdPerformance/SVDValidationModule.h>
#include <svd/persistenceManager/PersistenceManagerFactory.h>

using namespace Belle2;

SVDValidationModule::SVDValidationModule() : Module()
{
  addParam("storageType", m_storageType, "Type of storage to store the variables in.", std::string());
  persistenceManager = PersistenceManagerFactory::create(m_storageType);
}

void SVDValidationModule::initialize()
{
  persistenceManager->initialize();
}

void SVDValidationModule::event()
{
  persistenceManager->addEntry();
}

void SVDValidationModule::terminate()
{
  persistenceManager->store();
}