import random
import time
from enum import Enum


# ============================================================
# GRIDGUARD
# Intelligent Electrical Grid Protection Simulator
# ============================================================


class FaultType(Enum):
    NORMAL = "NORMAL"
    OVERCURRENT = "OVERCURRENT"
    UNDERVOLTAGE = "UNDERVOLTAGE"
    OVERVOLTAGE = "OVERVOLTAGE"
    TRANSFORMER_OVERHEAT = "TRANSFORMER OVERHEAT"
    PHASE_IMBALANCE = "PHASE IMBALANCE"
    SHORT_CIRCUIT = "SHORT CIRCUIT"


# ============================================================
# TRANSFORMER
# ============================================================

class Transformer:

    def __init__(self, rated_voltage=11.0, rated_current=100.0):
        self.rated_voltage = rated_voltage
        self.rated_current = rated_current
        self.temperature = 25.0

    def update_temperature(self, current):

        load_ratio = current / self.rated_current

        if load_ratio > 1.0:
            self.temperature += (load_ratio - 1.0) * 4.0
        else:
            self.temperature -= 0.5

        # Transformer cannot cool below ambient temperature
        self.temperature = max(25.0, self.temperature)


# ============================================================
# FAULT DETECTOR
# ============================================================

class FaultDetector:

    def detect(
        self,
        voltage,
        current,
        temperature,
        phase_a,
        phase_b,
        phase_c
    ):

        # Short circuit
        if current > 500:
            return FaultType.SHORT_CIRCUIT

        # Overcurrent
        if current > 120:
            return FaultType.OVERCURRENT

        # Undervoltage
        if voltage < 10:
            return FaultType.UNDERVOLTAGE

        # Overvoltage
        if voltage > 12:
            return FaultType.OVERVOLTAGE

        # Transformer overheating
        if temperature > 80:
            return FaultType.TRANSFORMER_OVERHEAT

        # Phase imbalance
        maximum = max(phase_a, phase_b, phase_c)
        minimum = min(phase_a, phase_b, phase_c)

        if maximum - minimum > 20:
            return FaultType.PHASE_IMBALANCE

        return FaultType.NORMAL


# ============================================================
# PROTECTION RELAY
# ============================================================

class ProtectionRelay:

    def __init__(self):
        self.breaker_closed = True

    def trip(self):

        if self.breaker_closed:

            self.breaker_closed = False

            print("\n")
            print("=" * 45)
            print("        !!! BREAKER TRIPPED !!!")
            print("=" * 45)
            print("Fault isolated from the network.")

    def reset(self):
        self.breaker_closed = True

    def is_closed(self):
        return self.breaker_closed


# ============================================================
# GRID SIMULATOR
# ============================================================

class GridSimulator:

    def __init__(self):

        self.transformer = Transformer(
            rated_voltage=11.0,
            rated_current=100.0
        )

        self.detector = FaultDetector()
        self.relay = ProtectionRelay()

    def generate_measurements(self):

        voltage = 11.0 + random.uniform(-0.2, 0.2)

        current = 80.0 + random.uniform(-10, 10)

        phase_a = 100.0 + random.uniform(-2, 2)
        phase_b = 100.0 + random.uniform(-2, 2)
        phase_c = 100.0 + random.uniform(-2, 2)

        return voltage, current, phase_a, phase_b, phase_c

    def inject_fault(
        self,
        time_step,
        voltage,
        current,
        phase_a,
        phase_b,
        phase_c
    ):

        # Overcurrent
        if time_step == 5:
            current = 145

        # Undervoltage
        if time_step == 10:
            voltage = 9.2

        # Overvoltage
        if time_step == 15:
            voltage = 12.8

        # Phase imbalance
        if time_step == 20:
            phase_a = 135
            phase_b = 95
            phase_c = 100

        # Short circuit
        if time_step == 25:
            current = 600

        return voltage, current, phase_a, phase_b, phase_c

    def run(self, simulation_time=30):

        print("\n")
        print("=" * 50)
        print("                 GRIDGUARD")
        print("   Intelligent Electrical Grid Protection")
        print("=" * 50)

        print("\nStarting simulation...\n")

        for t in range(1, simulation_time + 1):

            # Stop when breaker trips
            if not self.relay.is_closed():

                print("\nSimulation stopped.")
                print("Breaker is OPEN.")

                break

            # Generate normal measurements
            (
                voltage,
                current,
                phase_a,
                phase_b,
                phase_c
            ) = self.generate_measurements()

            # Inject artificial faults
            (
                voltage,
                current,
                phase_a,
                phase_b,
                phase_c
            ) = self.inject_fault(
                t,
                voltage,
                current,
                phase_a,
                phase_b,
                phase_c
            )

            # Update transformer
            self.transformer.update_temperature(current)

            temperature = self.transformer.temperature

            # Detect fault
            fault = self.detector.detect(
                voltage,
                current,
                temperature,
                phase_a,
                phase_b,
                phase_c
            )

            # ------------------------------------------------
            # DISPLAY
            # ------------------------------------------------

            print("-" * 45)

            print(f"Time:           {t:02d} s")
            print(f"Voltage:        {voltage:.2f} kV")
            print(f"Current:        {current:.2f} A")
            print(f"Transformer:    {temperature:.2f} °C")
            print(f"Phase A:        {phase_a:.2f} A")
            print(f"Phase B:        {phase_b:.2f} A")
            print(f"Phase C:        {phase_c:.2f} A")

            # ------------------------------------------------
            # PROTECTION DECISION
            # ------------------------------------------------

            if fault == FaultType.NORMAL:

                print("\nSTATUS:         ✓ NORMAL")
                print("Protection:     MONITORING")

            else:

                print("\nSTATUS:         ⚠ FAULT DETECTED!")
                print(f"Fault Type:     {fault.value}")
                print("Protection:     TRIP COMMAND")

                self.relay.trip()

            time.sleep(0.7)

        # ----------------------------------------------------
        # FINAL STATUS
        # ----------------------------------------------------

        print("\n")
        print("=" * 50)
        print("                 SIMULATION END")
        print("=" * 50)

        if self.relay.is_closed():
            print("Final Status:   ✓ GRID HEALTHY")
        else:
            print("Final Status:   ⚠ FAULT ISOLATED")


# ============================================================
# MAIN PROGRAM
# ============================================================

def main():

    grid = GridSimulator()

    grid.run(30)


if __name__ == "__main__":
    main()
