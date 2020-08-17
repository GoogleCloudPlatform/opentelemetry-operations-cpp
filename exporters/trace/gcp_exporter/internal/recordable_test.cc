#include "exporters/trace/gcp_exporter/recordable.h"
#include <gtest/gtest.h>


OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace gcp
{

using cloudtrace_v2_span = google::devtools::cloudtrace::v2::Span;
constexpr char kLongRegularString[] = "Some long string to be truncated";
constexpr char kLongUnicodeString[] = "些长字符串被截断 Некоторая длинная строка, подлежащая усечению";

class RecordableTestPeer: public testing::Test {
public:
    void SetTruncatableString(cloudtrace_v2_span& span,
                              const int limit,
                              nostd::string_view string_name)
    {
        dummy_rec.SetTruncatableString(limit, string_name, span.mutable_display_name());
    }

private:
    Recordable dummy_rec;
};

TEST_F(RecordableTestPeer, TruncatableStringNotEnforced) { 
    const int max_size_bytes = -1;

    cloudtrace_v2_span span; 
    SetTruncatableString(span, max_size_bytes, kLongRegularString);

    EXPECT_EQ(kLongRegularString, span.display_name().value()); 
}

TEST_F(RecordableTestPeer, TruncatableStringRegularWithLimit) { 
    const int max_size_bytes = 5;

    cloudtrace_v2_span span; 
    SetTruncatableString(span, max_size_bytes, kLongRegularString);

    EXPECT_EQ(std::string(kLongRegularString).substr(0, max_size_bytes), span.display_name().value()); 
}

TEST_F(RecordableTestPeer, TruncatableStringZeroLimit) { 
    const int max_size_bytes = 0;

    cloudtrace_v2_span span; 
    SetTruncatableString(span, max_size_bytes, kLongRegularString);

    EXPECT_EQ("", span.display_name().value()); 
}

TEST_F(RecordableTestPeer, TruncatableStringUnicodeMidCharacterBoundary) {
    // 些 is 3 bytes long, and 长 is 3 bytes long as well,
    // output should have single symbol 些 and 3 bytes with truncation limit of 4
    // or 5 bytes.
    const std::vector<int> max_size_bytes_vector = {4, 5};

    for (int max_size_bytes : max_size_bytes_vector) {
        cloudtrace_v2_span span; 
        SetTruncatableString(span, max_size_bytes, kLongUnicodeString);

        EXPECT_EQ("些", span.display_name().value());
    }
}

TEST_F(RecordableTestPeer, TruncatableStringUnicodeAtCharacterBoundary) {
    // 些 is 3 bytes long, output should have that symbol only, and be 3 bytes
    // when truncating by boundary.
    const int max_size_bytes = 3;

    cloudtrace_v2_span span; 
    SetTruncatableString(span, max_size_bytes, kLongUnicodeString);

    EXPECT_EQ("些", span.display_name().value());  
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