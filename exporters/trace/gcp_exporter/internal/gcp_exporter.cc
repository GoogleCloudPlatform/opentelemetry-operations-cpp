#include "../gcp_exporter.h"
#include <stdlib.h>
#include <grpcpp/grpcpp.h>
#include "opentelemetry/trace/trace_id.h"
#include "opentelemetry/trace/span_id.h"
#include "google/protobuf/timestamp.pb.h"

constexpr char kGoogleTraceAddress[] = "cloudtrace.googleapis.com";
constexpr int kByteSizeSpanId = 16;
constexpr int kByteSizeTraceId = 32;
constexpr char kProjectsPathStr[] = "projects/";
constexpr char kTracesPathStr[] = "/traces/";
constexpr char kSpansPathStr[] = "/spans/";
constexpr char kGCPEnvVar[] = "GOOGLE_CLOUD_PROJECT_ID";


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter 
{
namespace gcp 
{


/* ######################## HELPER FUNCTIONS ######################### */

/**
 * Generates a random trace id
 * NOTE: This is subject to change in the future and will be moved to 
 *       sdk/src/trace/span.cc
 */
trace::TraceId GenerateRandomTraceId() 
{
    uint8_t trace_id_buf[trace::TraceId::kSize];
    opentelemetry::sdk::common::Random::GenerateRandomBuffer(trace_id_buf);
    return trace::TraceId(trace_id_buf);
}


/**
 * Generates a random span id
 * NOTE: This is subject to change in the future and will be moved to 
 *       sdk/src/trace/span.cc
 */
trace::SpanId GenerateRandomSpanId()
{
    uint8_t span_id_buf[trace::SpanId::kSize];
    opentelemetry::sdk::common::Random::GenerateRandomBuffer(span_id_buf);
    return trace::SpanId(span_id_buf);
}


/**
 * Sets google protobuf object's unix start time, both seconds and nanos
 * 
 * @param from_span - The span whose start time we extract and convert to google protobuf timestamp
 * @param proto - A google protobuf timestamp whose seconds and nanoseconds we set in unix time 
 */
void SetStartTimestamp(const sdk::trace::SpanData* from_span, google::protobuf::Timestamp* proto)
{
    std::chrono::nanoseconds unix_time_nanoseconds(from_span->GetStartTime().time_since_epoch().count());
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(unix_time_nanoseconds);
    proto->set_seconds(seconds.count());
    proto->set_nanos(unix_time_nanoseconds.count()-std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}


/**
 * Sets google protobuf object's unix end time, both seconds and nanos
 * 
 * @param from_span - The span whose end time we extract and convert to google protobuf timestamp
 * @param proto - A google protobuf timestamp whose seconds and nanoseconds we set in unix time
 */
void SetEndTimestamp(const sdk::trace::SpanData* from_span, google::protobuf::Timestamp* proto)
{
    std::chrono::nanoseconds unix_time_nanoseconds(from_span->GetStartTime().time_since_epoch().count()
                                                    + from_span->GetDuration().count());
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(unix_time_nanoseconds);
    proto->set_seconds(seconds.count());
    proto->set_nanos(unix_time_nanoseconds.count()-std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}


/**
 * Converts each Recordable(Span) to a google protobuf span used by Cloud Trace v2 API
 * to display traces on GCP
 * 
 * @param spans - The list of spans to be exported to google cloud
 * @param project_id - User specified google cloud project id
 * @param trace_id_ - *This is subject to change in the future*
 * @param request - The request we send via gRPC to the cloud to export the traces
 */
void ConvertSpans(const nostd::span<std::unique_ptr<sdk::trace::Recordable>> &spans,
                  const std::string& project_id,
                  const trace::TraceId& trace_id_,
                  google::devtools::cloudtrace::v2::BatchWriteSpansRequest* request)
{

    for (auto &recordable : spans)
    {
        auto to_span = request->add_spans();
        auto from_span = std::unique_ptr<sdk::trace::SpanData>(
            static_cast<sdk::trace::SpanData *>(recordable.release()));

        /*                      SET SPAN DATA                      */

        // Generate random span id 
        // NOTE: This is subject to change in the future and will be moved to 
        //       sdk/src/trace/span.cc
        auto span_id = GenerateRandomSpanId();
        
        // NOTE: This implementation of trace id is subject to change in the future once parent 
        //       context is available for a span, wherein we can just retrieve trace id from the parent
        // Convert bytes to hex
        std::array<char, kByteSizeTraceId> hex_trace_buf; trace_id_.ToLowerBase16(hex_trace_buf);
        const std::string hex_trace(hex_trace_buf.data(), kByteSizeTraceId);
        std::array<char, kByteSizeSpanId> hex_span_buf; span_id.ToLowerBase16(hex_span_buf);
        const std::string hex_span(hex_span_buf.data(), kByteSizeSpanId);
        
        // Set span id
        to_span->set_span_id(hex_span);

        // Set names
        to_span->set_name(kProjectsPathStr + project_id + kTracesPathStr + hex_trace + kSpansPathStr + hex_span);
        to_span->mutable_display_name()->set_value(std::string(from_span->GetName()));
        
        // set start time
        SetStartTimestamp(from_span.get(), to_span->mutable_start_time());

        // set end time
        SetEndTimestamp(from_span.get(), to_span->mutable_end_time());

        // TODO:
        // Get other features of the span once more data is added to the official repo
    }
}


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


GcpExporter::GcpExporter() : GcpExporter(MakeServiceStub(), GenerateRandomTraceId()) {}


GcpExporter::GcpExporter(std::unique_ptr<google::devtools::cloudtrace::v2::TraceService::StubInterface> stub,
                         const trace::TraceId trace_id):
    trace_service_stub_(std::move(stub)), trace_id_(trace_id) {}


/* ############################### EXPORT FUNCTIONS ################################## */


std::unique_ptr<sdk::trace::Recordable> GcpExporter::MakeRecordable() noexcept
{
    return std::unique_ptr<sdk::trace::Recordable>(new sdk::trace::SpanData);
}


sdk::trace::ExportResult GcpExporter::Export(
      const nostd::span<std::unique_ptr<sdk::trace::Recordable>> &spans) noexcept 
{
    // Fetch project id
    const std::string project_id(getenv(kGCPEnvVar));

    // Set up gRPC request
    google::devtools::cloudtrace::v2::BatchWriteSpansRequest request;
    request.set_name(kProjectsPathStr + project_id);

    // Convert recordables to google protobufs
    ConvertSpans(spans, project_id, trace_id_, &request);

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

