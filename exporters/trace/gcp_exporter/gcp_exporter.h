#pragma once

#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "google/devtools/cloudtrace/v2/tracing.grpc.pb.h"
#include "src/common/random.h"

#include <iostream>
#include <functional>
#include <memory>
#include <string>


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter 
{
namespace gcp 
{

/**
 * This class handles all the functionality of exporting traces to Google Cloud
 */
class GcpExporter final : public sdk::trace::SpanExporter
{
public:
    /**
     * Class Constructor which invokes all the register/initialization functions
     */
    GcpExporter();

    /**
     * Creates a Recordable(Span) object
     */
    std::unique_ptr<sdk::trace::Recordable> MakeRecordable() noexcept;

    /**
     * Exports all gathered spans to the cloud
     * 
     * @param spans - List of spans to export to google cloud
     * @return Success or failure based on returned gRPC status 
     */
    sdk::trace::ExportResult Export(const nostd::span<std::unique_ptr<sdk::trace::Recordable>> &spans) noexcept;

    /**/
    void Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept {}

private:
     /* Test Class meant for testing purposes only */
    friend class GcpExporterTestPeer;

    /**
     * Internal constructor to initialize trace id and the RPC communication stub
     * Helps with testing purposes by injecting a mock stub
     * 
     * @param stub - The stub to inject into the member variable 'trace_service_stub_'
     * @param trace_id - The randomly generated trace id for the current trace
     */
    explicit GcpExporter(std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::StubInterface> stub,
                         const trace::TraceId trace_id);

    /* The stub to communicate via gRPC to the Google cloud */
    const std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::StubInterface> trace_service_stub_;

    /* NOTE: This is subject to change as currently we have no parent context for a span in OT */
    const trace::TraceId trace_id_;
};

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE