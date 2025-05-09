#include "litesql/backend.hpp"
#include "sqlite3.hpp"

using namespace litesql;

extern "C" {
__declspec(dllexport)  Backend* createBackend(const std::string& parameter)
{
  Backend* pBackend = nullptr;
  try {
    pBackend = new SQLite3(parameter);
  } catch(const DatabaseError& /*ex*/) {
    pBackend = nullptr;
  }
  return pBackend;
}

__declspec(dllexport) void deleteBackend(Backend* backend)
{
  if (backend)
  {
    delete backend;
  }
}
}
