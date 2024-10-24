#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {

absl::Status SimplePipeline() {
  // Configures a simple graph, which concatenates 2 PassThroughCalculators.
  // First, we have to create MP graph as protobuf text format
  std::string k_proto = R"pb(
    input_stream: "input"
    output_stream: "output"
    node {
      calculator: "PassThroughCalculator"
      input_stream: "input"
      output_stream: "output1"
    }
    node {
      calculator: "PassThroughCalculator"
      input_stream: "output1"
      output_stream: "output"
    }
  )pb";

  // Next, parse this string into a protobuf CalculatorGraphConfig object
  CalculatorGraphConfig config =
    ParseTextProtoOrDie<CalculatorGraphConfig>(k_proto);

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
  std::cout << "Example 1.1 : Simplest mediapipe pipeline ..." << std::endl;
  CHECK(mediapipe::SimplePipeline().ok());
  return 0;
}
