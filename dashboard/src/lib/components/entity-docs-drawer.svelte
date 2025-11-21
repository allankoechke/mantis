<script lang="ts">
	import * as Sheet from '$lib/components/ui/sheet/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import type { Entity } from '$lib/api/entities.js';

	interface Props {
		entity: Entity;
		open: boolean;
		onOpenChange: (open: boolean) => void;
	}

	let { entity, open, onOpenChange }: Props = $props();
</script>

<Sheet.Root bind:open={open} onOpenChange={onOpenChange}>
	<Sheet.Content side="right" class="w-full sm:max-w-2xl">
		<Sheet.Header>
			<Sheet.Title>Entity Documentation: {entity.name}</Sheet.Title>
			<Sheet.Description>
				Documentation and information about this entity
			</Sheet.Description>
		</Sheet.Header>

		<div class="mt-4 space-y-4">
			<div>
				<h3 class="text-lg font-semibold mb-2">Entity Information</h3>
				<div class="space-y-2">
					<p><strong>Name:</strong> {entity.name}</p>
					<p><strong>Type:</strong> {entity.type}</p>
					{#if entity.created}
						<p><strong>Created:</strong> {new Date(entity.created).toLocaleString()}</p>
					{/if}
				</div>
			</div>

			{#if entity.fields && entity.fields.length > 0}
				<div>
					<h3 class="text-lg font-semibold mb-2">Fields</h3>
					<div class="space-y-2">
						{#each entity.fields as field}
							<div class="border rounded-lg p-3">
								<p><strong>{field.name}</strong> ({field.type})</p>
								{#if field.required}
									<p class="text-sm text-muted-foreground">Required</p>
								{/if}
							</div>
						{/each}
					</div>
				</div>
			{/if}

			<div>
				<h3 class="text-lg font-semibold mb-2">API Endpoints</h3>
				<div class="space-y-2 font-mono text-sm">
					<p>GET /admin/entities/{entity.name}</p>
					<p>GET /admin/entities/{entity.name}/data</p>
					<p>POST /admin/entities/{entity.name}/records</p>
					<p>PUT /admin/entities/{entity.name}/records/:id</p>
					<p>DELETE /admin/entities/{entity.name}/records</p>
				</div>
			</div>
		</div>

		<Sheet.Footer>
			<Button variant="outline" onclick={() => onOpenChange(false)}>
				Close
			</Button>
		</Sheet.Footer>
	</Sheet.Content>
</Sheet.Root>
