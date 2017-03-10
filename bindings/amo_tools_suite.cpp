
#include <nan.h>
#include "losses.h"
#include "psat-bridge.h"

using namespace Nan;

NAN_MODULE_INIT(InitAll) {

    // PSAT
    Nan::Set(target, New<String>("achievableEfficiency").ToLocalChecked(),
             GetFunction(New<FunctionTemplate>(achievableEfficiency)).ToLocalChecked());


    // PHAST Losses
    Nan::Set(target, New<String>("fixtureLosses").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(fixtureLosses)).ToLocalChecked());

    Nan::Set(target, New<String>("gasCoolingLosses").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(gasCoolingLosses)).ToLocalChecked());

    Nan::Set(target, New<String>("gasLoadChargeMaterial").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(gasLoadChargeMaterial)).ToLocalChecked());

    Nan::Set(target, New<String>("liquidCoolingLosses").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(liquidCoolingLosses)).ToLocalChecked());

    Nan::Set(target, New<String>("liquidLoadChargeMaterial").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(liquidLoadChargeMaterial)).ToLocalChecked());

    Nan::Set(target, New<String>("liquidLoadChargeMaterial").ToLocalChecked(),
             GetFunction(New<FunctionTemplate>(liquidLoadChargeMaterial)).ToLocalChecked());

    Nan::Set(target, New<String>("openingLosses").ToLocalChecked(),
             GetFunction(New<FunctionTemplate>(openingLosses)).ToLocalChecked());

    Nan::Set(target, New<String>("solidLoadChargeMaterial").ToLocalChecked(),
             GetFunction(New<FunctionTemplate>(solidLoadChargeMaterial)).ToLocalChecked());

    Nan::Set(target, New<String>("wallLosses").ToLocalChecked(),
             GetFunction(New<FunctionTemplate>(wallLosses)).ToLocalChecked());

    Nan::Set(target, New<String>("waterCoolingLosses").ToLocalChecked(),
             GetFunction(New<FunctionTemplate>(waterCoolingLosses)).ToLocalChecked());

    Nan::Set(target, New<String>("initTest").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(initTest)).ToLocalChecked());

}

NODE_MODULE(amo_tools_suite, InitAll)