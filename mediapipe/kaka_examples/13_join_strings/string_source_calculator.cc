#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {
  // This is a source: a calcuator that has no inputs.
  class StringSourceCalculator : public CalculatorBase {
   public:
    static Status GetContract(CalculatorContract* cc) {
      // A single output "STR:" of type sdt::string
      cc->Outputs().Tag("STR").Set<std::string>();
      return absl::OkStatus(); // Never forget to say "OK"!
    }

    Status Process(CalculatorContext* cc) override {
      // This stream sends exactly 17 packets with timestamps 0 .. 16
      // Then sends the "STOP" status instead of "OK" to signal that the stream is finished
      // This works like CloseInputStream()
      if (t >= 17)
        return tool::StatusStop();

      // Construct the string and send
      std::string s = "JESSICA: " + std::to_string(t);
      Packet p = MakePacket<std::string>(s).At(Timestamp(t));
      cc->Outputs().Tag("STR").AddPacket(p);

      // Increment the counter
      t++;
      return absl::OkStatus(); // Never forget to say "OK"!
    }

   private:
    // A counter, used to create ascending timestamp
    int t = 0;
  };

  REGISTER_CALCULATOR(StringSourceCalculator);
}
