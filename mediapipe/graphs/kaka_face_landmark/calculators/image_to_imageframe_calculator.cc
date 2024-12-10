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

class ImageToImageFrameCalculator : public CalculatorBase {
public:
  static absl::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Index(0).Set<Image>();         // 接收 Image 作為輸入
    cc->Outputs().Index(0).Set<ImageFrame>();   // 輸出 ImageFrame
    return absl::OkStatus();
  }

  absl::Status Process(CalculatorContext* cc) override {
    // 獲取輸入的 Image
    const auto& input_image = cc->Inputs().Index(0).Get<Image>();

    // 獲取 Image 中的 ImageFrame 指針
    std::shared_ptr<ImageFrame> frame_ptr = input_image.GetImageFrameSharedPtr();

    // 構造新的 ImageFrame 以避免共享
    auto output_frame = std::make_unique<ImageFrame>(
        frame_ptr->Format(), frame_ptr->Width(), frame_ptr->Height());

    // 將數據從原始 ImageFrame 複製到新的 ImageFrame 中
    std::memcpy(output_frame->MutablePixelData(), frame_ptr->PixelData(),
                frame_ptr->PixelDataSize());

    // 將新創建的 ImageFrame 添加到輸出流
    cc->Outputs().Index(0).Add(output_frame.release(), cc->InputTimestamp());

    return absl::OkStatus();
  }
};
REGISTER_CALCULATOR(ImageToImageFrameCalculator);

}  // namespace mediapipe
