<script lang="ts">
	import * as SelectPrimitive from 'bits-ui/select';
	import type { ComponentProps } from 'svelte';

	let {
		selected = $bindable(),
		open = $bindable(false),
		...props
	}: ComponentProps<SelectPrimitive.Root> = $props();
</script>

<SelectPrimitive.Root bind:selected bind:open {...props}>
	{@render children?.()}
</SelectPrimitive.Root>
