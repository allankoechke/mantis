<p align="center">
  <img src="docs/banner.jpg" alt="Mantis Logo" width="100%" />
</p>

<h1 align="center">Mantis</h1>

<p align="center">
  <strong>A lightweight, pluggable Backend-as-a-Service (BaaS) library built in C++</strong><br />
  Portable. Embeddable. Syncable. Built for speed and extensibility.
</p>



## 🔧 Overview

**Mantis** is a modular, lightweight C++ library designed to power modern backend systems in embedded devices, desktop tools, or standalone server deployments. Inspired by systems like PocketBase and Supabase, Mantis focuses on:

- Minimal runtime footprint
- SQLite as the default local database (with optional MySQL/PSQL support)
- Built-in authentication and access control
- Auto-generated REST APIs
- Pluggable sync layer (client-server)
- Embeddable as a reusable C++ library

---

## 📦 Features

| Feature                          | Status         |
|----------------------------------|----------------|
| ✅ Modular C++ core library       | 🟡 In Progress |
| 🧩 Pluggable database interface   | 🟡 In Progress |
| 🔐 Authentication (JWT/session)  | ✅ Completed    |
| 📄 Auto API generation from schema | ✅ Completed    |
| 🧱 System metadata tables         | ✅ Completed    |
| 🔁 Client/server sync modes       | ⬜ Planned      |
| 🔄 WebSocket sync support         | ⬜ Planned      |
| 🧩 Middleware support             | 🟡 In Progress |
| 💾 Static file serving            | ⬜ Planned      |
| 🚀 Docker-ready deployment        | ⬜ Planned      |
| 🧪 Unit + integration tests       | ⬜ Planned      |
| 📘 CLI + embeddable modes         | 🟡 In Progress |

---

## 🛠️ Tech Stack

- **Language**: C++
- **Database**: SQLite (default), MySQL/PSQL (planned)
- **HTTP Server**: [httplib-cpp](https://github.com/allankoechke/httplib)
- **Build System**: CMake
- **Packaging**: Docker + CLI
- **Sync**: WebSocket / REST delta sync (planned)

---

## 🚀 Getting Started

```bash
git clone --recurse-submodules https://github.com/yourusername/mantis.git
cd mantis
cmake -B build
cmake --build build
./build/mantis-ctl -p 7070 -h 0.0.0.0 --serve
````

You can also embed Mantis as a library in your own C++ project:

```cpp
#include <mantis/core/database.hpp>
#include <mantis/api/server.hpp>
// Initialize and run
```

---

## 📁 Project Structure

```
mantis/
├── include/
│   └── mantis/         # Public API headers
├── src/                # Internal implementation
├── examples/           # Embedding examples
├── tests/              # Unit & integration tests
├── docker/             # Docker deployment
└── CMakeLists.txt
```

---

## 📚 Documentation

* [Architecture Overview](docs/architecture.md)
* [Embedding Guide](docs/embedding.md)
* [Sync Engine Design](docs/sync.md)
* [API Reference](docs/api.md)

---

##  Project Dependencies

* [httplib-cpp]()
* [soci - SQL lib]()
* [spdlog]()
* [AnyOption - CMD parser]()

---

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) and open an issue or PR.

---

## 📜 License

MIT License © 2025 Allan K. Koech

---

<p align="center"><i>Built with ❤️ using C++, SQLite, and a vision for portability.</i></p>

