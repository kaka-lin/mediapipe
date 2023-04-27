// Here I create a calculator to join two strings,
// also I introduce a source node (a calculator with no inputs)

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {

absl::Status RunMPPGraph() {
  // Create MP graph as protobuf text format
  std::string k_proto = R"pb(
    input_stream: "input"
    output_stream: "output"
    input_side_packet : "a"
    input_side_packet : "b"
    node {
      calculator: "GoblinCalculator"
      input_side_packet : "a"
      input_side_packet : "b"
      input_stream: "input"
      output_stream: "output"
    }
  )pb";

  // parse it
  CalculatorGraphConfig config;
  if (!ParseTextProto<CalculatorGraphConfig>(k_proto, &config)) {
    return absl::InternalError("Cannot parse the graph config !");
  }

  // Create MP Graph and intialize it with config
  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  // Output streams
  ASSIGN_OR_RETURN(OutputStreamPoller poller,
                   graph.AddOutputStreamPoller("output"));

  // Input side packets
  Packet side_a = MakePacket<double>(7.0);
  Packet side_b = MakePacket<double>(3.0);
  // Run the graph, supplying side packets at this point!
  // absl::Status StartRun(const std::map<std::string, Packet>& extra_side_packets,
  //                       const std::map<std::string, Packet>& stream_headers);
  MP_RETURN_IF_ERROR(graph.StartRun({{"a", side_a}, {"b", side_b}}));

  // Send input packets to the graph, stream "input", then close it.
  for (int i = 0; i < 13; ++i) {
    // MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input",
    //                    MakePacket<double>(i*0.1).At(Timestamp(i))));
    mediapipe::Timestamp ts(i);
    mediapipe::Packet packet = mediapipe::MakePacket<double>(i*0.1).At(ts);
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input", packet));
  }
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
  std::cout << "Example : Calculator with side packet" << std::endl;
  CHECK(mediapipe::RunMPPGraph().ok());
  return 0;
}
