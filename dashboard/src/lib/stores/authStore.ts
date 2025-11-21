import { writable } from 'svelte/store';
import type { LoginResponse } from '../api/auth.js';

interface AuthState {
	token: string | null;
	user: LoginResponse['user'] | null;
	isAuthenticated: boolean;
}

function createAuthStore() {
	const { subscribe, set, update } = writable<AuthState>({
		token: null,
		user: null,
		isAuthenticated: false,
	});

	// Initialize from localStorage
	if (typeof window !== 'undefined') {
		const token = localStorage.getItem('authToken');
		if (token) {
			update((state) => ({
				...state,
				token,
				isAuthenticated: true,
			}));
		}
	}

	return {
		subscribe,
		setAuth: (token: string, user: LoginResponse['user']) => {
			if (typeof window !== 'undefined') {
				localStorage.setItem('authToken', token);
			}
			set({
				token,
				user,
				isAuthenticated: true,
			});
		},
		clearAuth: () => {
			if (typeof window !== 'undefined') {
				localStorage.removeItem('authToken');
			}
			set({
				token: null,
				user: null,
				isAuthenticated: false,
			});
		},
		setUser: (user: LoginResponse['user']) => {
			update((state) => ({
				...state,
				user,
			}));
		},
	};
}

export const authStore = createAuthStore();
