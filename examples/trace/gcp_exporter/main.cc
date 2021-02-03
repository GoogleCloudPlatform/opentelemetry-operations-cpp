/*
 * Copyright 2021 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
