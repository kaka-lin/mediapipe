#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/timestamp.h"

#include "mediapipe/kaka_examples/23_video_pipeline_scale/kimage_scale_calculator.pb.h"

// All calculators must be in namespace mediapipe
namespace mediapipe {

constexpr char kImageFrameTag[] = "IMAGE";

class KImageScaleCalculator : public CalculatorBase {
 public:
  KImageScaleCalculator();
  ~KImageScaleCalculator() override;

  static Status GetContract(CalculatorContract* cc);

  Status Open(CalculatorContext* cc) override;
  Status Process(CalculatorContext* cc) override;
  // Status Close(CalculatorContext* cc) override;

 private:
  Status ResizeImage(CalculatorContext* cc);

  KImageScaleCalculatorOptions options_;
  int output_width_ = 0;
  int output_height_ = 0;
};

// We must register our calculator with MP, so that it can be used in graphs
REGISTER_CALCULATOR(KImageScaleCalculator);

KImageScaleCalculator::KImageScaleCalculator() {}
KImageScaleCalculator::~KImageScaleCalculator() {}

// 計算器的 GetContract() 函數，定義計算器的輸入輸出契約
absl::Status KImageScaleCalculator::GetContract(CalculatorContract* cc) {
  // specify a calculator with 1 input, 1 output
  RET_CHECK(cc->Outputs().HasTag(kImageFrameTag));
  cc->Inputs().Tag(kImageFrameTag).Set<ImageFrame>();
  cc->Outputs().Tag(kImageFrameTag).Set<ImageFrame>();
  return absl::OkStatus(); // Never forget to say "OK"!
}

// 計算器的初始化函數
absl::Status KImageScaleCalculator::Open(CalculatorContext* cc) {
  // Inform the framework that we always output at the same timestamp
  // as we receive a packet at.
  cc->SetOffset(TimestampDiff(0));

  // 從參數中讀取常數
  options_ = cc->Options<KImageScaleCalculatorOptions>();
  output_width_ = options_.output_width();
  output_height_ = options_.output_height();
  return absl::OkStatus();
}

// 計算器的處理函數，將輸入加上常數並輸出結果
absl::Status KImageScaleCalculator::Process(CalculatorContext* cc) {
  if (cc->Inputs().Tag(kImageFrameTag).IsEmpty()) {
    return absl::OkStatus();
  }

  return ResizeImage(cc);
  return absl::OkStatus();
}

absl::Status KImageScaleCalculator::ResizeImage(CalculatorContext* cc) {
  cv::Mat input_mat, scaled_mat;;
  mediapipe::ImageFormat::Format format;

  const auto& input = cc->Inputs().Tag(kImageFrameTag).Get<ImageFrame>();
  // From Imageframe to cv::Mat
  input_mat = formats::MatView(&input);
  format = input.Format();

  const int input_width = input_mat.cols;
  const int input_height = input_mat.rows;
  int output_width = output_width_;
  int output_height = output_height_;

  // Resize !!!
  int scale_flag =
      input_width > output_width && input_height > output_height
          ? cv::INTER_AREA : cv::INTER_LINEAR;
  cv::resize(input_mat, scaled_mat, cv::Size(output_width, output_height),
             0, 0, scale_flag);

  // Create output frame from scaled_mat
  std::unique_ptr<ImageFrame> output_frame(
      new ImageFrame(format, output_width, output_height));
  // Represent ImageFrame data as cv::Mat (MatView is a thin wrapper, no copying)
  // And copy scaled_mat to output_frame
  cv::Mat output_mat = formats::MatView(output_frame.get());
  scaled_mat.copyTo(output_mat);
  cc->Outputs().Tag(kImageFrameTag).Add(output_frame.release(), cc->InputTimestamp());

  return absl::OkStatus();
}

} // namespace mediapipe
