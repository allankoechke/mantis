<script lang="ts">
	import * as Sheet from '$lib/components/ui/sheet/index.js';
	import * as Tabs from '$lib/components/ui/tabs/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import { Label } from '$lib/components/ui/label/index.js';
	import * as Table from '$lib/components/ui/table/index.js';
	import * as Field from '$lib/components/ui/field/index.js';
	import {
		getEntityConfig,
		updateEntityConfig,
		type Entity,
		type EntityConfig,
		type AccessRule,
	} from '$lib/api/entities.js';

	interface Props {
		entity: Entity;
		open: boolean;
		onOpenChange: (open: boolean) => void;
		onSaved?: () => void;
	}

	let { entity, open, onOpenChange, onSaved }: Props = $props();

	let loading = $state(false);
	let error = $state<string | null>(null);
	let config: EntityConfig | null = $state(null);
	let activeTab = $state('schema');

	async function loadConfig() {
		loading = true;
		error = null;

		try {
			config = await getEntityConfig(entity.name);
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to load config';
		} finally {
			loading = false;
		}
	}

	async function handleSave() {
		if (!config) return;

		loading = true;
		error = null;

		try {
			await updateEntityConfig(entity.name, config);
			onSaved?.();
			onOpenChange(false);
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to save config';
		} finally {
			loading = false;
		}
	}

	function addField() {
		if (!config) return;
		config.schema.fields.push({
			name: '',
			type: 'string',
			required: false,
		});
		config = { ...config };
	}

	function removeField(index: number) {
		if (!config) return;
		config.schema.fields.splice(index, 1);
		config = { ...config };
	}

	function addAccessRule() {
		if (!config) return;
		config.accessRules.push({
			permission: 'read',
		});
		config = { ...config };
	}

	function removeAccessRule(index: number) {
		if (!config) return;
		config.accessRules.splice(index, 1);
		config = { ...config };
	}

	$effect(() => {
		if (open && !config) {
			loadConfig();
		}
	});
</script>

<Sheet.Root bind:open={open} onOpenChange={onOpenChange}>
	<Sheet.Content side="right" class="w-full sm:max-w-2xl">
		<Sheet.Header>
			<Sheet.Title>Entity Configuration: {entity.name}</Sheet.Title>
			<Sheet.Description>
				Manage schema and access rules for this entity
			</Sheet.Description>
		</Sheet.Header>

		{#if loading && !config}
			<div class="p-4">Loading...</div>
		{:else if error}
			<div class="bg-destructive/10 text-destructive p-4 rounded-lg m-4">
				{error}
			</div>
		{:else if config}
			<Tabs.Root bind:value={activeTab} class="mt-4">
				<Tabs.TabsList class="grid w-full grid-cols-2">
					<Tabs.TabsTrigger value="schema">Schema</Tabs.TabsTrigger>
					<Tabs.TabsTrigger value="access">Access Rules</Tabs.TabsTrigger>
				</Tabs.TabsList>

				<Tabs.TabsContent value="schema" class="mt-4">
					<div class="space-y-4">
						<div class="flex justify-between items-center">
							<h3 class="text-lg font-semibold">Fields</h3>
							<Button size="sm" onclick={addField}>Add Field</Button>
						</div>

						<div class="space-y-4">
							{#each config.schema.fields as field, index (index)}
								<div class="border rounded-lg p-4 space-y-2">
									<div class="grid grid-cols-2 gap-4">
										<Field.Field>
											<Field.Label>Name</Field.Label>
											<Input
												bind:value={field.name}
												placeholder="field_name"
											/>
										</Field.Field>
										<Field.Field>
											<Field.Label>Type</Field.Label>
											<Input
												bind:value={field.type}
												placeholder="string"
											/>
										</Field.Field>
									</div>
									<div class="flex items-center justify-between">
										<Label>
											<input
												type="checkbox"
												bind:checked={field.required}
												class="mr-2"
											/>
											Required
										</Label>
										<Button
											variant="destructive"
											size="sm"
											onclick={() => removeField(index)}
										>
											Remove
										</Button>
									</div>
								</div>
							{/each}
						</div>
					</div>
				</Tabs.TabsContent>

				<Tabs.TabsContent value="access" class="mt-4">
					<div class="space-y-4">
						<div class="flex justify-between items-center">
							<h3 class="text-lg font-semibold">Access Rules</h3>
							<Button size="sm" onclick={addAccessRule}>Add Rule</Button>
						</div>

						<div class="border rounded-lg overflow-hidden">
							<Table.Table>
								<Table.TableHeader>
									<Table.TableRow>
										<Table.TableHead>Role</Table.TableHead>
										<Table.TableHead>Permission</Table.TableHead>
										<Table.TableHead>Actions</Table.TableHead>
									</Table.TableRow>
								</Table.TableHeader>
								<Table.TableBody>
									{#each config.accessRules as rule, index (index)}
										<Table.TableRow>
											<Table.TableCell>
												<Input
													bind:value={rule.role}
													placeholder="role or *"
												/>
											</Table.TableCell>
											<Table.TableCell>
												<select bind:value={rule.permission} class="w-full p-2 border rounded">
													<option value="read">Read</option>
													<option value="write">Write</option>
													<option value="delete">Delete</option>
													<option value="admin">Admin</option>
												</select>
											</Table.TableCell>
											<Table.TableCell>
												<Button
													variant="destructive"
													size="sm"
													onclick={() => removeAccessRule(index)}
												>
													Remove
												</Button>
											</Table.TableCell>
										</Table.TableRow>
									{/each}
								</Table.TableBody>
							</Table.Table>
						</div>
					</div>
				</Tabs.TabsContent>
			</Tabs.Root>
		{/if}

		<Sheet.Footer>
			<Button variant="outline" onclick={() => onOpenChange(false)}>
				Cancel
			</Button>
			<Button onclick={handleSave} disabled={loading || !config}>
				{loading ? 'Saving...' : 'Save'}
			</Button>
		</Sheet.Footer>
	</Sheet.Content>
</Sheet.Root>
