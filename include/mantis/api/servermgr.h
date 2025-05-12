#ifndef MANTIS_SERVER_H
#define MANTIS_SERVER_H

#include <string>
#include <memory>

namespace Mantis {

class MantisApp;

class ServerMgr {
  public:
    explicit ServerMgr(const MantisApp& app);
    ~ServerMgr() = default;

    bool GenerateCrudApis();

private:
      std::shared_ptr<MantisApp> m_app;

    /// Rules & Schema Cache
};
}

#endif // MANTIS_SERVER_H