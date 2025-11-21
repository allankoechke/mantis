<script lang="ts">
  import { onMount } from 'svelte';
  import { page } from '$app/stores';
  import { goto } from '$app/navigation';
  import SignupForm from "$lib/components/signup-form.svelte";
  import { checkAdminExists } from '$lib/api/auth.js';

  let token: string | null = $state(null);
  let hasAdmin: boolean = $state(false);
  let loading: boolean = $state(true);
  let error: string | null = $state(null);

  onMount(async () => {
    // Extract token from URL
    const urlParams = new URLSearchParams(window.location.search);
    token = urlParams.get('token');

    if (!token) {
      error = 'No token provided. Please use a valid signup link.';
      loading = false;
      return;
    }

    // Check if admin already exists
    try {
      hasAdmin = await checkAdminExists();
      if (hasAdmin) {
        error = 'An admin account already exists. Please login instead.';
      }
    } catch (err) {
      error = 'Failed to check admin status. Please try again.';
    } finally {
      loading = false;
    }
  });
</script>

<div class="bg-muted flex min-h-svh flex-col items-center justify-center p-6 md:p-10">
  <div class="w-full max-w-sm md:max-w-4xl">
    {#if loading}
      <div class="flex items-center justify-center p-8">
        <p>Loading...</p>
      </div>
    {:else if error}
      <div class="bg-destructive/10 text-destructive p-4 rounded-lg">
        <p>{error}</p>
        <a href="/login" class="underline mt-2 block">Go to login</a>
      </div>
    {:else if token && !hasAdmin}
      <SignupForm {token} />
    {/if}
  </div>
</div>
