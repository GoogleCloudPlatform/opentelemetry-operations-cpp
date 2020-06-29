# Simple GCP Trace Exporter Example

In this example, the application in main.cc initializes and registers a tracer provider from the OpenTelemetry SDK. The application then calls a foo_library which has been instrumented using the OpenTelemetry API.
Resulting telemetry is exported to a user specified Google Cloud project.  

# Building
To build the example above, simple run the following command:
```
bazel build //examples/trace/gcp_exporter:example_simple
```

# Running
After building the example, run the following command but set your Google Project ID:
```
env STACKDRIVER_PROJECT_ID=<myproject> ./bazel-bin/examples/trace/gcp_exporter/example_simple
```
