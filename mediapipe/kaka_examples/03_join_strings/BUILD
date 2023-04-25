
# Note: this project has 3 source files
# From now on I put all calculators into separate cpp files

cc_binary(
    name = "join_strings",
    srcs = ["join_strings.cc", "string_source_calculator.cc", "string_join_calculator.cc"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
    ],
    data = [":k_graph.pbtxt"]
)
