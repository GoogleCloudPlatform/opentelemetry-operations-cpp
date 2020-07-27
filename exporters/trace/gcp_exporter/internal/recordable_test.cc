#include "exporters/trace/gcp_exporter/recordable.h"
#include <gtest/gtest.h>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{

TEST(Recordable, TestSetAttribute)
{
    Recordable rec;

    // Set 'bool' type
    const nostd::string_view bool_key = "bool_key";
    const common::AttributeValue bool_value = true;
    rec.SetAttribute(bool_key, std::move(bool_value));

    // Set 'int' type
    const nostd::string_view int_key = "int_key";
    const int seven_int = 7;
    const common::AttributeValue int_value = seven_int;
    rec.SetAttribute(int_key, std::move(int_value));

    // Set 'int64_t' type
    const nostd::string_view int64_t_key = "int64_t_key";
    const int64_t seven_int64_t = 7;
    const common::AttributeValue int64_t_value = seven_int64_t;
    rec.SetAttribute(int64_t_key, std::move(int64_t_value));

    // Set 'uint64_t' type
    const nostd::string_view uint64_t_key = "uint64_t_key";
    const uint64_t seven_uint64_t = 7;
    const common::AttributeValue uint64_t_value = seven_uint64_t;
    rec.SetAttribute(uint64_t_key, std::move(uint64_t_value));

    // Set 'unsigned int' type
    const nostd::string_view unsigned_int_key = "unsigned_int_key";
    const int64_t seven_unsigned_int = 7;
    const common::AttributeValue unsigned_int_value = seven_unsigned_int;
    rec.SetAttribute(unsigned_int_key, std::move(unsigned_int_value));

    // Set 'string' type
    const nostd::string_view string_key = "string_key";
    const common::AttributeValue string_value = "test";
    rec.SetAttribute(string_key, std::move(string_value));

    auto attr_map = rec.span().attributes().attribute_map();
    
    EXPECT_TRUE(attr_map["bool_key"].bool_value());
    EXPECT_EQ(seven_int, attr_map["int_key"].int_value());
    EXPECT_EQ(seven_uint64_t, attr_map["uint64_t_key"].int_value());
    EXPECT_EQ(seven_int64_t, attr_map["int64_t_key"].int_value());
    EXPECT_EQ(seven_unsigned_int, attr_map["unsigned_int_key"].int_value());
    EXPECT_EQ("test", attr_map["string_key"].string_value().value());
}

TEST(Recordable, TestSetIds)
{
    setenv("GOOGLE_CLOUD_PROJECT_ID", "test_project", 1);

    const opentelemetry::trace::TraceId trace_id(
    std::array<const uint8_t, opentelemetry::trace::TraceId::kSize>(
    {0, 1, 0, 2, 1, 3, 1, 4, 1, 5, 1, 6, 3, 7, 0, 0}));

    const opentelemetry::trace::SpanId span_id(
    std::array<const uint8_t, opentelemetry::trace::SpanId::kSize>(
    {1, 2, 3, 4, 5, 6, 7, 8}));

    const opentelemetry::trace::SpanId parent_span_id(
    std::array<const uint8_t, opentelemetry::trace::SpanId::kSize>(
    {4, 5, 0, 1, 1, 1, 1, 3}));

    Recordable rec;

    rec.SetIds(trace_id, span_id, parent_span_id);

    EXPECT_EQ("projects/test_project/traces/00010002010301040105010603070000/spans/0102030405060708", 
               rec.span().name());
    EXPECT_EQ("0102030405060708", rec.span().span_id());
    EXPECT_EQ("0405000101010103", rec.span().parent_span_id());
}


TEST(Recordable, TestSetName)
{
    Recordable rec;
    const nostd::string_view expected_name = "Test Span";
    rec.SetName(expected_name);
    EXPECT_EQ(expected_name, rec.span().display_name().value());
}


TEST(Recordable, TestSetStartTime)
{
    Recordable rec;

    const std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
    const core::SystemTimestamp start_timestamp(start_time);

    const int64_t expected_unix_start_time = 
        std::chrono::duration_cast<std::chrono::nanoseconds>(start_time.time_since_epoch()).count();

    rec.SetStartTime(start_timestamp);

    const std::chrono::nanoseconds start_time_nanos(rec.span().start_time().nanos());
    const std::chrono::seconds start_time_seconds(rec.span().start_time().seconds());
    const std::chrono::nanoseconds unix_start_time(
        std::chrono::duration_cast<std::chrono::nanoseconds>(start_time_seconds).count() 
        + start_time_nanos.count()); 

    EXPECT_EQ(expected_unix_start_time, unix_start_time.count());
}


TEST(Recordable, TestSetDuration)
{
    Recordable rec;

    const std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
    const core::SystemTimestamp start_timestamp(start_time);
    const std::chrono::nanoseconds duration(10);

    const int64_t expected_unix_end_time = start_timestamp.time_since_epoch().count() + duration.count();

    rec.SetStartTime(start_timestamp);
    rec.SetDuration(duration);

    const std::chrono::nanoseconds end_time_nanos(rec.span().end_time().nanos());
    const std::chrono::seconds end_time_seconds(rec.span().end_time().seconds());
    const std::chrono::nanoseconds unix_end_time(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_seconds).count() 
        + end_time_nanos.count());    

    EXPECT_EQ(expected_unix_end_time, unix_end_time.count());
}

}  // namespace gcp
}  // namespace exporter
OPENTELEMETRY_END_NAMESPACE