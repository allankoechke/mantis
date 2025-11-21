<script lang="ts">
	import { cn } from '$lib/utils.js';
	import { Dialog as DialogPrimitive } from 'bits-ui';
	import type { ComponentProps } from 'svelte';

	let {
		class: className,
		...props
	}: ComponentProps<DialogPrimitive.Title> & { class?: string } = $props();
</script>

<DialogPrimitive.Title
	class={cn('text-lg font-semibold leading-none tracking-tight', className)}
	{...props}
>
	{@render children?.()}
</DialogPrimitive.Title>
