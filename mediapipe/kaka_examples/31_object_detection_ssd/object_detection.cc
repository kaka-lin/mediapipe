// Here is a smapele that performs object detection with TensorFlow Lite on CPU.
// Model: SSD MobileNet v1

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

namespace mediapipe {

absl::Status ObjectDetection() {
  // Load the graph config from a file
  std::string graph_path("mediapipe/kaka_examples/31_object_detection_ssd/object_detection.pbtxt");
  std::string k_proto;
  MP_RETURN_IF_ERROR(file::GetContents(graph_path, &k_proto));

  // parse it
  CalculatorGraphConfig config;
  if (!ParseTextProto<CalculatorGraphConfig>(k_proto, &config)) {
    return absl::InternalError("Cannot parse the graph config !");
  }

  // Create MP Graph and intialize it with config
  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  // Output Streams
  LOG(INFO) << "Start running the calculator graph.";
  ASSIGN_OR_RETURN(OutputStreamPoller poller,
                   graph.AddOutputStreamPoller("output_video"));

  // Run the graph with `StartRun`,
  // it usually starts in parallel threads and waits for input data
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Start the camera and check that it works
  LOG(INFO) << "Open the camera !";
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
    // 1. convert BGR to RGB
    cv::cvtColor(frame_in, frame_in, cv::COLOR_BGR2RGB);
    // 2. Create an empty (black?) RGB ImageFrame with the same size as our image
    ImageFrame* inputFrame = new ImageFrame(
      ImageFormat::SRGB, frame_in.cols, frame_in.rows,
      ImageFrame::kDefaultAlignmentBoundary);
    // 3. Copy data from cv::Mat to Imageframe, using
    frame_in.copyTo(formats::MatView(inputFrame));

    // Create and send a video packet
    // `Adopt()` creates a new packet from a raw pointer, and takes this pointer under MP management.
    // So that you must not call delete on the pointer after that
    // MP will delete your object automatically when the packet is destroyed.
    // This is like creating `shared_ptr` from a raw pointer
    // This is useful for the classes which cannot be (easily) copied, like `ImageFrame`
    // Note that MakePacket<...>() we used previously contains a move or copy operation
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input_video",
                       Adopt(inputFrame).At(Timestamp(frame_timestamp_us))));

    // Get the graph result packet, or stop if that fails.
    mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    std::cout << packet.Timestamp() << ": RECEIVED VIDEO PACKET !" << std::endl;

    // Get data from packet (you should be used to this by now)
    auto& outputFrame = packet.Get<ImageFrame>();

    // Represent ImageFrame data as cv::Mat (MatView is a thin wrapper, no copying)
    cv::Mat frame_out = formats::MatView(&outputFrame);

    // Display
    cvtColor(frame_out, frame_out, cv::COLOR_RGB2BGR);
    cv::imshow("Image", frame_out);
    if (cv::waitKey(1) == 'q')
      grab_frames = false;
  }

  LOG(INFO) << "Shutting down.";
  MP_RETURN_IF_ERROR(graph.CloseInputStream("input_video"));
  return graph.WaitUntilDone();
}
} // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  std::cout << "Example : Object Detection" << std::endl;
  CHECK(mediapipe::ObjectDetection().ok());
  return 0;
}
