<script lang="ts">
	import '../app.css';
	import favicon from '$lib/assets/mantis.png';
	import { ModeWatcher } from "mode-watcher";
	import { page } from '$app/stores';
	
	let { children } = $props();

	// Pages that should not show the sidebar
	const noSidebarPages = ['/login', '/setup'];
	
	let showSidebar = $derived(!noSidebarPages.some(path => $page.url.pathname.startsWith(path)));
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

<ModeWatcher />
{@render children?.()}
