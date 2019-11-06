#include "ssmt/service/energy_and_cost/EnergyAndCostCalculator.h"

EnergyAndCostCalculationsDomain
EnergyAndCostCalculator::calc(const bool isBaselineCalc, const double baselinePowerDemand,
                              const OperationsInput &operationsInput, const Boiler &boiler,
                              const HighPressureHeaderCalculationsDomain &highPressureHeaderCalculationsDomain,
                              const std::shared_ptr<MediumPressureHeaderCalculationsDomain> &mediumPressureHeaderCalculationsDomain,
                              const double makeupWaterVolumeFlowAnnual) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    std::cout << methodName << "calculating powerGenerated" << std::endl;
    //9. Calculate Energy and Cost Values
    //9a. Calculate Power Generated
    const double powerGenerated =
            calcPowerGenerated(highPressureHeaderCalculationsDomain, mediumPressureHeaderCalculationsDomain);

    std::cout << methodName << "calculating sitePowerImportUpdated" << std::endl;
    //9b. Calculate Site Power Import
    const double sitePowerImportInput = operationsInput.getSitePowerImport();
    const double sitePowerImportUpdated =
            calcPowerImport(isBaselineCalc, sitePowerImportInput, baselinePowerDemand, powerGenerated);

    std::cout << methodName << "calculating powerDemand" << std::endl;
    //9c. Calculate Demand
    const double powerDemand = calcPowerDemand(sitePowerImportUpdated, powerGenerated);

    std::cout << methodName << "calculating powerGenerationCost" << std::endl;
    //9d. Calculate cost of power generation
    const double electricityCostsInput = operationsInput.getElectricityCosts();
    const double operatingHoursPerYearInput = operationsInput.getOperatingHoursPerYear();
    const double powerGenerationCost =
            calcPowerGenerationCost(sitePowerImportUpdated, electricityCostsInput, operatingHoursPerYearInput);

    std::cout << methodName << "calculating boilerFuelCost" << std::endl;
    //9e. Calculate cost of fuel for boiler
    const double fuelCostsInput = operationsInput.getFuelCosts();
    const double fuelEnergyInput = boiler.getFuelEnergy();
    const double boilerFuelCost = calcBoilerFuelCost(fuelEnergyInput, operatingHoursPerYearInput, fuelCostsInput);

    std::cout << methodName << "calculating makeupWaterCost" << std::endl;
    //9f. Calculate cost of make-up water
    const double makeUpWaterCostsInput = operationsInput.getMakeUpWaterCosts();
    const double makeupWaterCost = calcMakeupWaterCost(makeUpWaterCostsInput, makeupWaterVolumeFlowAnnual);

    std::cout << methodName << "calculating totalOperatingCost" << std::endl;
    //9g. Calculate total operating costs
    const double totalOperatingCost = calcTotalOperatingCost(powerGenerationCost, boilerFuelCost, makeupWaterCost);

    std::cout << methodName << "calculating boilerFuelUsage" << std::endl;
    //9h. Calculate boiler fuel usage
    const double boilerFuelUsage = calcBoilerFuelUsage(fuelEnergyInput, operatingHoursPerYearInput);

    return {powerGenerated, sitePowerImportUpdated, powerDemand, powerGenerationCost, boilerFuelCost, makeupWaterCost,
            totalOperatingCost, boilerFuelUsage};
}

double
EnergyAndCostCalculator::calcPowerGenerated(
        const HighPressureHeaderCalculationsDomain &highPressureHeaderCalculationsDomain,
        const std::shared_ptr<MediumPressureHeaderCalculationsDomain> &mediumPressureHeaderCalculationsDomain) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    //sum power generated by turbine
    double powerGenerated = 0;

    const std::shared_ptr<Turbine> &condensingTurbine = highPressureHeaderCalculationsDomain.condensingTurbine;
    powerGenerated = addPowerOutToPowerGenerated("condensingTurbine", condensingTurbine, powerGenerated);

    const std::shared_ptr<Turbine> &highToLowPressureTurbine =
            highPressureHeaderCalculationsDomain.highToLowPressureTurbine;
    powerGenerated = addPowerOutToPowerGenerated("highToLowPressureTurbine", highToLowPressureTurbine, powerGenerated);

    const std::shared_ptr<Turbine> &highToMediumPressureTurbine =
            highPressureHeaderCalculationsDomain.highToMediumPressureTurbine;
    powerGenerated =
            addPowerOutToPowerGenerated("highToMediumPressureTurbine", highToMediumPressureTurbine, powerGenerated);

    if (mediumPressureHeaderCalculationsDomain == nullptr) {
        std::cout << methodName << "mediumPressureHeaderCalculationsDomain is null, skipping mediumToLowPressureTurbine"
                  << std::endl;
    } else {
        std::cout << methodName
                  << "mediumPressureHeaderCalculationsDomain is not null, processing mediumToLowPressureTurbine"
                  << std::endl;
        const std::shared_ptr<Turbine> &mediumToLowPressureTurbine =
                mediumPressureHeaderCalculationsDomain->mediumToLowPressureTurbine;
        powerGenerated =
                addPowerOutToPowerGenerated("mediumToLowPressureTurbine", mediumToLowPressureTurbine, powerGenerated);
    }

    std::cout << methodName << "result=" << powerGenerated << std::endl;

    return powerGenerated;
}

double
EnergyAndCostCalculator::addPowerOutToPowerGenerated(const std::string &name, const std::shared_ptr<Turbine> &turbine,
                                                     const double powerGenerated) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    double result = powerGenerated;

    if (turbine == nullptr) {
        std::cout << "EnergyAndCostCalculator::" << __func__ << ": '" << name << "' is null, skipping" << std::endl;
    } else {
        const double powerOut = turbine->getPowerOut();
        std::cout << methodName << "adding " << name << "->powerOut=" << powerOut
                  << " to powerGenerated=" << powerGenerated << std::endl;
        result += powerOut;
    }

    return result;
}

double
EnergyAndCostCalculator::calcPowerImport(const bool isBaselineCalc, const double sitePowerImportInput,
                                         const double baselinePowerDemand, const double powerGenerated) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = (isBaselineCalc) ? sitePowerImportInput : baselinePowerDemand - powerGenerated;
    std::cout << methodName << "isBaselineCalc=" << isBaselineCalc
              << ", sitePowerImportInput=" << sitePowerImportInput << ", baselinePowerDemand=" << baselinePowerDemand
              << "powerGenerated=" << powerGenerated << ", result=" << result << std::endl;
    return result;
}

double
EnergyAndCostCalculator::calcPowerDemand(const double sitePowerImport, const double powerGenerated) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = sitePowerImport + powerGenerated;
    std::cout << methodName << "sitePowerImport=" << sitePowerImport << ", powerGenerated="
              << powerGenerated << ", result=" << result << std::endl;
    return result;
}

double
EnergyAndCostCalculator::calcPowerGenerationCost(const double sitePowerImportInput, const double electricityCostsInput,
                                                 const double operatingHoursPerYearInput) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = sitePowerImportInput * electricityCostsInput * operatingHoursPerYearInput;
    std::cout << methodName << "sitePowerImportInput=" << sitePowerImportInput
              << ", electricityCostsInput=" << electricityCostsInput << ", operatingHoursPerYearInput="
              << operatingHoursPerYearInput << ", result=" << result << std::endl;
    return result;
}

double
EnergyAndCostCalculator::calcBoilerFuelCost(const double fuelEnergyInput, const double operatingHoursPerYearInput,
                                            const double fuelCostsInput) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = fuelEnergyInput * operatingHoursPerYearInput * fuelCostsInput;
    std::cout << methodName << "fuelEnergyInput=" << fuelEnergyInput
              << ", operatingHoursPerYearInput=" << operatingHoursPerYearInput << ", fuelCostsInput=" << fuelCostsInput
              << ", result=" << result
              << std::endl;
    return result;
}

double
EnergyAndCostCalculator::calcMakeupWaterCost(const double makeUpWaterCostsInput,
                                             const double makeupWaterVolumeFlowAnnual) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = makeUpWaterCostsInput * makeupWaterVolumeFlowAnnual;
    std::cout << methodName << "makeUpWaterCostsInput=" << makeUpWaterCostsInput
              << ", makeupWaterVolumeFlowAnnual=" << makeupWaterVolumeFlowAnnual << ", result=" << result
              << std::endl;
    return result;
}

double
EnergyAndCostCalculator::calcTotalOperatingCost(const double powerGenerationCost, const double boilerFuelCost,
                                                const double makeupWaterCost) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = powerGenerationCost + boilerFuelCost + makeupWaterCost;
    std::cout << methodName << "powerGenerationCost=" << powerGenerationCost
              << ", boilerFuelCost=" << boilerFuelCost << ", makeupWaterCost=" << makeupWaterCost << ", result="
              << result << std::endl;
    return result;
}

double
EnergyAndCostCalculator::calcBoilerFuelUsage(const double fuelEnergyInput,
                                             const double operatingHoursPerYearInput) const {
    const std::string methodName = std::string("EnergyAndCostCalculator::") + std::string(__func__) + ": ";

    const double result = fuelEnergyInput * operatingHoursPerYearInput;
    std::cout << methodName << "fuelEnergyInput=" << fuelEnergyInput
              << ", operatingHoursPerYearInput=" << operatingHoursPerYearInput << ", result=" << result << std::endl;
    return result;
}
