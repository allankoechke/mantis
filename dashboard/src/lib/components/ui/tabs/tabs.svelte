<script lang="ts">
	import { Tabs as TabsPrimitive } from 'bits-ui';
	import type { ComponentProps } from 'svelte';

	let {
		value = $bindable(),
		...props
	}: ComponentProps<TabsPrimitive.Root> = $props();
</script>

<TabsPrimitive.Root bind:value {...props}>
	{@render children?.()}
</TabsPrimitive.Root>
