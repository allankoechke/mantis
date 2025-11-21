<script lang="ts">
	import { cn } from '$lib/utils.js';
	import type { HTMLInputAttributes } from 'svelte/elements';

	let {
		checked = $bindable(false),
		indeterminate = $bindable(false),
		class: className,
		onclick,
		...props
	}: HTMLInputAttributes & { 
		class?: string;
		indeterminate?: boolean;
	} = $props();
</script>

<input
	type="checkbox"
	bind:checked
	class={cn(
		'peer h-4 w-4 shrink-0 rounded-sm border border-primary ring-offset-background focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:cursor-not-allowed disabled:opacity-50 checked:bg-primary checked:text-primary-foreground',
		className
	)}
	{...props}
	{onclick}
/>