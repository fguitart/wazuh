#ifndef _KVDBSPACE_H
#define _KVDBSPACE_H

#include <kvdb2/iKVDBHandler.hpp>
#include <kvdb2/kvdbManagedHandler.hpp>

namespace kvdbManager
{

class IKVDBHandlerManager;

class KVDBSpace : public IKVDBHandler, public KVDBManagedHandler
{
public:
    KVDBSpace(IKVDBHandlerManager* manager, const std::string& spaceName, const std::string& scopeName) : KVDBManagedHandler(manager, scopeName), m_spaceName(spaceName) {}
    ~KVDBSpace();
    std::variant<bool, base::Error> set(const std::string& key, const std::string& value) override;
    bool add(const std::string& key) override;
    bool remove(const std::string& key) override;
    std::variant<bool, base::Error> contains(const std::string& key) override;
    std::variant<std::string, base::Error> get(const std::string& key) override;
protected:
    std::string m_spaceName;
};

} // namespace kvdbManager

#endif // _KVDBSPACE_H
