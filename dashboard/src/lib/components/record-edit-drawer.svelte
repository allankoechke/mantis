<script lang="ts">
	import * as Sheet from '$lib/components/ui/sheet/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import * as Field from '$lib/components/ui/field/index.js';
	import {
		createRecord,
		updateRecord,
		type Entity,
	} from '$lib/api/entities.js';

	interface Props {
		entity: Entity;
		record: Record<string, unknown>;
		open: boolean;
		onOpenChange: (open: boolean) => void;
		onSaved?: () => void;
	}

	let { entity, record, open, onOpenChange, onSaved }: Props = $props();

	let loading = $state(false);
	let error = $state<string | null>(null);
	let formData = $state<Record<string, unknown>>({});

	let isEdit = $derived(record && 'id' in record && record.id);
	
	$effect(() => {
		if (open && record) {
			formData = { ...record };
		}
	});

	async function handleSubmit(e: SubmitEvent) {
		e.preventDefault();
		if (!entity) return;

		loading = true;
		error = null;

		try {
			if (isEdit && record.id) {
				await updateRecord(entity.name, String(record.id), formData);
			} else {
				await createRecord(entity.name, formData);
			}
			onSaved?.();
			onOpenChange(false);
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to save record';
		} finally {
			loading = false;
		}
	}

	function getInputType(fieldType: string): string {
		switch (fieldType.toLowerCase()) {
			case 'number':
			case 'integer':
			case 'float':
				return 'number';
			case 'email':
				return 'email';
			case 'date':
				return 'date';
			case 'datetime':
				return 'datetime-local';
			case 'boolean':
				return 'checkbox';
			default:
				return 'text';
		}
	}
</script>

<Sheet.Root bind:open={open} onOpenChange={onOpenChange}>
	<Sheet.Content side="right" class="w-full sm:max-w-2xl">
		<Sheet.Header>
			<Sheet.Title>{isEdit ? 'Edit' : 'Create'} Record</Sheet.Title>
			<Sheet.Description>
				{isEdit ? 'Update' : 'Add'} a record to {entity.name}
			</Sheet.Description>
		</Sheet.Header>

		<form onsubmit={handleSubmit} class="mt-4 space-y-4">
			{#if error}
				<div class="bg-destructive/10 text-destructive p-4 rounded-lg">
					{error}
				</div>
			{/if}

			{#if entity.fields}
				{#each entity.fields as field}
					{@const fieldValue = formData[field.name] ?? field.default ?? ''}
					{@const inputType = getInputType(field.type)}
					<Field.Field>
						<Field.Label for={field.name}>
							{field.name}
							{#if field.required}
								<span class="text-destructive">*</span>
							{/if}
						</Field.Label>
						{#if inputType === 'checkbox'}
							<input
								type="checkbox"
								id={field.name}
								bind:checked={formData[field.name]}
								required={field.required}
							/>
						{:else}
							<Input
								id={field.name}
								type={inputType}
								bind:value={formData[field.name]}
								required={field.required}
								placeholder={field.name}
							/>
						{/if}
					</Field.Field>
				{/each}
			{/if}

			<Sheet.Footer>
				<Button type="button" variant="outline" onclick={() => onOpenChange(false)}>
					Cancel
				</Button>
				<Button type="submit" disabled={loading}>
					{loading ? 'Saving...' : isEdit ? 'Update' : 'Create'}
				</Button>
			</Sheet.Footer>
		</form>
	</Sheet.Content>
</Sheet.Root>
