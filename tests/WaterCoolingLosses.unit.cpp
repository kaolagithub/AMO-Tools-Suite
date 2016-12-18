#include "catch.hpp"
#include <calculator/WaterCoolingLosses.h>

TEST_CASE( "Calculate Heat Loss for liquid cooling Losses Water", "[Heat Loss]" ) {

    REQUIRE( WaterCoolingLosses(100.0, 80.0, 120.0).getHeatLoss() == Approx( 1989600.0 ) );

}