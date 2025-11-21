<script lang="ts">
	import { onMount } from 'svelte';
	import * as Table from '$lib/components/ui/table/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import { getLogs, type LogListResponse } from '$lib/api/logs.js';

	let logs: LogListResponse | null = $state(null);
	let loading = $state(false);
	let error = $state<string | null>(null);
	let currentPage = $state(1);
	const pageSize = 20;
	let searchQuery = $state('');

	async function loadLogs() {
		loading = true;
		error = null;

		try {
			logs = await getLogs(currentPage, pageSize, {
				search: searchQuery || undefined,
			});
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to load logs';
		} finally {
			loading = false;
		}
	}

	function handleSearch() {
		currentPage = 1;
		loadLogs();
	}

	onMount(() => {
		loadLogs();
	});
</script>

<div class="flex flex-col h-full">
	<header class="border-b p-4">
		<div class="flex items-center justify-between mb-4">
			<h1 class="text-2xl font-bold">Logs</h1>
		</div>
		<div class="flex gap-2">
			<Input
				placeholder="Search logs..."
				bind:value={searchQuery}
				onkeydown={(e) => {
					if (e.key === 'Enter') handleSearch();
				}}
				class="max-w-sm"
			/>
			<Button onclick={handleSearch}>Search</Button>
		</div>
	</header>

	<div class="flex-1 overflow-auto p-4">
		{#if error}
			<div class="bg-destructive/10 text-destructive p-4 rounded-lg">
				{error}
			</div>
		{:else if loading && !logs}
			<div class="flex items-center justify-center p-8">
				<p class="text-muted-foreground">Loading...</p>
			</div>
		{:else if logs}
			<div class="rounded-md border">
				<Table.Table>
					<Table.TableHeader>
						<Table.TableRow>
							<Table.TableHead>Timestamp</Table.TableHead>
							<Table.TableHead>Level</Table.TableHead>
							<Table.TableHead>Message</Table.TableHead>
							<Table.TableHead>Source</Table.TableHead>
						</Table.TableRow>
					</Table.TableHeader>
					<Table.TableBody>
						{#each logs.logs as log (log.id)}
							<Table.TableRow>
								<Table.TableCell>
									{new Date(log.timestamp).toLocaleString()}
								</Table.TableCell>
								<Table.TableCell>
									<span
										class="px-2 py-1 rounded text-xs font-medium {log.level === 'error'
											? 'bg-red-100 text-red-800 dark:bg-red-900 dark:text-red-200'
											: log.level === 'warn'
												? 'bg-yellow-100 text-yellow-800 dark:bg-yellow-900 dark:text-yellow-200'
												: log.level === 'info'
													? 'bg-blue-100 text-blue-800 dark:bg-blue-900 dark:text-blue-200'
													: 'bg-gray-100 text-gray-800 dark:bg-gray-900 dark:text-gray-200'}"
									>
										{log.level}
									</span>
								</Table.TableCell>
								<Table.TableCell class="max-w-md truncate">
									{log.message}
								</Table.TableCell>
								<Table.TableCell>{log.source || '-'}</Table.TableCell>
							</Table.TableRow>
						{/each}
					</Table.TableBody>
				</Table.Table>
			</div>

			<!-- Pagination -->
			<div class="flex items-center justify-between mt-4">
				<div class="text-sm text-muted-foreground">
					Showing {logs.page * logs.pageSize - logs.pageSize + 1} to{' '}
					{Math.min(logs.page * logs.pageSize, logs.total)} of {logs.total} results
				</div>
				<div class="flex items-center gap-2">
					<Button
						variant="outline"
						size="sm"
						disabled={logs.page <= 1}
						onclick={() => {
							currentPage--;
							loadLogs();
						}}
					>
						Previous
					</Button>
					<span class="text-sm">
						Page {logs.page} of {logs.totalPages}
					</span>
					<Button
						variant="outline"
						size="sm"
						disabled={logs.page >= logs.totalPages}
						onclick={() => {
							currentPage++;
							loadLogs();
						}}
					>
						Next
					</Button>
				</div>
			</div>
		{/if}
	</div>
</div>
