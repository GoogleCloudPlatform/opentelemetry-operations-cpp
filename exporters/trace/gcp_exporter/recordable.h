#pragma once

#include "google/devtools/cloudtrace/v2/tracing.grpc.pb.h"
#include "opentelemetry/sdk/trace/recordable.h"
#include "opentelemetry/version.h"
#include "opentelemetry/nostd/variant.h"


constexpr char kProjectsPathStr[] = "projects/";
constexpr char kTracesPathStr[] = "/traces/";
constexpr char kSpansPathStr[] = "/spans/";
constexpr char kGCPEnvVar[] = "GOOGLE_CLOUD_PROJECT_ID";


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{
class Recordable final : public sdk::trace::Recordable
{
public:
  const google::devtools::cloudtrace::v2::Span &span() const noexcept { return span_; }

  void SetIds(opentelemetry::trace::TraceId trace_id,
                      opentelemetry::trace::SpanId span_id,
                      opentelemetry::trace::SpanId parent_span_id) noexcept override;

  void SetAttribute(nostd::string_view key,
                            const opentelemetry::common::AttributeValue &value) noexcept override;

  void AddEvent(
      nostd::string_view name,
      core::SystemTimestamp timestamp = core::SystemTimestamp(std::chrono::system_clock::now()),
      const opentelemetry::trace::KeyValueIterable &attributes =
          opentelemetry::trace::KeyValueIterableView<std::map<std::string, int>>({})) noexcept override;

  void AddLink(
      opentelemetry::trace::SpanContext span_context,
      const opentelemetry::trace::KeyValueIterable &attributes =
          opentelemetry::trace::KeyValueIterableView<std::map<std::string, int>>({})) noexcept override;

  void SetStatus(opentelemetry::trace::CanonicalCode code,
                         nostd::string_view description) noexcept override;

  void SetName(nostd::string_view name) noexcept override;

  void SetStartTime(opentelemetry::core::SystemTimestamp start_time) noexcept override;

  void SetDuration(std::chrono::nanoseconds duration) noexcept override;

private:
  google::devtools::cloudtrace::v2::Span span_;
};

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE 