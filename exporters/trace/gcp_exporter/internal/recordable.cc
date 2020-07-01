#include "exporters/trace/gcp_exporter/recordable.h"
#include <assert.h>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{

void Recordable::SetIds(trace::TraceId trace_id,
                        trace::SpanId span_id,
                        trace::SpanId parent_span_id) noexcept
{
    (void)trace_id;
    span_.set_span_id(reinterpret_cast<const char *>(span_id.Id().data()), trace::SpanId::kSize);
    span_.set_parent_span_id(reinterpret_cast<const char *>(parent_span_id.Id().data()),
                            trace::SpanId::kSize);
}


void Recordable::SetAttribute(nostd::string_view key,
                              const common::AttributeValue &&value) noexcept
{
    assert(nostd::holds_alternative<bool>(value) ||
           nostd::holds_alternative<int64_t>(value) ||
           nostd::holds_alternative<nostd::string_view>(value));

    // Get the protobuf span's map
    auto map = span_.mutable_attributes()->mutable_attribute_map();

    if(nostd::holds_alternative<bool>(value))
    {
        (*map)[std::string(key)].set_bool_value(nostd::get<bool>(value));
    } 
    else if (nostd::holds_alternative<int64_t>(value))
    {
        (*map)[std::string(key)].set_int_value(nostd::get<int64_t>(value));
    } 
    else if (nostd::holds_alternative<nostd::string_view>(value))
    {
        // TODO: Truncate string to 128 bytes
        (*map)[std::string(key)].mutable_string_value()->set_value(std::string(nostd::get<nostd::string_view>(value)));
    }
}


void Recordable::AddEvent(nostd::string_view name, core::SystemTimestamp timestamp) noexcept
{
    (void)name;
}


void Recordable::SetStatus(trace::CanonicalCode code, nostd::string_view description) noexcept
{
    (void)code;
    (void)description;
}


void Recordable::SetName(nostd::string_view name) noexcept
{
    // TODO: Truncate string to 128 bytes
    span_.mutable_display_name()->set_value(std::string(name));
}


void Recordable::SetStartTime(opentelemetry::core::SystemTimestamp start_time) noexcept
{
    const std::chrono::nanoseconds unix_time_nanoseconds(start_time.time_since_epoch().count());
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(unix_time_nanoseconds);
    span_.mutable_start_time()->set_seconds(seconds.count());
    span_.mutable_start_time()->set_nanos(unix_time_nanoseconds.count()-std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}


void Recordable::SetDuration(std::chrono::nanoseconds duration) noexcept
{
    const std::chrono::nanoseconds start_time_nanos(reinterpret_cast<int32_t>(span_.start_time().nanos()));
    const std::chrono::seconds start_time_seconds(reinterpret_cast<int64_t>(span_.start_time().seconds()));
    const std::chrono::nanoseconds unix_end_time(std::chrono::duration_cast<std::chrono::nanoseconds>(start_time_seconds).count() 
                                             + start_time_nanos.count() + duration.count());    
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(unix_end_time);
    span_.mutable_end_time()->set_seconds(seconds.count());
    span_.mutable_end_time()->set_nanos(unix_end_time.count()-std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}

}  // namespace gcp
}  // namespace exporter
OPENTELEMETRY_END_NAMESPACE