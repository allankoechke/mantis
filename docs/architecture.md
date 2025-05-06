
# Mantis Architecture Overview

Mantis is a modular, pluggable Backend-as-a-Service (BaaS) framework written in C++. It is designed to be lightweight, embeddable, and flexible, targeting applications that need structured data APIs, authentication, and cloud sync — all without heavy dependencies.

---

## 📐 Core Principles

- **Pluggability**: All major subsystems (database, auth, sync) are replaceable or extendable.
- **Lightweight**: Optimized for embedded and desktop scenarios; SQLite is the default engine.
- **Modularity**: Organized into loosely coupled components for clean embedding or CLI use.
- **Sync-Aware**: Supports client/server roles with offline-first and sync-on-demand strategies.
- **Developer-First**: Embeddable as a library or deployable as a standalone binary.

---

## 🧱 High-Level Architecture

```
+-----------------------------+
|         Client App         |
| (Qt/CLI/Other C++ App)     |
+-------------+--------------+
              |
      embeds/links to
              ↓
+-------------+--------------+
|         Mantis Core        |
| +------------------------+ |
| |    API Router (Crow)   | |
| |    Middleware Engine   | |
| +------------------------+ |
| |    Auth Provider       | |
| |    Sync Engine         | |
| |    Database Interface  | |
| +------------------------+ |
+-------------+--------------+
              ↓
+-----------------------------+
|     Pluggable DB Layer      |
|    (SQLite / MySQL / ...)   |
+-----------------------------+
```

---

## 🧩 Components

### 1. **Database Layer**
- Abstracted via `IDatabase` interface.
- Default: SQLite (in-process, lightweight)
- Optional: MySQL/PostgreSQL (via plugin or dynamic linking)

### 2. **API Router**
- Powered by Crow (header-only HTTP server)
- Auto-generates REST endpoints for tables/views
- Supports middleware chaining (e.g., for auth, logging)

### 3. **Authentication Module**
- Supports JWT or session-based login
- Tied to access control and middleware
- Token generation and validation integrated into middleware

### 4. **Middleware System**
- Intercepts requests before/after route handlers
- Enables hooks for auth, logging, CORS, etc.
- Configurable per route or globally

### 5. **Sync Engine**
- Supports client/server sync topology
- Uses WebSocket for real-time (future) or HTTP polling
- Tracks sync status via internal `__sync_log` tables

### 6. **Static File Server**
- Optional serving of static files (e.g., web UI or assets)
- Configurable static root folder

### 7. **System Tables**
- `__tables`, `__columns` — tracks schema
- `__users`, `__sessions`, `__rules` — auth and access
- `__sync_log`, `__sync_state` — sync metadata

---

## 🔄 Sync Model

- Nodes start in either **server** or **client** mode (flag at launch)
- Client mode pulls/pushes changes to/from server
- Conflict resolution is timestamp or revision-based (configurable)
- Sync is incremental and tracked at row-level

---

## ⚙️ Deployment Modes

### ➕ Embeddable Library
- Link `libmantis.a` in your Qt/C++ project
- Use exposed headers and initialization APIs

### 🚀 Standalone Binary
- Run `mantisd --config config.json`
- Works with Docker, systemd, or embedded startup

---

## 📁 File Structure Overview

```
mantis/
├── include/mantis/      # Public API headers
├── src/                 # Internal implementation
├── docker/              # Dockerfile & entrypoint
├── examples/            # Example apps embedding Mantis
├── tests/               # Test suite
└── CMakeLists.txt
```

---

## 📌 Extensibility Ideas

- Lua scripting support for request customization
- Live schema migration tool (CLI command)
- Admin dashboard (served from static folder or separate frontend)

---

## 🏁 Conclusion

Mantis provides a solid, portable foundation for applications that need backend features without the weight of traditional stacks. Whether embedded in a GUI tool or deployed as a syncable API server, Mantis offers structure, simplicity, and extensibility in a small, powerful package.
