# package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "video_pipeline",
    srcs = ["video_pipeline.cc"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/api2:builder",
        "//mediapipe/framework/api2:port",
    ],
)
