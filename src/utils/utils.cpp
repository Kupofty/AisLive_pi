#define _USE_MATH_DEFINES

#include "utils.h"
#include <cmath>

namespace Utils {

double DegToRad(double deg)
{
    return deg * M_PI / 180.0;
}

double GreatCircleDistanceNm(double lat1, double lon1, double lat2, double lon2)
{
    constexpr double kEarthRadiusNm = 3440.065; // nautical miles

    double dLat = DegToRad(lat2 - lat1);
    double dLon = DegToRad(lon2 - lon1);

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(DegToRad(lat1)) * std::cos(DegToRad(lat2)) *
                   std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return kEarthRadiusNm * c;
}

} // namespace Utils