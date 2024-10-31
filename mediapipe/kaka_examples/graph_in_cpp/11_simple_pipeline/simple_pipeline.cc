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
// input_stream: "in"
// output_stream: "out"
// node {
//     calculator: "PassThroughCalculator"
//     input_stream: "in"
//     output_stream: "out1"
// }
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
// node {
//     calculator: "PassThroughCalculator"
//     input_stream: "out3"
//     output_stream: "out"
// }

///////////////////////////////////////////////////////////////////////////////
// 以下使用 CalculatorGraphConfig 構建圖形配置

// mediapipe::CalculatorGraphConfig BuildGraphConfig() {
//     mediapipe::CalculatorGraphConfig config;

//     // 定義輸入和輸出流
//     config.add_input_stream("in");
//     config.add_output_stream("out");

//     // 第一個 PassThroughCalculator 節點
//     auto* node1 = config.add_node();
//     node1->set_calculator("PassThroughCalculator");
//     node1->add_input_stream("in");
//     node1->add_output_stream("out1");

//     // 第二個 PassThroughCalculator 節點
//     auto* node2 = config.add_node();
//     node2->set_calculator("PassThroughCalculator");
//     node2->add_input_stream("out1");
//     node2->add_output_stream("out2");

//     // 第三個 PassThroughCalculator 節點
//     auto* node3 = config.add_node();
//     node3->set_calculator("PassThroughCalculator");
//     node3->add_input_stream("out2");
//     node3->add_output_stream("out3");

//     // 第四個 PassThroughCalculator 節點
//     auto* node4 = config.add_node();
//     node4->set_calculator("PassThroughCalculator");
//     node4->add_input_stream("out3");
//     node4->add_output_stream("out");

//     return config;
// }

///////////////////////////////////////////////////////////////////////////////
// 以下使用 Graph Builder API 構建圖形配置

// 寫法一: 使用 Stream 類型
// CalculatorGraphConfig BuildGraphConfig() {
//     // 使用 Graph Builder API
//     Graph graph;

//     // 定義輸入和輸出流
//     auto in= graph.In(0).SetName("in");
//     auto out= graph.Out(0);

//     // 添加四個 PassThroughCalculator 並連接
//     auto& node1 = graph.AddNode("PassThroughCalculator");
//     in >> node1.In(0);
//     auto out1 = node1.Out(0);

//     auto& node2 = graph.AddNode("PassThroughCalculator");
//     out1 >> node2.In(0);
//     auto out2 = node2.Out(0);

//     auto& node3 = graph.AddNode("PassThroughCalculator");
//     out2 >> node3.In(0);
//     auto out3 = node3.Out(0);

//     auto& node4 = graph.AddNode("PassThroughCalculator");
//     out3 >> node4.In(0);
//     node4.Out(0).SetName("out") >> out;

//     return graph.GetConfig();
// }

// 寫法二: 使用函數
CalculatorGraphConfig BuildGraphConfig() {
    // 使用 Graph Builder API
    Graph graph;

    // Graph inputs
    Stream<AnyType> in = graph.In(0).SetName("in");

    auto pass_through_fn = [](Stream<AnyType> in,
                            Graph& graph) -> Stream<AnyType> {
      auto& node = graph.AddNode("PassThroughCalculator");
      in.ConnectTo(node.In(0));
      return node.Out(0);
    };

    Stream<AnyType> out1 = pass_through_fn(in, graph);
    Stream<AnyType> out2 = pass_through_fn(out1, graph);
    Stream<AnyType> out3 = pass_through_fn(out2, graph);
    Stream<AnyType> out4 = pass_through_fn(out3, graph);

    // Graph outputs
    out4.SetName("out").ConnectTo(graph.Out(0));

    return graph.GetConfig();
}

// 運行影像處理管道
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

