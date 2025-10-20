<script lang="ts">
  import { onMount } from 'svelte';
  import { goto } from '$app/navigation';
  import { Skeleton } from "$lib/components/ui/skeleton/index.js";

  onMount(() => {
    // Check for a valid auth token if provided
    const token = localStorage.getItem('authToken');
    if (token) {
      // Here you would typically validate the token with your backend
      // For this example, we'll assume the token is valid if it exists
      goto('/tables');
      return;
    }

    // Maybe later add logic to check for token provided via URL query params
    // const urlParams = new URLSearchParams(window.location.search);
    // const tokenFromUrl = urlParams.get('token');
    // if (tokenFromUrl) {
    //   // Validate token from URL
    //   goto('/setup');
    //   return;
    // }

    // If no valid token, redirect to the tables page
    goto('/login');
  });
</script>

<div class="flex flex-col items-center justify-center space-y-4">
  <h1>Loading Mantis Admin</h1>
  <div class="flex items-center justify-center col space-x-4">
    <Skeleton class="size-12 rounded-full" />
    <div class="space-y-2">
        <Skeleton class="h-4 w-[250px]" />
        <Skeleton class="h-4 w-[200px]" />
    </div>
  </div>
</div>
