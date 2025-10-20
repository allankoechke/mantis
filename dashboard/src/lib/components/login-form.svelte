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
    import type { HTMLAttributes } from "svelte/elements";
    let { class: className, ...restProps }: HTMLAttributes<HTMLDivElement> =
        $props();
    const id = $props.id();
</script>

<div class={cn("flex flex-col gap-6", className)} {...restProps}>
    <Card.Root class="overflow-hidden p-0">
        <Card.Content class="grid p-0 md:grid-cols-2">
            <form class="p-6 md:p-8">
                <FieldGroup>
                    <div class="flex flex-col items-center gap-2 text-center">
                        <h1 class="text-2xl font-bold">Welcome back</h1>
                        <p class="text-muted-foreground text-balance">
                            Login to your Mantis Admin account
                        </p>
                    </div>
                    <Field>
                        <FieldLabel for="email-{id}">Email</FieldLabel>
                        <Input
                            id="email-{id}"
                            type="email"
                            placeholder="m@example.com"
                            required
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
                        <Input id="password-{id}" type="password" required />
                    </Field>
                    <Field>
                        <Button type="submit">Login</Button>
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
