<script lang="ts">
	import { onMount } from 'svelte';
	import { goto } from '$app/navigation';
	import { page } from '$app/stores';
	import { authStore } from '$lib/stores/authStore.js';
	import { validateToken } from '$lib/api/auth.js';
	import AppSidebar from '$lib/components/app-sidebar.svelte';

	let { children } = $props();
	let loading = $state(true);

	onMount(async () => {
		try {
			// Check if user is authenticated
			const token = localStorage.getItem('authToken');
			if (!token) {
				loading = false;
				goto('/login');
				return;
			}

			// Validate token
			const result = await validateToken();
			if (!result.valid) {
				authStore.clearAuth();
				loading = false;
				goto('/login');
				return;
			}
			
			// Set auth state
			if (result.user) {
				authStore.setAuth(token, result.user);
			} else {
				// If no user in result, use token to set basic auth
				authStore.setAuth(token, { email: 'admin@example.com' });
			}
		} catch (error) {
			console.error('Token validation error:', error);
			authStore.clearAuth();
		} finally {
			loading = false;
		}
	});
</script>

{#if loading}
	<div class="flex items-center justify-center min-h-screen">
		<p>Loading...</p>
	</div>
{:else}
	<div class="flex h-screen overflow-hidden">
		<AppSidebar />
		<div class="flex-1 overflow-hidden flex flex-col">
			{@render children?.()}
		</div>
	</div>
{/if}
