<script lang="ts">
	import { onMount } from 'svelte';
	import { page } from '$app/stores';
	import { goto } from '$app/navigation';
	import DatabaseIcon from '@lucide/svelte/icons/database';
	import UserIcon from '@lucide/svelte/icons/user';
	import EyeIcon from '@lucide/svelte/icons/eye';
	import ListPlusIcon from '@lucide/svelte/icons/list-plus';
	import { Input } from '$lib/components/ui/input/index.js';
	import CreateTableDrawer from './create-table-drawer.svelte';
	import { getEntities } from '$lib/api/entities.js';
	import { entityStore } from '$lib/stores/entityStore.js';
	import type { Entity } from '$lib/api/entities.js';

	let searchQuery = $state('');
	let loading = $state(false);

	// Subscribe to the store and update local state
	let entities = $derived($entityStore.entities || []);

	let filteredEntities = $derived(
		entities.filter((entity) =>
			entity.name.toLowerCase().includes(searchQuery.toLowerCase())
		)
	);

	let currentEntityName = $derived($page.params.name || null);

	function getEntityIcon(type: Entity['type']) {
		switch (type) {
			case 'base':
				return DatabaseIcon;
			case 'auth':
				return UserIcon;
			case 'view':
				return EyeIcon;
			default:
				return DatabaseIcon;
		}
	}

	async function loadEntities() {
		loading = true;
		try {
			const entities = await getEntities();
			entityStore.setEntities(entities);
		} catch (error) {
			console.error('Failed to load entities:', error);
		} finally {
			loading = false;
		}
	}

	onMount(() => {
		// Only load if store is empty
		if (!$entityStore.entities || $entityStore.entities.length === 0) {
			loadEntities();
		}
	});
</script>

<div class="w-80 border-r bg-sidebar flex flex-col h-full flex-shrink-0">
	<!-- Header -->
	<div class="border-b p-4 space-y-3">
		<div class="flex items-center justify-between">
			<h2 class="text-base font-semibold">Entities</h2>
			<CreateTableDrawer />
		</div>
		<Input 
			placeholder="Type to search..." 
			bind:value={searchQuery}
			class="w-full"
		/>
	</div>

	<!-- Content -->
	<div class="flex-1 overflow-auto">
		{#if loading}
			<div class="p-4 text-sm text-muted-foreground">Loading...</div>
		{:else if filteredEntities.length === 0}
			<div class="p-4 text-sm text-muted-foreground">
				{searchQuery ? 'No entities found' : 'No entities yet'}
			</div>
		{:else}
			<div class="py-2">
				{#each filteredEntities as entity (entity.name)}
					{@const Icon = getEntityIcon(entity.type)}
					{@const isActive = currentEntityName === entity.name}
					<a
						href={`/entities/${entity.name}`}
						class="flex items-center gap-3 px-4 py-2 text-sm hover:bg-sidebar-accent hover:text-sidebar-accent-foreground {isActive ? 'bg-sidebar-accent text-sidebar-accent-foreground' : ''}"
					>
						<Icon class="size-5 flex-shrink-0" />
						<span class="flex-1 truncate">{entity.name}</span>
						<span class="text-xs text-muted-foreground flex-shrink-0">{entity.type}</span>
					</a>
				{/each}
			</div>
		{/if}
	</div>
</div>
