<script lang="ts">
	import '../app.css';
	import favicon from '$lib/assets/mantis.png';
  import { ModeWatcher } from "mode-watcher";
	
	let { children } = $props();
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

<ModeWatcher />
{@render children?.()}
