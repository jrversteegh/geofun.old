#include "geofun.hpp"
#include <stdio.h>

//#include <iostream>
//using namespace std;

namespace geofun {

char IndexError::msg[64];

static WGS84 wgs84;
static Sphere sphere;

EarthModel* earth_model = &wgs84;

void set_earth_model(const std::string& model_name)
{
  if (model_name == "wgs84") {
    earth_model = &wgs84;
  }
  else if (model_name == "spherical") {
    earth_model = &sphere;
  }
  else {
    throw EarthModelError();
  }
}

Position& Position::operator+=(const Vector& vector)
{
  Coord deltas1 = cartesian_deltas();
  Coord cart = vector.cartesian();
  Position mid_pos;
  mid_pos.latlon(
      lat() + 0.5 * cart.x() / deltas1.x(),
      lon() + 0.5 * cart.y() / deltas1.y());
  Coord deltas2 = mid_pos.cartesian_deltas();
  mid_pos.latlon(
      lat() + cart.x() / (deltas1.x() + deltas2.x()),
      lon() + cart.y() / (deltas1.y() + deltas2.y()));
  deltas2 = mid_pos.cartesian_deltas();
  Position end_pos;
  end_pos.latlon(
      lat() + cart.x() / deltas2.x(),
      lon() + cart.y() / deltas2.y());
  Coord deltas3 = end_pos.cartesian_deltas();
  Coord inv_deltas = (1.0 / 6) * (1 / deltas1 + 4 / deltas2 + 1 / deltas3);
  latlon(lat() + cart.x() * inv_deltas.x(), lon() + cart.y() * inv_deltas.y());
  return *this;
}

Vector Position::operator-(const Position& position) const
{
  double dlat = angle_diff(this->lat(), position[0]);
  double dlon = angle_diff(this->lon(), position[1]);
  Coord deltas1 = position.cartesian_deltas();
  Coord deltas3 = this->cartesian_deltas();
  Position mid_lat;
  mid_lat.lat(0.5 * (this->lat() + position[0]));
  Coord deltas2 = mid_lat.cartesian_deltas();
  Coord deltas = 6.0 / (1.0 / deltas1 + 4.0 / deltas2 + 1.0 / deltas3);
  Coord cart = Coord(dlat  * deltas.x(), dlon  * deltas.y());
  return Vector(cart);
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
  double da1 = angle_diff(v11.a(), _v.a());
  double da2 = angle_diff(v12.a(), _v.a());
  if ((da1 > 0 and da2 < 0) or (da1 < 0 and da2 > 0)) {
    double da3 = angle_diff(v21.a(), line._v.a());
    double da4 = angle_diff(v22.a(), line._v.a());
    if ((da3 > 0 and da4 < 0) or (da3 < 0 and da4 > 0)) {
      return true;
    }
  }
  return false;
}

Position Line::intersection(const Line& line) const {
  if (!intersects(line)) {
    return Position(-pi, 0);
  }
  
  Position p(_p1);
  double l;
  do {
    Vector v = line._p1 - p;
    double a1 = fabs(angle_diff(line._v.a(), _v.a()));
    double a2 = fabs(angle_diff(_v.a(), v.a()));
    double a3 = pi - a2 - a1;
    double d = sin(a3) * v.r();
    l = d / sin(a1);
    double f = l / _v.r();
    p += _v * f;
  } while (fabs(l) > 10);

  return p;
}

void Arc::vincenty_inverse(const Position& p1, const Position& p2, Vector* v, Vector* r, double* alpha)
{
  // Formula obtained from http://en.wikipedia.org/wiki/Vincenty%27s_formulae
  double u1 = reduced_latitude(p1.lat());
  double u2 = reduced_latitude(p2.lat());
  double dlinit = p2.lon() - p1.lon();

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
  v->r(b * aa * (sig - dsig));
  r->r(v->r());
  v->a(atan2(cosu2 * sindl, cosu1sinu2 - sinu1cosu2 * cosdl));
  r->a(pi - atan2(cosu1 * sindl, -sinu1cosu2 + cosu1sinu2 * cosdl));
  *alpha = asin(sina);
}

void Arc::vincenty_direct(const Position& p1, const Vector& v, Position* p2, Vector* r, double* alpha)
{
  double u1 = reduced_latitude(p1.lat());
  double cosa1 = cos(v.a());
  double sina1 = sin(v.a());
  double sig1 = atan2(tan(u1), cosa1);
  double cosu1 = cos(u1);
  double sinu1 = sin(u1);
  double sina = cosu1 * sin(v.a());
  double sqcosa = (1 - sina) * (1 + sina);
  double squ = sqcosa * (sqa - sqb) / sqb;
  double sqrtsqup1 = sqrt(1 + squ);
  double k1 = (sqrtsqup1 - 1) / (sqrtsqup1 + 1);
  double aa = (1 + 0.25 * sqr(k1)) / (1 - k1);
  double bb = k1 * (1 - (3.0 / 8) * sqr(k1));
  double siginit = v.r() / (b * aa);
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
  r->a(pi - atan2(sina, -sinu1 * sins + cosu1 * coss * cosa1));
  r->r(v.r());
  p2->lat(f2);
  p2->lon(p1.lon() + dlinit);
  *alpha = asin(sina);
}

bool Arc::intersects(const Line& line) const
{
  // TODO
  return false;
}

Position Arc::intersection(const Line& line) const
{
  return Position(pi, 0);
}

bool Arc::intersects(const Arc& arc) const
{
  // TODO
  return false;
}

Position Arc::intersection(const Arc& arc) const
{
  return Position(-pi, 0);
}

}  // namespace geofun
