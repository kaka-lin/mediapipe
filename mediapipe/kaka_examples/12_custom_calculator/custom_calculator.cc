#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

// All calculators must be in namespace mediapipe
namespace mediapipe {
  // first calculator: This calculator processes a double number
  class GoblinCalculator : public CalculatorBase {
   public:
    // Every calculator must implement the static GetContract() method
    // Which defines allowed inputs and outputs
    static Status GetContract(CalculatorContract* cc) {
      // specify a calculator with 1 input, 1 output, both of type double
      cc->Inputs().Index(0).Set<double>();
      cc->Outputs().Index(0).Set<double>();
      return absl::OkStatus(); // Never forget to say "OK"!
    }

    // The method Process() receives input packets and emits output packets
    // It is called for every timestamp for which input packets are available
    // Synchronization. MediaPiPe automatically synchronizes streams
    // If there are several input streams, all input packets have the same timestamp
    // All operations are performed through the CalculatorContext object
    Status Process(CalculatorContext* cc) override {
      // Receive the input packet
      Packet p_in = cc->Inputs().Index(0).Value();

      // Extract the double number
      // A Packet is immutable, so we cannot edit it!
      double x = p_in.Get<double>();
      // Process the number
      double y = x * 2;

      // Create the output packet, with the input timestamp
      Packet p_out = MakePacket<double>(y).At(cc->InputTimestamp());
      // Send it to the output stream
      cc->Outputs().Index(0).AddPacket(p_out);
      return absl::OkStatus(); // Never forget to say "OK"!
    }
  };

  // We must register our calculator with MP, so that it can be used in graphs
  REGISTER_CALCULATOR(GoblinCalculator);

absl::Status SimplePipeline() {
  // Configures a simple graph, which concatenates 2 PassThroughCalculators.
  // First, we have to create MP graph as protobuf text format
  std::string k_proto = R"pb(
    input_stream: "input"
    output_stream: "output"
    node {
      calculator: "GoblinCalculator"
      input_stream: "input"
      output_stream: "output"
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
                       MakePacket<double>(i*0.1).At(Timestamp(i))));
  }
  // Close the input stream "input" to finish the graph run.
  // signal MP that no more packets will be sent to "input"
  MP_RETURN_IF_ERROR(graph.CloseInputStream("input"));

  // Get the output packets string.
  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    std::cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<double>() << std::endl;
  }

  // Wait for the graph to finish, and return graph status
  // = `MP_RETURN_IF_ERROR(graph.WaitUntilDone())` + `return mediapipe::OkStatus()`
  return graph.WaitUntilDone();
}
} // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  std::cout << "Example 1.2 : First custom calculator..." << std::endl;
  CHECK(mediapipe::SimplePipeline().ok());
  return 0;
}
