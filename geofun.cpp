#include "geofun.h"

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
  Coord deltas = (1 / 6) * (deltas1 + 4 * deltas2 + deltas3);
  set_latlon(lat() + cart.x() / deltas.x(), lon() + cart.y() / deltas.y());
  return *this;
}

Vector operator-(const Position& position1, const Position& position2)
{
  double dlat = norm_angle_pipi(position2.lat() - position1.lat());
  double dlon = norm_angle_pipi(position2.lon() - position1.lon());
  Coord deltas1 = position1.cartesian_deltas();
  Coord deltas3 = position2.cartesian_deltas();
  Position mid_pos;
  mid_pos.set_latlon(
      0.5 * (position1.lat() + position2.lat() + 0.5 * dlat * (deltas1.x() - deltas3.x())),
      0.5 * (position1.lon() + position2.lon() + 0.5 * dlon * (deltas1.y() - deltas3.y())));
  Coord deltas2 = mid_pos.cartesian_deltas();
  Coord deltas = (1 / 6) * (deltas1 + 4 * deltas2 + deltas3);
  return Vector(Coord(dlat  * deltas.x(), dlon  * deltas.y()));
}

}  // namespace geofun
