#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {
  // This calculator joins two strings
  class StringJoinCalculator : public CalculatorBase {
   public:
    static Status GetContract(CalculatorContract* cc) {
      // When calculator has more than one input or output, "anonymous streams" become messy.
      // So we use named streams. Here we use `tag+number` to identify the stream.
      // Two inputs are named "STR:0:" and "STR:1:" and one output is named "STR:".
      cc->Inputs().Get("STR", 0).Set<std::string>();
      cc->Inputs().Get("STR", 1).Set<std::string>();
      cc->Outputs().Tag("STR").Set<std::string>();
      return absl::OkStatus(); // Never forget to say "OK"!
    }

    Status Process(CalculatorContext* cc) override {
      // Receive the two input packets
      Packet p_in_1 = cc->Inputs().Get("STR", 0).Value();
      Packet p_in_2 = cc->Inputs().Get("STR", 1).Value();

      // Extract strings, if the packet is not empty.
      // Otherwise, use "<EMPTY>" as a placeholder (default value).
      // MediaPipe automatically synchronizes streams by timestamps
      //
      // If, say, timestamp 12 is missing in one stream, the packet will be empty
      // At least one input packet is always nonempty
      std::string s1("<EMPTY>"), s2("<EMPTY>");
      if (!p_in_1.IsEmpty())
        s1 = p_in_1.Get<std::string>();
      if (!p_in_2.IsEmpty())
        s2 = p_in_2.Get<std::string>();

      // Join the two strings
      std::string s = s1 + s2;

      // Create the output packet and send
      Packet p_out = MakePacket<std::string>(s).At(cc->InputTimestamp());
      cc->Outputs().Tag("STR").AddPacket(p_out);
      return absl::OkStatus(); // Never forget to say "OK"!
    }
  };

  REGISTER_CALCULATOR(StringJoinCalculator);
}
