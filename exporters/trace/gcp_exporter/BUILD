licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:public"])

# Libraries
# ========================================================================= #
cc_library(
    name = "gcp_exporter",
    srcs = ["internal/gcp_exporter.cc"],
    hdrs = ["gcp_exporter.h"],
    deps = [
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@io_opentelemetry_cpp//sdk/src/common:random",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_googleapis//google/devtools/cloudtrace/v2:cloudtrace_cc_grpc",
        "@com_google_googleapis//google/devtools/cloudtrace/v2:cloudtrace_cc_proto",
        "@com_google_googletest//:gtest_prod"
    ],
)

# Tests
# ========================================================================= #

cc_test(
    name = "gcp_exporter_test",
    srcs = ["internal/gcp_exporter_test.cc"],
    deps = [
        ":gcp_exporter",
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@io_opentelemetry_cpp//api",
        "@com_google_googletest//:gtest_main"
    ],
)
