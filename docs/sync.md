
# Mantis Sync Engine Design

The Mantis sync engine is designed to support offline-first applications that require robust and efficient synchronization between local nodes (clients) and remote nodes (servers). This document outlines how the sync system works, how it is configured, and how conflicts are handled.

---

## 🔄 Sync Model Overview

Mantis supports a **two-mode sync model**:
- **Client Mode**: Local-first apps that store data offline and push/pull to a remote sync server.
- **Server Mode**: Central node that aggregates data, applies sync policies, and serves updates.

Both modes are configured via the startup configuration file.

---

## ⚙️ Sync Workflow

```
+-------------+       Push       +-------------+
|  Client DB  | ---------------> |  Server DB  |
| (SQLite)    |                 | (MySQL)     |
+-------------+                 +-------------+
       ^                             |
       |         Pull/Update         |
       +-----------------------------+
```



### 1. **Change Detection**
- Each table has an internal `__updated_at` timestamp or revision ID.
- Sync engine scans for new/changed rows since the last sync.

### 2. **Conflict Resolution**
- Defaults to **latest timestamp wins**.
- Optional: version-based merge or manual conflict hooks.

### 3. **Transport Layer**
- REST API or WebSocket-based sync messages.
- All data is serialized as JSON.
- Sync headers include node ID, table revision, and auth token.

### 4. **Sync Tables**
- `__sync_log`: Stores outgoing sync queue
- `__sync_state`: Tracks last pull/push per table and remote

---

## 🔐 Security

- All sync endpoints require authentication (JWT or session).
- Encrypted transport via HTTPS or TLS WebSocket.
- Sync can be enabled/disabled per client in config.

---

## 🧪 Sync Testing

Simulate sync with curl:

```bash
curl -X POST http://localhost:8080/sync/push   -H "Authorization: Bearer <token>"   -d @sync_payload.json
```

Use `mantisd --mode server` on one node and `mantisd --mode client` on another to test full duplex sync.

---

## 🔧 Configuration Example

```json
{
  "mode": "client",
  "sync": {
    "enabled": true,
    "server_url": "https://api.mantis.io",
    "interval_seconds": 60,
    "conflict_policy": "timestamp"
  }
}
```

---

## 📌 Sync Customization

- Hook into sync lifecycle via middleware or plugin interface.
- Add table-level filters (e.g., sync only "active" records).
- Implement custom conflict resolvers per table.

---

## 🏁 Summary

Mantis provides a robust but flexible sync system tailored for offline-capable and distributed applications. With per-table tracking, pluggable conflict resolution, and secure transport, it supports both embedded clients and production server sync strategies.
