
# Embedding Mantis into Your Application

Mantis is designed as a lightweight C++ library that can be embedded directly into your desktop, mobile, or embedded application. This guide explains how to integrate Mantis into your project, configure it, and expose APIs through your own app.

---

## 🔧 Why Embed Mantis?

- Add full-featured local storage and REST API support to your Qt, Slint, or CLI app.
- Serve data to local UI components through HTTP or direct C++ API calls.
- Retain full control of the application lifecycle and logic.

---

## 📦 Integration Options

### 1. **As a Static or Shared Library**
Link the compiled `libmantis.a` or `libmantis.so` into your CMake project:

```cmake
add_subdirectory(mantis)
target_link_libraries(your_app PRIVATE mantis)
```

### 2. **Header-only Modules**
For lighter setups, core components like schema parsing or basic table handling may be available as header-only utilities.

---

## 🧱 Minimal Example

```cpp
#include <mantis/api/server.hpp>
#include <mantis/core/config.hpp>

int main(int argc, char** argv) {
    mantis::Config config = mantis::Config::load("config.json");

    mantis::Server server(config);
    server.start(); // Starts HTTP server with auto API and static file support

    return 0;
}
```

---

## ⚙️ Configuration

The config file supports options like:

```json
{
  "mode": "client", // or "server"
  "database": "data.db",
  "port": 8080,
  "static_dir": "./public",
  "auth": {
    "enable": true,
    "jwt_secret": "your-secret-key"
  }
}
```

---

## 📂 Example Project Structure

```
your-app/
├── main.cpp
├── config.json
├── public/              # Static assets
└── mantis/              # Submodule or library
```

---

## 🧪 Testing in Embedded Context

Use tools like `curl` or Postman to test your APIs:

```bash
curl http://localhost:8080/api/users
```

In Qt, you can use `QNetworkAccessManager` to consume the local REST APIs.

---

## 🛠️ Advanced: Use Mantis as a Submodule

```bash
git submodule add https://github.com/yourusername/mantis.git
git submodule update --init --recursive
```

Then link and include as part of your build system.

---

## 📌 Notes

- Mantis APIs respect all access rules and auth even in embedded mode.
- You can override route handlers or middleware for custom behavior.
- Sync engine can be enabled/disabled based on config.

---

## ✅ Result

By embedding Mantis, you gain powerful backend features — including database storage, auth, and syncing — without requiring a separate server deployment.
