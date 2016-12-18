#include "catch.hpp"
#include <calculator/LiquidCoolingLosses.h>

TEST_CASE( "Calculate Heat Loss for liquid cooling Losses ethylene glycon", "[Heat Loss][Ethylene Glycon][Liquid]") {
    CHECK(LiquidCoolingLosses(100.0, 9.35, 80.0, 210.0, 0.52).getHeatLoss() == Approx( 3792360.0));
}