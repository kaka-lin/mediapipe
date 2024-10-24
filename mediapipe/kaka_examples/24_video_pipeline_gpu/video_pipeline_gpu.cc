// Here we use OpenCV, and MediaPipe has its own wrappers for opencv headers (How stupid is this?)
// MediaPipe uses an mutable class `ImageFrame` for images
// One must convert `cv::Mat <-> ImageFrame` back and forth
// Note that cv::Mat uses BGR by default, while ImageFrame uses RGB by default
// We stick to this convention and put cv::cvtColor when needed
// This example requires a GPU with EGL support drivers.
#include <iostream>
#include <string>
#include <memory>
#include <atomic>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/file_helpers.h"

#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gpu_shared_data_internal.h"

namespace mediapipe {

absl::Status VideoPipeline() {
  // Create MediaPipe graph as protobuf text format
  std::string k_proto = R"pb(
    input_stream: "input"
    output_stream: "output"
    node {
      calculator: "PassThroughCalculator",
      input_stream: "input",
      output_stream: "output",
    }
  )pb";

  // Parse this string into a protobuf CalculatorGraphConfig object
  CalculatorGraphConfig config;
  if (!ParseTextProto<mediapipe::CalculatorGraphConfig>(k_proto, &config)) {
    return absl::InternalError("Cannot parse the graph config !");
  }

  // Create MP Graph and intialize it with config
  LOG(INFO) << "Initialize the calculator graph.";
  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  // Initialize GPU
  LOG(INFO) << "Initialize the GPU.";
  MP_ASSIGN_OR_RETURN(auto gpu_resources, mediapipe::GpuResources::Create());
  MP_RETURN_IF_ERROR(graph.SetGpuResources(std::move(gpu_resources)));
  mediapipe::GlCalculatorHelper gpu_helper;
  gpu_helper.InitializeForTest(graph.GetGpuResources().get());

  // Output packets
  LOG(INFO) << "Start running the calculator graph.";
  MP_ASSIGN_OR_RETURN(OutputStreamPoller poller,
                   graph.AddOutputStreamPoller("output"));

  // Run the graph with `StartRun`,
  // it usually starts in parallel threads and waits for input data
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Start the camera and check that it works
  cv::VideoCapture cap(0);
  if (!cap.isOpened())
    return absl::NotFoundError("CANNOT OPEN CAMERA !");

  // Endless loop over frames
  cv::Mat frame_in;
  bool grab_frames = true;
  while (grab_frames) {
    cap.read(frame_in);
    if (frame_in.empty())
      return absl::NotFoundError("ERROR! blank frame grabbed !");

    // convert cv::Mat to ImageFrame
    // 1. convert BGR to RGBA
    cv::Mat camera_frame;
    cv::cvtColor(frame_in, camera_frame, cv::COLOR_BGR2RGBA);
    // 2. Create an empty (black?) RGBA ImageFrame with the same size as our image
    auto inputFrame = absl::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGBA, camera_frame.cols, camera_frame.rows,
        mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
    // 3. Copy data from cv::Mat to Imageframe, using
    cv::Mat camera_frame_mat = formats::MatView(inputFrame.get());
    camera_frame.copyTo(camera_frame_mat);

    // Prepare and add graph input packet.
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(gpu_helper.RunInGlContext(
        [&inputFrame, &frame_timestamp_us, &graph, &gpu_helper]() -> absl::Status {
          // Convert ImageFrame to GpuBuffer.
          auto texture = gpu_helper.CreateSourceTexture(*inputFrame.get());
          auto gpu_frame = texture.GetFrame<mediapipe::GpuBuffer>();
          glFlush();
          texture.Release();
          // Send GPU image packet into the graph.
          MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input",
                             Adopt(gpu_frame.release()).At(Timestamp(frame_timestamp_us))));
          return absl::OkStatus();
        }));

    // Get the graph result packet, or stop if that fails.
    mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    std::cout << packet.Timestamp() << ": RECEIVED VIDEO PACKET !" << std::endl;
    std::unique_ptr<ImageFrame> outputFrame;

    // Convert GpuBuffer to ImageFrame.
    MP_RETURN_IF_ERROR(gpu_helper.RunInGlContext(
        [&packet, &outputFrame, &gpu_helper]() -> absl::Status {
          auto& gpu_frame = packet.Get<mediapipe::GpuBuffer>();
          auto texture = gpu_helper.CreateSourceTexture(gpu_frame);
          outputFrame = absl::make_unique<mediapipe::ImageFrame>(
              mediapipe::ImageFormatForGpuBufferFormat(gpu_frame.format()),
              gpu_frame.width(), gpu_frame.height(),
              mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
          gpu_helper.BindFramebuffer(texture);
          const auto info = mediapipe::GlTextureInfoForGpuBufferFormat(
              gpu_frame.format(), 0, gpu_helper.GetGlVersion());
          glReadPixels(0, 0, texture.width(), texture.height(), info.gl_format,
                       info.gl_type, outputFrame->MutablePixelData());
          glFlush();
          texture.Release();
          return absl::OkStatus();
        }));

    // Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = mediapipe::formats::MatView(outputFrame.get());
    if (output_frame_mat.channels() == 4)
      cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGBA2BGR);
    else
      cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);

    cv::imshow("Image", output_frame_mat);
    const int pressed_key = cv::waitKey(1);
    if (pressed_key == 'q')
      grab_frames = false;
  }

  LOG(INFO) << "Shutting down.";
  MP_RETURN_IF_ERROR(graph.CloseInputStream("input"));
  return graph.WaitUntilDone();
}
} // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  std::cout << "Example : Video pipeline" << std::endl;
  CHECK(mediapipe::VideoPipeline().ok());
  return 0;
}
