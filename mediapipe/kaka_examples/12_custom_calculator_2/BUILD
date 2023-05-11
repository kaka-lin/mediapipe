load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")

mediapipe_proto_library(
    name = "addconstant_calculator_proto",
    srcs = ["addconstant_calculator.proto"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
    ],
)

cc_binary(
    name = "custom_calculator",
    srcs = ["custom_calculator.cc"],
    deps = [
        ":addconstant_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
    ],
)
