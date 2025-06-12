#include "litesql/backend.hpp"
#include "sqlite3.hpp"

using namespace litesql;

extern "C" {
#ifdef WIN32
__declspec(dllexport)
#endif
  Backend* createBackend(const std::string& parameter)
{
  Backend* pBackend = nullptr;
  try {
    pBackend = new SQLite3(parameter);
  } catch(const DatabaseError& /*ex*/) {
    pBackend = nullptr;
  }
  return pBackend;
}
#ifdef WIN32
__declspec(dllexport)
#endif
 void deleteBackend(Backend* backend)
{
  if (backend)
  {
    delete backend;
  }
}
}
