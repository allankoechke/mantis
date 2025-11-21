<script lang="ts">
	import { onMount } from 'svelte';
	import { page } from '$app/stores';
	import EntityList from '$lib/components/entity-list.svelte';
	import EntityTable from '$lib/components/entity-table.svelte';
	import EntityConfigDrawer from '$lib/components/entity-config-drawer.svelte';
	import EntityDocsDrawer from '$lib/components/entity-docs-drawer.svelte';
	import RecordEditDrawer from '$lib/components/record-edit-drawer.svelte';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import * as DropdownMenu from '$lib/components/ui/dropdown-menu/index.js';
	import RefreshCwIcon from '@lucide/svelte/icons/refresh-cw';
	import PlusIcon from '@lucide/svelte/icons/plus';
	import SettingsIcon from '@lucide/svelte/icons/settings';
	import TrashIcon from '@lucide/svelte/icons/trash';
	import FileTextIcon from '@lucide/svelte/icons/file-text';
	import SearchIcon from '@lucide/svelte/icons/search';
	import {
		getEntity,
		getEntityData,
		deleteRecords,
		type EntityDataResponse,
		type Entity,
	} from '$lib/api/entities.js';
	import { entityStore } from '$lib/stores/entityStore.js';

	let entity: Entity | null = $state(null);
	let data: EntityDataResponse | null = $state(null);
	let loading = $state(false);
	let error: string | null = $state(null);
	let filterQuery = $state('');
	let selectedColumns = $state<string[]>([]);
	let selectedRows = $state(new Set<string>());
	let configDrawerOpen = $state(false);
	let docsDrawerOpen = $state(false);
	let editDrawerOpen = $state(false);
	let editingRecord: Record<string, unknown> | null = $state(null);
	let currentPage = $state(1);
	const pageSize = 10;

	let entityName = $derived($page.params.name);

	async function loadEntity() {
		if (!entityName) return;

		loading = true;
		error = null;

		try {
			entity = await getEntity(entityName);
			entityStore.setCurrentEntity(entity);
			
			// Initialize selected columns with all columns
			if (entity.fields && selectedColumns.length === 0) {
				selectedColumns = entity.fields.map((f) => f.name);
			}

			await loadData();
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to load entity';
		} finally {
			loading = false;
		}
	}

	let dataLoading = $state(false);

	async function loadData() {
		if (!entityName) return;

		dataLoading = true;
		try {
			data = await getEntityData(entityName, currentPage, pageSize, {
				search: filterQuery || undefined,
			});
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to load data';
		} finally {
			dataLoading = false;
		}
	}

	async function handleReload() {
		await loadData();
	}

	function handleAddRecord() {
		editingRecord = {};
		editDrawerOpen = true;
	}

	function handleRowClick(row: Record<string, unknown>) {
		editingRecord = { ...row };
		editDrawerOpen = true;
	}

	async function handleDeleteSelected() {
		if (selectedRows.size === 0 || !entityName) return;

		if (!confirm(`Delete ${selectedRows.size} record(s)?`)) return;

		try {
			await deleteRecords(entityName, Array.from(selectedRows));
			selectedRows.clear();
			await loadData();
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to delete records';
		}
	}

	function handleColumnToggle(column: string) {
		if (selectedColumns.includes(column)) {
			selectedColumns = selectedColumns.filter((c) => c !== column);
		} else {
			selectedColumns = [...selectedColumns, column];
		}
	}

	function handlePageChange(page: number) {
		currentPage = page;
		loadData();
	}

	onMount(() => {
		loadEntity();
	});

	let lastEntityName = $state('');

	$effect(() => {
		const newEntityName = $page.params.name;
		if (newEntityName && newEntityName !== lastEntityName) {
			lastEntityName = newEntityName;
			currentPage = 1;
			selectedRows.clear();
			loadEntity();
		}
	});
</script>

	<div class="flex h-full">
	<EntityList />
	<div class="flex-1 flex flex-col overflow-hidden">
		<!-- Header -->
		<header class="border-b p-4">
			<div class="flex items-center justify-between mb-4">
				<h1 class="text-2xl font-bold">{entity?.name || entityName}</h1>
				<div class="flex items-center gap-2">
					<Button variant="outline" size="sm" onclick={handleReload} disabled={dataLoading || loading}>
						<RefreshCwIcon class="mr-2 size-4" />
						Reload
					</Button>
					<Button variant="outline" size="sm" onclick={() => (docsDrawerOpen = true)}>
						<FileTextIcon class="mr-2 size-4" />
						Docs
					</Button>
					<Button variant="outline" size="sm" onclick={handleAddRecord}>
						<PlusIcon class="mr-2 size-4" />
						Add Entry
					</Button>
					<Button
						variant="outline"
						size="sm"
						onclick={() => (configDrawerOpen = true)}
					>
						<SettingsIcon class="mr-2 size-4" />
						Config
					</Button>
					{#if selectedRows.size > 0}
						<Button
							variant="destructive"
							size="sm"
							onclick={handleDeleteSelected}
						>
							<TrashIcon class="mr-2 size-4" />
							Delete ({selectedRows.size})
						</Button>
					{/if}
				</div>
			</div>

			<div class="flex items-center gap-2">
				<div class="flex-1 flex items-center gap-2">
					<Input
						placeholder="Filter records..."
						bind:value={filterQuery}
						oninput={() => {
							currentPage = 1;
							loadData();
						}}
						class="flex-1"
					/>
					<Button variant="outline" size="sm" onclick={() => {
						currentPage = 1;
						loadData();
					}}>
						<SearchIcon class="size-4" />
					</Button>
				</div>
				<DropdownMenu.Root>
					<DropdownMenu.Trigger>
						<Button variant="outline" size="sm">
							Columns ({selectedColumns.length})
						</Button>
					</DropdownMenu.Trigger>
					<DropdownMenu.Content>
						{#if entity?.fields}
							{#each entity.fields as field}
								<DropdownMenu.CheckboxItem
									checked={selectedColumns.includes(field.name)}
									onCheckedChange={() => handleColumnToggle(field.name)}
								>
									{field.name}
								</DropdownMenu.CheckboxItem>
							{/each}
						{/if}
					</DropdownMenu.Content>
				</DropdownMenu.Root>
			</div>
		</header>

		<!-- Content -->
		<div class="flex-1 overflow-auto p-4">
			{#if error}
				<div class="bg-destructive/10 text-destructive p-4 rounded-lg mb-4">
					{error}
				</div>
			{/if}
			{#if entity}
				<EntityTable
					data={data || {
						data: [],
						total: 0,
						page: 1,
						pageSize: 10,
						totalPages: 0,
					}}
					columns={entity.fields?.map((f) => f.name) || []}
					{selectedColumns}
					{selectedRows}
					loading={dataLoading}
					onColumnToggle={handleColumnToggle}
					onRowSelect={(id, selected) => {
						if (selected) {
							selectedRows.add(id);
						} else {
							selectedRows.delete(id);
						}
						// Trigger reactivity by creating new Set
						selectedRows = new Set(selectedRows);
					}}
					onRowClick={handleRowClick}
					onPageChange={handlePageChange}
				/>
			{:else if loading}
				<div class="flex items-center justify-center p-8">
					<p class="text-muted-foreground">Loading entity...</p>
				</div>
			{/if}
		</div>
	</div>
</div>

{#if entity && configDrawerOpen}
	<EntityConfigDrawer
		entity={entity}
		open={configDrawerOpen}
		onOpenChange={(open) => (configDrawerOpen = open)}
		onSaved={loadEntity}
	/>
{/if}

{#if entity && docsDrawerOpen}
	<EntityDocsDrawer
		entity={entity}
		open={docsDrawerOpen}
		onOpenChange={(open) => (docsDrawerOpen = open)}
	/>
{/if}

{#if entity && editDrawerOpen && editingRecord !== null}
	<RecordEditDrawer
		entity={entity}
		record={editingRecord}
		open={editDrawerOpen}
		onOpenChange={(open) => {
			editDrawerOpen = open;
			if (!open) editingRecord = null;
		}}
		onSaved={() => {
			editDrawerOpen = false;
			editingRecord = null;
			loadData();
		}}
	/>
{/if}
