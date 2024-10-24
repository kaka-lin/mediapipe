#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/kaka_examples/12_custom_calculator_2/addconstant_calculator.pb.h"

// All calculators must be in namespace mediapipe
namespace mediapipe {
  // 自定義計算器，實現將輸入的整數加上一個常數的功能
  class AddConstantCalculator : public CalculatorBase {
   public:
    // 計算器的輸入和輸出流
    static constexpr char kInputTag[] = "INPUT";
    static constexpr char kOutputTag[] = "OUTPUT";

    // 計算器的參數
    static constexpr char kConstantTag[] = "CONSTANT";
    int constant_;

    // 計算器的 GetContract() 函數，定義計算器的輸入輸出契約
    static Status GetContract(CalculatorContract* cc) {
      // specify a calculator with 1 input, 1 output, both of type double
      cc->Inputs().Tag(kInputTag).Set<int>();
      cc->Outputs().Tag(kOutputTag).Set<int>();
      return absl::OkStatus(); // Never forget to say "OK"!
    }

    // 計算器的初始化函數
    Status Open(CalculatorContext* cc) override {
      // 從參數中讀取常數
      auto options = cc->Options<AddConstantCalculatorOptions>();
      constant_ = options.constant();
      return absl::OkStatus();
    }

    // 計算器的處理函數，將輸入加上常數並輸出結果
    Status Process(CalculatorContext* cc) override {
      if (cc->Inputs().Tag(kInputTag).IsEmpty()) {
        return absl::OkStatus();
      }

      const int input = cc->Inputs().Tag(kInputTag).Get<int>();
      const int output = input + constant_;
      Packet p_out = MakePacket<int>(output).At(cc->InputTimestamp());
      cc->Outputs().Tag(kOutputTag).AddPacket(p_out);
      return absl::OkStatus();
    }
  };

  // We must register our calculator with MP, so that it can be used in graphs
  REGISTER_CALCULATOR(AddConstantCalculator);

absl::Status SimplePipeline() {
  // Configures a simple graph, which concatenates 2 PassThroughCalculators.
  // First, we have to create MP graph as protobuf text format
  std::string k_proto = R"pb(
    input_stream: "input"
    output_stream: "output"
    node {
      calculator: "AddConstantCalculator"
      input_stream: "INPUT:input"
      output_stream: "OUTPUT:output"
      options {
        [mediapipe.AddConstantCalculatorOptions.ext] {
          constant: 10
        }
      }
    }
  )pb";

  // Next, parse this string into a protobuf CalculatorGraphConfig object
  CalculatorGraphConfig config;
  if (!ParseTextProto<CalculatorGraphConfig>(k_proto, &config)) {
    // mediapipe::Status is actually absl::Status (at least in the current mediapipe)
    // So we can create BAD statuses like this
    return absl::InternalError("Cannot parse the graph config !");
  }

  // Create MP Graph and intialize it with config
  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  ////////////////////////////////////////////////////////////////////////
  // Input packets and Output packets
  //
  // How do you recieve output packets from the graph (stream "output")?
  // 1. OutputStreamPoller (synchronous logic)
  // 2. ObserveOutputStream (a callback, asynchronous logic)
  //
  // Create output stream "output"
  // OutputStreamPoller object is connected to the output stream
  // in order to later retrieve the graph output
  MP_ASSIGN_OR_RETURN(OutputStreamPoller poller,
                   graph.AddOutputStreamPoller("output"));

  // Run the graph with `StartRun`,
  // it usually starts in parallel threads and waits for input data
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Send input packets to the graph, stream "input"
  for (int i = 0; i < 13; ++i) {
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input",
                       MakePacket<int>(i).At(Timestamp(i))));
  }
  // Close the input stream "input" to finish the graph run.
  // signal MP that no more packets will be sent to "input"
  MP_RETURN_IF_ERROR(graph.CloseInputStream("input"));

  // Get the output packets string.
  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    std::cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<int>() << std::endl;
  }

  // Wait for the graph to finish, and return graph status
  // = `MP_RETURN_IF_ERROR(graph.WaitUntilDone())` + `return mediapipe::OkStatus()`
  return graph.WaitUntilDone();
}
} // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  std::cout << "Example 1.2.2 : Custom calculator..." << std::endl;
  CHECK(mediapipe::SimplePipeline().ok());
  return 0;
}
