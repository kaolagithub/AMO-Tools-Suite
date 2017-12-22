/**
 * @brief Contains the definition of functions of MotorEfficiency class.
 *      calculate(): Calculates the motor efficiency given a load factor and other parameters.
 *
 * @author Subhankar Mishra (mishras)
 * @author Gina Accawi (accawigk)
 * @bug No known bugs.
 *
 */

#include <cmath>
#include "calculator/motor/MotorEfficiency.h"
#include "calculator/util/CurveFitVal.h"
#include "calculator/motor/MotorEfficiency25.h"

double MotorEfficiency::calculate() {

    if (efficiencyClass == Motor::EfficiencyClass::ENERGY_EFFICIENT
        || efficiencyClass == Motor::EfficiencyClass::STANDARD
        || efficiencyClass == Motor::EfficiencyClass::PREMIUM)
    {
        motorEfficiency = MotorEfficiency25(efficiencyClass, motorRatedPower, motorRpm, lineFrequency).calculate();
    } else if (efficiencyClass == Motor::EfficiencyClass::SPECIFIED) {
        // For specified efficiency, you have to first choose the nominal efficiency.
        motorEfficiency = MotorEfficiency25(Motor::EfficiencyClass::ENERGY_EFFICIENT, motorRatedPower, motorRpm,
                                            lineFrequency).calculate();
        auto const stdMotorEffs = MotorEfficiency25(Motor::EfficiencyClass::STANDARD, motorRatedPower, motorRpm,
                                                         lineFrequency).calculate();
	    
        if (std::fabs(motorEfficiency[3] - specifiedEfficiency) > std::fabs(stdMotorEffs[3] - specifiedEfficiency)) {
            //SE chosen
            double C = specifiedEfficiency / stdMotorEffs[3];
            for (std::size_t i = 0; i < 5; ++i) {
                motorEfficiency[i] = stdMotorEffs[i] * C;
            }
        } else { // EE chosen
            specifiedEfficiency = specifiedEfficiency / 100;
            double C = specifiedEfficiency / motorEfficiency[3];

            for (std::size_t i = 0; i < 5; ++i) {
                motorEfficiency[i] = motorEfficiency[i] * C;
            }
        }
    }

    /**
     * Calculating the 1% interval values based on the load factor
     * 0 - 25
     * 1. Using the 0% and 25% kW loss values, develop a linear table of loss by 1% load
     * Equation: kWloss (X) = kWloss (0) + X * (kWloss(25) - kWloss(0))/25      where X = 0,1,2,3…..25
     * 2. Assemble an array of shaft power from 0 to 25
     * Equation: kW shaft (X) = rated hp *0.746 * (X / 100)
     * Assemble an array of kWe (input electric power
     * Equation: kWe(X) = kWloss(x) + kWshaft(X)
     * Calculate efficiency
     * Equation: Eff(X) = kW shaft (X) / kWe (X)
     */

    /**
     * Setting up kWloss0 and kWloss25
     * Make sure motorEfficiency is in decimal rather %.
     * E.g.: 0.92 is correct, 92 is wrong.
     */
    const double kWloss25 = ((1 / motorEfficiency[0]) - 1) * motorRatedPower * 0.746 * 0.25;
    kWloss0 = 0.8 * kWloss25;

    if (loadFactor <= 0.25001) {
        // loadFactor <= 0.25
        const double kWloss = kWloss0 + loadFactor * 100 * (kWloss25 - kWloss0) / 25;
        const double kWshaft = motorRatedPower * 0.746 * (loadFactor); // Make sure motorRatedPower is in hp
        const double kWe = kWloss + kWshaft; // Input electric power
        motorEff = kWshaft / kWe; //Final efficiency calculation
    } else if (loadFactor <= 1.25001) {
        /**
         * 26 - 125 load factor (0.25 - 1.25 non-inclusive)
         * Fitting tabular, 25% interval, motor efficiency data to assemble an overall curve with 1% interval data from 26 to 150%
         * 25% load interval efficiency values come from "Adjusting 25 percent arrays for specified efficiency"
         * Pick the 25,50,75,100,and 125% motor efficiency values and do a 4th order polynomial fit.
         * Use the fit coefficients to populate, in 1% load intervals, from 26 to 125% load
         */
        CurveFitVal cfv({.25, .50, .75, 1.00, 1.25}, {motorEfficiency[0], motorEfficiency[1], motorEfficiency[2], motorEfficiency[3], motorEfficiency[4]}, 4, loadFactor);
        motorEff = cfv.calculate();
    } else {
        /**
         * 126 - 150 load factor (1.25 (non-inclusive) - 1.50 (inclusive))
         * Pick the 75, 100, and 125% motor efficiency values and do a 2nd order polynomial fit
         * Use the fit coefficients to populate, in 1% load intervals, the current range from 126 to 150% load
         */
        if (loadFactor > 1.5) {
            loadFactor = 1.5;
        }
        CurveFitVal cfv({.75, 1.00, 1.25}, {motorEfficiency[2], motorEfficiency[3], motorEfficiency[4]}, 2, loadFactor);
        motorEff = cfv.calculate();
    }

    return motorEff;
}
