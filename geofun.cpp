#include "geofun.h"
#include <stdio.h>

#include <iostream>
using namespace std;

namespace geofun {


Position& Position::operator+=(const Vector& value) 
{
  Coord deltas1 = cartesian_deltas();
  Coord cart = value.cartesian();
  Position mid_pos;
  mid_pos.set_latlon(
      lat() + 0.5 * cart.x() / deltas1.x(),
      lon() + 0.5 * cart.y() / deltas1.y());
  Coord deltas2 = mid_pos.cartesian_deltas();
  mid_pos.set_latlon(
      lat() + cart.x() / (deltas1.x() + deltas2.x()),
      lon() + cart.y() / (deltas1.y() + deltas2.y()));
  deltas2 = mid_pos.cartesian_deltas();
  Position end_pos;
  end_pos.set_latlon(
      lat() + cart.x() / deltas2.x(),
      lon() + cart.y() / deltas2.y());
  Coord deltas3 = end_pos.cartesian_deltas();
  Coord inv_deltas = (1.0 / 6) * (1 / deltas1 + 4 / deltas2 + 1 / deltas3);
  set_latlon(lat() + cart.x() * inv_deltas.x(), lon() + cart.y() * inv_deltas.y());
  return *this;
}

Vector operator-(const Position& position2, const Position& position1)
{
  double dlat = angle_diff(position2.lat(), position1.lat());
  double dlon = angle_diff(position2.lon(), position1.lon());
  Coord deltas1 = position1.cartesian_deltas();
  Coord deltas3 = position2.cartesian_deltas();
  Position mid_lat;
  mid_lat.set_lat(0.5 * (position1.lat() + position2.lat()));
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
  if ((da1 >= 0 and da2 < 0) or (da1 < 0 and da2 >=0)) {
    double da3 = angle_diff(v21.a(), line._v.a());
    double da4 = angle_diff(v22.a(), line._v.a());
    if ((da3 >= 0 and da4 < 0) or (da3 < 0 and da4 >=0)) {
      return true;
    }
  }
  return false;
}

Position Line::intersection(const Line& line) const {
  if (!intersects(line)) {
    return Position(pi, 0);
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


}  // namespace geofun
