<script lang="ts">
	import { cn } from '$lib/utils.js';
	import type { HTMLAttributes } from 'svelte/elements';

	let { class: className, ...restProps }: HTMLAttributes<HTMLTableCaptionElement> = $props();
</script>

<caption class={cn('mt-4 text-sm text-muted-foreground', className)} {...restProps}>
	{@render children?.()}
</caption>
