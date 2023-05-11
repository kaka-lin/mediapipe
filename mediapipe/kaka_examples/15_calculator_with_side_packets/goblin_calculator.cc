#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {
  // A calculator with input and output side packets
  class GoblinCalculator : public CalculatorBase {
   public:
    static Status GetContract(CalculatorContract* cc) {
      // specify a calculator with 1 input, 1 output, both of type double
      cc->Inputs().Index(0).Set<double>();
      cc->Outputs().Index(0).Set<double>();
      // Specify side packets
      cc->InputSidePackets().Index(0).Set<double>();
      cc->InputSidePackets().Index(1).Set<double>();
      return absl::OkStatus();
    }

    // Open method is executed once when starting the graph
    // "It can be used to parse input side packets, they are already available"
    Status Open(CalculatorContext* cc) override {
      // Extract "a" and "b" from input side packets
      Packet p1 = cc->InputSidePackets().Index(0);
      Packet p2 = cc->InputSidePackets().Index(1);
      a = p1.Get<double>();
      b = p2.Get<double>();
      std::cout << "GoblinCalculator::Open() : a = " << a << ", b = " << b << std::endl;
      return OkStatus();
    }

    Status Process(CalculatorContext* cc) override {
      // Receive the input packet
      Packet p_in = cc->Inputs().Index(0).Value();

      // Extract the double number
      // A Packet is immutable, so we cannot edit it!
      double x = p_in.Get<double>();
      // Process the number
      double y = x * a + b;

      // Create the output packet, with the input timestamp
      Packet p_out = MakePacket<double>(y).At(cc->InputTimestamp());
      // Send it to the output stream
      cc->Outputs().Index(0).AddPacket(p_out);
      return absl::OkStatus(); // Never forget to say "OK"!
    }

   private:
    double a=2, b=0;
  };

  REGISTER_CALCULATOR(GoblinCalculator);
}
