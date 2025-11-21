<script lang="ts">
	import { cn } from '$lib/utils.js';
	import { Dialog as DialogPrimitive } from 'bits-ui';
	import type { ComponentProps } from 'svelte';

	let {
		class: className,
		...props
	}: ComponentProps<DialogPrimitive.Description> & { class?: string } = $props();
</script>

<DialogPrimitive.Description
	class={cn('text-sm text-muted-foreground', className)}
	{...props}
>
	{@render children?.()}
</DialogPrimitive.Description>
