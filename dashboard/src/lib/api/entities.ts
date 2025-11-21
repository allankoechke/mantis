import { apiClient } from './client.js';
import type { ApiError } from './client.js';
import { USE_MOCK_DATA, mockEntities, getMockEntityData, getMockEntityConfig } from './mock-data.js';

export interface Entity {
	name: string;
	type: 'base' | 'auth' | 'view';
	id?: string;
	created?: string;
	fields?: EntityField[];
}

export interface EntityField {
	name: string;
	type: string;
	required?: boolean;
	default?: unknown;
	constraints?: Record<string, unknown>;
}

export interface EntitySchema {
	name: string;
	type: 'base' | 'auth' | 'view';
	fields: EntityField[];
}

export interface EntityDataResponse {
	data: Record<string, unknown>[];
	total: number;
	page: number;
	pageSize: number;
	totalPages: number;
}

export interface EntityDataFilters {
	[key: string]: unknown;
}

export interface CreateEntityRequest {
	name: string;
	type: 'base' | 'auth' | 'view';
	fields: EntityField[];
}

export interface UpdateEntityRequest {
	name?: string;
	type?: 'base' | 'auth' | 'view';
	fields?: EntityField[];
}

export interface AccessRule {
	id?: string;
	role?: string;
	permission: 'read' | 'write' | 'delete' | 'admin';
	conditions?: Record<string, unknown>;
}

export interface EntityConfig {
	schema: EntitySchema;
	accessRules: AccessRule[];
}

export async function getEntities(): Promise<Entity[]> {
	if (USE_MOCK_DATA) {
		// Simulate network delay
		await new Promise((resolve) => setTimeout(resolve, 300));
		return [...mockEntities];
	}
	try {
		return await apiClient.get<Entity[]>('/entities');
	} catch (error) {
		if (import.meta.env.DEV) {
			// Fallback to mock data in dev
			return [...mockEntities];
		}
		throw error as ApiError;
	}
}

export async function getEntity(name: string): Promise<Entity> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 200));
		const entity = mockEntities.find((e) => e.name === name);
		if (entity) return { ...entity };
		throw { message: `Entity ${name} not found` } as ApiError;
	}
	try {
		return await apiClient.get<Entity>(`/entities/${name}`);
	} catch (error) {
		if (import.meta.env.DEV) {
			const entity = mockEntities.find((e) => e.name === name);
			if (entity) return { ...entity };
		}
		throw error as ApiError;
	}
}

export async function getEntityData(
	name: string,
	page: number = 1,
	pageSize: number = 10,
	filters?: EntityDataFilters
): Promise<EntityDataResponse> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 400));
		let data = getMockEntityData(name, page, pageSize);
		
		// Apply search filter if provided
		if (filters?.search) {
			const searchTerm = String(filters.search).toLowerCase();
			data = {
				...data,
				data: data.data.filter((row) =>
					Object.values(row).some((val) =>
						String(val).toLowerCase().includes(searchTerm)
					)
				),
			};
		}
		
		return data;
	}
	try {
		const params = new URLSearchParams({
			page: page.toString(),
			pageSize: pageSize.toString(),
		});

		if (filters) {
			Object.entries(filters).forEach(([key, value]) => {
				if (value !== undefined && value !== null) {
					params.append(key, String(value));
				}
			});
		}

		return await apiClient.get<EntityDataResponse>(
			`/entities/${name}/data?${params.toString()}`
		);
	} catch (error) {
		if (import.meta.env.DEV) {
			return getMockEntityData(name, page, pageSize);
		}
		throw error as ApiError;
	}
}

export async function createEntity(data: CreateEntityRequest): Promise<Entity> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 500));
		const newEntity: Entity = {
			...data,
			id: `mock-${Date.now()}`,
			created: new Date().toISOString(),
		};
		mockEntities.push(newEntity);
		return newEntity;
	}
	try {
		return await apiClient.post<Entity>('/entities', data);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function updateEntity(
	name: string,
	data: UpdateEntityRequest
): Promise<Entity> {
	try {
		return await apiClient.put<Entity>(`/entities/${name}`, data);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function deleteEntity(name: string): Promise<void> {
	try {
		await apiClient.delete(`/entities/${name}`);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function createRecord(
	entity: string,
	data: Record<string, unknown>
): Promise<Record<string, unknown>> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 300));
		const newRecord = {
			...data,
			id: `${entity}-${Date.now()}`,
		};
		return newRecord;
	}
	try {
		return await apiClient.post<Record<string, unknown>>(
			`/entities/${entity}/records`,
			data
		);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function updateRecord(
	entity: string,
	id: string,
	data: Record<string, unknown>
): Promise<Record<string, unknown>> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 300));
		return { ...data, id };
	}
	try {
		return await apiClient.put<Record<string, unknown>>(
			`/entities/${entity}/records/${id}`,
			data
		);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function deleteRecords(
	entity: string,
	ids: string[]
): Promise<void> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 300));
		return;
	}
	try {
		await apiClient.post(`/entities/${entity}/records/delete`, { ids });
	} catch (error) {
		throw error as ApiError;
	}
}

export async function getEntityConfig(name: string): Promise<EntityConfig> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 200));
		return getMockEntityConfig(name);
	}
	try {
		return await apiClient.get<EntityConfig>(`/entities/${name}/config`);
	} catch (error) {
		if (import.meta.env.DEV) {
			return getMockEntityConfig(name);
		}
		throw error as ApiError;
	}
}

export async function updateEntityConfig(
	name: string,
	config: EntityConfig
): Promise<EntityConfig> {
	try {
		return await apiClient.put<EntityConfig>(`/entities/${name}/config`, config);
	} catch (error) {
		throw error as ApiError;
	}
}
