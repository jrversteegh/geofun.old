#ifndef __GEOFUN_HPP
#define __GEOFUN_HPP

#include <math.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>

namespace geofun {

static const double a = 6378137.0;   // Earth equatorial radius
static const double b = 6356752.3;   // Earth polar radius
static const double r = (a + b) / 2; // Average earth radius
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

inline double m_to_nm(const double meters)
{
  return meters / 1852;
}

inline double nm_to_m(const double miles)
{
  return miles * 1852;
}

inline double sqr(const double x)
{
  return x * x;
}

inline double reduced_latitude(const double geodetic_latitude) 
{
  return atan2((1 - f) * sin(geodetic_latitude), cos(geodetic_latitude));
}

inline double angle_pipi(const double angle)
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

inline double angle_2pi(const double angle)
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

inline bool angle_pi2pi2(double* angle)
{
  *angle = angle_pipi(*angle);

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
  return angle_pipi(angle1 - angle2);
}

inline bool floats_equal(const double value1, const double value2)
{
  double abs1 = fabs(value1);
  double abs2 = fabs(value2);
  double absmax = std::max(abs1, abs2);
  double eps = 1E-13;
  // Get relative eps value except for values very close to zero
  if (absmax > 1E-6) {
    eps *= absmax;
  }
  return fabs(value1 - value2) < eps;
}

inline bool float_smaller(const double value1, const double value2)
{
  return value1 < value2 and not floats_equal(value1, value2);
}


struct IndexError {
  IndexError(const int i): _i(i) {}
  const char* what() const throw() {
    snprintf(msg, 64, "Index out of range: %d", _i);
    return msg;
  }
private:
  int _i;
  static char msg[64];
};

struct EarthModelError {
  EarthModelError() {} 
  const char* what() const throw() {
    return "Unknown earth model. Available are: \"wgs84\", \"spherical\"";
  }
};
  
struct AngleModeError {
  AngleModeError() {} 
  const char* what() const throw() {
    return "Unknown angle mode. Available are: \"radians\", \"degrees\"";
  }
};

struct Simple {
  virtual ~Simple() {}
  virtual double operator[](int i) const {
    throw IndexError(i);
  }
  virtual int size() const {
    return 0;
  }
};

struct Complex {
  virtual ~Complex() {}
  virtual const Simple& operator[](int i) const = 0; 
  virtual int size() const {
    return 0;
  }
};

struct Coord: Simple {
  Coord(): _x(0), _y(0) {}
  Coord(const double x, const double y): _x(x), _y(y) {}
  Coord(const Coord& coord): _x(coord._x), _y(coord._y) {}
  Coord& operator=(const Coord& value) {
    _x = value._x;
    _y = value._y;
    return *this;
  }
  Coord operator-() const {
    return Coord(-_x, -_y);
  }
  bool operator==(const Coord& coord) const {
    return floats_equal(_x, coord._x) and floats_equal(_y, coord._y);
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
      default: throw IndexError(i);
    }
  }
  virtual int size() const {
    return 2;
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

  double dot(const Coord& coord) {
    return _x * coord._x + _y * coord._y;
  }
  double cross(const Coord& coord) {
    return _x * coord._y - _y * coord._x;
  }

  double get_x() const {
    return _x;
  }
  double get_y() const {
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

inline Coord operator*(const double value, const Coord& coord) 
{
  Coord c = coord;
  c *= value;
  return c;
}

inline Coord operator/(const double value, const Coord& coord)
{
  return Coord(value / coord.get_x(), value / coord.get_y());
}

struct EarthModel {
  virtual ~EarthModel() {}
  virtual Coord cartesian_deltas(const double lat) {
    return Coord(1, 1);
  };
};

struct Sphere: EarthModel {
  virtual Coord cartesian_deltas(const double lat) {
    return Coord(r, r * cos(lat));
  }
};

struct WGS84: EarthModel {
  virtual Coord cartesian_deltas(const double lat) {
    double rl = reduced_latitude(lat);
    return Coord(
        a * b * sqrt(sqa * sqr(sin(rl)) + sqb * sqr(cos(rl))) 
            / ((sqa - sqb) * sqr(cos(lat)) + sqb),
        a * cos(rl));
  }
};


extern EarthModel* get_earth_model();
extern void set_earth_model(const std::string& model_name);
extern void set_angle_mode(const std::string& angle_mode);
typedef enum {am_radians, am_degrees} AngleMode;
extern AngleMode angle_mode;

inline double from_rads(const double value)
{
  if (angle_mode == am_radians)
    return value;
  else
    return value * 180 / pi;
}

inline double to_rads(const double value)
{
  if (angle_mode == am_radians)
    return value;
  else
    return value * pi / 180;
}

inline double to_degs(const double value)
{
  if (angle_mode == am_degrees)
    return value;
  else
    return value * 180 / pi;
}

inline double from_degs(const double value)
{
  if (angle_mode == am_degrees)
    return value;
  else
    return value * pi / 180;
}


struct Vector: Simple {
  Vector(): _a(0), _r(0) {}
  Vector(const double angle, const double range): _a(angle_2pi(to_rads(angle))), _r(range) {}
  Vector(const Vector& vector): _a(vector._a), _r(vector._r) {}
  Vector(const Coord& coord) {
    _r = sqrt(sqr(coord.get_x()) + sqr(coord.get_y()));
    _a = angle_2pi(atan2(coord.get_y(), coord.get_x()));
  }

  Vector& operator=(const Vector& vector) {
    _a = vector._a;
    _r = vector._r;
    return *this;
  }
  Vector operator-() const {
    Vector v(*this);
    v._a = angle_2pi(_a + pi);
    return v;
  }
  bool operator==(const Vector& vector) const {
    return floats_equal(_a, vector._a) and floats_equal(_r, vector._r);
  }

  Vector& operator*=(const double value) {
    _r *= value;
    return *this;
  }
  Vector& operator+=(const Vector& vector) {
    set_cartesian(cartesian() + vector.cartesian());
    return *this;
  }
  Vector& operator-=(const Vector& vector) {
    set_cartesian(cartesian() - vector.cartesian());
    return *this;
  }
  Vector operator*(const double value) const
  {
    Vector result = *this;
    result *= value;
    return result;
  }
  Vector operator+(const Vector& vector) const {
    Vector result(*this);
    result += vector;
    return result;
  }
  Vector operator-(const Vector& vector) const {
    Vector result(*this);
    result -= vector;
    return result;
  }
  virtual double operator[](int i) const {
    switch (i) {
      case 0: return from_rads(_a);
      case 1: return _r;
      default: throw IndexError(i);
    }
  }
  virtual int size() const {
    return 2;
  }
  bool operator>(const Vector& vector) const {
    return _r > vector._r;
  }
  bool operator<(const Vector& vector) const {
    return _r < vector._r;
  }
  bool operator>=(const Vector& vector) const {
    return _r >= vector._r;
  }
  bool operator<=(const Vector& vector) const {
    return _r <= vector._r;
  }

  Coord cartesian() const {
    return Coord(_r * cos(_a), _r * sin(_a));
  }
  void set_cartesian(const Coord& coord) {
    _r = hypot(coord.get_x(), coord.get_y());
    _a = angle_2pi(atan2(coord.get_y(), coord.get_x()));
  }
  double dot(const Vector& vector) const {
    return _r * vector._r * cos(vector._a - _a);
  }
  double cross(const Vector& vector) const {
    return _r * vector._r * sin(vector._a - _a);
  }
  double get_r() const {
    return _get_r();
  }
  void set_r(const double value) {
    _set_r(value);
  }
  double get_a() const {
    return from_rads(_get_a());
  }
  void set_a(const double value) {
    _set_a(to_rads(value));
  }
  friend class Line;
  friend class Arc;
private:
  double _a;
  double _r;
  double _get_r() const {
    return _r;
  }
  void _set_r(const double value) {
    _r = value;
  }
  double _get_a() const {
    return _a;
  }
  void _set_a(const double value) {
    _a = angle_2pi(value);
  }

};


inline Vector operator*(const double value, const Vector& vector)
{
  Vector result = vector;
  result *= value;
  return result;
}

inline double operator*(const Vector& v1, const Vector& v2)
{
  return v1.dot(v2);
}


struct Position: Simple {
  Position(): _lat(0), _lon(0) {}
  Position(const double latitude, const double longitude): _lat(0), _lon(0) {
    set_latlon(latitude, longitude);
  }
  Position(const Position& position): _lat(position._lat), _lon(position._lon) {}
  Position(const Simple& position): _lat(0), _lon(0) {
    *this = position;
  }
  Position& operator=(const Position& position) { 
    _lat = position._lat;
    _lon = position._lon;
    return *this;
  }
  Position& operator=(const Simple& position) { 
    if (position.size() > 2)  {
      _lat = position[0];
      _lon = position[1];
    }
    return *this;
  }
  bool operator==(const Position& position) const {
    return floats_equal(_lat, position._lat) and floats_equal(_lon, position._lon);
  }
  bool operator==(const Simple& position) const {
    if (position.size() < 2) 
      return false;
    return floats_equal(_lat, position[0]) and floats_equal(_lon, position[1]);
  }
  Position& operator+=(const Vector& vector); 
  Position& operator-=(const Vector& vector) {
    return *this += -vector;
  }
  Vector operator-(const Position& position) const;
  Vector operator-(const Simple& position) const;
  Position operator+(const Vector& vector) const {
    Position result(*this);
    result += vector;
    return result;
  }
  Position operator-(const Vector& vector) const {
    Position result(*this);
    result -= vector;
    return result;
  }

  virtual double operator[](int i) const {
    switch (i) {
      case 0: return from_rads(_lat);
      case 1: return from_rads(_lon);
      default: throw IndexError(i);
    }
  }
  bool operator<(const Simple& position) const {
    return float_smaller(_lat, position[0])
      or (floats_equal(_lat, position[0]) and float_smaller(_lon, position[1]));
  }
  bool operator<=(const Simple& position) const {
    return not operator>(position);
  }
  bool operator>(const Simple& position) const {
    return float_smaller(position[0], _lat)
      or (floats_equal(position[0], _lat) and float_smaller(position[1], _lon));
  }
  bool operator>=(const Simple& position) const {
    return not operator<(position);
  }
  int compare(const Simple& position) const {
    if (_lat < position[0])
      return -1;
    else if (_lat > position[0])
      return 1;
    else if (_lon < position[1])
      return -1;
    else if (_lon > position[1])
      return 1;
    else
      return 0;
  }

  virtual int size() const {
    return 2;
  }

  double get_lat() const {
    return from_rads(_lat);
  }
  double get_lon() const {
    return from_rads(_lon);
  }
  void set_lat(const double value) {
    _set_lat(to_rads(value));
  }
  void set_lon(const double value) {
    _set_lon(to_rads(value));
  }

  void set_latlon(const double latitude, const double longitude) {
    // When doing it in this order flying over the pole should work
    set_lon(longitude);
    set_lat(latitude);
  }
  Coord cartesian_deltas(void) const {
    return get_earth_model()->cartesian_deltas(_lat);
  }
private:
  friend class Line;
  friend class Arc;
  double _lat;
  double _lon;
  void _set_lon(const double value) {
    _lon = angle_pipi(value);
  }

  void _set_lat(const double value) {
    _lat = value;
    if (angle_pi2pi2(&_lat)) {
      _lon = angle_pipi(_lon + pi);
    }
  }
  void _set_latlon(const double latitude, const double longitude) {
    // When doing it in this order flying over the pole should work
    _set_lon(longitude);
    _set_lat(latitude);
  }
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
      default: throw IndexError(i);
    }
  }
  virtual int size() const {
    return 3;
  }
  const Position& get_p1() const {
    return _p1;
  }
  const Position& get_p2() const {
    return _p2;
  }
  const Vector& get_v() const {
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
    return std::min(_p1.get_lat(), _p2.get_lat());
  }
  double max_lat() const {
    return std::max(_p1.get_lat(), _p2.get_lat());
  }
  double min_lon() const {
    return _v._a <= pi ? _p1.get_lon() : _p2.get_lon();
  }
  double max_lon() const {
    return _v._a <= pi ? _p2.get_lon() : _p1.get_lon();
  }
  double get_length() const {
    return _v.get_r();
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
    set_p2(p);
    return *this;
  }
  virtual const Simple& operator[](int i) const {
    switch(i) {
      case 0: return _p1;
      case 1: return _v;
      case 2: return _p2;
      case 3: return _r;
      default: throw IndexError(i);
    }
  }
  virtual int size() const {
    return 4;
  }

  const Position& get_p1() const {
    return _p1;
  }
  const Position& get_p2() const {
    return _p2;
  }
  const Vector& get_v() const {
    return _v;
  }
  const Vector& get_r() const {
    return _r;
  }
  void set_p1(const Position& position) {
    _p1 = position;
    vincenty_inverse(_p1, _p2, &_v, &_r, &_alpha);
  }
  void set_p2(const Position& position) {
    _p2 = position;
    vincenty_inverse(_p1, _p2, &_v, &_r, &_alpha);
  }
  void set_v(const Vector& vector) {
    _v = vector;
    vincenty_direct(_p1, vector, &_p2, &_r, &_alpha);
  }
  void set_r(const Vector& vector) {
    _r = vector;
    vincenty_direct(_p2, vector, &_p1, &_v, &_alpha);
  }
  double min_lat() const {
    // TODO
    return std::min(_p1.get_lat(), _p2.get_lat());
  }
  double max_lat() const {
    // TODO
    return std::max(_p1.get_lat(), _p2.get_lat());
  }
  double min_lon() const {
    return std::min(_p1.get_lon(), _p1.get_lon());
  }
  double max_lon() const {
    return std::max(_p1.get_lon(), _p2.get_lon());
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
