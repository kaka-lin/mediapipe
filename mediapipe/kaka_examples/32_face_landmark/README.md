# Face Landmark

## Usage

```bash
$ bazel build --define MEDIAPIPE_DISABLE_GPU=1 \
    mediapipe/kaka_examples/32_face_landmark:face_landmark_cpu
```

```bash
$ ./bazel-bin/mediapipe/kaka_examples/32_face_landmark/face_landmark_cpu \
    --calculator_graph_config_file mediapipe/graphs/kaka_face_landmark/face_landmark_desktop_live.pbtxt
```
