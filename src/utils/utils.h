#ifndef UTILS_H
#define UTILS_H

namespace Utils {

// Converts degrees to radians.
double DegToRad(double deg);

// Great-circle (Haversine) distance between two lat/lon points, in
// nautical miles.
double GreatCircleDistanceNm(double lat1, double lon1, double lat2, double lon2);

} // namespace Utils

#endif // UTILS_H