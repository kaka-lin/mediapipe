#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/api2/builder.h"
#include "mediapipe/framework/api2/port.h"

namespace mediapipe {

using ::mediapipe::api2::Input;
using ::mediapipe::api2::Output;
using ::mediapipe::api2::SideInput;
using ::mediapipe::api2::SideOutput;
using ::mediapipe::api2::builder::Graph;

class ImageFrameToGrayCalculator : public CalculatorBase {
 public:
  static absl::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag("IMAGE").Set<ImageFrame>();
    cc->Outputs().Tag("GRAY").Set<ImageFrame>();
    return absl::OkStatus();
  }

  absl::Status Process(CalculatorContext* cc) override {
    const auto& input_frame = cc->Inputs().Tag("IMAGE").Get<ImageFrame>();

    // 將 ImageFrame 轉換為 OpenCV Mat
    cv::Mat input_mat = formats::MatView(&input_frame);
    cv::Mat gray_mat;
    cv::cvtColor(input_mat, gray_mat, cv::COLOR_RGB2GRAY);

    // 將灰階影像轉回 ImageFrame 並輸出
    auto output_frame = absl::make_unique<ImageFrame>(
       ImageFormat::GRAY8, gray_mat.cols, gray_mat.rows,
       ImageFrame::kDefaultAlignmentBoundary);

    cv::Mat output_mat = formats::MatView(output_frame.get());
    gray_mat.copyTo(output_mat);

    cc->Outputs().Tag("GRAY").Add(output_frame.release(), cc->InputTimestamp());
    return absl::OkStatus();
  }
};
REGISTER_CALCULATOR(ImageFrameToGrayCalculator);

// mediapipe::CalculatorGraphConfig BuildGraphConfig() {
//   mediapipe::CalculatorGraphConfig config;

//   // 定義影像輸入串流
//   config.add_input_stream("IMAGE:input_video");

//   // 定義灰階轉換計算節點
//   auto* grayscale_node = config.add_node();
//   grayscale_node->set_calculator("ImageFrameToGrayCalculator");
//   grayscale_node->add_input_stream("IMAGE:input_video");
//   grayscale_node->add_output_stream("GRAY:output_video");

//   // 定義影像輸出串流
//   config.add_output_stream("GRAY:output_video");

//   return config;
// }

CalculatorGraphConfig BuildGraphConfig() {
  // 使用 Graph Builder API
  Graph graph;

  // 定義影像輸入串流
  auto input_tensors = graph.In(0).SetName("input_video").Cast<mediapipe::ImageFrame>();

  // 定義灰階轉換計算節點
  auto& grayscale_node = graph.AddNode("ImageFrameToGrayCalculator");
  input_tensors >> grayscale_node.In("IMAGE");

  // 連接灰階輸出的串流
  auto output_tensors = grayscale_node.Out("GRAY").Cast<mediapipe::ImageFrame>();
  output_tensors.SetName("output_video").ConnectTo(graph.Out(0));

  // 返回構建的圖形配置
  return graph.GetConfig();
}

// 運行影像處理管道
absl::Status RunVideoPipeline() {
  CalculatorGraphConfig config = BuildGraphConfig();

  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  // 設置輸出流
  LOG(INFO) << "Start running the calculator graph.";
  MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                      graph.AddOutputStreamPoller("output_video"));

  // 啟動圖形
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // 啟動相機
  LOG(INFO) << "Open the camera !";
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    return absl::NotFoundError("CANNOT OPEN CAMERA !");
  }

  // 循環讀取影像幀並處理
  cv::Mat frame_in;
  bool grab_frames = true;
  while (grab_frames) {
    cap.read(frame_in);
    if (frame_in.empty())
      return absl::NotFoundError("ERROR! blank frame grabbed !");

    // 將 BGR 轉為 RGB 格式
    cv::cvtColor(frame_in, frame_in, cv::COLOR_BGR2RGB);

    // 創建 ImageFrame 並拷貝數據
    ImageFrame* input_frame = new ImageFrame(
        ImageFormat::SRGB, frame_in.cols, frame_in.rows,
        ImageFrame::kDefaultAlignmentBoundary);
    frame_in.copyTo(formats::MatView(input_frame));

    // 發送影像幀到圖形
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input_video",
                       Adopt(input_frame).At(Timestamp(frame_timestamp_us))));

    // 獲取灰階處理結果
    mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    std::cout << packet.Timestamp() << ": RECEIVED VIDEO PACKET !" << std::endl;

    // Get data from packet (you should be used to this by now)
    auto& output_frame = packet.Get<ImageFrame>();

    // 將 ImageFrame 轉換回 OpenCV Mat 並顯示
    cv::Mat frame_out = formats::MatView(&output_frame);

    // 顯示灰階影像
    cv::imshow("Grayscale Video", frame_out);
    if (cv::waitKey(1) == 'q')
      grab_frames = false;
  }

  LOG(INFO) << "Shutting down.";
  MP_RETURN_IF_ERROR(graph.CloseInputStream("input_video"));
  return graph.WaitUntilDone();
}
}  // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::Status status = mediapipe::RunVideoPipeline();
  if (!status.ok()) {
      LOG(ERROR) << "Failed to run the pipeline: " << status.message();
      return -1;
  }
  return 0;
}

