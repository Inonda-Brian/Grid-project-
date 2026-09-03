#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace std;

// ============================================================
// GRIDGUARD
// Intelligent Electrical Grid Fault Detection Simulator
// ============================================================

enum class FaultType {
    NONE,
    OVERCURRENT,
    UNDERVOLTAGE,
    OVERVOLTAGE,
    TRANSFORMER_OVERHEAT,
    PHASE_IMBALANCE,
    SHORT_CIRCUIT
};

// ------------------------------------------------------------
// Transformer
// ------------------------------------------------------------
class Transformer {
private:
    double ratedVoltage;
    double ratedCurrent;
    double temperature;

public:
    Transformer(double voltage, double current)
        : ratedVoltage(voltage),
          ratedCurrent(current),
          temperature(25.0) {}

    void updateTemperature(double current) {

        // Simple thermal model
        double loadRatio = current / ratedCurrent;

        if (loadRatio > 1.0)
            temperature += (loadRatio - 1.0) * 4.0;
        else
            temperature -= 0.5;

        temperature = max(25.0, temperature);
    }

    double getTemperature() const {
        return temperature;
    }

    double getRatedVoltage() const {
        return ratedVoltage;
    }

    double getRatedCurrent() const {
        return ratedCurrent;
    }
};

// ------------------------------------------------------------
// Fault Detector
// ------------------------------------------------------------
class FaultDetector {
public:

    FaultType detect(
        double voltage,
        double current,
        double temperature,
        double phaseA,
        double phaseB,
        double phaseC
    ) {

        // Short circuit
        if (current > 500.0)
            return FaultType::SHORT_CIRCUIT;

        // Overcurrent
        if (current > 120.0)
            return FaultType::OVERCURRENT;

        // Undervoltage
        if (voltage < 10.0)
            return FaultType::UNDERVOLTAGE;

        // Overvoltage
        if (voltage > 12.0)
            return FaultType::OVERVOLTAGE;

        // Transformer overheating
        if (temperature > 80.0)
            return FaultType::TRANSFORMER_OVERHEAT;

        // Phase imbalance
        double maxPhase = max({phaseA, phaseB, phaseC});
        double minPhase = min({phaseA, phaseB, phaseC});

        if ((maxPhase - minPhase) > 20.0)
            return FaultType::PHASE_IMBALANCE;

        return FaultType::NONE;
    }

    string faultName(FaultType fault) {

        switch (fault) {

            case FaultType::OVERCURRENT:
                return "OVERCURRENT";

            case FaultType::UNDERVOLTAGE:
                return "UNDERVOLTAGE";

            case FaultType::OVERVOLTAGE:
                return "OVERVOLTAGE";

            case FaultType::TRANSFORMER_OVERHEAT:
                return "TRANSFORMER OVERHEAT";

            case FaultType::PHASE_IMBALANCE:
                return "PHASE IMBALANCE";

            case FaultType::SHORT_CIRCUIT:
                return "SHORT CIRCUIT";

            default:
                return "NORMAL";
        }
    }
};

// ------------------------------------------------------------
// Protection Relay
// ------------------------------------------------------------
class ProtectionRelay {
private:
    bool breakerClosed = true;

public:

    void trip() {

        if (breakerClosed) {

            breakerClosed = false;

            cout << "\n";
            cout << "========================================\n";
            cout << "        !!! BREAKER TRIPPED !!!\n";
            cout << "========================================\n";
            cout << "Fault isolated from the network.\n";
        }
    }

    void reset() {
        breakerClosed = true;
    }

    bool isClosed() const {
        return breakerClosed;
    }
};

// ------------------------------------------------------------
// Grid Simulator
// ------------------------------------------------------------
class GridSimulator {

private:

    Transformer transformer;
    FaultDetector detector;
    ProtectionRelay relay;

    mt19937 generator;

    uniform_real_distribution<double> voltageNoise;
    uniform_real_distribution<double> currentNoise;

public:

    GridSimulator()
        : transformer(11.0, 100.0),
          generator(random_device{}()),
          voltageNoise(-0.2, 0.2),
          currentNoise(-10.0, 10.0) {}

    void run(int simulationTime) {

        cout << "\n";
        cout << "============================================\n";
        cout << "              GRIDGUARD\n";
        cout << " Intelligent Electrical Grid Protection\n";
        cout << "============================================\n";

        cout << "\nStarting simulation...\n\n";

        for (int t = 1; t <= simulationTime; ++t) {

            // Stop simulation if breaker has tripped
            if (!relay.isClosed()) {

                cout << "\nSimulation stopped because the breaker is OPEN.\n";
                break;
            }

            // ------------------------------------------------
            // Simulate normal grid measurements
            // ------------------------------------------------

            double voltage = 11.0 + voltageNoise(generator);

            double current =
                80.0 + currentNoise(generator);

            double phaseA =
                100.0 + currentNoise(generator) * 0.2;

            double phaseB =
                100.0 + currentNoise(generator) * 0.2;

            double phaseC =
                100.0 + currentNoise(generator) * 0.2;

            // ------------------------------------------------
            // Introduce artificial faults
            // ------------------------------------------------

            if (t == 5) {
                current = 145.0;
            }

            if (t == 10) {
                voltage = 9.2;
            }

            if (t == 15) {
                voltage = 12.8;
            }

            if (t == 20) {
                phaseA = 135.0;
                phaseB = 95.0;
                phaseC = 100.0;
            }

            if (t == 25) {
                current = 600.0;
            }

            // Update transformer temperature
            transformer.updateTemperature(current);

            double temperature =
                transformer.getTemperature();

            // Detect fault
            FaultType fault =
                detector.detect(
                    voltage,
                    current,
                    temperature,
                    phaseA,
                    phaseB,
                    phaseC
                );

            // ------------------------------------------------
            // Display measurements
            // ------------------------------------------------

            cout << "--------------------------------------------\n";

            cout << "Time:           " << setw(2)
                 << t << " s\n";

            cout << fixed << setprecision(2);

            cout << "Voltage:        "
                 << voltage << " kV\n";

            cout << "Current:        "
                 << current << " A\n";

            cout << "Transformer:    "
                 << temperature << " C\n";

            cout << "Phase A:        "
                 << phaseA << " A\n";

            cout << "Phase B:        "
                 << phaseB << " A\n";

            cout << "Phase C:        "
                 << phaseC << " A\n";

            // ------------------------------------------------
            // Protection decision
            // ------------------------------------------------

            if (fault == FaultType::NONE) {

                cout << "\nSTATUS:         NORMAL\n";
                cout << "Protection:     MONITORING\n";

            } else {

                cout << "\nSTATUS:         FAULT DETECTED!\n";

                cout << "Fault Type:     "
                     << detector.faultName(fault)
                     << "\n";

                cout << "Protection:     TRIP COMMAND\n";

                relay.trip();
            }

            // Small delay so simulation looks live
            this_thread::sleep_for(
                chrono::milliseconds(700)
            );
        }

        cout << "\n============================================\n";
        cout << "              SIMULATION END\n";
        cout << "============================================\n";

        if (relay.isClosed())
            cout << "Final Status: GRID HEALTHY\n";
        else
            cout << "Final Status: FAULT ISOLATED\n";
    }
};

// ============================================================
// MAIN
// ============================================================

int main() {

    GridSimulator grid;

    grid.run(30);

    return 0;
}
