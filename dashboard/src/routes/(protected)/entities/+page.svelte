<script lang="ts">
	import { onMount } from 'svelte';
	import EntityList from '$lib/components/entity-list.svelte';
	import { Button } from '$lib/components/ui/button/index.js';
	import { getEntities } from '$lib/api/entities.js';
	import { entityStore } from '$lib/stores/entityStore.js';
	import { goto } from '$app/navigation';
	import ListPlusIcon from '@lucide/svelte/icons/list-plus';

	let loading = $state(true);

	onMount(async () => {
		try {
			// Only load if store is empty
			if (!$entityStore.entities || $entityStore.entities.length === 0) {
				const entities = await getEntities();
				entityStore.setEntities(entities);
			}
			
			// If there are entities, redirect to the first one
			if ($entityStore.entities && $entityStore.entities.length > 0) {
				// Small delay to ensure layout is ready
				setTimeout(() => {
					goto(`/entities/${$entityStore.entities[0].name}`);
				}, 100);
			}
		} catch (error) {
			console.error('Failed to load entities:', error);
		} finally {
			loading = false;
		}
	});
</script>

<div class="flex h-full">
	<EntityList />
	<div class="flex-1 flex items-center justify-center p-8">
		{#if loading}
			<p class="text-muted-foreground">Loading entities...</p>
		{:else if $entityStore.entities.length === 0}
			<div class="flex flex-col items-center gap-4 text-center">
				<h2 class="text-2xl font-bold">No entities yet</h2>
				<p class="text-muted-foreground max-w-md">
					Get started by creating your first database table. Click the button below to add a new table.
				</p>
				<Button onclick={() => goto('/entities')}>
					<ListPlusIcon class="mr-2 size-4" />
					Add Table
				</Button>
			</div>
		{:else}
			<div class="flex flex-col items-center gap-4 text-center">
				<p class="text-muted-foreground">Select an entity from the sidebar to view its data.</p>
			</div>
		{/if}
	</div>
</div>
