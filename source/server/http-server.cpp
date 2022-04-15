#include <memory>
#include <httpserver.hpp>
#include "server/router.hpp"
#include "../../include/server/http-server.hpp"

const std::shared_ptr<httpserver::http_response> ClockServerResource::render_GET( const httpserver::http_request& request)
{
    std::unordered_map<std::string, std::string> params;
    auto args = request.get_args();
    std::transform( args.cbegin(), args.cend(), std::inserter( params, params.begin()), []( const auto& each)
    {
        return std::pair{ each.first, each.second};
    });

    return std::shared_ptr<httpserver::http_response>( new httpserver::string_response( Router::generateResponse( params)));
}
