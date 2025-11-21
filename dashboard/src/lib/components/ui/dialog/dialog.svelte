<script lang="ts">
	import { Dialog as DialogPrimitive } from 'bits-ui';
	import type { ComponentProps } from 'svelte';

	let { open = $bindable(false), ...props }: ComponentProps<DialogPrimitive.Root> = $props();
</script>

<DialogPrimitive.Root bind:open {...props}>
	{@render children?.()}
</DialogPrimitive.Root>
