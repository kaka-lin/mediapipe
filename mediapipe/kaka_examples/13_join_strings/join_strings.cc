// Here I create a calculator to join two strings,
// also I introduce a source node (a calculator with no inputs)

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {

absl::Status JoinStrings() {
  // Load the graph config from a file
  std::string graph_path("mediapipe/kaka_examples/13_join_strings/k_graph.pbtxt");
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

  // Output streams
  MP_ASSIGN_OR_RETURN(OutputStreamPoller poller,
                   graph.AddOutputStreamPoller("output"));

  // Run the graph
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Send input packets to the graph, stream "input", then close it.
  // Here I introduce a "time shift" tTshift for "input" timestamps, always 15 packets
  // But timestamps of stream "gen" (generated by StringSourceCalculator)
  // always start with 0, and there are 17 packets
  // How does MP sync two streams? Try it and find out!
  int tShift = -2;
  for (int i = 0; i < 15; ++i) {
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("input",
                       MakePacket<std::string>("BRIANNA: " + std::to_string(i) + ", ").At(Timestamp(i + tShift))));
  }
  // Close the input stream "input" to finish the graph run.
  // signal MP that no more packets will be sent to "input"
  MP_RETURN_IF_ERROR(graph.CloseInputStream("input"));

  // Get the output packets string.
  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    std::cout << packet.Timestamp() << ": RECEIVED PACKET - " << packet.Get<std::string>() << std::endl;
  }

  // Wait for the graph to finish, and return graph status
  // = `MP_RETURN_IF_ERROR(graph.WaitUntilDone())` + `return mediapipe::OkStatus()`
  return graph.WaitUntilDone();
}
} // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  std::cout << "Example 1.3 : Joining strings, synchronization, source" << std::endl;
  CHECK(mediapipe::JoinStrings().ok());
  return 0;
}
