# Simple Pipeline

## Usage

```bash
$ bazel build --define MEDIAPIPE_DISABLE_GPU=1 \
    mediapipe/kaka_examples/11_simple_pipeline:simple_pipeline
```

```bash
$ ./bazel-bin/mediapipe/kaka_examples/11_simple_pipeline/simple_pipeline
```