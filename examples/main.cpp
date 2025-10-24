#include <mantis/app/app.h>
#include "mantis/core/database.h"
#include "mantis/core/http.h"
#include "mantis/utils/utils.h"

using namespace mantis;

int main(int argc, char** argv)
{
    // Create app instance
    auto& app = MantisApp::create(argc, argv);

    // app.http().Get(path, handler_func, {vector of middlewares});
    // handler_func -> [](Request, Response, Context);
    app.http().Get("/echo", [](MantisRequest& req, MantisResponse& res)
    {
        res.sendText(200, "Hello World!");
    }, {});

    // Using JSON
    app.http().Get("/json", [](MantisRequest& req, MantisResponse& res)
    {
        json response;
        response["id"] = generateReadableTimeId();
        response["time"] = generateTimeBasedId();
        response["message"] = "Hello World!";

        res.sendJson(200, response);
    }, {});

    // With Middleware
    app.http().Get("/json", [](MantisRequest& req, MantisResponse& res)
    {
        json response;
        response["id"] = generateReadableTimeId();
        response["time"] = generateTimeBasedId();
        response["message"] = "Hello World!";

        res.sendJson(200, response);
    }, {
        [](MantisRequest& req, MantisResponse& res) -> bool
        {
            // We can pass some data through to the other middleware
            req.set("key", "value");
            req.set("key2", true);

            // If request has not been handled in the middleware, return true.
            return true;
        },
        [](MantisRequest& req, MantisResponse& res) -> bool
        {
            // Get the context data
            const auto key = req.get<std::string>("key");
            const auto key2 = req.getOr<bool>("key2", false);

            // Context .get() returns an optional, check if it has data
            if (key.has_value())
            {
                std::cout << "key: " << key.value() << std::endl;
            }

            // Give a default value if key is missing
            req.set("key2", key2);

            // If request has been handled in the middleware, maybe an error occurred
            // return false.
            res.sendText(500, "");
            return false;
        }
    });

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
