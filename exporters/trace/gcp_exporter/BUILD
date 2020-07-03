# Copyright 2020, OpenTelemetry Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])  # Apache License 2.0

package(default_visibility = ["//visibility:public"])

# Libraries
# ========================================================================= #

cc_library(
    name = "recordable",
    srcs = [
        "internal/recordable.cc",
    ],
    hdrs = [
        "recordable.h",
    ],
    deps = [
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_googleapis//google/devtools/cloudtrace/v2:cloudtrace_cc_proto",
        "@com_google_googleapis//google/devtools/cloudtrace/v2:cloudtrace_cc_grpc"
    ],
)


cc_library(
    name = "gcp_exporter",
    srcs = ["internal/gcp_exporter.cc"],
    hdrs = ["gcp_exporter.h"],
    deps = [
        ":recordable",
        "@io_opentelemetry_cpp//sdk/src/trace"
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

cc_test(
    name = "recordable_test",
    srcs = ["internal/recordable_test.cc"],
    deps = [
        ":recordable",
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@io_opentelemetry_cpp//api",
        "@com_google_googletest//:gtest_main"
    ],
)
