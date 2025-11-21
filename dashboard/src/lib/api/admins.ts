import { apiClient } from './client.js';
import type { ApiError } from './client.js';
import { USE_MOCK_DATA, getMockAdmins, mockAdmins } from './mock-data.js';

export interface AdminUser {
	id: string;
	email: string;
	name?: string;
	createdAt?: string;
	updatedAt?: string;
}

export interface AdminListResponse {
	admins: AdminUser[];
	total: number;
	page: number;
	pageSize: number;
	totalPages: number;
}

export interface CreateAdminRequest {
	email: string;
	password: string;
	name?: string;
}

export interface UpdateAdminRequest {
	email?: string;
	password?: string;
	name?: string;
}

export async function getAdmins(
	page: number = 1,
	pageSize: number = 10
): Promise<AdminListResponse> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 300));
		return getMockAdmins(page, pageSize);
	}
	try {
		const params = new URLSearchParams({
			page: page.toString(),
			pageSize: pageSize.toString(),
		});
		return await apiClient.get<AdminListResponse>(`/admins?${params.toString()}`);
	} catch (error) {
		if (import.meta.env.DEV) {
			return getMockAdmins(page, pageSize);
		}
		throw error as ApiError;
	}
}

export async function getAdmin(id: string): Promise<AdminUser> {
	try {
		return await apiClient.get<AdminUser>(`/admins/${id}`);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function createAdmin(data: CreateAdminRequest): Promise<AdminUser> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 400));
		const newAdmin: AdminUser = {
			id: `admin-${Date.now()}`,
			email: data.email,
			name: data.name,
			createdAt: new Date().toISOString(),
			updatedAt: new Date().toISOString(),
		};
		mockAdmins.push(newAdmin);
		return newAdmin;
	}
	try {
		return await apiClient.post<AdminUser>('/admins', data);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function updateAdmin(
	id: string,
	data: UpdateAdminRequest
): Promise<AdminUser> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 400));
		const admin = mockAdmins.find((a) => a.id === id);
		if (admin) {
			Object.assign(admin, data, { updatedAt: new Date().toISOString() });
			return { ...admin };
		}
		throw { message: 'Admin not found' } as ApiError;
	}
	try {
		return await apiClient.put<AdminUser>(`/admins/${id}`, data);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function deleteAdmin(id: string): Promise<void> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 300));
		const index = mockAdmins.findIndex((a) => a.id === id);
		if (index !== -1) {
			mockAdmins.splice(index, 1);
		}
		return;
	}
	try {
		await apiClient.delete(`/admins/${id}`);
	} catch (error) {
		throw error as ApiError;
	}
}

export async function changePassword(
	currentPassword: string,
	newPassword: string
): Promise<void> {
	try {
		await apiClient.post('/admins/change-password', {
			currentPassword,
			newPassword,
		});
	} catch (error) {
		throw error as ApiError;
	}
}
