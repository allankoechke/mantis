import { apiClient } from './client.js';
import type { ApiError } from './client.js';
import { USE_MOCK_DATA, mockSettings } from './mock-data.js';

export interface Settings {
	projectName: string;
	baseUrl: string;
	jwtValidityAdmins: number; // in seconds
	jwtValidityCollections: number; // in seconds
	[key: string]: unknown;
}

export async function getSettings(): Promise<Settings> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 200));
		return { ...mockSettings };
	}
	try {
		return await apiClient.get<Settings>('/settings');
	} catch (error) {
		if (import.meta.env.DEV) {
			return { ...mockSettings };
		}
		throw error as ApiError;
	}
}

export async function updateSettings(data: Partial<Settings>): Promise<Settings> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 400));
		Object.assign(mockSettings, data);
		return { ...mockSettings };
	}
	try {
		return await apiClient.put<Settings>('/settings', data);
	} catch (error) {
		throw error as ApiError;
	}
}
