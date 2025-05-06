
# Mantis API Reference

Mantis provides auto-generated RESTful APIs for interacting with database tables and views. This document covers the default API structure, supported HTTP methods, authentication handling, and customization options.

---

## 🌐 Base URL

When Mantis is running locally:

```
http://localhost:8080/api/
```

You can configure the port and base path in the `config.json` file.

---

## 📄 REST API Endpoints

Mantis automatically exposes the following endpoints for each table or view:

| Method | Endpoint                    | Description                    |
|--------|-----------------------------|--------------------------------|
| GET    | `/api/<table>`              | List all records               |
| GET    | `/api/<table>/:id`          | Get a specific record          |
| POST   | `/api/<table>`              | Create a new record            |
| PUT    | `/api/<table>/:id`          | Update a record (full replace) |
| PATCH  | `/api/<table>/:id`          | Update partial fields          |
| DELETE | `/api/<table>/:id`          | Delete a record                |

---

## 🔐 Authentication

If auth is enabled, all API calls must include a valid JWT in the `Authorization` header:

```
Authorization: Bearer <token>
```

### Auth Endpoints

| Method | Endpoint         | Description         |
|--------|------------------|---------------------|
| POST   | `/auth/login`    | Login and get token |
| POST   | `/auth/register` | Create new account  |
| GET    | `/auth/me`       | Get current user    |

---

## 🎛️ Filtering & Query Parameters

APIs support filtering, sorting, and pagination:

```
GET /api/tasks?status=done&limit=10&offset=20&sort=-created_at
```

- `limit`, `offset` – pagination
- `sort` – sort by field (`-` prefix for descending)
- Field filters – simple equality filters via query string

---

## ⚙️ API Customization

- Override routes with custom handlers using the middleware system.
- Add pre/post hooks to enforce access control or transform data.
- Future: custom endpoints via configuration or scripting.

---

## 🗃️ System Tables API

You can also interact with system tables to introspect schema or access logs:

- `/api/__tables`
- `/api/__columns`
- `/api/__users`
- `/api/__rules`
- `/api/__sync_log`

> ⚠️ Access to system tables may be restricted to admin users.

---

## 🧪 Example Requests

```bash
# List tasks
curl http://localhost:8080/api/tasks

# Create task
curl -X POST http://localhost:8080/api/tasks   -H "Authorization: Bearer <token>"   -H "Content-Type: application/json"   -d '{"title": "Write API docs", "status": "in_progress"}'
```

---

## 🏁 Summary

Mantis provides zero-setup APIs for all your structured data. With support for CRUD, authentication, query filtering, and future extensions, it's a ready-to-use backend interface for apps, tools, and embedded systems.
