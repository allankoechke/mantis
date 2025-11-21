<script lang="ts">
	import * as Sheet from "$lib/components/ui/sheet/index.js";
	import { buttonVariants } from "$lib/components/ui/button/index.js";
	import { Button } from "$lib/components/ui/button/index.js";
	import { Input } from "$lib/components/ui/input/index.js";
	import { Label } from "$lib/components/ui/label/index.js";
	import * as Field from "$lib/components/ui/field/index.js";
	import ListPlusIcon from "@lucide/svelte/icons/list-plus";
	import { createEntity, type CreateEntityRequest } from "$lib/api/entities.js";
	import { entityStore } from "$lib/stores/entityStore.js";

	let open = $state(false);
	let loading = $state(false);
	let error = $state<string | null>(null);
	let tableName = $state("");
	let tableType = $state<"base" | "auth" | "view">("base");
	let fields = $state<Array<{ name: string; type: string; required: boolean }>>([
		{ name: "id", type: "string", required: true },
	]);

	function addField() {
		fields = [...fields, { name: "", type: "string", required: false }];
	}

	function removeField(index: number) {
		fields = fields.filter((_, i) => i !== index);
	}

	async function handleSave() {
		if (!tableName.trim()) {
			error = "Table name is required";
			return;
		}

		loading = true;
		error = null;

		try {
			const request: CreateEntityRequest = {
				name: tableName,
				type: tableType,
				fields: fields.map((f) => ({
					name: f.name,
					type: f.type,
					required: f.required,
				})),
			};

			const entity = await createEntity(request);
			entityStore.addEntity(entity);
			open = false;
			tableName = "";
			tableType = "base";
			fields = [{ name: "id", type: "string", required: true }];
		} catch (err) {
			error = err instanceof Error ? err.message : "Failed to create table";
		} finally {
			loading = false;
		}
	}
</script>

<Sheet.Root bind:open>
	<Sheet.Trigger class={buttonVariants({ variant: "outline", size: "sm" })}>
		<ListPlusIcon />
		New Table
	</Sheet.Trigger>
	<Sheet.Content side="right">
		<Sheet.Header>
			<Sheet.Title>Create Table</Sheet.Title>
			<Sheet.Description>
				Define database table information and schema. Click save when
				you're done.
			</Sheet.Description>
		</Sheet.Header>
		<form onsubmit={(e) => { e.preventDefault(); handleSave(); }} class="mt-4 space-y-4">
			{#if error}
				<div class="bg-destructive/10 text-destructive p-4 rounded-lg">
					{error}
				</div>
			{/if}

			<Field.Field>
				<Field.Label for="name">Table Name</Field.Label>
				<Input id="name" bind:value={tableName} placeholder="my_table" required />
			</Field.Field>

			<Field.Field>
				<Field.Label for="type">Table Type</Field.Label>
				<select id="type" bind:value={tableType} class="w-full p-2 border rounded">
					<option value="base">Base</option>
					<option value="auth">Auth</option>
					<option value="view">View</option>
				</select>
			</Field.Field>

			<div class="space-y-4">
				<div class="flex justify-between items-center">
					<Label>Fields</Label>
					<Button type="button" size="sm" onclick={addField}>Add Field</Button>
				</div>

				{#each fields as field, index (index)}
					<div class="border rounded-lg p-4 space-y-2">
						<div class="grid grid-cols-2 gap-4">
							<Field.Field>
								<Field.Label>Name</Field.Label>
								<Input bind:value={field.name} placeholder="field_name" />
							</Field.Field>
							<Field.Field>
								<Field.Label>Type</Field.Label>
								<Input bind:value={field.type} placeholder="string" />
							</Field.Field>
						</div>
						<div class="flex items-center justify-between">
							<Label>
								<input type="checkbox" bind:checked={field.required} class="mr-2" />
								Required
							</Label>
							<Button
								type="button"
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

			<Sheet.Footer>
				<Button type="button" variant="outline" onclick={() => (open = false)}>
					Cancel
				</Button>
				<Button type="submit" disabled={loading}>
					{loading ? "Creating..." : "Save Table"}
				</Button>
			</Sheet.Footer>
		</form>
	</Sheet.Content>
</Sheet.Root>
