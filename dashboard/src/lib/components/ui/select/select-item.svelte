<script lang="ts">
	import { cn } from '$lib/utils.js';
	import * as SelectPrimitive from 'bits-ui/select';
	import type { ComponentProps } from 'svelte';

	let {
		class: className,
		...props
	}: ComponentProps<SelectPrimitive.Item> & { class?: string } = $props();
</script>

<SelectPrimitive.Item
	class={cn(
		'relative flex w-full cursor-default select-none items-center rounded-sm py-1.5 pl-8 pr-2 text-sm outline-none focus:bg-accent focus:text-accent-foreground data-[disabled]:pointer-events-none data-[disabled]:opacity-50',
		className
	)}
	{...props}
>
	{@render children?.()}
</SelectPrimitive.Item>
