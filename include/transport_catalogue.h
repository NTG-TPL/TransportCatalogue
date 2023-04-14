
#ifndef TRANSPORT_CATALOGUE_H
#define TRANSPORT_CATALOGUE_H


#include <cstdint>
#include <deque>
#include <set>
#include <execution>
#include <iostream>
#include <unordered_map>
#include <optional>
#include "include/domain.h"

namespace tc {

    size_t CountStops(const domain::Route *route) noexcept;

    size_t CountUniqueStops(const domain::Route *route) noexcept;

    double CalculateRouteLengthGeo(const domain::Route *route) noexcept;

    // Транспортный справочник
    class TransportCatalogue{

    public:

       void AddStop(domain::Stop stop) noexcept;

       const domain::Stop* FindStop(std::string_view name) const;
       const domain::Stop* FindStop(const std::string& name) const;

       void AddRoute(domain::Route route) noexcept;

       const domain::Route*  FindRoute(std::string_view name) const;
       const domain::Route*  FindRoute(const std::string& name) const;

       domain::RouteInfo GetRouteInfo(const std::string& name) const ;

       std::optional<std::set<std::string_view>> GetBusesOnStop(const std::string& name) const;

       void SetDistance(std::pair<const domain::Stop*,const domain::Stop*> pair_stops, size_t distance);
       void SetDistance(const domain::Stop* begin, const domain::Stop* end, size_t distance);

       const std::deque<domain::Stop> GetStops() const;

       const std::deque<domain::Route> GetRoutes() const;

       size_t GetAmountStops() const;

       size_t GetAmountRoutes() const;

       size_t GetAmountStopsDistances() const;

       size_t GetStopsDistances(const std::pair<std::string, std::string>& pair) const;
       size_t GetStopsDistances(std::pair<const domain::Stop*, const domain::Stop*>) const;
       size_t GetStopsDistances(const domain::Stop*, const domain::Stop*) const;
       size_t GetStopsDistances(const std::string& begin, const std::string& end) const;

       size_t CalculateRouteLength(const domain::Route* route) const;

    private:
       // остановки
       std::deque<domain::Stop> stops_;
       // Формат хранения: имя_остановки - указатель_на_остановку
       std::unordered_map<std::string_view, const domain::Stop*> stops_by_names_;
       // автобусы на каждой остановке. // Формат хранения: имя_остановки - множество_автобусов
       std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
       // маршруты
       std::deque<domain::Route> routes_;
       // Формат хранения для поиска: имя_маршрута(совпадает с именем автобуса) - указатель на маршрут
       std::unordered_map<std::string_view, const domain::Route*> routes_by_names_;

       // расчёт расстояний между остановками .
       std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, size_t, domain::StopsHasher> distance_stops;
    };

}

#endif