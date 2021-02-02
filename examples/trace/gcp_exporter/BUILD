# Copyright 2021, Google Inc.
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
