#include "exporters/trace/gcp_exporter/recordable.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{

constexpr size_t kAttributeStringLen = 256;
constexpr size_t kDisplayNameStringLen = 128;


// Taken from the Unilib namespace 
// Link: http://35.193.25.4/TensorFlow/models/research/syntaxnet/util/utf8/unilib_utf8_utils.h
bool IsTrailByte(char x)
{
    return static_cast<signed char>(x) < -0x40;
}

void SetTruncatableString(const int limit,
                          nostd::string_view string_name,
                          google::devtools::cloudtrace::v2::TruncatableString* str) 
{ 
    if (limit < 0 || string_name.size() < limit) 
    {
        str->set_value(string_name.data());
        str->set_truncated_byte_count(0);
        return;
    }

    // If limit points to beginning of utf8 character, truncate at the limit,
    // backtrack to the beginning of utf8 character otherwise.
    int truncation_pos = limit;

    while (truncation_pos > 0 && IsTrailByte(string_name[truncation_pos])) 
    {
        --truncation_pos;
    }

    const size_t original_size = string_name.size();
    auto truncated_str = string_name.substr(0, truncation_pos);
    str->set_value(std::string(truncated_str));
    str->set_truncated_byte_count(original_size - str->value().size());
}

void Recordable::SetIds(trace::TraceId trace_id,
                        trace::SpanId span_id,
                        trace::SpanId parent_span_id) noexcept
{
    std::array<char, 2*trace::TraceId::kSize> hex_trace_buf; 
    trace_id.ToLowerBase16(hex_trace_buf);
    const std::string hex_trace(hex_trace_buf.data(), 2*trace::TraceId::kSize);

    std::array<char, 2*trace::SpanId::kSize> hex_span_buf; 
    span_id.ToLowerBase16(hex_span_buf);
    const std::string hex_span(hex_span_buf.data(), 2*trace::SpanId::kSize);

    std::array<char, 2*trace::SpanId::kSize> hex_parent_span_buf; 
    parent_span_id.ToLowerBase16(hex_parent_span_buf);
    const std::string hex_parent_span(hex_parent_span_buf.data(), 2*trace::SpanId::kSize);

    // Get Project Id
    const std::string project_id(getenv(kGCPEnvVar));

    span_.set_name(kProjectsPathStr + project_id + kTracesPathStr + hex_trace + kSpansPathStr + hex_span);
    span_.set_span_id(hex_span);
    span_.set_parent_span_id(hex_parent_span);
}

void Recordable::SetAttribute(nostd::string_view key,
                              const common::AttributeValue &value) noexcept
{
    // Get the protobuf span's map
    auto* map = span_.mutable_attributes()->mutable_attribute_map();

    if(nostd::holds_alternative<bool>(value))
    {
        (*map)[key.data()].set_bool_value(nostd::get<bool>(value));
    } 
    else if (nostd::holds_alternative<int>(value))
    {
        (*map)[key.data()].set_int_value(nostd::get<int>(value));
    }
    else if (nostd::holds_alternative<int64_t>(value))
    {
        (*map)[key.data()].set_int_value(nostd::get<int64_t>(value));
    }
    else if (nostd::holds_alternative<unsigned int>(value))
    {
        (*map)[key.data()].set_int_value(nostd::get<unsigned int>(value));
    }
    else if (nostd::holds_alternative<uint64_t>(value))
    {
        (*map)[key.data()].set_int_value(nostd::get<uint64_t>(value));
    }
    else if (nostd::holds_alternative<int64_t>(value))
    {
        (*map)[key.data()].set_int_value(nostd::get<int64_t>(value));
    } 
    else if (nostd::holds_alternative<nostd::string_view>(value))
    {
        SetTruncatableString(kAttributeStringLen,
                             nostd::get<nostd::string_view>(value), 
                             (*map)[key.data()].mutable_string_value());
    }
}

void Recordable::AddEvent(nostd::string_view name, 
                          core::SystemTimestamp timestamp,
                          const opentelemetry::common::KeyValueIterable &attributes) noexcept
{
    (void)name;
    (void)timestamp;
    (void)attributes;
}

void Recordable::AddLink(
      const opentelemetry::trace::SpanContext &span_context,
      const opentelemetry::common::KeyValueIterable &attributes) noexcept
{
    (void)span_context;
    (void)attributes;
}

void Recordable::SetStatus(trace::CanonicalCode code, nostd::string_view description) noexcept
{
    (void)code;
    (void)description;
}

void Recordable::SetName(nostd::string_view name) noexcept
{
    SetTruncatableString(kDisplayNameStringLen, name,
                         span_.mutable_display_name());
}

void Recordable::SetStartTime(opentelemetry::core::SystemTimestamp start_time) noexcept
{
    const std::chrono::nanoseconds unix_time_nanoseconds(start_time.time_since_epoch().count());
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(unix_time_nanoseconds);
    span_.mutable_start_time()->set_seconds(seconds.count());
    span_.mutable_start_time()->set_nanos(unix_time_nanoseconds.count()-
        std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}

void Recordable::SetDuration(std::chrono::nanoseconds duration) noexcept
{
    const std::chrono::nanoseconds start_time_nanos(span_.start_time().nanos());
    const std::chrono::seconds start_time_seconds(span_.start_time().seconds());
    const std::chrono::nanoseconds unix_end_time(
        std::chrono::duration_cast<std::chrono::nanoseconds>(start_time_seconds).count() 
        + start_time_nanos.count() 
        + duration.count());    
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(unix_end_time);
    span_.mutable_end_time()->set_seconds(seconds.count());
    span_.mutable_end_time()->set_nanos(
        unix_end_time.count()-
        std::chrono::duration_cast<std::chrono::nanoseconds>(seconds).count());
}

}  // namespace gcp
}  // namespace exporter
OPENTELEMETRY_END_NAMESPACE