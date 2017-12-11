/**
 * @brief Contains the definition of functions of OptimalPumpShaftPower class.
 *      calculate(): Calculates the Optimal Pump ShaftPower
 *
 * @author Subhankar Mishra (mishras)
 * @author Gina Accawi (accawigk)
 * @bug No known bugs.
 *
 */

#include "calculator/pump/OptimalPumpShaftPower.h"

double OptimalPumpShaftPower::calculate() {
    FluidPower fluidPower(specificGravity,flowRate,head);
    double fluidPower_ = fluidPower.calculate();
    double requiredShaftPowerkW = fluidPower_ / pumpEfficiency;
    double requiredShaftPowerhP = requiredShaftPowerkW / 0.746;
    return requiredShaftPowerhP;
}
