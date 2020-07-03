#include "gtest/gtest.h"
#include <stdlib.h>
#include "../gcp_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/core/timestamp.h"
#include "google/devtools/cloudtrace/v2/tracing_mock.grpc.pb.h"

using testing::_;
using testing::AtLeast;
using testing::Return;
using grpc::Status;

namespace gcp = opentelemetry::exporter::gcp;
namespace cloudtrace_v2 = google::devtools::cloudtrace::v2;


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter {
namespace gcp {

class GcpExporterTestPeer : public ::testing::Test
{
public:
    std::unique_ptr<GcpExporter> GetExporter(cloudtrace_v2::TraceService::StubInterface* mock_stub) 
    {
        return std::unique_ptr<GcpExporter>(new GcpExporter(std::unique_ptr<cloudtrace_v2::TraceService::StubInterface>(mock_stub),
                                            "test_project"));
    }
};


TEST_F(GcpExporterTestPeer, TestGeneralFunctionality)
{
    // Set up mock stub
    auto mock_stub = new cloudtrace_v2::MockTraceServiceStub();
    EXPECT_CALL(*mock_stub, BatchWriteSpans(_,_,_)).Times(AtLeast(1)).WillRepeatedly(Return(Status::OK));

    auto gcp_exporter  = std::unique_ptr<sdk::trace::SpanExporter>(GetExporter(mock_stub));
    auto processor = std::shared_ptr<sdk::trace::SpanProcessor>(new sdk::trace::SimpleSpanProcessor(std::move(gcp_exporter)));
    auto provider = nostd::shared_ptr<trace::TracerProvider>(new sdk::trace::TracerProvider(processor));
    
    auto tracer = provider->GetTracer("Test Export");

    auto test_span = tracer->StartSpan("Test Span");

    auto span_1 = tracer->StartSpan("Span 1");
    auto span_1_1 = tracer->StartSpan("Span 1.1");
    auto span_1_1_1 = tracer->StartSpan("Span 1.1.1");
    span_1_1_1->End();
    span_1_1->End();
    span_1->End();

    auto span_2 = tracer->StartSpan("Span 2");
    auto span_2_1 = tracer->StartSpan("Span 2.1");
    span_2_1->End();
    auto span_2_2 = tracer->StartSpan("Span 2.2");
    span_2_2->End();
    span_2->End();

    test_span->End();
}


TEST_F(GcpExporterTestPeer, TestExportResults)
{
    // Set up mock stub
    auto mock_stub = new cloudtrace_v2::MockTraceServiceStub();
    auto gcp_exporter  = std::unique_ptr<sdk::trace::SpanExporter>(GetExporter(mock_stub));

    // Make sample recordables
    auto recordable_1 = gcp_exporter->MakeRecordable();
    recordable_1->SetName("Sample span 1");
    auto recordable_2 = gcp_exporter->MakeRecordable();
    recordable_2->SetName("Sample span 2");
    
    // Test Success
    EXPECT_CALL(*mock_stub, BatchWriteSpans(_,_,_)).Times(1).WillOnce(Return(Status::OK));
    nostd::span<std::unique_ptr<sdk::trace::Recordable>> batch_1(&recordable_1, 1);
    auto result_1 = gcp_exporter->Export(batch_1);
    EXPECT_EQ(sdk::trace::ExportResult::kSuccess, result_1);

    // Test Failure
    EXPECT_CALL(*mock_stub, BatchWriteSpans(_,_,_)).Times(1).WillOnce(Return(Status::CANCELLED));
    nostd::span<std::unique_ptr<sdk::trace::Recordable>> batch_2(&recordable_2, 1);
    auto result_2 = gcp_exporter->Export(batch_2);
    EXPECT_EQ(sdk::trace::ExportResult::kFailure, result_2);
}

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE