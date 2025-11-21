<script lang="ts">
  import { cn } from "$lib/utils.js";
  import { Button } from "$lib/components/ui/button/index.js";
  import * as Card from "$lib/components/ui/card/index.js";
  import * as Field from "$lib/components/ui/field/index.js";
  import { Input } from "$lib/components/ui/input/index.js";
  import { goto } from '$app/navigation';
  import { signup } from '$lib/api/auth.js';
  import type { HTMLAttributes } from "svelte/elements";
  
  let { 
    class: className, 
    token 
  }: HTMLAttributes<HTMLDivElement> & { token: string } = $props();

  let email = $state('');
  let password = $state('');
  let loading = $state(false);
  let error = $state<string | null>(null);
  let success = $state(false);

  async function handleSubmit(e: SubmitEvent) {
    e.preventDefault();
    if (!token) return;

    loading = true;
    error = null;

    try {
      await signup(token, email, password);
      success = true;
      setTimeout(() => {
        goto('/login');
      }, 2000);
    } catch (err: unknown) {
      error = err instanceof Error ? err.message : 'Failed to create account. Please try again.';
    } finally {
      loading = false;
    }
  }
</script>
<div class={cn("flex flex-col gap-6", className)} {...restProps}>
  <Card.Root class="overflow-hidden p-0">
    <Card.Content class="grid p-0 md:grid-cols-2">
      <form class="p-6 md:p-8" onsubmit={handleSubmit}>
        <Field.Group>
          <div class="flex flex-col items-center gap-2 text-center">
            <h1 class="text-2xl font-bold">Create your account</h1>
            <p class="text-muted-foreground text-balance text-sm">
              Enter your email and password to create the admin account
            </p>
          </div>
          {#if error}
            <div class="bg-destructive/10 text-destructive p-3 rounded-md text-sm">
              {error}
            </div>
          {/if}
          {#if success}
            <div class="bg-green-500/10 text-green-600 dark:text-green-400 p-3 rounded-md text-sm">
              Account created successfully! Redirecting to login...
            </div>
          {/if}
          <Field.Field>
            <Field.Label for="email">Email</Field.Label>
            <Input 
              id="email" 
              type="email" 
              placeholder="m@example.com" 
              bind:value={email}
              required 
              disabled={loading || success}
            />
            <Field.Description>
              We'll use this to contact you. We will not share your email with anyone
              else.
            </Field.Description>
          </Field.Field>
          <Field.Field>
            <Field.Label for="password">Password</Field.Label>
            <Input 
              id="password" 
              type="password" 
              bind:value={password}
              required 
              disabled={loading || success}
            />
            <Field.Description>Must be at least 8 characters long.</Field.Description>
          </Field.Field>
          <Field.Field>
            <Button type="submit" disabled={loading || success}>
              {loading ? 'Creating Account...' : 'Create Account'}
            </Button>
          </Field.Field>
          <Field.Description class="text-center">
            Already have an account? <a href="/login">Sign in</a>
          </Field.Description>
        </Field.Group>
      </form>
      <div class="bg-muted relative hidden md:block">
        <img
          src="/banner.jpg"
          alt=""
          class="absolute inset-0 h-full w-full object-cover dark:brightness-[0.2] dark:grayscale"
        />
      </div>
    </Card.Content>
  </Card.Root>
  <Field.Description class="px-6 text-center">
    By clicking continue, you agree to our <a href="#/">Terms of Service</a>
    and <a href="#/">Privacy Policy</a>.
  </Field.Description>
</div>