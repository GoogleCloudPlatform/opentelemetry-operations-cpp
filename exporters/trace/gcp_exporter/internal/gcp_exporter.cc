#include "../gcp_exporter.h"
#include <grpcpp/grpcpp.h>


constexpr char kGoogleTraceAddress[] = "cloudtrace.googleapis.com";


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter 
{
namespace gcp 
{


/* ################### INITIALIZATION/REGISTER FUNCTIONS ########################## */

/**
 * Establishes gRPC communication channel to the Google Trace Address
 * 
 * @return A cloudtrace v2 API trace service stub to communicate over via gRPC
 */
std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::Stub> MakeServiceStub()
{
    grpc::ChannelArguments args;
    args.SetUserAgentPrefix("opentelemetry-cpp/" OPENTELEMETRY_VERSION);
    auto channel = grpc::CreateCustomChannel(kGoogleTraceAddress, 
                                            grpc::GoogleDefaultCredentials(),
                                            args);
    return google::devtools::cloudtrace::v2::TraceService::NewStub(channel);
}


GcpExporter::GcpExporter() : GcpExporter(MakeServiceStub(), getenv(kGCPEnvVar)) {}


GcpExporter::GcpExporter(std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::StubInterface> stub,
                         const char* project_id):
    trace_service_stub_(std::move(stub)), project_id_(project_id) {}


/* ############################### EXPORT FUNCTIONS ################################## */


std::unique_ptr<sdk::trace::Recordable> GcpExporter::MakeRecordable() noexcept
{
    return std::unique_ptr<sdk::trace::Recordable>(new Recordable);
}


sdk::trace::ExportResult GcpExporter::Export(
      const nostd::span<std::unique_ptr<sdk::trace::Recordable>> &spans) noexcept 
{
    // Set up gRPC request
    google::devtools::cloudtrace::v2::BatchWriteSpansRequest request;
    request.set_name(kProjectsPathStr + project_id_);
    for(auto& recordable: spans){
        auto span = std::unique_ptr<Recordable>(static_cast<Recordable*>(recordable.release()));
        *request.add_spans() = std::move(span->span());
    }

    // Send the RPC
    google::protobuf::Empty response;
    grpc::ClientContext context;
    grpc::Status status = trace_service_stub_->BatchWriteSpans(&context, request, &response);

    // Check status and return results
    if(status.ok()){
        return sdk::trace::ExportResult::kSuccess;
    } else {
        return sdk::trace::ExportResult::kFailure;
    }
}

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE

