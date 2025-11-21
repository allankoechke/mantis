<script lang="ts">
  import { onMount } from 'svelte';
  import { goto } from '$app/navigation';
  import { page } from '$app/stores';
  import { Skeleton } from "$lib/components/ui/skeleton/index.js";
  import { authStore } from '$lib/stores/authStore.js';

  onMount(() => {
    // Check for token in URL query params for signup
    const urlParams = new URLSearchParams(window.location.search);
    const tokenFromUrl = urlParams.get('token');
    
    if (tokenFromUrl) {
      goto(`/setup?token=${tokenFromUrl}`);
      return;
    }

    // Check for a valid auth token
    const token = localStorage.getItem('authToken');
    if (token) {
      // Validate token and redirect to entities
      goto('/entities');
      return;
    }

    // If no valid token, redirect to login
    goto('/login');
  });
</script>

<div class="flex flex-col items-center justify-center space-y-4">
  <h1>Loading MantisBase Admin</h1>
  <div class="flex items-center justify-center col space-x-4">
    <Skeleton class="size-12 rounded-full" />
    <div class="space-y-2">
        <Skeleton class="h-4 w-[250px]" />
        <Skeleton class="h-4 w-[200px]" />
    </div>
  </div>
</div>
