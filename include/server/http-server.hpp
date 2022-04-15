#ifndef HTTP_SERVER
#define HTTP_SERVER

class ClockServerResource : public httpserver::http_resource
{
public:
    const std::shared_ptr<httpserver::http_response> render_GET( const httpserver::http_request& request) override;
};

#endif