import type {
	Entity,
	EntityDataResponse,
	EntityConfig,
} from './entities.js';
import type {
	AdminUser,
	AdminListResponse,
} from './admins.js';
import type {
	LogEntry,
	LogListResponse,
} from './logs.js';
import type {
	Settings,
} from './settings.js';

// Mock entities data
export const mockEntities: Entity[] = [
	{
		name: 'users',
		type: 'auth',
		id: '1',
		created: new Date().toISOString(),
		fields: [
			{ name: 'id', type: 'string', required: true },
			{ name: 'email', type: 'string', required: true },
			{ name: 'name', type: 'string', required: false },
			{ name: 'createdAt', type: 'datetime', required: false },
		],
	},
	{
		name: 'products',
		type: 'base',
		id: '2',
		created: new Date().toISOString(),
		fields: [
			{ name: 'id', type: 'string', required: true },
			{ name: 'name', type: 'string', required: true },
			{ name: 'price', type: 'number', required: true },
			{ name: 'description', type: 'string', required: false },
			{ name: 'stock', type: 'number', required: false },
		],
	},
	{
		name: 'orders',
		type: 'base',
		id: '3',
		created: new Date().toISOString(),
		fields: [
			{ name: 'id', type: 'string', required: true },
			{ name: 'userId', type: 'string', required: true },
			{ name: 'total', type: 'number', required: true },
			{ name: 'status', type: 'string', required: true },
			{ name: 'createdAt', type: 'datetime', required: false },
		],
	},
	{
		name: 'analytics',
		type: 'view',
		id: '4',
		created: new Date().toISOString(),
		fields: [
			{ name: 'id', type: 'string', required: true },
			{ name: 'metric', type: 'string', required: true },
			{ name: 'value', type: 'number', required: true },
			{ name: 'date', type: 'date', required: true },
		],
	},
];

// Mock entity data
export function getMockEntityData(
	entityName: string,
	page: number = 1,
	pageSize: number = 10
): EntityDataResponse {
	const mockData: Record<string, unknown>[] = [];

	if (entityName === 'users') {
		for (let i = 0; i < 25; i++) {
			mockData.push({
				id: `user-${i + 1}`,
				email: `user${i + 1}@example.com`,
				name: `User ${i + 1}`,
				createdAt: new Date(Date.now() - i * 86400000).toISOString(),
			});
		}
	} else if (entityName === 'products') {
		for (let i = 0; i < 30; i++) {
			mockData.push({
				id: `product-${i + 1}`,
				name: `Product ${i + 1}`,
				price: Math.floor(Math.random() * 1000) + 10,
				description: `Description for product ${i + 1}`,
				stock: Math.floor(Math.random() * 100),
			});
		}
	} else if (entityName === 'orders') {
		for (let i = 0; i < 20; i++) {
			mockData.push({
				id: `order-${i + 1}`,
				userId: `user-${Math.floor(Math.random() * 10) + 1}`,
				total: Math.floor(Math.random() * 500) + 50,
				status: ['pending', 'completed', 'cancelled'][Math.floor(Math.random() * 3)],
				createdAt: new Date(Date.now() - i * 3600000).toISOString(),
			});
		}
	} else if (entityName === 'analytics') {
		for (let i = 0; i < 15; i++) {
			mockData.push({
				id: `analytics-${i + 1}`,
				metric: ['views', 'clicks', 'conversions'][Math.floor(Math.random() * 3)],
				value: Math.floor(Math.random() * 1000),
				date: new Date(Date.now() - i * 86400000).toISOString().split('T')[0],
			});
		}
	} else {
		// Generic mock data
		for (let i = 0; i < 10; i++) {
			mockData.push({
				id: `${entityName}-${i + 1}`,
				name: `Item ${i + 1}`,
				value: Math.floor(Math.random() * 100),
			});
		}
	}

	const total = mockData.length;
	const startIndex = (page - 1) * pageSize;
	const endIndex = startIndex + pageSize;
	const paginatedData = mockData.slice(startIndex, endIndex);

	return {
		data: paginatedData,
		total,
		page,
		pageSize,
		totalPages: Math.ceil(total / pageSize),
	};
}

// Mock entity config
export function getMockEntityConfig(entityName: string): EntityConfig {
	const entity = mockEntities.find((e) => e.name === entityName);
	return {
		schema: {
			name: entity?.name || entityName,
			type: entity?.type || 'base',
			fields: entity?.fields || [],
		},
		accessRules: [
			{ id: '1', role: '*', permission: 'read' },
			{ id: '2', role: 'admin', permission: 'admin' },
		],
	};
}

// Mock admin users
export const mockAdmins: AdminUser[] = [
	{
		id: 'admin-1',
		email: 'admin@example.com',
		name: 'Admin User',
		createdAt: new Date(Date.now() - 30 * 86400000).toISOString(),
		updatedAt: new Date().toISOString(),
	},
	{
		id: 'admin-2',
		email: 'manager@example.com',
		name: 'Manager',
		createdAt: new Date(Date.now() - 15 * 86400000).toISOString(),
		updatedAt: new Date().toISOString(),
	},
];

export function getMockAdmins(page: number = 1, pageSize: number = 10): AdminListResponse {
	const total = mockAdmins.length;
	const startIndex = (page - 1) * pageSize;
	const endIndex = startIndex + pageSize;
	const paginatedAdmins = mockAdmins.slice(startIndex, endIndex);

	return {
		admins: paginatedAdmins,
		total,
		page,
		pageSize,
		totalPages: Math.ceil(total / pageSize),
	};
}

// Mock logs
export function getMockLogs(page: number = 1, pageSize: number = 20): LogListResponse {
	const levels: LogEntry['level'][] = ['info', 'warn', 'error', 'debug'];
	const sources = ['api', 'database', 'auth', 'system'];
	const messages = [
		'User logged in successfully',
		'Database query executed',
		'Failed to connect to external service',
		'Cache updated',
		'Request processed',
		'Authentication failed',
		'Data validation error',
		'File uploaded successfully',
	];

	const logs: LogEntry[] = [];
	for (let i = 0; i < 50; i++) {
		logs.push({
			id: `log-${i + 1}`,
			timestamp: new Date(Date.now() - i * 60000).toISOString(),
			level: levels[Math.floor(Math.random() * levels.length)],
			message: messages[Math.floor(Math.random() * messages.length)],
			source: sources[Math.floor(Math.random() * sources.length)],
			metadata: { requestId: `req-${i + 1}` },
		});
	}

	const total = logs.length;
	const startIndex = (page - 1) * pageSize;
	const endIndex = startIndex + pageSize;
	const paginatedLogs = logs.slice(startIndex, endIndex);

	return {
		logs: paginatedLogs,
		total,
		page,
		pageSize,
		totalPages: Math.ceil(total / pageSize),
	};
}

// Mock settings
export const mockSettings: Settings = {
	projectName: 'Mantis Admin Dashboard',
	baseUrl: 'http://localhost:3000/admin',
	jwtValidityAdmins: 86400, // 24 hours
	jwtValidityCollections: 3600, // 1 hour
};

// Check if we should use mock data (dev mode without server)
export const USE_MOCK_DATA = import.meta.env.DEV && import.meta.env.VITE_USE_MOCK_DATA !== 'false';
