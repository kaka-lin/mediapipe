# Object Detection on Desktop

## Get Started

1. Build the example, using `object_detection_tflite` as an example, for other binary please see the `BUILD` file.

    ```bash
    $ bazel build --define MEDIAPIPE_DISABLE_GPU=1 \
    mediapipe/examples/desktop/object_detection:object_detection_tflite
    ```

2. Run the example

    ```bash
    # Replace <input video path> and <output video path>.
    $ ./bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_tensorflow \
        --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_desktop_tensorflow_graph.pbtxt \
        --input_side_packets=input_video_path=<input video path>,output_video_path=<output video path>
    ```

    For example:

    ```bash
    $ ./bazel-bin/mediapipe/examples/desktop/object_detection/object_detection_tflite \
        --calculator_graph_config_file=mediapipe/graphs/object_detection/object_detection_desktop_tflite_graph.pbtxt \
        --input_side_packets=input_video_path=mediapipe/examples/desktop/object_detection/test_video.mp4,output_video_path=mediapipe/examples/desktop/object_detection/out.mp4
    ```
