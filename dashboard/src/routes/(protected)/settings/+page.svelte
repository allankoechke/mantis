<script lang="ts">
	import { onMount } from 'svelte';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import * as Field from '$lib/components/ui/field/index.js';
	import { getSettings, updateSettings, type Settings } from '$lib/api/settings.js';

	let settings: Settings | null = $state(null);
	let loading = $state(false);
	let saving = $state(false);
	let error = $state<string | null>(null);
	let success = $state(false);

	let formData = $state({
		projectName: '',
		baseUrl: '',
		jwtValidityAdmins: 0,
		jwtValidityCollections: 0,
	});

	async function loadSettings() {
		loading = true;
		error = null;

		try {
			settings = await getSettings();
			formData = {
				projectName: settings.projectName || '',
				baseUrl: settings.baseUrl || '',
				jwtValidityAdmins: settings.jwtValidityAdmins || 0,
				jwtValidityCollections: settings.jwtValidityCollections || 0,
			};
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to load settings';
		} finally {
			loading = false;
		}
	}

	async function handleSave() {
		saving = true;
		error = null;
		success = false;

		try {
			await updateSettings(formData);
			success = true;
			await loadSettings();
			setTimeout(() => {
				success = false;
			}, 3000);
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to save settings';
		} finally {
			saving = false;
		}
	}

	onMount(() => {
		loadSettings();
	});
</script>

<div class="flex flex-col h-full">
	<header class="border-b p-4">
		<h1 class="text-2xl font-bold">Settings</h1>
	</header>

	<div class="flex-1 overflow-auto p-4">
		{#if loading}
			<div class="flex items-center justify-center p-8">
				<p class="text-muted-foreground">Loading settings...</p>
			</div>
		{:else if settings}
			<form onsubmit={(e) => { e.preventDefault(); handleSave(); }} class="max-w-2xl space-y-6">
				{#if error}
					<div class="bg-destructive/10 text-destructive p-4 rounded-lg">
						{error}
					</div>
				{/if}

				{#if success}
					<div class="bg-green-500/10 text-green-600 dark:text-green-400 p-4 rounded-lg">
						Settings saved successfully!
					</div>
				{/if}

				<Field.Field>
					<Field.Label for="projectName">Project Name</Field.Label>
					<Input
						id="projectName"
						bind:value={formData.projectName}
						placeholder="My Project"
						required
					/>
					<Field.Description>The name of your project</Field.Description>
				</Field.Field>

				<Field.Field>
					<Field.Label for="baseUrl">Base URL</Field.Label>
					<Input
						id="baseUrl"
						type="url"
						bind:value={formData.baseUrl}
						placeholder="https://api.example.com"
						required
					/>
					<Field.Description>The base URL for API calls</Field.Description>
				</Field.Field>

				<Field.Field>
					<Field.Label for="jwtValidityAdmins">JWT Validity Duration (Admins) - seconds</Field.Label>
					<Input
						id="jwtValidityAdmins"
						type="number"
						bind:value={formData.jwtValidityAdmins}
						min="0"
						required
					/>
					<Field.Description>
						How long admin JWT tokens remain valid (in seconds)
					</Field.Description>
				</Field.Field>

				<Field.Field>
					<Field.Label for="jwtValidityCollections">
						JWT Validity Duration (Collections) - seconds
					</Field.Label>
					<Input
						id="jwtValidityCollections"
						type="number"
						bind:value={formData.jwtValidityCollections}
						min="0"
						required
					/>
					<Field.Description>
						How long collection JWT tokens remain valid (in seconds)
					</Field.Description>
				</Field.Field>

				<div class="flex gap-2">
					<Button type="submit" disabled={saving}>
						{saving ? 'Saving...' : 'Save Settings'}
					</Button>
					<Button
						type="button"
						variant="outline"
						onclick={loadSettings}
						disabled={saving}
					>
						Reset
					</Button>
				</div>
			</form>
		{/if}
	</div>
</div>
