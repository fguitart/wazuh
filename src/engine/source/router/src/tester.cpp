#include "tester.hpp"

namespace
{
/**
 * @brief Return the current time in seconds since epoch
 */
int64_t getStartTime()
{
    auto startTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(startTime.time_since_epoch()).count();
}

} // namespace

namespace router
{

namespace test
{
class InternalOutput : public Output
{
private:
    std::unordered_map<std::string, std::list<DataPair>::iterator> m_dataMap;

public:
    InternalOutput()
        : Output()
        , m_dataMap()
    {
    }

    void addTrace(const std::string& asset, const std::string& traceContent, bool result)
    {
        if (traceContent.empty())
        {
            return;
        }

        // Try inserting the asset into the map.
        auto [it, inserted] = m_dataMap.try_emplace(asset, m_traces.end());

        // If is new, insert it into the list.
        if (inserted)
        {
            m_traces.emplace_back(asset, Output::AssetTrace {});
            it->second = std::prev(m_traces.end());
        }

        auto& data = it->second->second;

        if (traceContent == "SUCCESS")
        {
            data.success = true;
        }
        else
        {
            data.traces.push_back(traceContent);
        }
    }
};
} // namespace test

base::OptError Tester::addEntry(const test::EntryPost& entryPost)
{
    auto entry = RuntimeEntry(entryPost);
    try
    {
        entry.setController(m_envBuilder->makeController(entryPost.policy()));
        entry.status(env::State::DISABLED); // It is disabled until all tester are ready
        entry.lifetime(entry.lifetime());
    }
    catch (const std::exception& e)
    {
        return base::Error {fmt::format("Failed to create the testing environment: {}", e.what())};
    }

    // Add the entry to the table
    {
        std::unique_lock<std::shared_mutex> lock {m_mutex};
        if (m_table.find(entryPost.name()) != m_table.end())
        {
            return base::Error {"The name of the testing environment already exist"};
        }

        m_table.insert({entryPost.name(), std::move(entry)});
    }

    return std::nullopt;
}

base::OptError Tester::removeEntry(const std::string& name)
{
    std::unique_lock lock {m_mutex};
    if (m_table.find(name) == m_table.end())
    {
        return base::Error {"The testing environment not exist"};
    }
    m_table.erase(name);
    return std::nullopt;
}

base::OptError Tester::rebuildEntry(const std::string& name)
{
    std::unique_lock lock {m_mutex};
    if (m_table.find(name) == m_table.end())
    {
        return base::Error {"The testing environment not exist"};
    }
    auto& entry = m_table.at(name);
    try
    {
        auto controller = m_envBuilder->makeController(entry.policy());
        entry.setController(std::move(controller));
    }
    catch (const std::exception& e)
    {
        return base::Error {fmt::format("Failed to create the testing environment: {}", e.what())};
    }
    return std::nullopt;
}

base::OptError Tester::enableEntry(const std::string& name)
{
    std::unique_lock lock {m_mutex};
    if (m_table.find(name) == m_table.end())
    {
        return base::Error {"The testing environment not exist"};
    }
    auto& entry = m_table.at(name);
    if (entry.controller() == nullptr)
    {
        return base::Error {"The testing environment is not builded"};
    }
    entry.status(env::State::ENABLED);
    return std::nullopt;
}

std::list<test::Entry> Tester::getEntries() const
{
    std::shared_lock lock {m_mutex};
    std::list<test::Entry> entries;
    for (const auto& [name, entry] : m_table)
    {
        entries.push_back(entry);
    }
    return entries;
}

base::RespOrError<test::Entry> Tester::getEntry(const std::string& name) const
{
    std::shared_lock lock {m_mutex};
    if (m_table.find(name) == m_table.end())
    {
        return base::Error {"The testing environment not exist"};
    }
    return m_table.at(name);
}

// Testing
base::RespOrError<test::Output> Tester::ingestTest(base::Event&& event, const test::Opt& opt)
{
    std::shared_lock lock {m_mutex};

    if (m_table.find(opt.environmentName()) == m_table.end())
    {
        return base::Error {"The testing environment not exist"};
    }
    auto& entry = m_table.at(opt.environmentName());

    if (entry.status() != env::State::ENABLED)
    {
        return base::Error {"The testing environment is not enabled"};
    }

    // Configure the environment
    auto result = std::make_shared<test::InternalOutput>();
    for (const auto& asset : opt.assets())
    {

        auto err = entry.controller()->subscribe(asset,
                                                 [asset, result](const std::string& trace, bool success) -> void
                                                 { result->addTrace(asset, trace, success); });
        if (base::isError(err))
        {
            entry.controller()->unsubscribeAll();
            return base::getError(err);
        }
    }

    // Run the test
    result->event() = entry.controller()->ingestGet(std::move(event));

    // Reset controller
    entry.controller()->unsubscribeAll();

    return *result;
}
} // namespace router