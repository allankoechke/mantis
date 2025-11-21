<script lang="ts">
	import { cn } from '$lib/utils.js';
	import * as SelectPrimitive from 'bits-ui/select';
	import type { ComponentProps } from 'svelte';
	import ChevronDownIcon from '@lucide/svelte/icons/chevron-down';

	let {
		class: className,
		...props
	}: ComponentProps<SelectPrimitive.Trigger> & { class?: string } = $props();
</script>

<SelectPrimitive.Trigger
	class={cn(
		'flex h-10 w-full items-center justify-between rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background placeholder:text-muted-foreground focus:outline-none focus:ring-2 focus:ring-ring focus:ring-offset-2 disabled:cursor-not-allowed disabled:opacity-50 [&>span]:line-clamp-1',
		className
	)}
	{...props}
>
	{@render children?.()}
	<ChevronDownIcon class="h-4 w-4 opacity-50" />
</SelectPrimitive.Trigger>
