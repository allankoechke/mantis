<script lang="ts">
	import { onMount } from 'svelte';
	import { page } from '$app/stores';
	import { goto } from '$app/navigation';
	import DatabaseIcon from '@lucide/svelte/icons/database';
	import UserIcon from '@lucide/svelte/icons/user';
	import EyeIcon from '@lucide/svelte/icons/eye';
	import ListPlusIcon from '@lucide/svelte/icons/list-plus';
	import * as Sidebar from '$lib/components/ui/sidebar/index.js';
	import CreateTableDrawer from './create-table-drawer.svelte';
	import { getEntities } from '$lib/api/entities.js';
	import { entityStore } from '$lib/stores/entityStore.js';
	import type { Entity } from '$lib/api/entities.js';

	let searchQuery = $state('');
	let loading = $state(false);

	// Subscribe to the store and update local state
	let entities: Entity[] = $state([]);
	
	$effect(() => {
		const unsubscribe = entityStore.subscribe((state) => {
			entities = state.entities || [];
		});
		return unsubscribe;
	});

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

	let lastPath = $state($page.url.pathname);

	onMount(() => {
		loadEntities();
	});

	// Reload entities when route changes to entities page
	$effect(() => {
		const currentPath = $page.url.pathname;
		if (currentPath.startsWith('/entities') && currentPath !== lastPath) {
			lastPath = currentPath;
			loadEntities();
		}
	});
</script>

<Sidebar.Root collapsible="none" class="hidden flex-1 md:flex">
	<Sidebar.Header class="gap-3.5 border-b p-4">
		<div class="flex w-full items-center justify-between">
			<div class="text-foreground text-base font-medium">
				Entities
			</div>
			<CreateTableDrawer />
		</div>
		<Sidebar.Input 
			placeholder="Type to search..." 
			bind:value={searchQuery}
		/>
	</Sidebar.Header>
	<Sidebar.Content>
		<Sidebar.Group class="px-0">
			<Sidebar.GroupContent>
				{#if loading}
					<div class="p-4 text-sm text-muted-foreground">Loading...</div>
				{:else if filteredEntities.length === 0}
					<div class="p-4 text-sm text-muted-foreground">
						{searchQuery ? 'No entities found' : 'No entities yet'}
					</div>
				{:else}
					{#each filteredEntities as entity (entity.name)}
						{@const Icon = getEntityIcon(entity.type)}
						{@const isActive = currentEntityName === entity.name}
						<a
							href={`/entities/${entity.name}`}
							class="hover:bg-sidebar-accent hover:text-sidebar-accent-foreground flex items-center gap-2 whitespace-nowrap border-b p-4 text-sm leading-tight last:border-b-0 {isActive ? 'bg-sidebar-accent text-sidebar-accent-foreground' : ''}"
						>
							<Icon class="size-4" />
							<span class="flex-1">{entity.name}</span>
							<span class="ml-auto text-xs text-muted-foreground">{entity.type}</span>
						</a>
					{/each}
				{/if}
			</Sidebar.GroupContent>
		</Sidebar.Group>
	</Sidebar.Content>
</Sidebar.Root>
