#include "exporters/trace/gcp_exporter/gcp_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "foo_library/foo_library.h"

namespace
{
void initTracer()
{
  auto exporter  = std::unique_ptr<opentelemetry::sdk::trace::SpanExporter>(new opentelemetry::exporter::gcp::GcpExporter);
  auto processor = std::shared_ptr<opentelemetry::sdk::trace::SpanProcessor>(
      new opentelemetry::sdk::trace::SimpleSpanProcessor(std::move(exporter)));
  auto provider = opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(new opentelemetry::sdk::trace::TracerProvider(processor));
  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);
}
}  // namespace

int main()
{
  // Removing this line will leave the default noop TracerProvider in place.
  initTracer();

  foo_library();
}