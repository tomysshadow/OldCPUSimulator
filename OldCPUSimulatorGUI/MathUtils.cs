using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace OldCPUSimulatorGUI {
    public static class MathUtils {
        public static uint Clamp(uint number, uint min, uint max) {
            return Math.Min(max, Math.Max(min, number));
        }
    }
}
