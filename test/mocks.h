#pragma once

#include <chrono>
#include <memory>

#include "envoy/api/api.h"
#include "envoy/common/time.h"
#include "envoy/event/dispatcher.h"
#include "envoy/stats/store.h"
#include "envoy/upstream/cluster_manager.h"

#include "nighthawk/client/benchmark_client.h"
#include "nighthawk/client/factories.h"
#include "nighthawk/client/options.h"
#include "nighthawk/common/header_source.h"
#include "nighthawk/common/platform_util.h"
#include "nighthawk/common/rate_limiter.h"
#include "nighthawk/common/sequencer.h"
#include "nighthawk/common/statistic.h"
#include "nighthawk/common/termination_predicate.h"
#include "nighthawk/common/uri.h"

#include "external/envoy/test/test_common/simulated_time_system.h"

#include "common/utility.h"

#include "gmock/gmock.h"

using namespace std::chrono_literals;

constexpr std::chrono::milliseconds TimeResolution = 1ms;

namespace Nighthawk {

// TODO(oschaaf): split this out in files for common/ and client/ mocks

class MockPlatformUtil : public PlatformUtil {
public:
  MockPlatformUtil();

  MOCK_CONST_METHOD0(yieldCurrentThread, void());
  MOCK_CONST_METHOD1(sleep, void(std::chrono::microseconds));
};

class MockRateLimiter : public RateLimiter {
public:
  MockRateLimiter();

  MOCK_METHOD0(tryAcquireOne, bool());
  MOCK_METHOD0(releaseOne, void());
};

class MockSequencer : public Sequencer {
public:
  MockSequencer();

  MOCK_METHOD0(start, void());
  MOCK_METHOD0(waitForCompletion, void());
  MOCK_CONST_METHOD0(completionsPerSecond, double());
  MOCK_CONST_METHOD0(executionDuration, std::chrono::nanoseconds());
  MOCK_CONST_METHOD0(statistics, StatisticPtrMap());
  MOCK_METHOD0(cancel, void());
};

class MockOptions : public Client::Options {
public:
  MockOptions();

  MOCK_CONST_METHOD0(requestsPerSecond, uint32_t());
  MOCK_CONST_METHOD0(connections, uint32_t());
  MOCK_CONST_METHOD0(duration, std::chrono::seconds());
  MOCK_CONST_METHOD0(timeout, std::chrono::seconds());
  MOCK_CONST_METHOD0(uri, std::string());
  MOCK_CONST_METHOD0(h2, bool());
  MOCK_CONST_METHOD0(concurrency, std::string());
  MOCK_CONST_METHOD0(verbosity, nighthawk::client::Verbosity::VerbosityOptions());
  MOCK_CONST_METHOD0(outputFormat, nighthawk::client::OutputFormat::OutputFormatOptions());
  MOCK_CONST_METHOD0(prefetchConnections, bool());
  MOCK_CONST_METHOD0(burstSize, uint32_t());
  MOCK_CONST_METHOD0(addressFamily, nighthawk::client::AddressFamily::AddressFamilyOptions());
  MOCK_CONST_METHOD0(requestMethod, envoy::api::v2::core::RequestMethod());
  MOCK_CONST_METHOD0(requestHeaders, std::vector<std::string>());
  MOCK_CONST_METHOD0(requestBodySize, uint32_t());
  MOCK_CONST_METHOD0(tlsContext, envoy::api::v2::auth::UpstreamTlsContext&());
  MOCK_CONST_METHOD0(maxPendingRequests, uint32_t());
  MOCK_CONST_METHOD0(maxActiveRequests, uint32_t());
  MOCK_CONST_METHOD0(maxRequestsPerConnection, uint32_t());
  MOCK_CONST_METHOD0(toCommandLineOptions, Client::CommandLineOptionsPtr());
  MOCK_CONST_METHOD0(sequencerIdleStrategy,
                     nighthawk::client::SequencerIdleStrategy::SequencerIdleStrategyOptions());
  MOCK_CONST_METHOD0(trace, std::string());
  MOCK_CONST_METHOD0(terminationPredicates, Client::TerminationPredicateMap());
  MOCK_CONST_METHOD0(failurePredicates, Client::TerminationPredicateMap());
  MOCK_CONST_METHOD0(openLoop, bool());
};

class MockBenchmarkClientFactory : public Client::BenchmarkClientFactory {
public:
  MockBenchmarkClientFactory();
  MOCK_CONST_METHOD7(create,
                     Client::BenchmarkClientPtr(Envoy::Api::Api&, Envoy::Event::Dispatcher&,
                                                Envoy::Stats::Scope&,
                                                Envoy::Upstream::ClusterManagerPtr&,
                                                Envoy::Tracing::HttpTracerPtr&, absl::string_view,
                                                HeaderSource& header_generator));
};

class MockSequencerFactory : public Client::SequencerFactory {
public:
  MockSequencerFactory();
  MOCK_CONST_METHOD6(create, SequencerPtr(Envoy::TimeSource& time_source,
                                          Envoy::Event::Dispatcher& dispatcher,
                                          Envoy::MonotonicTime start_time,
                                          Client::BenchmarkClient& benchmark_client,
                                          TerminationPredicate& termination_predicate,
                                          Envoy::Stats::Scope& scope));
};

class MockStoreFactory : public Client::StoreFactory {
public:
  MockStoreFactory();
  MOCK_CONST_METHOD0(create, Envoy::Stats::StorePtr());
};

class MockStatisticFactory : public Client::StatisticFactory {
public:
  MockStatisticFactory();
  MOCK_CONST_METHOD0(create, StatisticPtr());
};

class MockHeaderSourceFactory : public HeaderSourceFactory {
public:
  MockHeaderSourceFactory();
  MOCK_CONST_METHOD0(create, HeaderSourcePtr());
};

class MockTerminationPredicateFactory : public TerminationPredicateFactory {
public:
  MockTerminationPredicateFactory();
  MOCK_CONST_METHOD3(create, TerminationPredicatePtr(Envoy::TimeSource& time_source,
                                                     Envoy::Stats::Scope& scope,
                                                     const Envoy::MonotonicTime start));
};

class FakeSequencerTarget {
public:
  virtual ~FakeSequencerTarget() = default;
  // A fake method that matches the sequencer target signature.
  virtual bool callback(OperationCallback) PURE;
};

class MockSequencerTarget : public FakeSequencerTarget {
public:
  MockSequencerTarget();
  MOCK_METHOD1(callback, bool(OperationCallback));
};

class MockBenchmarkClient : public Client::BenchmarkClient {
public:
  MockBenchmarkClient();

  MOCK_METHOD0(terminate, void());
  MOCK_METHOD1(setMeasureLatencies, void(bool));
  MOCK_CONST_METHOD0(statistics, StatisticPtrMap());
  MOCK_METHOD1(tryStartRequest, bool(Client::CompletionCallback));
  MOCK_CONST_METHOD0(scope, Envoy::Stats::Scope&());
  MOCK_CONST_METHOD0(measureLatencies, bool());
  MOCK_CONST_METHOD0(requestHeaders, const Envoy::Http::HeaderMap&());
};

class MockHeaderSource : public HeaderSource {
public:
  MockHeaderSource();
  MOCK_METHOD0(get, HeaderGenerator());
};

class MockTerminationPredicate : public TerminationPredicate {
public:
  MockTerminationPredicate();
  MOCK_METHOD1(link, TerminationPredicate&(TerminationPredicatePtr&&));
  MOCK_METHOD0(evaluateChain, TerminationPredicate::Status());
  MOCK_METHOD0(evaluate, TerminationPredicate::Status());
};

class MockDiscreteNumericDistributionSampler : public DiscreteNumericDistributionSampler {
public:
  MockDiscreteNumericDistributionSampler();
  MOCK_METHOD0(getValue, uint64_t());
};

} // namespace Nighthawk
