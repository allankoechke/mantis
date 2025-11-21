const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:3000/admin';

export interface ApiError {
	message: string;
	status?: number;
}

class ApiClient {
	private baseUrl: string;

	constructor(baseUrl: string = API_BASE_URL) {
		this.baseUrl = baseUrl;
	}

	private getToken(): string | null {
		if (typeof window !== 'undefined') {
			return localStorage.getItem('authToken');
		}
		return null;
	}

	private setToken(token: string): void {
		if (typeof window !== 'undefined') {
			localStorage.setItem('authToken', token);
		}
	}

	private clearToken(): void {
		if (typeof window !== 'undefined') {
			localStorage.removeItem('authToken');
		}
	}

	private async request<T>(
		endpoint: string,
		options: RequestInit = {}
	): Promise<T> {
		const token = this.getToken();
		const headers: HeadersInit = {
			'Content-Type': 'application/json',
			...options.headers,
		};

		if (token) {
			headers['Authorization'] = `Bearer ${token}`;
		}

		const url = `${this.baseUrl}${endpoint}`;

		try {
			const response = await fetch(url, {
				...options,
				headers,
			});

			if (!response.ok) {
				if (response.status === 401) {
					// Unauthorized - clear token and redirect to login
					this.clearToken();
					if (typeof window !== 'undefined') {
						window.location.href = '/login';
					}
				}

				const errorData = await response.json().catch(() => ({
					message: response.statusText || 'An error occurred',
				}));

				const error: ApiError = {
					message: errorData.message || 'An error occurred',
					status: response.status,
				};

				throw error;
			}

			// Handle empty responses
			const contentType = response.headers.get('content-type');
			if (contentType && contentType.includes('application/json')) {
				return await response.json();
			}

			return {} as T;
		} catch (error) {
			if (error instanceof Error && 'status' in error) {
				throw error;
			}
			throw {
				message: error instanceof Error ? error.message : 'Network error',
			} as ApiError;
		}
	}

	async get<T>(endpoint: string): Promise<T> {
		return this.request<T>(endpoint, { method: 'GET' });
	}

	async post<T>(endpoint: string, data?: unknown): Promise<T> {
		return this.request<T>(endpoint, {
			method: 'POST',
			body: data ? JSON.stringify(data) : undefined,
		});
	}

	async put<T>(endpoint: string, data?: unknown): Promise<T> {
		return this.request<T>(endpoint, {
			method: 'PUT',
			body: data ? JSON.stringify(data) : undefined,
		});
	}

	async patch<T>(endpoint: string, data?: unknown): Promise<T> {
		return this.request<T>(endpoint, {
			method: 'PATCH',
			body: data ? JSON.stringify(data) : undefined,
		});
	}

	async delete<T>(endpoint: string): Promise<T> {
		return this.request<T>(endpoint, { method: 'DELETE' });
	}

	// Auth-specific methods
	setAuthToken(token: string): void {
		this.setToken(token);
	}

	clearAuthToken(): void {
		this.clearToken();
	}

	hasToken(): boolean {
		return this.getToken() !== null;
	}
}

export const apiClient = new ApiClient();
