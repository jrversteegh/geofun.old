#include <math.h>

#ifndef __GEOFUN_H
#define __GEOFUN_H

namespace geofun {

static const double a = 6378137.0;   // Earth equatorial radius
static const double b = 6356752.3;   // Earth polar radius
static const double f = (a - b) / a; // Earth ellipsoid flattening
static const double pi = 3.14159265358979323846;
static const double two_pi = 2 * pi;
static const double half_pi = 0.5 * pi;
static const double sqra = a * a;
static const double sqrb = b * b;



inline double sqr(const double x)
{
  return x * x;
}

inline double reduced_latitude(const double geodetic_latitude) 
{
  return atan((1 - f) * tan(geodetic_latitude));
}

inline double norm_angle_pipi(const double angle)
{
  double result = angle;
  while (result < -pi) {
    result += two_pi;
  }
  while (result >= pi) {
    result -= two_pi;
  }
  return result;
}

inline double norm_angle_2pi(const double angle)
{
  double result = angle;
  while (result < 0) {
    result += two_pi;
  }
  while (result >= two_pi) {
    result -= two_pi;
  }
  return result;
}

inline bool norm_angle_pi2pi2(double* angle)
{
  *angle = norm_angle_pipi(*angle);
  if (*angle > half_pi) {
    *angle = pi - *angle;
    return true;
  }
  else if (*angle < -half_pi) {
    *angle = -pi - *angle;
    return true;
  }
  else {
    return false;
  }
}


inline double angle_diff(const double angle1, const double angle2) 
{
  return norm_angle_pipi(angle1 - angle2);
}


struct Coord {
  Coord(): _x(0), _y(0) {}
  Coord(const double x, const double y): _x(x), _y(y) {}
  Coord& operator=(const Coord& value) {
    _x = value._x;
    _y = value._y;
    return *this;
  }
  Coord& operator*=(const double value) {
    _x *= value;
    _y *= value;
    return *this;
  }
  Coord& operator+=(const Coord& value) {
    _x += value._x;
    _y += value._y;
    return *this;
  }
  Coord& operator-=(const Coord& value) {
    _x -= value._x;
    _y -= value._y;
    return *this;
  }
  double operator[](int i) {
    switch (i) {
      case 0: return _x;
      case 1: return _y;
      default: return 0;
    }
  }
  double x() const {
    return _x;
  }
  double y() const {
    return _y;
  }
private:
  double _x;
  double _y;
};

inline Coord operator+(const Coord& value1, const Coord& value2) 
{
  Coord c = value1;
  c += value2;
  return c;
}

inline Coord operator-(const Coord& value1, const Coord& value2) 
{
  Coord c = value1;
  c -= value2;
  return c;
}

inline Coord operator*(const Coord& value1, const double value2) 
{
  Coord c = value1;
  c *= value2;
  return c;
}

inline Coord operator*(const double value1, const Coord& value2) 
{
  Coord c = value2;
  c *= value1;
  return c;
}


struct Vector {
  Vector(const double angle, const double range): _a(angle), _r(range) {}
  Vector(const Vector& vector): _a(vector._a), _r(vector._r) {}
  Vector(const Coord& coord) {
    _r = sqrt(sqr(coord.x()) + sqr(coord.y()));
    _a = norm_angle_2pi(atan2(coord.y(), coord.x()));
  }

  Vector& operator=(const Vector& vector) {
    _a = vector._a;
    _r = vector._r;
    return *this;
  }
  Vector& operator*=(const double value) {
    _r *= value;
  }
  double operator[](int i) {
    switch (i) {
      case 0: return _a;
      case 1: return _r;
      default: return 0;
    }
  }

  Coord cartesian() const {
    return Coord(_r * cos(_a), _r * sin(_a));
  }
  double a() const {
    return _a;
  }
  double r() const {
    return _r;
  }
  void set_a(const double value) {
    _a = value;
  }
  void set_r(const double value) {
    _r = value;
  }
private:
  double _a;
  double _r;
};

inline Vector operator*(const Vector& value1, const double value2)
{
  Vector result = value1;
  result *= value2;
  return result;
}

inline Vector operator*(const double value1, const Vector& value2)
{
  Vector result = value2;
  result *= value1;
  return result;
}

struct Position {
  Position(): _lat(0), _lon(0) {}
  Position(const double latitude, const double longitude): _lat(latitude), _lon(longitude) {}
  Position(const Position& position): _lat(position._lat), _lon(position._lon) {}
  Position& operator=(const Position& position) { 
    _lat = position._lat;
    _lon = position._lon;
    return *this;
  }
  Position& operator+=(const Vector& value); 
  double lat() const {
    return _lat;
  }
  double operator[](int i) {
    switch (i) {
      case 0: return _lat;
      case 1: return _lon;
      default: return 0;
    }
  }
  double lon() const {
    return _lon;
  }
  void set_lat(const double value) {
    _lat = value;
    if (norm_angle_pi2pi2(&_lat)) {
      set_lon(lon() + pi);
    }
  }
  void set_lon(const double value) {
    _lon = norm_angle_pipi(value);
  }
  void set_latlon(const double latitude, const double longitude) {
    // When doing it in this order flying over the pole should work
    set_lon(longitude);
    set_lat(latitude);
  }
  Coord cartesian_deltas(void) const {
    double rl = reduced_latitude(lat());
    return Coord(
        a * b * sqrt(sqra * sqr(sin(rl)) + sqrb * sqr(cos(rl))) 
            / ((sqra - sqrb) * sqr(cos(lat())) + sqrb),
        a * cos(rl));
  }
private:
  double _lat;
  double _lon;
};

Vector operator-(const Position& position1, const Position& position2);

};  // namespace geofun

#endif // __GEOFUN_H
