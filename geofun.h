#include <math.h>
#include <algorithm>

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
  void set_x(const double value) {
    _x = value;
  }
  void set_y(const double value) {
    _y = value;
  }
private:
  double _x;
  double _y;
};

inline Coord operator+(const Coord& coord1, const Coord& coord2) 
{
  Coord c = coord1;
  c += coord2;
  return c;
}

inline Coord operator-(const Coord& coord1, const Coord& coord2) 
{
  Coord c = coord1;
  c -= coord2;
  return c;
}

inline Coord operator*(const Coord& coord, const double value) 
{
  Coord c = coord;
  c *= value;
  return c;
}

inline Coord operator*(const double value, const Coord& coord) 
{
  Coord c = coord;
  c *= value;
  return c;
}

inline Coord operator/(const double value, const Coord& coord)
{
  return Coord(value / coord.x(), value / coord.y());
}


struct Vector {
  Vector(): _a(0), _r(0) {}
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
  Vector operator-() {
    Vector v(*this);
    v._r = norm_angle_2pi(_r + pi);
    return v;
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
    _a = norm_angle_2pi(value);
  }
  void set_r(const double value) {
    _r = value;
  }
private:
  double _a;
  double _r;
};

inline Vector operator*(const Vector& vector1, const double vector2)
{
  Vector result = vector1;
  result *= vector2;
  return result;
}

inline Vector operator*(const double value, const Vector& vector)
{
  Vector result = vector;
  result *= value;
  return result;
}

struct Position {
  Position(): _lat(0), _lon(0) {}
  Position(const double latitude, const double longitude): _lat(0), _lon(0) {
    set_latlon(latitude, longitude);
  }
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
inline Position operator+(const Position& position, const Vector& vector) 
{
  Position result(position);
  result += vector;
  return result;
}

struct Line {
  Line(): _p1(), _p2(), _v() {}
  Line(const Line& line): _p1(line._p1), _p2(line._p2), _v(line._v) {}
  Line(const Position& position1, const Position& position2):
    _p1(position1), _p2(position2), _v(position2 - position1) {}
  Line(const Position& position, const Vector& vector):
    _p1(position), _p2(position + vector), _v(_p2 - position) {}
  Line& operator=(const Line& line) {
    _p1 = line._p1;
    _p2 = line._p2;
    _v = line._v;
  }
  const Position& p1() const {
    return _p1;
  }
  const Position& p2() const {
    return _p2;
  }
  const Vector& v() const {
    return _v;
  }
  void set_p1(const Position& position) {
    _p1 = position;
    _v = _p2 - _p1;
  }
  void set_p2(const Position& position) {
    _p2 = position;
    _v = _p2 - _p1;
  }
  void set_v(const Vector& vector) {
    _p2 = _p1 + vector;
    // v will not be set exactly...
    _v = _p2 - _p1;
  }
  double min_lat() const {
    std::min(_p1.lat(), _p2.lat());
  }
  double max_lat() const {
    std::max(_p1.lat(), _p2.lat());
  }
  double min_lon() const {
    std::min(_p1.lon(), _p1.lon());
  }
  double max_lon() const {
    std::max(_p1.lon(), _p2.lon());
  }
  bool intersects(const Line& line) const;
  Position intersection(const Line& line) const;
private:
  Position _p1;
  Position _p2;
  Vector _v;
};

};  // namespace geofun

#endif // __GEOFUN_H
