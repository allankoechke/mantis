//
// Created by allan on 22/06/2025.
//

#include <mantis/app/app.h>
#include "mantis/core/database.h"
#include "mantis/core/http.h"
#include "mantis/utils/utils.h"

using namespace mantis;

int main(int argc, char** argv) {
    // Create app instance
    mantis::MantisApp app(argc, argv);

    // Initialize the units
    app.init();

    // app.http().Get(path, handler_func, {vector of middlewares});
    // handler_func -> [](Request, Response, Context);
    app.http().Get("/echo", [](const Request& req, Response& res, Context& ctx)
    {
        res.status = 200;
        res.body = "Hello World!";
    }, {});

    // Using JSON
    app.http().Get("/json", [](const Request& req, Response& res, Context& ctx)
    {
        json response;
        response["id"] = generateReadableTimeId();
        response["time"] = generateTimeBasedId();
        response["message"] = "Hello World!";

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }, {});

    // With Middleware
    app.http().Get("/json", [](const Request& req, Response& res, Context& ctx)
    {
        json response;
        response["id"] = generateReadableTimeId();
        response["time"] = generateTimeBasedId();
        response["message"] = "Hello World!";

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }, {[](const Request& req, Response& res, Context& ctx) -> bool
    {
        // We can pass some data through to the other middleware
        ctx.set("key", "value");
        ctx.set("key2", true);

        // If request has not been handled in the middleware, return true.
        return true;
    }, [](const Request& req, Response& res, Context& ctx) -> bool
    {
        // Get the context data
        auto key = ctx.get<std::string>("key");
        auto key2 = ctx.get<bool>("key2");

        // Context .get() returns an optional, check if it has data
        if (key.has_value())
        {
            std::cout << "key: " << key.value() << std::endl;
        }

        // Give a default value if key is missing
        ctx.set("key2", key2.value_or(false));

        // If request has been handled in the middleware, maybe an error occurred
        // return false.
        res.status = 500;
        return false;
    }});

    // Get a database session
    const auto sql = app.db().session();
    soci::row row;
    *sql << "SELECT * FROM users", soci::into(row);

    if (sql->got_data())
    {
        // Check soci docs for handling data here ...
    }

    // Start the server event loop
    return app.run();
}
