#include <benchmark/benchmark.h>
#include "exporters/trace/gcp_exporter/gcp_exporter.h"
#include "opentelemetry/nostd/span.h"

namespace cloudtrace_v2 = google::devtools::cloudtrace::v2;

// These constants affect the overall runtime of the benchmark tests
constexpr int kNumSpans = 100;
constexpr int kNumIntAttributes = 30;
constexpr int kNumStrAttributes = 30;
constexpr int kNumBoolAttributes = 30;
constexpr int kNumIterations = 1000;


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter 
{
namespace gcp 
{

/* ################################## CUSTOM MOCK STUB ######################################## */

/**
 * This class implements a mock functionality of the StubInterface for benchmarking purposes
 */
class MockStub final : public cloudtrace_v2::TraceService::StubInterface 
{
public:
  grpc::Status BatchWriteSpans(grpc::ClientContext* context,
                               const cloudtrace_v2::BatchWriteSpansRequest& request, 
                               google::protobuf::Empty* response)
  {
    (void) context;
    (void) request;
    (void) response;
    return grpc::Status::OK;
  }

  grpc::Status CreateSpan(grpc::ClientContext* context, 
                          const cloudtrace_v2::Span& request, 
                          cloudtrace_v2::Span* response)
  {
    (void) context;
    (void) request;
    (void) response;
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "");
  }

private:
  grpc::ClientAsyncResponseReaderInterface<google::protobuf::Empty>* AsyncBatchWriteSpansRaw(grpc::ClientContext* context, 
                                                                                             const cloudtrace_v2::BatchWriteSpansRequest& request, 
                                                                                             grpc::CompletionQueue* cq)
  {
    (void) context;
    (void) request;
    (void) cq;
    return nullptr;
  }

  grpc::ClientAsyncResponseReaderInterface<google::protobuf::Empty>* PrepareAsyncBatchWriteSpansRaw(grpc::ClientContext* context, 
                                                                                                    const cloudtrace_v2::BatchWriteSpansRequest& request, 
                                                                                                    grpc::CompletionQueue* cq)
  {
    (void) context;
    (void) request;
    (void) cq;
    return nullptr;
  }

  grpc::ClientAsyncResponseReaderInterface<cloudtrace_v2::Span>* AsyncCreateSpanRaw(grpc::ClientContext* context, 
                                                                                    const cloudtrace_v2::Span& request, 
                                                                                    grpc::CompletionQueue* cq)
  {
    (void) context;
    (void) request;
    (void) cq;
    return nullptr;
  }

  grpc::ClientAsyncResponseReaderInterface<cloudtrace_v2::Span>* PrepareAsyncCreateSpanRaw(grpc::ClientContext* context, 
                                                                                           const cloudtrace_v2::Span& request, 
                                                                                           grpc::CompletionQueue* cq)
  {
    (void) context;
    (void) request;
    (void) cq;
    return nullptr;
  };

};


/* ################################# FIXTURE CLASS ####################################### */

/**
 * Fixture class which handles all the benchmark-specific helper methods, variables and the
 * setup functionality 
 */
class GcpExporterBenchmark : public benchmark::Fixture 
{
public:
  GcpExporterBenchmark(){
    setenv(kGCPEnvVar, "test_project", 1);
  }

  /**
   * Generates a mock stub and returns an exporter registered on that mock stub
   */
  std::unique_ptr<GcpExporter> GetMockExporter() 
  {
    auto mock_stub = new MockStub();
    return std::unique_ptr<GcpExporter>(new GcpExporter(std::unique_ptr<cloudtrace_v2::TraceService::StubInterface>(mock_stub),
                                        "test_project"));
  }

  /**
   * Generates a batch of lightweight spans
   * 
   * @param sparse_spans - The array to populate with the generated sparse spans 
   */
  void GenerateSparseSpans(std::array<std::unique_ptr<sdk::trace::Recordable>, kNumSpans>& sparse_spans){
    for(int i = 0; i < kNumSpans; ++i){
      // Make the span
      auto rec = std::unique_ptr<sdk::trace::Recordable>(new Recordable);

      rec->SetName("Test Span");

      rec->SetIds(trace_id_, span_id_, parent_span_id_);

      // Set start time (making it 0 in this case)
      rec->SetStartTime(start_timestamp);

      rec->SetDuration(std::chrono::nanoseconds(100));

      // Add to array
      sparse_spans[i] = std::move(rec);
    }
  }


  /**
   * Generates a batch of heavyweight, dense spans
   * 
   * @param dense_spans - The array to populate with dense spans
   */
  void GenerateDenseSpans(std::array<std::unique_ptr<sdk::trace::Recordable>, kNumSpans>& dense_spans){
    for(int i = 0; i < kNumSpans; ++i){
      // Make the span
      auto rec = std::unique_ptr<sdk::trace::Recordable>(new Recordable);

      rec->SetName("Test Span");

      rec->SetIds(trace_id_, span_id_, parent_span_id_);

      // Set start time (making it 0 in this case)
      rec->SetStartTime(start_timestamp);

      rec->SetDuration(std::chrono::nanoseconds(100));

      // Add several 'integer' attributes 
      for(int i = 0 ; i < kNumIntAttributes; ++i){
        rec->SetAttribute("int_key_" + i, static_cast<int64_t>(i));
      }

      // Add several 'string' attributes 
      for(int i = 0 ; i < kNumStrAttributes; ++i){
        rec->SetAttribute("str_key_" + i, "string_val_" + i);
      }

      // Add several 'bool' attributes 
      for(int i = 0 ; i < kNumBoolAttributes; ++i){
        // Setting all to true
        rec->SetAttribute("bool_key_" + i, true);
      }
      
      // Add to array
      dense_spans[i] = std::move(rec);
    }
  }

private:
  const trace::TraceId trace_id_ = trace::TraceId(
  std::array<const uint8_t, trace::TraceId::kSize>(
  {0, 1, 0, 2, 1, 3, 1, 4, 1, 5, 1, 6, 3, 7, 0, 0}));

  const trace::SpanId span_id_ = trace::SpanId(
  std::array<const uint8_t, trace::SpanId::kSize>(
  {1, 2, 3, 4, 5, 6, 7, 8}));

  const trace::SpanId parent_span_id_ = trace::SpanId(
  std::array<const uint8_t, trace::SpanId::kSize>(
  {4, 5, 0, 1, 1, 1, 1, 3}));
  
  const core::SystemTimestamp start_timestamp;
};

/* ################################## BENCHMARKS ######################################## */

BENCHMARK_F(GcpExporterBenchmark, SparseExportTest)(benchmark::State& state) {
  // Get mock exporter
  const auto gcp_exporter = GetMockExporter();

  while(state.KeepRunningBatch(kNumIterations))
  {
    std::array<std::unique_ptr<sdk::trace::Recordable>, kNumSpans> recordables;
    GenerateSparseSpans(recordables);
    gcp_exporter->Export(nostd::span<std::unique_ptr<sdk::trace::Recordable>>(recordables));
  }
}

BENCHMARK_F(GcpExporterBenchmark, DenseExportTest)(benchmark::State& state) {
  // Get mock exporter
  const auto gcp_exporter = GetMockExporter();

  while(state.KeepRunningBatch(kNumIterations))
  {
    std::array<std::unique_ptr<sdk::trace::Recordable>, kNumSpans> recordables;
    GenerateDenseSpans(recordables);    
    gcp_exporter->Export(nostd::span<std::unique_ptr<sdk::trace::Recordable>>(recordables));
  }
}


} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE

// Run benchmarks
BENCHMARK_MAIN();