#pragma once

#include "opentelemetry/sdk/trace/exporter.h"
#include "exporters/trace/gcp_exporter/recordable.h"

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
     * Internal constructor to initialize the RPC communication stub and the Google project ID
     * Helps with testing purposes by injecting a mock stub
     * 
     * @param stub - The stub to inject into the member variable 'trace_service_stub_'
     * @param project_id - The Id of the Google Cloud project to export the traces to 
     */
    explicit GcpExporter(std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::StubInterface> stub,
                         const char* project_id);

    /* The stub to communicate via gRPC to the Google Cloud */
    const std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::StubInterface> trace_service_stub_;

    /* The Id of the Google Cloud project to export the traces to */
    const std::string project_id_;
};

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE