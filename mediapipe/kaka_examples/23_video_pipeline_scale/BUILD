# This is a macro to compile proto files basically
load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")

mediapipe_proto_library(
    name = "kimage_scale_calculator_proto",
    srcs = ["kimage_scale_calculator.proto"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
    ],
)

cc_binary(
    name = "video_pipeline",
    srcs = ["video_pipeline.cc", "kimage_scale_calculator.cc"],
    deps = [
        ":kimage_scale_calculator_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/calculators/image:scale_image_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:opencv_core",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:status",
    ]
)
