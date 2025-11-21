<script lang="ts">
	import * as SelectPrimitive from 'bits-ui/select';
	import type { ComponentProps } from 'svelte';

	let props: ComponentProps<SelectPrimitive.Value> = $props();
</script>

<SelectPrimitive.Value {...props}>
	{@render children?.()}
</SelectPrimitive.Value>
