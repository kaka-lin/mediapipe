# 2023 Kaka All Rights Reserved

android_library(
    name = "basic_lib",
    srcs = glob(["*.java"]),
    manifest = "AndroidManifest.xml",
    resource_files = glob(["res/**"]),
    visibility = ["//visibility:public"],
    deps = [
        "//third_party:androidx_constraint_layout",
        "//third_party:androidx_appcompat",
    ],
)

android_binary(
    name = "helloworld",
    manifest = "AndroidManifest.xml",
    manifest_values = {
        "applicationId": "com.kaka.mediapipe.apps.helloworld",
        "appName": "Kaka Hello World",
        "mainActivity": ".MainActivity",
    },
    multidex = "native",
    deps = [
        ":basic_lib",
        "//third_party:libc++_shared_lib"
    ],
)
