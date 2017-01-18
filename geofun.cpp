#include "geofun.hpp"
#include <stdio.h>

//#include <iostream>
//using namespace std;

namespace geofun {

char IndexError::msg[64];

static WGS84 wgs84;
static Sphere sphere;


static EarthModel* earth_model = &wgs84;
AngleMode angle_mode = am_radians;

void set_earth_model(const std::string& model_name)
{
  if (model_name == "wgs84") {
    earth_model = &wgs84;
  }
  else if (model_name == "spherical") {
    earth_model = &sphere;
  }
  else if (model_name == "sphere") {
    earth_model = &sphere;
  }
  else {
    throw EarthModelError();
  }
}

void set_angle_mode(const std::string& mode_name)
{
  if (mode_name == "radians") {
    angle_mode = am_radians;
  }
  else if (mode_name == "degrees") {
    angle_mode = am_degrees;
  }
  else {
    throw AngleModeError();
  }
}

EarthModel* get_earth_model()
{
  return earth_model;
}

Position& Position::operator+=(const Vector& vector)
{
  Coord deltas1 = cartesian_deltas();
  Coord cart = vector.cartesian();
  Position mid_pos;
  mid_pos.set_latlon(
      _lat + 0.5 * cart.get_x() / deltas1.get_x(),
      _lon + 0.5 * cart.get_y() / deltas1.get_y());
  Coord deltas2 = mid_pos.cartesian_deltas();
  mid_pos.set_latlon(
      _lat + cart.get_x() / (deltas1.get_x() + deltas2.get_x()),
      _lon + cart.get_y() / (deltas1.get_y() + deltas2.get_y()));
  deltas2 = mid_pos.cartesian_deltas();
  Position end_pos;
  end_pos.set_latlon(
      _lat + cart.get_x() / deltas2.get_x(),
      _lon + cart.get_y() / deltas2.get_y());
  Coord deltas3 = end_pos.cartesian_deltas();
  Coord inv_deltas = (1.0 / 6) * (1 / deltas1 + 4 / deltas2 + 1 / deltas3);
  set_latlon(_lat + cart.get_x() * inv_deltas.get_x(), _lon + cart.get_y() * inv_deltas.get_y());
  return *this;
}

Vector Position::operator-(const Position& position) const
{
  double dlat = angle_diff(this->_lat, position._lat);
  double dlon = angle_diff(this->_lon, position._lon);
  Coord deltas1 = position.cartesian_deltas();
  Coord deltas3 = this->cartesian_deltas();
  Position mid_lat;
  mid_lat.set_lat(from_rads(0.5 * (this->_lat + position._lat)));
  Coord deltas2 = mid_lat.cartesian_deltas();
  Coord deltas = 6.0 / (1.0 / deltas1 + 4.0 / deltas2 + 1.0 / deltas3);
  Coord cart = Coord(dlat  * deltas.get_x(), dlon  * deltas.get_y());
  return Vector(cart);
}

Vector Position::operator-(const Simple& position) const
{
  Position p(position);
  return this->operator-(p);
}

bool Line::intersects(const Line& line) const { 
  if ((line.min_lat() > max_lat())
      or (line.max_lat() < min_lat())
      or (angle_diff(min_lon(), line.max_lon()) > 0 )
      or (angle_diff(line.min_lon(), max_lon()) > 0)) {
    return false;
  }
  Vector v11 = line._p1 - _p1;
  Vector v12 = line._p2 - _p1;
  Vector v21 = _p1 - line._p1;
  Vector v22 = _p2 - line._p1;
  double da1 = angle_diff(v11._a, _v._a);
  double da2 = angle_diff(v12._a, _v._a);
  if ((da1 > 0 and da2 < 0) or (da1 < 0 and da2 > 0)) {
    double da3 = angle_diff(v21._a, line._v._a);
    double da4 = angle_diff(v22._a, line._v._a);
    if ((da3 > 0 and da4 < 0) or (da3 < 0 and da4 > 0)) {
      return true;
    }
  }
  return false;
}

Position Line::intersection(const Line& line) const {
  if (!intersects(line)) {
    return Position(from_rads(-pi), 0);
  }
  
  Position p(_p1);
  double l;
  do {
    Vector v = line._p1 - p;
    double a1 = fabs(angle_diff(line._v._a, _v._a));
    double a2 = fabs(angle_diff(_v._a, v._a));
    double a3 = pi - a2 - a1;
    double d = sin(a3) * v._r;
    l = d / sin(a1);
    double f = l / _v._r;
    p += _v * f;
  } while (fabs(l) > 10);

  return p;
}

void Arc::vincenty_inverse(const Position& p1, const Position& p2, Vector* v, Vector* r, double* alpha)
{
  // Formula obtained from http://en.wikipedia.org/wiki/Vincenty%27s_formulae
  double u1 = reduced_latitude(p1._lat);
  double u2 = reduced_latitude(p2._lat);
  double dlinit = p2._lon - p1._lon;

  double sinu1 = sin(u1);
  double cosu1 = cos(u1);
  double sinu2 = sin(u2);
  double cosu2 = cos(u2);
  double sinu1sinu2 = sinu1 * sinu2;
  double cosu1cosu2 = cosu1 * cosu2;
  double cosu1sinu2 = cosu1 * sinu2;
  double sinu1cosu2 = sinu1 * cosu2;

  double dl = dlinit;
  
  double cosdl, sindl;
  double sig, sins, coss;
  double sina, sqcosa;
  double cos2sm, sqcos2sm, coss2sqcos2smm1;
  
  double dlprev;

  do {
    dlprev = dl;
    cosdl = cos(dl);
    sindl = sin(dl);
    sins = sqrt(sqr(cosu2 * sindl) + sqr(cosu1sinu2 - sinu1cosu2 * cosdl));
    coss = sinu1sinu2 + cosu1cosu2 * cosdl;
    sig = atan2(sins, coss);
    sina = cosu1cosu2 * sindl / sins;
    sqcosa = 1 - sqr(sina);
    cos2sm = coss - 2 * sinu1sinu2 / sqcosa;
    double c = f / 16 * sqcosa * (4 + f * (4 - 3 * sqcosa));
    sqcos2sm = sqr(cos2sm);
    coss2sqcos2smm1 = coss * (2 * sqcos2sm - 1);
    dl = dlinit + (1 - c) * f * sina * (sig + c * sins * (cos2sm + c * coss2sqcos2smm1));
  } while (fabs(dl -  dlprev) > 1E-7);

  double squ = sqcosa * (sqa - sqb) / sqb;
  /* Original ->
  double aa = 1 + squ / 16384 * (4096 + squ * (-768 + squ * (320 - 175 * squ)));
  double bb = squ / 1024 * (256 + squ * (-128 + squ * (74 - 47 * squ)));
  */
  // Modified ->
  double sqrtsqup1 = sqrt(1 + squ);
  double k1 = (sqrtsqup1 - 1) / (sqrtsqup1 + 1);
  double aa = (1 + 0.25 * sqr(k1)) / (1 - k1);
  double bb = k1 * (1 - (3.0 / 8) * sqr(k1));
    
  double dsig = bb * sins * (cos2sm + 0.25 * bb * (coss2sqcos2smm1 - 
      (1.0 / 6) * bb * cos2sm * (-3 + 4 * sqr(sins)) * (-3 + 4 * sqcos2sm)));
  v->set_r(b * aa * (sig - dsig));
  r->set_r(v->_r);
  v->set_a(atan2(cosu2 * sindl, cosu1sinu2 - sinu1cosu2 * cosdl));
  r->set_a(pi - atan2(cosu1 * sindl, -sinu1cosu2 + cosu1sinu2 * cosdl));
  *alpha = asin(sina);
}

void Arc::vincenty_direct(const Position& p1, const Vector& v, Position* p2, Vector* r, double* alpha)
{
  double u1 = reduced_latitude(p1._lat);
  double cosa1 = cos(v._a);
  double sina1 = sin(v._a);
  double sig1 = atan2(tan(u1), cosa1);
  double cosu1 = cos(u1);
  double sinu1 = sin(u1);
  double sina = cosu1 * sin(v._a);
  double sqcosa = (1 - sina) * (1 + sina);
  double squ = sqcosa * (sqa - sqb) / sqb;
  double sqrtsqup1 = sqrt(1 + squ);
  double k1 = (sqrtsqup1 - 1) / (sqrtsqup1 + 1);
  double aa = (1 + 0.25 * sqr(k1)) / (1 - k1);
  double bb = k1 * (1 - (3.0 / 8) * sqr(k1));
  double siginit = v._r / (b * aa);
  double sig = siginit;

  double sins, coss;
  double tsm, cos2sm, sqcos2sm, coss2sqcos2smm1;
 
  double sigprev;
  do {
    sigprev = sig;
    sins = sin(sig);
    coss = cos(sig);
    tsm = 2 * sig1 + sig;
    cos2sm = cos(tsm);
    sqcos2sm = sqr(cos2sm);
    coss2sqcos2smm1 = coss * (2 * sqcos2sm - 1);
    double dsig = bb * sins * (cos2sm + 0.25 * bb * (coss2sqcos2smm1 - 
        (1.0 / 6) * bb * cos2sm * (-3 + 4 * sqr(sins)) * (-3 + 4 * sqcos2sm)));
    sig = siginit + dsig;
  } while (fabs(sig - sigprev) > 1E-7);

  double f2 = atan2(sinu1 * coss + cosu1 * sins * cosa1,
      (1 - f) * sqrt(sqr(sina) + sqr(sinu1 * sins - cosu1 * coss * cosa1)));
  double dl = atan2(sins * sina1, cosu1 * coss - sinu1 * sins * cosa1);
  double c = f / 16 * sqcosa * (4 + f * (4 - 3 * sqcosa));
  double dlinit = dl 
      - (1 - c) * f * sina * (sig + c * sins * (cos2sm + c * coss2sqcos2smm1));
  r->set_a(pi - atan2(sina, -sinu1 * sins + cosu1 * coss * cosa1));
  r->set_r(v._r);
  p2->set_lat(f2);
  p2->set_lon(p1._lon + dlinit);
  *alpha = asin(sina);
}

bool Arc::intersects(const Line& line) const
{
  // TODO
  return false;
}

Position Arc::intersection(const Line& line) const
{
  return Position(from_rads(pi), 0);
}

bool Arc::intersects(const Arc& arc) const
{
  // TODO
  return false;
}

Position Arc::intersection(const Arc& arc) const
{
  return Position(from_rads(-pi), 0);
}

}  // namespace geofun
