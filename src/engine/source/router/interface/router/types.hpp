#ifndef _ROUTER_TYPES_HPP
#define _ROUTER_TYPES_HPP

#include <functional>
#include <list>
#include <optional>
#include <string>

#include <baseTypes.hpp>
#include <error.hpp>
#include <logging/logging.hpp>
#include <name.hpp>

namespace router
{

namespace env
{
enum class State : std::uint8_t
{
    UNKNOWN,  ///< Unset state
    DISABLED, ///< Environment is inactive, it's not being used or error
    ENABLED,  ///< Environment is active, it's being used
};

enum class Sync : std::uint8_t
{
    UNKNOWN,  ///< Unset sync status
    UPDATED,  ///< Policy is updated
    OUTDATED, ///< Policy is outdated respect to the store
    DELETED,  ///< Policy is deleted not exist in the store
    ERROR     ///< Error, can't get the policy status
};

} // namespace env

/**************************************************************************
 *                      Production types (router)                         *
 *************************************************************************/
namespace prod
{
/**
 * @brief Request for adding a new entry in production
 */
class EntryPost
{
private:
    std::string m_name;                       ///< Name of the environment
    base::Name m_policy;                      ///< Policy of the environment
    base::Name m_filter;                      ///< Filter of the environment
    std::size_t m_priority;                   ///< Priority of the environment
    std::optional<std::string> m_description; ///< Description of the environment

public:
    EntryPost() = delete;

    EntryPost(std::string name, base::Name policy, base::Name filter, std::size_t priority)
        : m_name {std::move(name)}
        , m_policy {std::move(policy)}
        , m_description {}
        , m_filter {std::move(filter)}
        , m_priority {priority}
    {
    }

    base::OptError validate() const
    {
        if (m_name.empty())
        {
            return base::Error {"Name cannot be empty"};
        }
        if (m_policy.parts().size() == 0)
        {
            return base::Error {"Policy name is empty"};
        }
        if (m_filter.parts().size() == 0)
        {
            return base::Error {"Filter name is empty"};
        }
        if (m_priority == 0)
        {
            return base::Error {"Priority cannot be 0"};
        }
        return base::OptError {};
    }

    // Setters and getters
    const std::string& name() const { return m_name; }
    const base::Name& policy() const { return m_policy; }

    const std::optional<std::string>& description() const { return m_description; }
    void description(std::string_view description) { m_description = description; }

    const base::Name& filter() const { return m_filter; }
    void filter(base::Name filter) { m_filter = filter; }

    std::size_t priority() const { return m_priority; }
    void priority(std::size_t priority) { m_priority = priority; }
};

/**
 * @brief Response for get an entry in production
 */
class Entry : public EntryPost
{
private:
    env::Sync m_policySync;     ///< Policy sync status
    env::State m_status;        ///< Status of the environment
    std::uint64_t m_lastUpdate; /// Last update of the environment [TODO: Review this metadata]

public:
    Entry(const EntryPost& entryPost)
        : EntryPost {entryPost}
        , m_lastUpdate {0}
        , m_policySync {env::Sync::UNKNOWN}
        , m_status {env::State::UNKNOWN} {};

    // Setters and getters
    std::uint64_t lastUpdate() const { return m_lastUpdate; }
    void lastUpdate(std::uint64_t lastUpdate) { m_lastUpdate = lastUpdate; }

    env::Sync policySync() const { return m_policySync; }
    void policySync(env::Sync policySync) { m_policySync = policySync; }

    env::State status() const { return m_status; }
    void status(env::State status) { m_status = status; }
};

} // namespace prod

/**************************************************************************
 *                      Testing types (tester)                            *
 *************************************************************************/
namespace test
{

/**
 * @brief Request for adding a new entry in production
 */
class EntryPost
{
private:
    std::string m_name;                       ///< Name of the environment
    base::Name m_policy;                      ///< Policy of the environment
    std::optional<std::string> m_description; ///< Description of the environment
    std::size_t m_lifetime;                   ///< Lifetime of the environment

public:
    EntryPost() = delete;

    EntryPost(std::string name, base::Name policy)
        : m_name {std::move(name)}
        , m_policy {std::move(policy)}
        , m_description {}
    {
    }

    base::OptError validate() const
    {
        if (m_name.empty())
        {
            return base::Error {"Name cannot be empty"};
        }
        if (m_policy.parts().size() == 0)
        {
            return base::Error {"Policy name is empty"};
        }
        return base::OptError {};
    }

    // Setters and getters
    const std::string& name() const { return m_name; }
    const base::Name& policy() const { return m_policy; }

    const std::optional<std::string>& description() const { return m_description; }
    void description(std::string_view description) { m_description = description; }

    std::size_t lifetime() const { return m_lifetime; }
    void lifetime(std::size_t lifetime) { m_lifetime = lifetime; }
};

/**
 * @brief Response for get an entry in production
 */
class Entry : public EntryPost
{
private:
    env::Sync m_policySync;  ///< Policy sync status
    env::State m_status;     ///< Status of the environment
    std::uint64_t m_lastUse; /// Last use of the entry.

public:
    Entry(const EntryPost& entryPost)
        : EntryPost {entryPost}
        , m_lastUse {0}
        , m_policySync {env::Sync::UNKNOWN}
        , m_status {env::State::UNKNOWN} {};

    // Setters and getters
    std::uint64_t lastUpdate() const { return m_lastUse; }
    void lastUpdate(std::uint64_t lastUpdate) { m_lastUse = lastUpdate; }

    env::Sync policySync() const { return m_policySync; }
    void policySync(env::Sync policySync) { m_policySync = policySync; }

    env::State status() const { return m_status; }
    void status(env::State status) { m_status = status; }
};


/**
 * @brief Options for request a testing event
 */
class Opt
{
public:
    /**
     * @brief Tracin level for testing
     */
    enum class TraceLevel : std::uint8_t
    {
        NONE = 0,
        ASSET_ONLY,
        ALL
    };

private:
    TraceLevel m_traceLevel;
    std::vector<std::string> m_assets;
    std::string m_environmetName;

    // Missing namespace and asset list
public:
    Opt(TraceLevel traceLevel, const decltype(m_assets)& assets, const std::string& envName)
        : m_traceLevel {traceLevel}
        , m_assets {assets}
        , m_environmetName {envName}
    {
        // validate();
    }

    const std::string& environmentName() const { return m_environmetName; }
    auto assets() const -> const decltype(m_assets)& { return m_assets; }
    TraceLevel traceLevel() const { return m_traceLevel; }
};

/**
 * @brief Represent a output of a testing event
 */
class Output
{
public:
    struct AssetTrace ///< Represent a trace of an asset
    {
        bool success = false;            ///< True if the asset has success
        std::vector<std::string> traces; ///< List of traces of the asset (if any)
    };
    using DataPair = std::pair<std::string, AssetTrace>; ///< Pair of asset name and tracing data

protected:
    base::Event m_event;          ///< Result event of the testing
    std::list<DataPair> m_traces; ///< List of traces of the testing

public:
    Output()
        : m_event {}
        , m_traces {}
    {
    }

    base::Event& event() { return m_event; }
    const base::Event& event() const { return m_event; }

    std::list<DataPair>& traceList() { return m_traces; }
    const std::list<DataPair>& traceList() const { return m_traces; }

    bool isValid() const
    {
        if (m_event == nullptr)
        {
            return false;
        }
        return true;
    }
};

using TestingTuple = std::tuple<base::Event, Opt, std::function<void(base::RespOrError<Output>&&)>>;
using QueueType = std::shared_ptr<TestingTuple>;

} // namespace test

} // namespace router

#endif // _ROUTER_TYPES_HPP