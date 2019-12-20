// #ifndef AMO_TOOLS_SUITE_CALCULATOR_H
// #define AMO_TOOLS_SUITE_CALCULATOR_H

// #include "NanDataConverters.h"
#include <nan.h>
#include <node.h>
#include <string>
#include <stdexcept>
#include <array>
#include <cmath>
#include <vector>
#include "calculator/util/ElectricityReduction.h"
#include "calculator/util/NaturalGasReduction.h"
#include "calculator/util/CompressedAirReduction.h"
#include "calculator/util/CompressedAirPressureReduction.h"
#include "calculator/util/WaterReduction.h"
#include "calculator/util/insulation/pipes/InsulatedPipeInput.h"
#include "calculator/util/insulation/pipes/InsulatedPipeCalculator.h"
#include "calculator/util/insulation/pipes/InsulatedPipeOutput.h"
#include "calculator/util/insulation/tanks/InsulatedTankInput.h"
#include "calculator/util/insulation/tanks/InsulatedTankCalculator.h"
#include "calculator/util/insulation/tanks/InsulatedTankOutput.h"

#include <iostream>
#include "ssmt/SaturatedProperties.h"
#include "ssmt/SteamSystemModelerTool.h"
#include "calculator/util/SteamReduction.h"

using namespace Nan;
using namespace v8;

Local<Object> inp;
Local<Object> r;

// double Get(std::string const &nm)
// {
//     Local<String> getName = Nan::New<String>(nm).ToLocalChecked();
//     auto rObj = Nan::To<Object>(inp).ToLocalChecked()->Get(GetCurrent(), getName);
//     if (rObj->IsUndefined())
//     {
//         ThrowTypeError(std::string("Get method in calculator.h: " + nm + " not present in object").c_str());
//     }
//     return Nan::To<double>(rObj).FromJust();
// }

double GetDouble(std::string const &nm, Local<Object> obj)
{
    Local<String> getName = Nan::New<String>(nm).ToLocalChecked();
    auto rObj = Nan::To<Object>(obj).ToLocalChecked()->Get(getName);
    if (rObj->IsUndefined())
    {
        ThrowTypeError(std::string("Get method in calculator.h: " + nm + " not present in object").c_str());
    }
    return Nan::To<double>(rObj).FromJust();
}

std::vector<double> GetVector(const std::string &key, Local<Object> obj)
{
    auto const &arrayTmp = Nan::To<Object>(obj).ToLocalChecked()->Get(Nan::New<String>(key).ToLocalChecked());
    auto const &jsArray = v8::Local<v8::Array>::Cast(arrayTmp);
    std::vector<double> array;
    for (unsigned int i = 0; i < jsArray->Length(); i++)
    {
        v8::Local<v8::Value> jsElement = jsArray->Get(i);
        double val = Nan::To<double>(jsElement).FromJust();
        array.push_back(val);
    }
    return array;
}

template <typename T>
T GetEnumVal(std::string const &key, Local<Object> obj)
{
    auto rObj = Nan::To<Object>(obj).ToLocalChecked()->Get(Nan::New<String>(key).ToLocalChecked());
    if (rObj->IsUndefined())
    {
        ThrowTypeError(std::string("GetEnumVal method in calculator.h: " + key + " not present in object").c_str());
    }
    return static_cast<T>(Nan::To<double>(rObj).FromJust());
}

bool GetBool(std::string const &key, Local<Object> obj)
{
    auto rObj = Nan::To<Object>(obj).ToLocalChecked()->Get(Nan::New<String>(key).ToLocalChecked());
    if (rObj->IsUndefined())
    {
        ThrowTypeError(std::string("GetBool method in calculator.h: Boolean value " + key + " not present in object").c_str());
    }
    return rObj->BooleanValue();
}

std::string GetStr(std::string const &key, Local<Object> obj)
{
    auto const &rObj = Nan::To<Object>(obj).ToLocalChecked()->Get(Nan::New<String>(key).ToLocalChecked());
    if (rObj->IsUndefined())
    {
        ThrowTypeError(std::string("GetStr method in calculator.h: String " + key + " not present in object").c_str());
    }
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::String::Utf8Value s(isolate, rObj);
    return std::string(*s);
}

bool isDefined(Local<Object> obj, std::string const &key)
{
    return !obj->Get(Nan::New<String>(key).ToLocalChecked())->IsUndefined();
}

inline void SetR(const std::string &key, double val)
{
    Nan::Set(r, Nan::New<String>(key).ToLocalChecked(), Nan::New<Number>(val));
}

// ============== Electricity Reduction ==============

MultimeterData getMultimeterData(Local<Object> obj)
{
    auto multimeterDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("multimeterData").ToLocalChecked())).ToLocalChecked();
    return {
        static_cast<int>(GetDouble("numberOfPhases", multimeterDataV8)),
        GetDouble("supplyVoltage", multimeterDataV8),
        GetDouble("averageCurrent", multimeterDataV8),
        GetDouble("powerFactor", multimeterDataV8)};
}

NameplateData getNameplateData(Local<Object> obj)
{
    auto nameplateDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("nameplateData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("ratedMotorPower", nameplateDataV8),
        GetBool("variableSpeedMotor", nameplateDataV8),
        GetDouble("operationalFrequency", nameplateDataV8),
        GetDouble("lineFrequency", nameplateDataV8),
        GetDouble("motorAndDriveEfficiency", nameplateDataV8),
        GetDouble("loadFactor", nameplateDataV8)};
}

PowerMeterData getPowerMeterData(Local<Object> obj)
{
    auto powerMeterDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("powerMeterData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("power", powerMeterDataV8)};
}

OtherMethodData getOtherMethodData(Local<Object> obj)
{
    auto otherMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("otherMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("energy", otherMethodDataV8)};
}

ElectricityReductionInput constructElectricityReductionInput(Local<Object> obj)
{
    return {
        static_cast<int>(GetDouble("operatingHours", obj)),
        GetDouble("electricityCost", obj),
        static_cast<int>(GetDouble("measurementMethod", obj)),
        getMultimeterData(obj),
        getNameplateData(obj),
        getPowerMeterData(obj),
        getOtherMethodData(obj),
        static_cast<int>(GetDouble("units", obj))};
}

ElectricityReduction getElectricityReductionInputVector()
{
    auto const electricityReductionInputVecTemp = Nan::To<Object>(inp).ToLocalChecked()->Get(Nan::New<String>("electricityReductionInputVec").ToLocalChecked());
    auto const &electricityReductionInputArray = v8::Local<v8::Array>::Cast(electricityReductionInputVecTemp);
    std::vector<ElectricityReductionInput> inputVec;
    for (std::size_t i = 0; i < electricityReductionInputArray->Length(); i++)
    {
        inputVec.emplace_back(constructElectricityReductionInput(Nan::To<Object>(electricityReductionInputArray->Get(i)).ToLocalChecked()));
    }
    return inputVec;
}

NAN_METHOD(electricityReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();
    try
    {
        auto rv = ElectricityReduction(getElectricityReductionInputVector()).calculate();
        SetR("energyUse", rv.energyUse);
        SetR("energyCost", rv.energyCost);
        SetR("power", rv.power);
    }
    catch (std::runtime_error const &e)
    {
        std::string const what = e.what();
        ThrowError(std::string("std::runtime_error thrown in electricityReduction - calculator.h: " + what).c_str());
    }
    info.GetReturnValue().Set(r);
}
// ============== END Electricity Reduction ==============

// ============== Natural Gas ==============

FlowMeterMethodData getFlowMeterMethodData(Local<Object> obj)
{
    auto flowMeterMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("flowMeterMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("flowRate", flowMeterMethodDataV8)};
}

AirMassFlowMeasuredData getAirMassFlowMeasuredData(Local<Object> obj)
{
    auto airMassFlowMeasuredDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("airMassFlowMeasuredData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("areaOfDuct", airMassFlowMeasuredDataV8),
        GetDouble("airVelocity", airMassFlowMeasuredDataV8)};
}

AirMassFlowNameplateData getAirMassFlowNameplateData(Local<Object> obj)
{
    auto airMassFlowNameplateDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("airMassFlowNameplateData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("airFlow", airMassFlowNameplateDataV8)};
}

AirMassFlowData getAirMassFlowData(Local<Object> obj)
{
    auto airMassFlowDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("airMassFlowData").ToLocalChecked())).ToLocalChecked();
    return {
        GetBool("isNameplate", airMassFlowDataV8),
        getAirMassFlowMeasuredData(airMassFlowDataV8),
        getAirMassFlowNameplateData(airMassFlowDataV8),
        GetDouble("inletTemperature", airMassFlowDataV8),
        GetDouble("outletTemperature", airMassFlowDataV8),
        GetDouble("systemEfficiency", airMassFlowDataV8) / 100};
}

WaterMassFlowData getWaterMassFlowData(Local<Object> obj)
{
    auto waterMassFlowDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("waterMassFlowData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("waterFlow", waterMassFlowDataV8),
        GetDouble("inletTemperature", waterMassFlowDataV8),
        GetDouble("outletTemperature", waterMassFlowDataV8),
        GetDouble("systemEfficiency", waterMassFlowDataV8) / 100};
}

NaturalGasOtherMethodData naturalGasGetOtherMethodData(Local<Object> obj)
{
    auto otherMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("otherMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("consumption", otherMethodDataV8)};
}

NaturalGasReductionInput constructNaturalGasReductionInput(Local<Object> obj)
{
    return {
        static_cast<int>(GetDouble("operatingHours", obj)),
        GetDouble("fuelCost", obj),
        static_cast<int>(GetDouble("measurementMethod", obj)),
        getFlowMeterMethodData(obj),
        naturalGasGetOtherMethodData(obj),
        getAirMassFlowData(obj),
        getWaterMassFlowData(obj),
        static_cast<int>(GetDouble("units", obj))};
}

NaturalGasReduction getNaturalGasReductionInputVector()
{
    auto const naturalGasReductionInputVecTemp = Nan::To<Object>(inp).ToLocalChecked()->Get(Nan::New<String>("naturalGasReductionInputVec").ToLocalChecked());
    auto const &naturalGasReductionInputArray = v8::Local<v8::Array>::Cast(naturalGasReductionInputVecTemp);
    std::vector<NaturalGasReductionInput> inputVec;
    for (std::size_t i = 0; i < naturalGasReductionInputArray->Length(); i++)
    {
        inputVec.emplace_back(constructNaturalGasReductionInput(Nan::To<Object>(naturalGasReductionInputArray->Get(i)).ToLocalChecked()));
    }
    return inputVec;
}

NAN_METHOD(naturalGasReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();
    try
    {
        auto rv = NaturalGasReduction(getNaturalGasReductionInputVector()).calculate();
        SetR("energyUse", rv.energyUse);
        SetR("energyCost", rv.energyCost);
        SetR("heatFlow", rv.heatFlow);
        SetR("totalFlow", rv.totalFlow);
    }
    catch (std::runtime_error const &e)
    {
        std::string const what = e.what();
        ThrowError(std::string("std::runtime_error thrown in naturalGasReduction - calculator.h: " + what).c_str());
    }
    info.GetReturnValue().Set(r);
}
// ============== END Natural Gas ==============

// ============== Compressed Air ==============

CompressedAirFlowMeterMethodData getCompressedAirFlowMeterMethodData(Local<Object> obj)
{
    auto flowMeterMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("flowMeterMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("meterReading", flowMeterMethodDataV8)};
}

BagMethodData getBagMethodData(Local<Object> obj)
{
    auto bagMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("bagMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("height", bagMethodDataV8),
        GetDouble("diameter", bagMethodDataV8),
        GetDouble("fillTime", bagMethodDataV8)};
}

PressureMethodData getPressureMethodData(Local<Object> obj)
{
    auto pressureMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("pressureMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        static_cast<int>(GetDouble("nozzleType", pressureMethodDataV8)),
        static_cast<int>(GetDouble("numberOfNozzles", pressureMethodDataV8)),
        GetDouble("supplyPressure", pressureMethodDataV8)};
}

CompressedAirOtherMethodData getCompressedAirOtherMethodData(Local<Object> obj)
{
    auto otherMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("otherMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("consumption", otherMethodDataV8)};
}

CompressorElectricityData getCompressorElectricityData(Local<Object> obj)
{
    auto compressorElectricityDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("compressorElectricityData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("compressorControlAdjustment", compressorElectricityDataV8) / 100,
        GetDouble("compressorSpecificPower", compressorElectricityDataV8)};
}

CompressedAirReductionInput constructCompressedAirReductionInput(Local<Object> obj)
{
    return {
        static_cast<int>(GetDouble("hoursPerYear", obj)),
        static_cast<int>(GetDouble("utilityType", obj)),
        GetDouble("utilityCost", obj),
        static_cast<int>(GetDouble("measurementMethod", obj)),
        getCompressedAirFlowMeterMethodData(obj),
        getBagMethodData(obj),
        getPressureMethodData(obj),
        getCompressedAirOtherMethodData(obj),
        getCompressorElectricityData(obj),
        static_cast<int>(GetDouble("units", obj))};
}

CompressedAirReduction getCompressedAirReductionInputVec()
{
    auto const compressedAirReductionInputVecTemp = Nan::To<Object>(inp).ToLocalChecked()->Get(Nan::New<String>("compressedAirReductionInputVec").ToLocalChecked());
    auto const &compressedAirReductionInputArray = v8::Local<v8::Array>::Cast(compressedAirReductionInputVecTemp);
    std::vector<CompressedAirReductionInput> inputVec;
    for (std::size_t i = 0; i < compressedAirReductionInputArray->Length(); i++)
    {
        inputVec.emplace_back(constructCompressedAirReductionInput(Nan::To<Object>(compressedAirReductionInputArray->Get(i)).ToLocalChecked()));
    }
    return inputVec;
}

NAN_METHOD(compressedAirReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();
    try
    {
        auto rv = CompressedAirReduction(getCompressedAirReductionInputVec()).calculate();
        SetR("energyUse", rv.energyUse);
        SetR("energyCost", rv.energyCost);
        SetR("flowRate", rv.flowRate);
        SetR("singleNozzleFlowRate", rv.singleNozzleFlowRate);
        SetR("consumption", rv.consumption);
    }
    catch (std::runtime_error const &e)
    {
        std::string const what = e.what();
        ThrowError(std::string("std::runtime_error thrown in compressedAirReduction - calculator.h: " + what).c_str());
    }
    info.GetReturnValue().Set(r);
}

// ========== END Compressed Air ==============

// ========== Start Water Reduction ===========

MeteredFlowMethodData getMeteredFlowMethodData(Local<Object> obj)
{
    auto meteredFlowMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("meteredFlowMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("meterReading", meteredFlowMethodDataV8)};
}

VolumeMeterMethodData getVolumeMeterMethodData(Local<Object> obj)
{
    auto volumeMeterMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("volumeMeterMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("finalMeterReading", volumeMeterMethodDataV8),
        GetDouble("initialMeterReading", volumeMeterMethodDataV8),
        GetDouble("elapsedTime", volumeMeterMethodDataV8)};
}

BucketMethodData getBucketMethodData(Local<Object> obj)
{
    auto bucketMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("bucketMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("bucketVolume", bucketMethodDataV8),
        GetDouble("bucketFillTime", bucketMethodDataV8)};
}

WaterOtherMethodData getWaterOtherMethodData(Local<Object> obj)
{
    auto otherMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("otherMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("consumption", otherMethodDataV8)};
}

WaterReductionInput constructWaterReductionInput(Local<Object> obj)
{
    return {
        static_cast<int>(GetDouble("hoursPerYear", obj)),
        GetDouble("waterCost", obj),
        static_cast<int>(GetDouble("measurementMethod", obj)),
        getMeteredFlowMethodData(obj),
        getVolumeMeterMethodData(obj),
        getBucketMethodData(obj),
        getWaterOtherMethodData(obj)};
}

WaterReduction getWaterReductionInputVec()
{
    auto waterReductionInputVecV8 = Nan::To<Object>(inp).ToLocalChecked()->Get(Nan::New<String>("waterReductionInputVec").ToLocalChecked());
    auto const &waterReductionInputArray = v8::Local<v8::Array>::Cast(waterReductionInputVecV8);
    std::vector<WaterReductionInput> inputVec;
    for (std::size_t i = 0; i < waterReductionInputArray->Length(); i++)
    {
        inputVec.emplace_back(constructWaterReductionInput(Nan::To<Object>(waterReductionInputArray->Get(i)).ToLocalChecked()));
    }
    return inputVec;
}

NAN_METHOD(waterReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();
    try
    {
        auto rv = WaterReduction(getWaterReductionInputVec()).calculate();
        SetR("waterUse", rv.waterUse);
        SetR("waterCost", rv.waterCost);
    }
    catch (std::runtime_error const &e)
    {
        std::string const what = e.what();
        ThrowError(std::string("std::runtime_error thrown in waterReduction - calculator.h: " + what).c_str());
    }
    info.GetReturnValue().Set(r);
}

// ========== END water reduction =============

// ========== Start CA Pressure Reduction ===========
CompressedAirPressureReductionInput constructCompressedAirPressureReductionInput(Local<Object> obj)
{
    return {
        GetBool("isBaseline", obj),
        static_cast<int>(GetDouble("hoursPerYear", obj)),
        GetDouble("electricityCost", obj),
        GetDouble("compressorPower", obj),
        GetDouble("pressure", obj),
        GetDouble("proposedPressure", obj)};
}

CompressedAirPressureReduction getCompressedAirPressureReductionInputVec()
{
    auto const compressedAirPressureReductionInputVecV8 = Nan::To<Object>(inp).ToLocalChecked()->Get(Nan::New<String>("compressedAirPressureReductionInputVec").ToLocalChecked());
    auto const &compressedAirPressureReductionInputArray = v8::Local<v8::Array>::Cast(compressedAirPressureReductionInputVecV8);
    std::vector<CompressedAirPressureReductionInput> inputVec;
    for (std::size_t i = 0; i < compressedAirPressureReductionInputArray->Length(); i++)
    {
        inputVec.emplace_back(constructCompressedAirPressureReductionInput(Nan::To<Object>(compressedAirPressureReductionInputArray->Get(i)).ToLocalChecked()));
    }
    return inputVec;
}

NAN_METHOD(compressedAirPressureReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();
    try
    {
        auto rv = CompressedAirPressureReduction(getCompressedAirPressureReductionInputVec()).calculate();
        SetR("energyUse", rv.energyUse);
        SetR("energyCost", rv.energyCost);
    }
    catch (std::runtime_error const &e)
    {
        std::string const what = e.what();
        ThrowError(std::string("std::runtime_error thrown in compressedAirPressureReduction - calculator.h: " + what).c_str());
    }
    info.GetReturnValue().Set(r);
}
// ========== END CA Pressure Reduction ===========

// ========== Start Pipe Insulation Reduction ===========
NAN_METHOD(pipeInsulationReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();

    int operatingHours = static_cast<int>(GetDouble("operatingHours", inp));
    double pipeLength = GetDouble("pipeLength", inp);
    double pipeDiameter = GetDouble("pipeDiameter", inp);
    double pipeThickness = GetDouble("pipeThickness", inp);
    double pipeTemperature = GetDouble("pipeTemperature", inp);
    double ambientTemperature = GetDouble("ambientTemperature", inp);
    double windVelocity = GetDouble("windVelocity", inp);
    double systemEfficiency = GetDouble("systemEfficiency", inp) / 100.0;
    double insulationThickness = GetDouble("insulationThickness", inp);
    double pipeEmissivity = GetDouble("pipeEmissivity", inp);
    double jacketEmissivity = GetDouble("jacketEmissivity", inp);
    std::vector<double> pipeMaterialCoefficients = GetVector("pipeMaterialCoefficients", inp);
    std::vector<double> insulationMaterialCoefficients = GetVector("insulationMaterialCoefficients", inp);

    InsulatedPipeInput input(
        operatingHours,
        pipeLength,
        pipeDiameter,
        pipeThickness,
        pipeTemperature,
        ambientTemperature,
        windVelocity,
        systemEfficiency,
        insulationThickness,
        pipeEmissivity,
        jacketEmissivity,
        pipeMaterialCoefficients,
        insulationMaterialCoefficients);
    InsulatedPipeCalculator calculator(input);
    InsulatedPipeOutput output = calculator.calculate();
    SetR("heatLength", output.getHeatLength());
    SetR("annualHeatLoss", output.getAnnualHeatLoss());
    info.GetReturnValue().Set(r);
}
// ========== END Pipe Insulation Reduction ===========

// ========== Start Tank Insulation Reduction ===========
NAN_METHOD(tankInsulationReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();

    int operatingHours = static_cast<int>(GetDouble("operatingHours", inp));
    double tankHeight = GetDouble("tankHeight", inp);
    double tankDiameter = GetDouble("tankDiameter", inp);
    double tankThickness = GetDouble("tankThickness", inp);
    double tankEmissivity = GetDouble("tankEmissivity", inp);
    double tankConductivity = GetDouble("tankConductivity", inp);
    double tankTemperature = GetDouble("tankTemperature", inp);
    double ambientTemperature = GetDouble("ambientTemperature", inp);
    double systemEfficiency = GetDouble("systemEfficiency", inp) / 100.0;
    double insulationThickness = GetDouble("insulationThickness", inp);
    double insulationConductivity = GetDouble("insulationConductivity", inp);
    double jacketEmissivity = GetDouble("jacketEmissivity", inp);

    InsulatedTankInput input(
        operatingHours,
        tankHeight,
        tankDiameter,
        tankThickness,
        tankEmissivity,
        tankConductivity,
        tankTemperature,
        ambientTemperature,
        systemEfficiency,
        insulationThickness,
        insulationConductivity,
        jacketEmissivity);
    InsulatedTankCalculator calculator(input);
    InsulatedTankOutput output = calculator.calculate();
    SetR("heatLoss", output.getHeatLoss());
    SetR("annualHeatLoss", output.getAnnualHeatLoss());
    info.GetReturnValue().Set(r);
}
// ========== END Tank Insulation Reduction ===========

// ============ Start Steam Reduction =============
SteamFlowMeterMethodData getSteamFlowMeterMethodData(Local<Object> obj)
{
    auto flowMeterMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("flowMeterMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("flowRate", flowMeterMethodDataV8)};
}

SteamMassFlowMeasuredData getSteamMassFlowMeasuredData(Local<Object> obj)
{
    auto massFlowMeasuredDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("massFlowMeasuredData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("areaOfDuct", massFlowMeasuredDataV8),
        GetDouble("airVelocity", massFlowMeasuredDataV8)};
}

SteamMassFlowNameplateData getSteamMassFlowNameplateData(Local<Object> obj)
{
    auto massFlowNameplateDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("massFlowNameplateData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("flowRate", massFlowNameplateDataV8)};
}

SteamMassFlowMethodData getSteamAirMassFlowMethodData(Local<Object> obj)
{
    auto massFlowMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("airMassFlowMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetBool("isNameplate", massFlowMethodDataV8),
        getSteamMassFlowMeasuredData(massFlowMethodDataV8),
        getSteamMassFlowNameplateData(massFlowMethodDataV8),
        GetDouble("inletTemperature", massFlowMethodDataV8),
        GetDouble("outletTemperature", massFlowMethodDataV8)};
}

SteamMassFlowMethodData getSteamWaterMassFlowMethodData(Local<Object> obj)
{
    auto massFlowMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("waterMassFlowMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetBool("isNameplate", massFlowMethodDataV8),
        getSteamMassFlowMeasuredData(massFlowMethodDataV8),
        getSteamMassFlowNameplateData(massFlowMethodDataV8),
        GetDouble("inletTemperature", massFlowMethodDataV8),
        GetDouble("outletTemperature", massFlowMethodDataV8)};
}

SteamOtherMethodData getSteamOtherMethodData(Local<Object> obj)
{
    auto otherMethodDataV8 = Nan::To<Object>(obj->Get(Nan::New<String>("otherMethodData").ToLocalChecked())).ToLocalChecked();
    return {
        GetDouble("consumption", otherMethodDataV8)};
}

SteamReductionInput constructSteamReductionInput(Local<Object> obj)
{
    return {
        static_cast<int>(GetDouble("hoursPerYear", obj)),
        static_cast<int>(GetDouble("utilityType", obj)),
        GetDouble("utilityCost", obj),
        static_cast<int>(GetDouble("measurementMethod", obj)),
        GetDouble("systemEfficiency", obj) / 100.0,
        GetDouble("pressure", obj),
        getSteamFlowMeterMethodData(obj),
        getSteamAirMassFlowMethodData(obj),
        getSteamWaterMassFlowMethodData(obj),
        getSteamOtherMethodData(obj),
        static_cast<int>(GetDouble("units", obj))};
}

SteamReduction getSteamReductionInputVec()
{
    auto const steamReductionInputVecV8 = Nan::To<Object>(inp).ToLocalChecked()->Get(Nan::New<String>("steamReductionInputVec").ToLocalChecked());
    auto const &steamReductionInputArray = v8::Local<v8::Array>::Cast(steamReductionInputVecV8);
    std::vector<SteamReductionInput> inputVec;
    for (std::size_t i = 0; i < steamReductionInputArray->Length(); i++)
    {
        inputVec.emplace_back(constructSteamReductionInput(Nan::To<Object>(steamReductionInputArray->Get(i)).ToLocalChecked()));
    }
    return inputVec;
}

NAN_METHOD(steamReduction)
{
    inp = Nan::To<Object>(info[0]).ToLocalChecked();
    r = Nan::New<Object>();
    try
    {
        auto rv = SteamReduction(getSteamReductionInputVec()).calculate();
        SetR("steamUse", rv.steamUse);
        SetR("energyUse", rv.energyUse);
        SetR("energyCost", rv.energyCost);
    }
    catch (std::runtime_error const &e)
    {
        std::string const what = e.what();
        ThrowError(std::string("std::runtime_error thrown in steamReduction - calculator.h: " + what).c_str());
    }
    info.GetReturnValue().Set(r);
}
// ============ END Steam Reduction =============
