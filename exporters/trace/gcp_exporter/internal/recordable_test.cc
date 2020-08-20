#include "exporters/trace/gcp_exporter/recordable.h"
#include "opentelemetry/context/threadlocal_context.h"

#include <gtest/gtest.h>


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{

constexpr char kLongRegularString[] = "Some long regular string not meant to be truncated";

TEST(Recordable, TruncatableStringNotEnforcedDisplayName) { 
    Recordable rec;
    
    // Test regular string
    rec.SetName(kLongRegularString); 
    EXPECT_EQ(kLongRegularString, rec.span().display_name().value()); 

    // Test exactly 128 byte long string
    const std::string exactly_128_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符串被");

    ASSERT_EQ(128, exactly_128_byte_long_string.size());
                                
    rec.SetName(exactly_128_byte_long_string);
    EXPECT_EQ(exactly_128_byte_long_string, rec.span().display_name().value());
}

TEST(Recordable, TruncatableStringEnforcedDisplayName) { 
    Recordable rec;
    
    const std::string exactly_127_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符");
    const std::string exactly_129_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符д");  
    const std::string exactly_130_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符断");

    ASSERT_EQ(127, exactly_127_byte_long_string.size());
    ASSERT_EQ(129, exactly_129_byte_long_string.size());
    ASSERT_EQ(130, exactly_130_byte_long_string.size());
    
    // The last symbols of the 129 and 130 byte long strings are "д" and "断" respectively.
    // "д" is 2 bytes long and "断" is 3 bytes long.
    // Thus, the display name should be truncated to `exactly_127_byte_long_string` 
    // to conform to limit of <= 128 bytes.
    rec.SetName(exactly_129_byte_long_string); 
    EXPECT_EQ(exactly_127_byte_long_string, rec.span().display_name().value()); 

    rec.SetName(exactly_130_byte_long_string); 
    EXPECT_EQ(exactly_127_byte_long_string, rec.span().display_name().value()); 
}

TEST(Recordable, TruncatableStringNotEnforcedAttributeString) { 
    Recordable rec;
    
    // Test Regular String
    rec.SetAttribute("string_key_1", common::AttributeValue(kLongRegularString));

    // Test exactly 256 byte long string
    const char kExactly256byteUnicodeString[] = "些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                 "些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                 "些长字符串被截断 Некот";

    rec.SetAttribute("string_key_2", common::AttributeValue(kExactly256byteUnicodeString));

    auto attr_map = rec.span().attributes().attribute_map();
    
    EXPECT_EQ(kLongRegularString, attr_map["string_key_1"].string_value().value());   
    EXPECT_EQ(kExactly256byteUnicodeString, attr_map["string_key_2"].string_value().value());
}

TEST(Recordable, TruncatableStringEnforcedAttributeString) { 
    Recordable rec;
    
    const std::string exactly_255_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符 些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符");
    const std::string exactly_257_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符 些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符д");  
    const std::string exactly_258_byte_long_string("些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符 些长字符串被截断 Некоторая длинная строка, подлежащая усечению"
                                                   "些长字符а符断");

    ASSERT_EQ(255, exactly_255_byte_long_string.size());
    ASSERT_EQ(257, exactly_257_byte_long_string.size());
    ASSERT_EQ(258, exactly_258_byte_long_string.size());
    
    // The last symbols of the 257 and 258 byte long strings are "д" and "断" respectively.
    // "д" is 2 bytes long and "断" is 3 bytes long.
    // Thus, the attribute string should be truncated to `exactly_255_byte_long_string` 
    // to conform to limit of <= 256 bytes.
    rec.SetAttribute("string_key_1", common::AttributeValue(exactly_257_byte_long_string));
    rec.SetAttribute("string_key_2", common::AttributeValue(exactly_258_byte_long_string));

    auto attr_map = rec.span().attributes().attribute_map();

    EXPECT_EQ(exactly_255_byte_long_string, attr_map["string_key_1"].string_value().value());   
    EXPECT_EQ(exactly_255_byte_long_string, attr_map["string_key_2"].string_value().value());
}

TEST(Recordable, TestSetNonIntAttribute)
{
    Recordable rec;

    // Set 'bool' type
    const nostd::string_view bool_key = "bool_key";
    const common::AttributeValue bool_value = true;
    rec.SetAttribute(bool_key, std::move(bool_value));

    // Set 'string' type
    const nostd::string_view string_key = "string_key";
    const common::AttributeValue string_value = "test";
    rec.SetAttribute(string_key, std::move(string_value));

    auto attr_map = rec.span().attributes().attribute_map();
    
    EXPECT_TRUE(attr_map["bool_key"].bool_value());
    EXPECT_EQ("test", attr_map["string_key"].string_value().value());
}

template <typename T>
struct IntAttributeTest : public testing::Test
{
    using IntParamType = T;
};

using IntTypes = testing::Types<int, int64_t, unsigned int, uint64_t>;
TYPED_TEST_CASE(IntAttributeTest, IntTypes);

TYPED_TEST(IntAttributeTest, SetIntSingleAttribute)
{
    using IntType = typename TestFixture::IntParamType;
    IntType i     = 2;
    common::AttributeValue int_val(i);

    Recordable rec;
    rec.SetAttribute("int_key", int_val);

    auto attr_map = rec.span().attributes().attribute_map();

    EXPECT_EQ(nostd::get<IntType>(int_val), attr_map["int_key"].int_value());
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