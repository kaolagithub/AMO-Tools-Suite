#include <catch.hpp>
#include "chillers/CoolingTower.h"
#include <tuple>

TEST_CASE("Cooling Tower Makeup Water Calculator", "[CoolingTower][Chillers]")
{
    auto const test = [](const std::tuple<double, double, int, double, int, double, int, double> &input,
                         const std::tuple<double, double, double> &ExpectedOutput)
    {
        double flowRate = std::get<0>(input);
        double coolingLoad = std::get<1>(input);
        int operationalHours = std::get<2>(input);
        double lossCorrectionFactor = std::get<3>(input);

        int baselineCyclesOfConcentration = std::get<4>(input);
        double baselineDriftLossFactor = std::get<5>(input);

        int modificationCyclesOfConcentration = std::get<6>(input);
        double modificationDriftLossFactor = std::get<7>(input);

        CoolingTowerOperatingConditionsData operatingConditionsData(flowRate, coolingLoad, operationalHours, lossCorrectionFactor);
        CoolingTowerWaterConservationData waterConservationBaselineData(baselineCyclesOfConcentration, baselineDriftLossFactor);
        CoolingTowerWaterConservationData waterConservationModificationData(modificationCyclesOfConcentration, modificationDriftLossFactor);

        CoolingTowerMakeupWaterCalculator coolingTowerMakeupWaterCalculator(operatingConditionsData, waterConservationBaselineData, waterConservationModificationData);
        CoolingTowerMakeupWaterCalculator::Output results = coolingTowerMakeupWaterCalculator.calculate();

        CHECK(results.wcBaseline == std::get<0>(ExpectedOutput));
        CHECK(results.wcModification == std::get<1>(ExpectedOutput));
        CHECK(results.waterSavings == std::get<2>(ExpectedOutput));
    };

    //Test 1

    double flowRate = 2500;
    double coolingLoad = 10.00;
    int operationalHours = 1000;
    double lossCorrectionFactor = 1.00;

    int baselineCyclesOfConcentration = 3;
    double baselineDriftLossFactor = 0.002;

    int modificationCyclesOfConcentration = 3;
    double modificationDriftLossFactor = 0.0001;

    std::tuple<double, double, int, double, int, double, int, double> input1;
    input1 = std::make_tuple(flowRate, coolingLoad, operationalHours, lossCorrectionFactor, baselineCyclesOfConcentration,
                             baselineDriftLossFactor, modificationCyclesOfConcentration, modificationDriftLossFactor);

    std::tuple<double, double, double> expectedOutput1;
    expectedOutput1 = std::make_tuple(2100000.0, 1815000.0, 285000.0);

    test(input1, expectedOutput1);

    //Test 2

    flowRate = 2500;
    coolingLoad = 5.00;
    operationalHours = 2000;
    lossCorrectionFactor = 1.00;

    baselineCyclesOfConcentration = 3;
    baselineDriftLossFactor = 0.002;

    modificationCyclesOfConcentration = 3;
    modificationDriftLossFactor = 0.0001;

    std::tuple<double, double, int, double, int, double, int, double> input2;
    input2 = std::make_tuple(flowRate, coolingLoad, operationalHours, lossCorrectionFactor, baselineCyclesOfConcentration,
                             baselineDriftLossFactor, modificationCyclesOfConcentration, modificationDriftLossFactor);

    std::tuple<double, double, double> expectedOutput2;
    expectedOutput2 = std::make_tuple(2400000.0, 1830000.0, 570000.0);

    test(input2, expectedOutput2);

    //Test 3

    flowRate = 2500;
    coolingLoad = 2.00;
    operationalHours = 3000;
    lossCorrectionFactor = 1.00;

    baselineCyclesOfConcentration = 3;
    baselineDriftLossFactor = 0.002;

    modificationCyclesOfConcentration = 3;
    modificationDriftLossFactor = 0.0001;

    std::tuple<double, double, int, double, int, double, int, double> input3;
    input3 = std::make_tuple(flowRate, coolingLoad, operationalHours, lossCorrectionFactor, baselineCyclesOfConcentration,
                             baselineDriftLossFactor, modificationCyclesOfConcentration, modificationDriftLossFactor);
                             
    std::tuple<double, double, double> expectedOutput3;
    expectedOutput3 = std::make_tuple(1980000.0, 1125000.0, 855000.0);

    test(input3, expectedOutput3);
}