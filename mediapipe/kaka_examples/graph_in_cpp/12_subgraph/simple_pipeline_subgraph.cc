#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/api2/builder.h"
#include "mediapipe/framework/api2/port.h"

namespace mediapipe {

using ::mediapipe::api2::Input;
using ::mediapipe::api2::Output;
using ::mediapipe::api2::builder::Graph;
using ::mediapipe::api2::builder::Stream;
using ::mediapipe::api2::AnyType;

// Example Graph:
//
// # This subgraph is defined in `two_pass_through_subgraph.pbtxt`
// # and is registered as "TwoPassThroughSubgraph"
//
// type: "TwoPassThroughSubgraph"
// input_stream: "out1"
// output_stream: "out3"
//
// node {
//     calculator: "PassThroughCalculator"
//     input_stream: "out1"
//     output_stream: "out2"
// }
// node {
//     calculator: "PassThroughCalculator"
//     input_stream: "out2"
//     output_stream: "out3"
// }
///////////////////////////////////////////////////////////////
// # This main graph is defined in `main_pass_throughcals.pbtxt"
// # using subgraph called "TwoPassThroughSubgraph"

// input_stream: "in"
// node {
//     calculator: "PassThroughCalculator"
//     input_stream: "in"
//     output_stream: "out1"
// }
// node {
//     calculator: "TwoPassThroughSubgraph"
//     input_stream: "out1"
//     output_stream: "out3"
// }
// node {
//     calculator: "PassThroughCalculator"
//     input_stream: "out3"
//     output_stream: "out4"
// }

///////////////////////////////////////////////////////////////////////////////
// 以下使用 CalculatorGraphConfig 構建圖形配置

// class TwoPassThroughSubgraph : public Subgraph {
//  public:
//   absl::StatusOr<CalculatorGraphConfig> GetConfig(
//       SubgraphContext* context) override {
//     CalculatorGraphConfig config;

//     // 定義輸入和輸出標籤
//     config.add_input_stream("out1");
//     config.add_output_stream("out3");

//     // 定義第一個 PassThroughCalculator 節點
//     auto* node1 = config.add_node();
//     node1->set_calculator("PassThroughCalculator");
//     node1->add_input_stream("out1");
//     node1->add_output_stream("out2");

//     // 定義第二個 PassThroughCalculator 節點
//     auto* node2 = config.add_node();
//     node2->set_calculator("PassThroughCalculator");
//     node2->add_input_stream("out2");
//     node2->add_output_stream("out3");

//     return config;
//   }
// };
// REGISTER_MEDIAPIPE_GRAPH(TwoPassThroughSubgraph);

// CalculatorGraphConfig BuildGraphConfig() {
//     mediapipe::CalculatorGraphConfig config;

//     // 定義輸入和輸出流
//     config.add_input_stream("in");
//     config.add_output_stream("out");

//     // 第一個 PassThroughCalculator 節點
//     auto* node1 = config.add_node();
//     node1->set_calculator("PassThroughCalculator");
//     node1->add_input_stream("in");
//     node1->add_output_stream("out1");

//     // 使用 TwoPassThroughSubgraph 子圖
//     auto* subgraph_node = config.add_node();
//     subgraph_node->set_calculator("TwoPassThroughSubgraph");
//     subgraph_node->add_input_stream("out1");
//     subgraph_node->add_output_stream("out3");

//     // 第三個 PassThroughCalculator 節點
//     auto* node4 = config.add_node();
//     node4->set_calculator("PassThroughCalculator");
//     node4->add_input_stream("out3");
//     node4->add_output_stream("out");

//     return config;
// }

///////////////////////////////////////////////////////////////////////////////
// 以下使用 Graph Builder API 構建圖形配置

class TwoPassThroughSubgraph : public Subgraph {
 public:
  absl::StatusOr<CalculatorGraphConfig> GetConfig(
      SubgraphContext* context) override {
    // 使用 Graph Builder API 構建子圖
    Graph graph;

    // Graph inputs
    auto in = graph.In(0).SetName("out1");

    // 定義第一個 PassThroughCalculator
    auto& node1 = graph.AddNode("PassThroughCalculator");
    in >> node1.In(0);
    auto out2 = node1.Out(0);

    // 定義第二個 PassThroughCalculator
    auto& node2 = graph.AddNode("PassThroughCalculator");
    out2 >> node2.In(0);
    node2.Out(0).SetName("out3") >> graph.Out(0);

    return graph.GetConfig();
  }
};
REGISTER_MEDIAPIPE_GRAPH(TwoPassThroughSubgraph);

CalculatorGraphConfig BuildGraphConfig() {
    // 使用 Graph Builder API
    Graph graph;

    // Graph inputs
    auto in= graph.In(0).SetName("in");

    // 添加四個 PassThroughCalculator 並連接
    auto& node1 = graph.AddNode("PassThroughCalculator");
    in >> node1.In(0);
    auto out1 = node1.Out(0);

    auto& node2 = graph.AddNode("TwoPassThroughSubgraph");
    out1 >> node2.In(0);
    auto out3 = node2.Out(0);

    auto& node4 = graph.AddNode("PassThroughCalculator");
    out3 >> node4.In(0);
    node4.Out(0).SetName("out") >> graph.Out(0);

    return graph.GetConfig();
}

absl::Status SimplePipeline() {
  CalculatorGraphConfig config = BuildGraphConfig();

  // Create MP Graph and intialize it with config
  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  // 設置輸出流
  LOG(INFO) << "Start running the calculator graph.";
  MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                      graph.AddOutputStreamPoller("out"));

  // 啟動圖形
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Send input packets to the graph, stream "input"
  for (int i = 0; i < 13; ++i) {
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in",
                       MakePacket<double>(i*0.1).At(Timestamp(i))));
  }
  // Close the input stream "input" to finish the graph run.
  // signal MP that no more packets will be sent to "input"
  MP_RETURN_IF_ERROR(graph.CloseInputStream("in"));

  // Get the output packets string.
  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    std::cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<double>() << std::endl;
  }

  // Wait for the graph to finish, and return graph status
  return graph.WaitUntilDone();
}
}  // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::Status status = mediapipe::SimplePipeline();
  if (!status.ok()) {
      LOG(ERROR) << "Failed to run the pipeline: " << status.message();
      return -1;
  }
  return 0;
}

