# package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "simple_pipeline",
    srcs = ["simple_pipeline.cc"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/api2:builder",
        "//mediapipe/framework/api2:port",
    ],
)
