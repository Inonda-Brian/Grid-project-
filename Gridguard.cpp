#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>

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
    SHORT_CIRCUIT,
    FREQUENCY_DEVIATION,
    LOW_POWER_FACTOR
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

// ============================================================
// FREQUENCY MONITOR
// ============================================================
class FrequencyMonitor {
private:
    double nominalFrequency;  // 50 or 60 Hz
    double currentFrequency;
    double frequencyThresholdHigh;  // Hz above nominal
    double frequencyThresholdLow;   // Hz below nominal

public:
    FrequencyMonitor(double nominal = 60.0)
        : nominalFrequency(nominal),
          currentFrequency(nominal),
          frequencyThresholdHigh(62.0),
          frequencyThresholdLow(58.0) {}

    void updateFrequency(double load, double generation) {
        // Frequency changes based on balance between generation and load
        // Excess generation -> frequency rises
        // Excess load -> frequency falls
        
        double imbalance = generation - load;  // MW difference
        double frequencyDeviation = imbalance * 0.5;  // 0.5 Hz per MW imbalance
        
        currentFrequency = nominalFrequency + frequencyDeviation;
        
        // Damping - frequency returns toward nominal
        currentFrequency = currentFrequency * 0.9 + nominalFrequency * 0.1;
    }

    double getCurrentFrequency() const {
        return currentFrequency;
    }

    double getFrequencyDeviation() const {
        return abs(currentFrequency - nominalFrequency);
    }

    bool isFrequencyHealthy() const {
        return currentFrequency >= frequencyThresholdLow &&
               currentFrequency <= frequencyThresholdHigh;
    }

    string getFrequencyStatus() const {
        if (currentFrequency < frequencyThresholdLow) {
            return "LOW FREQUENCY - Load Shedding Needed";
        } else if (currentFrequency > frequencyThresholdHigh) {
            return "HIGH FREQUENCY - Excess Generation";
        } else {
            return "FREQUENCY NORMAL";
        }
    }
};

// ============================================================
// POWER FACTOR MONITOR
// ============================================================
class PowerFactorMonitor {
private:
    double powerFactor;           // 0.0 to 1.0
    double powerFactorThreshold;  // Typically 0.85
    double reactiveLoad;          // kVAR

public:
    PowerFactorMonitor()
        : powerFactor(0.95),
          powerFactorThreshold(0.85),
          reactiveLoad(0.0) {}

    void updatePowerFactor(double voltage, double current, double phaseAngle) {
        // Power factor = cos(phase angle)
        // Phase angle represents phase shift between voltage and current
        powerFactor = cos(phaseAngle * M_PI / 180.0);
        
        // Clamp to valid range
        powerFactor = max(0.0, min(1.0, powerFactor));
        
        // Calculate reactive power (VAR = V * I * sin(angle))
        reactiveLoad = voltage * current * sin(phaseAngle * M_PI / 180.0);
    }

    double getPowerFactor() const {
        return powerFactor;
    }

    double getReactivePower() const {
        return reactiveLoad;
    }

    bool isPowerFactorHealthy() const {
        return powerFactor >= powerFactorThreshold;
    }

    string getPowerFactorStatus() const {
        if (powerFactor < powerFactorThreshold) {
            return "LOW POWER FACTOR - Reactive Compensation Needed";
        } else if (powerFactor > 0.95) {
            return "EXCELLENT POWER FACTOR";
        } else {
            return "POWER FACTOR ACCEPTABLE";
        }
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
        double phaseC,
        double frequency,
        double powerFactor
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

        // Frequency deviation (severe)
        if (frequency < 58.0 || frequency > 62.0)
            return FaultType::FREQUENCY_DEVIATION;

        // Low power factor
        if (powerFactor < 0.80)
            return FaultType::LOW_POWER_FACTOR;

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

            case FaultType::FREQUENCY_DEVIATION:
                return "FREQUENCY DEVIATION";

            case FaultType::LOW_POWER_FACTOR:
                return "LOW POWER FACTOR";

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

// ============================================================
// LOAD SHEDDING CONTROLLER
// ============================================================
class LoadSheddingController {
private:
    double totalLoad;
    bool isActive;

public:
    LoadSheddingController() : totalLoad(100.0), isActive(false) {}

    void activateLoadShedding(double frequencyDeviation) {
        // Shed 5% of load for every 0.5 Hz below nominal
        if (frequencyDeviation > 1.5) {
            totalLoad *= 0.90;  // Reduce by 10%
            isActive = true;
        }
    }

    void deactivateLoadShedding() {
        totalLoad = 100.0;
        isActive = false;
    }

    double getTotalLoad() const {
        return totalLoad;
    }

    bool isLoadSheddingActive() const {
        return isActive;
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
    FrequencyMonitor freqMonitor;
    PowerFactorMonitor pfMonitor;
    LoadSheddingController loadShedding;

    mt19937 generator;

    uniform_real_distribution<double> voltageNoise;
    uniform_real_distribution<double> currentNoise;
    uniform_real_distribution<double> phaseAngleNoise;

public:

    GridSimulator()
        : transformer(11.0, 100.0),
          generator(random_device{}()),
          voltageNoise(-0.2, 0.2),
          currentNoise(-10.0, 10.0),
          phaseAngleNoise(-5.0, 5.0) {}

    void run(int simulationTime) {

        cout << "\n";
        cout << "============================================\n";
        cout << "              GRIDGUARD v2.0\n";
        cout << " Intelligent Electrical Grid Protection\n";
        cout << "  With Frequency & Power Factor Monitoring\n";
        cout << "============================================\n";

        cout << "\nStarting simulation...\n\n";

        double generation = 90.0;  // MW
        double load = 85.0;         // MW

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

            double phaseAngle = 20.0 + phaseAngleNoise(generator);

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

            // Frequency fault: Loss of generation at t=28
            if (t == 28) {
                generation = 50.0;  // Major generation loss
            }

            // Power factor fault: Inductive load at t=32
            if (t == 32) {
                phaseAngle = 45.0;  // Low PF
            }

            if (t == 25) {
                current = 600.0;
            }

            // ------------------------------------------------
            // Update grid parameters
            // ------------------------------------------------

            transformer.updateTemperature(current);
            freqMonitor.updateFrequency(loadShedding.getTotalLoad(), generation);
            pfMonitor.updatePowerFactor(voltage, current, phaseAngle);

            // Load shedding if frequency critical
            if (!freqMonitor.isFrequencyHealthy()) {
                loadShedding.activateLoadShedding(freqMonitor.getFrequencyDeviation());
            } else {
                loadShedding.deactivateLoadShedding();
            }

            double temperature = transformer.getTemperature();
            double frequency = freqMonitor.getCurrentFrequency();
            double powerFactor = pfMonitor.getPowerFactor();

            // Detect fault
            FaultType fault =
                detector.detect(
                    voltage,
                    current,
                    temperature,
                    phaseA,
                    phaseB,
                    phaseC,
                    frequency,
                    powerFactor
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
            // Display NEW parameters
            // ------------------------------------------------

            cout << "\nFrequency:      "
                 << frequency << " Hz";
            cout << " (" << freqMonitor.getFrequencyDeviation() << " Hz deviation)\n";

            cout << "Power Factor:   " << setprecision(3)
                 << powerFactor << "\n";

            cout << "Reactive Power: " << setprecision(2)
                 << pfMonitor.getReactivePower() << " kVAR\n";

            if (loadShedding.isLoadSheddingActive()) {
                cout << "Load Shedding:  ACTIVE - Load: "
                     << loadShedding.getTotalLoad() << " MW\n";
            }

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

    grid.run(35);

    return 0;
}
