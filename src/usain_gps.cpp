//
// Created by Bas on 23-5-2018.
//

#include "usain_gps.h"
#include "drv_gps.h"

#define pi 3.141592653589793238462

double deg2rad(double deg);

double rad2deg(double rad);

UsainGPS::UsainGPS() {}

UsainGPS::~UsainGPS() {

}

uint8_t UsainGPS::init() {
    uint8_t return_value = 0;
    int status;
    status = _gps.coldstart();
    if (status)
    {
        return_value |= 0x01;
    }


    while (_gps.setbaudrateto115200());
    Timer delaytje;
    delaytje.reset();
    delaytje.start();
    while (delaytje.read() < 1);
    while(_gps.setupdaterate((char *) "100"));
    if (status)
    {
        return_value |= 0x02;
    }
    status = _gps.onlyreceivegprmcdata();
    if (status)
    {
        return_value |= 0x04;
    }

    if (_update.start(callback(this, &UsainGPS::update)) == osOK)
    {
    };

    return return_value;
}

int UsainGPS::get_gps_message(AdafruitUltimateGPS::gprmc_data_t &dest) {
    _gps.GetLastGprmcData(&dest);
    if (!_gps.ReceievedNewGPRMC())
    {
        return -1;
    }
    _gps.ReceievedNewGPRMC(false);
    if (*dest.validity == 'V')
    {
        return -2; // data not valid
    }
    return 0;
}

//haversine method
void UsainGPS::get_distance_centimeter(AdafruitUltimateGPS::gprmc_data_t &home_position,
                                        AdafruitUltimateGPS::gprmc_data_t &destination_position, double *distance_cm, double *bearing_degrees) {
    const double R = 6378.137e5;
    double rlat1 = home_position.latitude_fixed*(pi/180);
    double rlat2 = destination_position.latitude_fixed*(pi/180);
    double dlon = (destination_position.longitude_fixed - home_position.longitude_fixed) *(pi/180);
    double dlat = (destination_position.latitude_fixed - home_position.latitude_fixed)*(pi/180);
    double a = pow(sin(dlat/2.0),2.0) + cos(rlat1) * cos(rlat2) * pow(sin(dlon/2.0),2.0);
    //double c = 2 * asin(min(one,sqrt(a)));
    double c = 2 * atan2(sqrt(a),sqrt(1.0-a));

//    *distance_cm = R * c;
    double rad_bearing = atan2(sin(dlon) * cos(rlat2),
                              cos(rlat1) * sin(rlat2)
                              - sin(rlat1) * cos(rlat2) * cos(dlon));

    *distance_cm = (R * c);
    *bearing_degrees = (rad2deg(rad_bearing)) < 0 ? (rad2deg(rad_bearing)) + 360: (rad2deg(rad_bearing));
}


bool UsainGPS::data_received() {
    return _gps.ReceievedNewGPRMC();
}

void UsainGPS::update() {
    while (1)
    {
        _gps.parsedata();
    }
}


double deg2rad(double deg) {

    return (deg * (pi / 180));

}

double rad2deg(double rad) {

    return (rad * (180 / pi));

}

