<script lang="ts">
	import * as Table from '$lib/components/ui/table/index.js';
	import { Checkbox } from '$lib/components/ui/checkbox/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import type { EntityDataResponse } from '$lib/api/entities.js';

	interface Props {
		data: EntityDataResponse;
		columns: string[];
		selectedColumns: string[];
		selectedRows: Set<string>;
		loading?: boolean;
		onColumnToggle: (column: string) => void;
		onRowSelect: (id: string, selected: boolean) => void;
		onRowClick: (row: Record<string, unknown>) => void;
		onPageChange: (page: number) => void;
	}

	let {
		data,
		columns,
		selectedColumns,
		selectedRows,
		loading = false,
		onColumnToggle,
		onRowSelect,
		onRowClick,
		onPageChange,
	}: Props = $props();

	let visibleColumns = $derived(selectedColumns.length > 0 ? selectedColumns : columns);
	let hasData = $derived(data && data.data && data.data.length > 0);
	let allSelected = $derived(hasData && data.data.every((row) => selectedRows.has(String(row.id || ''))));
	let someSelected = $derived(hasData && data.data.some((row) => selectedRows.has(String(row.id || ''))));

	function handleSelectAll(checked: boolean) {
		data.data.forEach((row) => {
			const id = String(row.id || '');
			onRowSelect(id, checked);
		});
		// Trigger reactivity
		selectedRows = new Set(selectedRows);
	}

	function formatValue(value: unknown): string {
		if (value === null || value === undefined) return '';
		if (typeof value === 'object') return JSON.stringify(value);
		return String(value);
	}
</script>

<div class="rounded-md border">
	{#if loading}
		<div class="flex items-center justify-center p-8">
			<p class="text-muted-foreground">Loading...</p>
		</div>
	{:else}
		<Table.Table>
			<Table.TableHeader>
				<Table.TableRow>
					<Table.TableHead class="w-12">
						{#if hasData}
							<Checkbox
								checked={allSelected}
								indeterminate={someSelected && !allSelected}
								onclick={() => handleSelectAll(!allSelected)}
							/>
						{/if}
					</Table.TableHead>
					{#each visibleColumns as column}
						<Table.TableHead>{column}</Table.TableHead>
					{/each}
				</Table.TableRow>
			</Table.TableHeader>
			<Table.TableBody>
				{#if !hasData}
					<Table.TableRow>
						<Table.TableCell colspan={visibleColumns.length + 1} class="text-center py-8">
							<div class="flex flex-col items-center justify-center gap-4">
								<p class="text-muted-foreground">No records found</p>
								<div class="flex gap-2">
									<Button onclick={() => onRowClick({})}>Add Record</Button>
								</div>
							</div>
						</Table.TableCell>
					</Table.TableRow>
				{:else}
					{#each data.data as row (String(row.id || ''))}
						{@const rowId = String(row.id || '')}
						{@const isSelected = selectedRows.has(rowId)}
						<Table.TableRow
							data-state={isSelected ? 'selected' : undefined}
							onclick={() => onRowClick(row)}
							class="cursor-pointer"
						>
							<Table.TableCell
								class="w-12"
								onclick={(e) => e.stopPropagation()}
							>
								<Checkbox
									checked={isSelected}
									onclick={(e) => {
										e.stopPropagation();
										onRowSelect(rowId, !isSelected);
									}}
								/>
							</Table.TableCell>
							{#each visibleColumns as column}
								<Table.TableCell>{formatValue(row[column])}</Table.TableCell>
							{/each}
						</Table.TableRow>
					{/each}
				{/if}
			</Table.TableBody>
		</Table.Table>
	{/if}
</div>

{#if hasData && data}
	<!-- Pagination -->
	<div class="flex items-center justify-between px-2 py-4">
		<div class="text-sm text-muted-foreground">
			Showing {data.page * data.pageSize - data.pageSize + 1} to{' '}
			{Math.min(data.page * data.pageSize, data.total)} of {data.total} results
		</div>
		<div class="flex items-center gap-2">
			<Button
				variant="outline"
				size="sm"
				disabled={data.page <= 1}
				onclick={() => onPageChange(data.page - 1)}
			>
				Previous
			</Button>
			<span class="text-sm">
				Page {data.page} of {data.totalPages}
			</span>
			<Button
				variant="outline"
				size="sm"
				disabled={data.page >= data.totalPages}
				onclick={() => onPageChange(data.page + 1)}
			>
				Next
			</Button>
		</div>
	</div>
{/if}
