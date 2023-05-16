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
  std::string scale_mode_ = "normal";
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
  scale_mode_ = options_.scale_mode();
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
  std::string scale_mode = scale_mode_;

  ///////////////////////////////////////////////////////////////////////////
  // Resize !!!
  if (scale_mode == "letterbox") {
    // Resize with keep aspect ratio
    const float scale = std::min(static_cast<float>(output_width) / input_width,
                                 static_cast<float>(output_height) / input_height);
    const int target_width = std::round(input_width * scale);
    const int target_height = std::round(input_height * scale);

    int scale_flag = scale < 1.0f ? cv::INTER_AREA : cv::INTER_LINEAR;

    cv::Mat intermediate_mat;
    cv::resize(input_mat, intermediate_mat,
               cv::Size(target_width, target_height), 0, 0, scale_flag);

    // Put the image that after resized into the center of new image
    const int top = (output_height - target_height) / 2;
    const int bottom = output_height - target_height - top;
    const int left = (output_width - target_width) / 2;
    const int right = output_width - target_width - left;
    cv::copyMakeBorder(intermediate_mat, scaled_mat, top, bottom, left, right,
                       cv::BORDER_CONSTANT, CV_RGB(0, 0, 0));
  } else {
    // Resize without keep aspect ratio
    int scale_flag =
        input_width > output_width && input_height > output_height
            ? cv::INTER_AREA : cv::INTER_LINEAR;
    cv::resize(input_mat, scaled_mat, cv::Size(output_width, output_height),
               0, 0, scale_flag);
  }
  ////////////////////////////////////////////////////////////////////////////

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
