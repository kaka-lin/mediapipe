# This is a macro to compile proto files basically
load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")

mediapipe_proto_library(
    name = "goblin_calculator_proto",
    srcs = ["goblin_calculator.proto"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
    ],
)

# This is our main target
# Note: the dependency name: goblin_calculator_cc_proto (NOT goblin_calculator_proto !)
cc_binary(
    name = "calculator_options",
    srcs = ["calculator_options.cc", "goblin_calculator.cc"],
    deps = [
        ":goblin_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:status",
    ],
)
