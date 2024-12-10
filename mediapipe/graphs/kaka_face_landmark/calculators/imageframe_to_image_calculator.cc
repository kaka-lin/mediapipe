#include <iostream>
#include <string>
#include <cstring>  // for std::memcpy

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image.h"
#include "mediapipe/framework/formats/image_frame.h"
// #include "mediapipe/framework/formats/image_frame_opencv.h"
// #include "mediapipe/framework/port/opencv_core_inc.h"
// #include "mediapipe/framework/port/opencv_imgproc_inc.h"
// #include "mediapipe/framework/port/status.h"
// #include "mediapipe/framework/timestamp.h"

namespace mediapipe {

class ImageFrameToImageCalculator : public CalculatorBase {
public:
  static absl::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Index(0).Set<ImageFrame>(); // 接收 ImageFrame 作為輸入
    cc->Outputs().Index(0).Set<Image>();     // 設定輸出類型為 mediapipe::Image
    return absl::OkStatus();
  }

  absl::Status Process(CalculatorContext* cc) override {
    // 獲取輸入的 ImageFrame
    const auto& input_frame = cc->Inputs().Index(0).Get<ImageFrame>();

    // 使用 shared_ptr 包裝 input_frame，並將其轉換為 mediapipe::Image
    auto frame_ptr = std::make_shared<ImageFrame>(
        input_frame.Format(), input_frame.Width(), input_frame.Height());
    std::memcpy(frame_ptr->MutablePixelData(), input_frame.PixelData(),
                input_frame.PixelDataSize());

    // 使用 shared_ptr<ImageFrame> 構造 mediapipe::Image
    mediapipe::Image output_image(frame_ptr);

    // 將 mediapipe::Image 添加到輸出流
    cc->Outputs().Index(0).AddPacket(MakePacket<mediapipe::Image>(output_image).At(cc->InputTimestamp()));

    return absl::OkStatus();
  }
};

REGISTER_CALCULATOR(ImageFrameToImageCalculator);

}  // namespace mediapipe
