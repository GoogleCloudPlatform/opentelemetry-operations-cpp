cc_library(
    name = "foo_library",
    srcs = [
        "foo_library/foo_library.cc",
    ],
    hdrs = [
        "foo_library/foo_library.h",
    ],
    deps = [
        "@io_opentelemetry_cpp//api",
    ],
)

cc_binary(
    name = "example_simple",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":foo_library",
        "//exporters/trace/gcp_exporter:gcp_exporter",
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//sdk/src/trace",
    ],
)