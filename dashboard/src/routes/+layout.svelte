<script lang="ts">
	import "../app.css";
	import favicon from "$lib/assets/mantis.png";
	import { ModeWatcher } from "mode-watcher";
	import AppSidebar from "$lib/components/app-sidebar.svelte";
	import * as Sidebar from "$lib/components/ui/sidebar/index.js";
	import { page } from "$app/state";
	import { derived } from "svelte/store";

	let hideNav = $derived(["/login", "/signup", "/setup"].includes(page.url.pathname));
	let { children } = $props();
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

<ModeWatcher />

<div class="bg-muted flex min-h-svh">
	{#if !hideNav}
		<div class="flex h-screen">
			<Sidebar.Provider style="--sidebar-width: 350px;">
				<AppSidebar />
				{@render children?.()}
			</Sidebar.Provider>
		</div>
	{:else}
		{@render children?.()}
	{/if}
</div>
