<script lang="ts">
	import { cn } from '$lib/utils.js';
	import type { HTMLAttributes } from 'svelte/elements';

	let {
		class: className,
		...restProps
	}: HTMLAttributes<HTMLTableRowElement> & { 'data-state'?: string } = $props();
</script>

<tr
	class={cn(
		'border-b transition-colors hover:bg-muted/50 data-[state=selected]:bg-muted',
		className
	)}
	{...restProps}
>
	{@render children?.()}
</tr>
