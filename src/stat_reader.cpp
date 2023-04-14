#include "include/stat_reader.h"

namespace tc {

    using namespace detail;

        void GetRoute(const std::string& name, const TransportCatalogue& catalogue, std::ostream& out){
            domain::RouteInfo info;

            bool check = false;
            try {
                info = catalogue.GetRouteInfo(name);
                check = true;
            }  catch (...) {
                out<< "Bus " << name << ": not found" << std::endl;
            }
            if(check == true){
                out << std::setprecision(6);
                out << "Bus " << info.name << ": "
                          << info.stop_count << " stops on route, "
                          << info.unique_stop_count << " unique stops, "
                          << info.route_length << " route length, "
                          << info.curvature << " curvature" << std::endl;
            }
        }

        void GetStop(const std::string& name, const TransportCatalogue& catalogue, std::ostream& out){
            auto buses_on_stop = catalogue.GetBusesOnStop(name);

            if(buses_on_stop.has_value()){
                if((*buses_on_stop).empty()){
                    out<< "Stop " << name << ": no buses" << std::endl;
                }else{
                    out << std::setprecision(6);
                    out << "Stop " << name << ": buses";
                       for(auto& bus : *buses_on_stop){
                            out << " " << std::string(bus);
                        }
                         out << std::endl;
                }
            }else{
                out<< "Stop " << name << ": not found" << std::endl;
            }
        }

        void PrintQuery(const QueryType& type, const std::string& name, const TransportCatalogue& catalogue, std::ostream& out){
            switch (type) {
            case QueryType::Route:
                GetRoute(name, catalogue, out);
                break;
            case QueryType::Stop:
                GetStop(name, catalogue, out);
                break;
            default:
                break;
            }
        }

        std::vector<std::pair<QueryType, std::string>> ParseOutputQuery(std::istream& in){
            using namespace std::literals;

            std::string line;
            size_t number_queries = CheckNumberLines(in);
            std::vector<std::pair<QueryType, std::string>> queries;
            queries.reserve(number_queries);
            for(size_t i = 0; i < number_queries; ++i){
                 getline(in, line);
                 std::string_view line_view = line;
                 LeftTrim(line_view);
                 auto end_type = std::min(line_view.find_first_of(" "s), line_view.size());
                 std::string_view type = line_view.substr(0, end_type);
                 auto str_type = std::string(type);

                 QueryType type_mode;

                 if(str_type == "Bus"s || str_type == "bus"s){
                     type_mode = QueryType::Route;
                 } else if(str_type == "Stop"s || str_type == "stop"s){
                     type_mode = QueryType::Stop;
                 }else{
                     throw std::invalid_argument(" Ошибка: Запрос неправлиьно построен. Нет такого типа запросов "s);
                 }

                 line_view.remove_prefix(end_type+1);
                 Trim(line_view);

                 queries.push_back({type_mode ,std::string(line_view)});
            }
            return queries;
        }


        void EqualLogFiles(std::filesystem::path my_log, std::filesystem::path test, std::ostream& out){
            using namespace std::literals;
            std::ifstream in1(my_log.string());
            std::ifstream in2(test.string());

            std::string line1;
            std::string line2;
            if(in1.is_open() && in2.is_open()){
                out << "Файл: " << test.filename() << std::endl;
                for(size_t i = 1; !in1.eof(); ++i){
                    std::getline(in1, line1);
                    std::getline(in2, line2);
                    if(line2[line2.size()-1] == '\r'){
                        line2 = line2.substr(0, line2.size()-1);
                    }
                    if(line1.size() != line2.size() || line1 != line2){
                        out << "Строка: " << i << "\n"
                            << "Мой вывод: " << line1 << "\n"
                            << "Эталон: " << line2 << "\n";
                    }
                }
            }else{
                std::cerr << "Ошибка: файлы " << my_log.string() << " и " << test.string() << " не открыты " << std::endl;
            }
        }

        json::Node GetNotFound(size_t id){
            using namespace std::literals;
            return json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(static_cast<int>(id))
                            .Key("error_message"s).Value("not found"s)
                        .EndDict()
                        .Build();
        }


        json::Node GetStop(size_t id, const std::string& name, const TransportCatalogue& catalogue){
            using namespace std::literals;

            json::Builder builder;
            auto buses_on_stop = catalogue.GetBusesOnStop(name);
            bool buses_has_value = buses_on_stop.has_value();
            if(buses_has_value){
                builder.StartDict()
                        .Key("buses"s).StartArray();
                if((*buses_on_stop).empty()){

                }else{
                    for(auto& bus : *buses_on_stop){
                        builder.Value(std::string(bus));
                    }
                }
                builder.EndArray();
                builder.Key("request_id"s).Value(static_cast<int>(id))
                       .EndDict();
            }else{
                return GetNotFound(id);
            }

            return builder.Build();
        }

        json::Node GetRoute(size_t id, const std::string& name, const TransportCatalogue& catalogue){
            using namespace std::literals;
            domain::RouteInfo info;
            bool check = false;
            json::Node not_found, route_info;

            try {
                info = catalogue.GetRouteInfo(name);
                check = true;
            }  catch (const std::out_of_range& ) {
                return GetNotFound(id);
            }

            if(check == true){
                route_info = json::Builder{}
                             .StartDict()
                                .Key("curvature"s).Value(info.curvature)
                                .Key("request_id"s).Value(static_cast<int>(id))
                                .Key("route_length"s).Value(info.route_length)
                                .Key("stop_count"s).Value(static_cast<int>(info.stop_count))
                                .Key("unique_stop_count"s).Value(static_cast<int>(info.unique_stop_count))
                             .EndDict()
                        .Build();
            }
            return route_info;
        }

        json::Node GetDict(size_t id, const DictRenderer& renderer){
            using namespace std::literals;
            std::ostringstream ss;
            renderer.Print(ss);
            return  json::Builder{}
                        .StartDict()
                            .Key("map"s).Value(ss.str())
                            .Key("request_id"s).Value(static_cast<int>(id))
                        .EndDict()
                    .Build();
        }

        void PrintQuery(const std::vector<StatRequest> requests, const TransportCatalogue& catalogue, const DictRenderer& renderer, std::ostream& out){
            using namespace std::literals;

            auto get_response = [&catalogue, &renderer](const StatRequest& request){
                if(request.type == QueryType::Stop){
                    return GetStop(request.id, request.name, catalogue);
                }else if(request.type == QueryType::Route){
                    return GetRoute(request.id, request.name, catalogue);
                }else if(request.type == QueryType::Map){
                    if(renderer.GetSettings().is_init == true){
                        return GetDict(request.id, renderer);
                    }
                }
                return json::Node{};
            };


            json::Builder builder;
            builder.StartArray();
            for(const auto& request: requests){
                 builder.Value(get_response(request));
            }
            builder.EndArray();

            out << std::setprecision(6);
            json::Print(
                json::Document{
                    builder.Build()
                }, out
            );
        }

}