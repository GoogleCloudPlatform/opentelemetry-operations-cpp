#pragma once

#include "google/devtools/cloudtrace/v2/tracing.grpc.pb.h"
#include "opentelemetry/sdk/trace/recordable.h"
#include "opentelemetry/version.h"
#include "opentelemetry/nostd/variant.h"


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{
class Recordable final : public sdk::trace::Recordable
{
public:
  const google::devtools::cloudtrace::v2::Span &span() const noexcept { return span_; }

  void SetIds(trace::TraceId trace_id,
              trace::SpanId span_id,
              trace::SpanId parent_span_id) noexcept override;

  void SetAttribute(nostd::string_view key,
                    const opentelemetry::common::AttributeValue &&value) noexcept override;

  void AddEvent(nostd::string_view name, core::SystemTimestamp timestamp) noexcept override;

  void SetStatus(trace::CanonicalCode code, nostd::string_view description) noexcept override;

  void SetName(nostd::string_view name) noexcept override;

  void SetStartTime(opentelemetry::core::SystemTimestamp start_time) noexcept override;

  void SetDuration(std::chrono::nanoseconds duration) noexcept override;

private:
  google::devtools::cloudtrace::v2::Span span_;
};

} // gcp
} // exporter
OPENTELEMETRY_END_NAMESPACE 