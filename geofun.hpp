#ifndef __GEOFUN_HPP
#define __GEOFUN_HPP

#include <math.h>
#include <algorithm>
#include <assert.h>

namespace geofun {

static const double a = 6378137.0;   // Earth equatorial radius
static const double b = 6356752.3;   // Earth polar radius
static const double f = (a - b) / a; // Earth ellipsoid flattening
static const double pi = 3.14159265358979323846;
static const double two_pi = 2 * pi;
static const double half_pi = 0.5 * pi;
static const double sqa = a * a;
static const double sqb = b * b;

inline double deg_to_rad(const double degs)
{
  return degs * pi / 180;
}

inline double rad_to_deg(const double rads)
{
  return rads * 180 / pi;
}

inline double sqr(const double x)
{
  return x * x;
}

inline double reduced_latitude(const double geodetic_latitude) 
{
  return atan2((1 - f) * sin(geodetic_latitude), cos(geodetic_latitude));
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

struct Simple {
  virtual double operator[](int i) const {
    return 0;
  }
};

struct Complex {
  virtual const Simple& operator[](int i) const = 0; 
};

struct Coord: Simple {
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
  virtual double operator[](int i) const {
    switch (i) {
      case 0: return _x;
      case 1: return _y;
      default: return 0;
    }
  }
  Coord operator+(const Coord& coord) const
  {
    Coord c = coord;
    c += *this;
    return c;
  }
  Coord operator-(const Coord& coord) const
  {
    Coord c = *this;
    c -= coord;
    return c;
  }
  Coord operator*(const double value) const
  {
    Coord c = *this;
    c *= value;
    return c;
  }

  double x() const {
    return _x;
  }
  double y() const {
    return _y;
  }
  void x(const double value) {
    _x = value;
  }
  void y(const double value) {
    _y = value;
  }
private:
  double _x;
  double _y;
};

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


struct Vector: Simple {
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
    v._a = norm_angle_2pi(_a + pi);
    return v;
  }
  Vector& operator*=(const double value) {
    _r *= value;
    return *this;
  }
  Vector operator*(const double value) const
  {
    Vector result = *this;
    result *= value;
    return result;
  }
  virtual double operator[](int i) const {
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
  void a(const double value) {
    _a = norm_angle_2pi(value);
  }
  void r(const double value) {
    _r = value;
  }
private:
  double _a;
  double _r;
};


inline Vector operator*(const double value, const Vector& vector)
{
  Vector result = vector;
  result *= value;
  return result;
}

struct Position: Simple {
  Position(): _lat(0), _lon(0) {}
  Position(const double latitude, const double longitude): _lat(0), _lon(0) {
    latlon(latitude, longitude);
  }
  Position(const Position& position): _lat(position._lat), _lon(position._lon) {}
  Position& operator=(const Position& position) { 
    _lat = position._lat;
    _lon = position._lon;
    return *this;
  }
  Position& operator+=(const Vector& value); 
  Vector operator-(const Position& position) const;
  inline Position operator+(const Vector& vector) const 
  {
    Position result(*this);
    result += vector;
    return result;
  }
  virtual double operator[](int i) const {
    switch (i) {
      case 0: return _lat;
      case 1: return _lon;
      default: return 0;
    }
  }
  double lat() const {
    return _lat;
  }
  double lon() const {
    return _lon;
  }
  void lat(const double value) {
    _lat = value;
    if (norm_angle_pi2pi2(&_lat)) {
      lon(lon() + pi);
    }
  }
  void lon(const double value) {
    _lon = norm_angle_pipi(value);
  }
  void latlon(const double latitude, const double longitude) {
    // When doing it in this order flying over the pole should work
    lon(longitude);
    lat(latitude);
  }
  Coord cartesian_deltas(void) const {
    double rl = reduced_latitude(lat());
    return Coord(
        a * b * sqrt(sqa * sqr(sin(rl)) + sqb * sqr(cos(rl))) 
            / ((sqa - sqb) * sqr(cos(lat())) + sqb),
        a * cos(rl));
  }
private:
  double _lat;
  double _lon;
};


struct Line: Complex {
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
    return *this;
  }
  virtual const Simple& operator[](int i) const {
    switch(i) {
      case 0: return _p1;
      case 1: return _v;
      case 2: return _p2;
    }
    return _p1;
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
  void p1(const Position& position) {
    _p1 = position;
    _v = _p2 - _p1;
  }
  void p2(const Position& position) {
    _p2 = position;
    _v = _p2 - _p1;
  }
  void v(const Vector& vector) {
    _p2 = _p1 + vector;
    // v will not be set exactly...
    _v = _p2 - _p1;
  }
  double min_lat() const {
    return std::min(_p1.lat(), _p2.lat());
  }
  double max_lat() const {
    return std::max(_p1.lat(), _p2.lat());
  }
  double min_lon() const {
    return std::min(_p1.lon(), _p1.lon());
  }
  double max_lon() const {
    return std::max(_p1.lon(), _p2.lon());
  }
  bool intersects(const Line& line) const;
  Position intersection(const Line& line) const;
private:
  Position _p1;
  Position _p2;
  Vector _v;
};

struct Arc: Complex {
  Arc(): _p1(), _p2(), _v(), _r() {}
  Arc(const Position& p1, const Position& p2): _p1(p1), _p2(p2) {
    vincenty_inverse(p1, p2, &_v, &_r, &_alpha);
  }
  Arc(const Position& p1, const Vector& v): _p1(p1), _v(v) {
    vincenty_direct(p1, v, &_p2, &_r, &_alpha);
  }
  Arc(const Arc& arc): _p1(arc._p1), _p2(arc._p2), _v(arc._v), _r(arc._r) {}
  Arc& operator=(const Arc& arc) {
    _p1 = arc._p1;
    _p2 = arc._p2;
    _v = arc._v;
    _r = arc._r;
    return *this;
  }
  Arc& operator+=(const Vector& vector) {
    Position p;
    Vector r;
    double alpha;
    vincenty_direct(_p2, vector, &p, &r, &alpha);
    p2(p);
    return *this;
  }
  virtual const Simple& operator[](int i) const {
    switch(i) {
      case 0: return _p1;
      case 1: return _v;
      case 2: return _p2;
      case 3: return _r;
    }
    return _p1;
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
  const Vector& r() const {
    return _r;
  }
  void p1(const Position& position) {
    _p1 = position;
    vincenty_inverse(_p1, _p2, &_v, &_r, &_alpha);
  }
  void p2(const Position& position) {
    _p2 = position;
    vincenty_inverse(_p1, _p2, &_v, &_r, &_alpha);
  }
  void v(const Vector& vector) {
    _v = vector;
    vincenty_direct(_p1, vector, &_p2, &_r, &_alpha);
  }
  void r(const Vector& vector) {
    _r = vector;
    vincenty_direct(_p2, vector, &_p1, &_v, &_alpha);
  }
  double min_lat() const {
    // TODO
    return std::min(_p1.lat(), _p2.lat());
  }
  double max_lat() const {
    // TODO
    return std::max(_p1.lat(), _p2.lat());
  }
  double min_lon() const {
    return std::min(_p1.lon(), _p1.lon());
  }
  double max_lon() const {
    return std::max(_p1.lon(), _p2.lon());
  }
  bool intersects(const Line& line) const;
  Position intersection(const Line& line) const;
  bool intersects(const Arc& arc) const;
  Position intersection(const Arc& arc) const;
protected:
  static void vincenty_inverse(const Position& p1, const Position& p2, Vector* v, Vector* r, double* alpha);
  static void vincenty_direct(const Position& p1, const Vector& v, Position* p2, Vector* r, double* alpha);
private:
  Position _p1;
  Position _p2;
  Vector _v;     // forward vector
  Vector _r;     // reverse vector
  double _alpha; // azimuth at equator
};

};  // namespace geofun

#endif // __GEOFUN_H
