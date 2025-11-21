import { apiClient } from './client.js';
import type { ApiError } from './client.js';
import { USE_MOCK_DATA } from './mock-data.js';

export interface LoginResponse {
	token: string;
	user: {
		email: string;
		name?: string;
	};
}

export interface SignupRequest {
	token: string;
	email: string;
	password: string;
}

export interface SignupResponse {
	message: string;
}

export interface AdminCheckResponse {
	hasAdmin: boolean;
}

export async function login(
	email: string,
	password: string
): Promise<LoginResponse> {
	if (USE_MOCK_DATA) {
		// Simulate network delay
		await new Promise((resolve) => setTimeout(resolve, 500));
		// Accept any email/password in mock mode
		const mockToken = `mock-token-${Date.now()}`;
		apiClient.setAuthToken(mockToken);
		return {
			token: mockToken,
			user: {
				email,
				name: email.split('@')[0],
			},
		};
	}
	try {
		const response = await apiClient.post<LoginResponse>('/auth/login', {
			email,
			password,
		});
		apiClient.setAuthToken(response.token);
		return response;
	} catch (error) {
		throw error as ApiError;
	}
}

export async function signup(
	token: string,
	email: string,
	password: string
): Promise<SignupResponse> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 500));
		return { message: 'Admin account created successfully' };
	}
	try {
		const response = await apiClient.post<SignupResponse>('/auth/signup', {
			token,
			email,
			password,
		});
		return response;
	} catch (error) {
		throw error as ApiError;
	}
}

export async function validateToken(): Promise<{ valid: boolean; user?: LoginResponse['user'] }> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 100));
		const token = localStorage.getItem('authToken');
		if (token && (token.startsWith('mock-token-') || token.length > 0)) {
			// In mock mode, accept any token
			return {
				valid: true,
				user: {
					email: 'admin@example.com',
					name: 'Admin User',
				},
			};
		}
		return { valid: false };
	}
	try {
		const response = await apiClient.get<{ valid: boolean; user?: LoginResponse['user'] }>('/auth/validate');
		return response;
	} catch (error) {
		return { valid: false };
	}
}

export async function checkAdminExists(): Promise<boolean> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 200));
		// In mock mode, return false so signup page works
		return false;
	}
	try {
		const response = await apiClient.get<AdminCheckResponse>('/auth/check-admin');
		return response.hasAdmin;
	} catch (error) {
		// If endpoint doesn't exist, assume admin exists to be safe
		return true;
	}
}

export function logout(): void {
	apiClient.clearAuthToken();
}
