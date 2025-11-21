<script lang="ts">
	import { Dialog as DialogPrimitive } from 'bits-ui';
	import type { ComponentProps } from 'svelte';

	let props: ComponentProps<DialogPrimitive.Trigger> = $props();
</script>

<DialogPrimitive.Trigger {...props}>
	{@render children?.()}
</DialogPrimitive.Trigger>
