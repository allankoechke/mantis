<script lang="ts">
    import * as Card from "$lib/components/ui/card/index.js";
    import {
        FieldGroup,
        Field,
        FieldLabel,
        FieldDescription,
    } from "$lib/components/ui/field/index.js";
    import { Input } from "$lib/components/ui/input/index.js";
    import { Button } from "$lib/components/ui/button/index.js";
    import { cn } from "$lib/utils.js";
    import { goto } from '$app/navigation';
    import { login } from '$lib/api/auth.js';
    import { authStore } from '$lib/stores/authStore.js';
    import type { HTMLAttributes } from "svelte/elements";
    
    let { class: className, ...restProps }: HTMLAttributes<HTMLDivElement> =
        $props();
    const id = $props.id();

    let email = $state('');
    let password = $state('');
    let loading = $state(false);
    let error = $state<string | null>(null);

    async function handleSubmit(e: SubmitEvent) {
        e.preventDefault();
        loading = true;
        error = null;

        try {
            const response = await login(email, password);
            authStore.setAuth(response.token, response.user);
            goto('/entities');
        } catch (err: unknown) {
            error = err instanceof Error ? err.message : 'Invalid email or password. Please try again.';
        } finally {
            loading = false;
        }
    }
</script>

<div class={cn("flex flex-col gap-6", className)} {...restProps}>
    <Card.Root class="overflow-hidden p-0">
        <Card.Content class="grid p-0 md:grid-cols-2">
            <form class="p-6 md:p-8" onsubmit={handleSubmit}>
                <FieldGroup>
                    <div class="flex flex-col items-center gap-2 text-center">
                        <h1 class="text-2xl font-bold">Welcome back</h1>
                        <p class="text-muted-foreground text-balance">
                            Login to your Mantis Admin account
                        </p>
                    </div>
                    {#if error}
                        <div class="bg-destructive/10 text-destructive p-3 rounded-md text-sm">
                            {error}
                        </div>
                    {/if}
                    <Field>
                        <FieldLabel for="email-{id}">Email</FieldLabel>
                        <Input
                            id="email-{id}"
                            type="email"
                            placeholder="m@example.com"
                            bind:value={email}
                            required
                            disabled={loading}
                        />
                    </Field>
                    <Field>
                        <div class="flex items-center">
                            <FieldLabel for="password-{id}">Password</FieldLabel
                            >
                            <a
                                href="##"
                                class="ml-auto text-sm underline-offset-2 hover:underline"
                            >
                                Forgot your password?
                            </a>
                        </div>
                        <Input 
                            id="password-{id}" 
                            type="password" 
                            bind:value={password}
                            required 
                            disabled={loading}
                        />
                    </Field>
                    <Field>
                        <Button type="submit" disabled={loading}>
                            {loading ? 'Logging in...' : 'Login'}
                        </Button>
                    </Field>
                </FieldGroup>
            </form>
            <div class="bg-muted relative hidden md:block">
                <img
                    src="/banner.jpg"
                    alt="banner"
                    class="absolute inset-0 h-full w-full object-cover dark:brightness-[0.2] dark:grayscale"
                />
            </div>
        </Card.Content>
    </Card.Root>
    <FieldDescription class="px-6 text-center">
        By clicking continue, you agree to our <a href="##">Terms of Service</a>
        and
        <a href="##">Privacy Policy</a>.
    </FieldDescription>
</div>
