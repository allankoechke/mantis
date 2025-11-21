<script lang="ts">
	import { onMount } from 'svelte';
	import * as Table from '$lib/components/ui/table/index.js';
	import { Button } from '$lib/components/ui/button/index.js';
	import { Input } from '$lib/components/ui/input/index.js';
	import * as Dialog from '$lib/components/ui/dialog/index.js';
	import * as Field from '$lib/components/ui/field/index.js';
	import PlusIcon from '@lucide/svelte/icons/plus';
	import EditIcon from '@lucide/svelte/icons/edit';
	import TrashIcon from '@lucide/svelte/icons/trash';
	import {
		getAdmins,
		createAdmin,
		updateAdmin,
		deleteAdmin,
		type AdminUser,
		type AdminListResponse,
	} from '$lib/api/admins.js';

	let admins: AdminListResponse | null = $state(null);
	let loading = $state(false);
	let error = $state<string | null>(null);
	let currentPage = $state(1);
	const pageSize = 10;
	let dialogOpen = $state(false);
	let editingAdmin: AdminUser | null = $state(null);
	let formEmail = $state('');
	let formPassword = $state('');
	let formName = $state('');

	async function loadAdmins() {
		loading = true;
		error = null;

		try {
			admins = await getAdmins(currentPage, pageSize);
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to load admins';
		} finally {
			loading = false;
		}
	}

	function handleAdd() {
		editingAdmin = null;
		formEmail = '';
		formPassword = '';
		formName = '';
		dialogOpen = true;
	}

	function handleEdit(admin: AdminUser) {
		editingAdmin = admin;
		formEmail = admin.email;
		formPassword = '';
		formName = admin.name || '';
		dialogOpen = true;
	}

	async function handleDelete(id: string) {
		if (!confirm('Delete this admin user?')) return;

		try {
			await deleteAdmin(id);
			await loadAdmins();
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to delete admin';
		}
	}

	async function handleSave() {
		loading = true;
		error = null;

		try {
			if (editingAdmin) {
				await updateAdmin(editingAdmin.id, {
					email: formEmail,
					password: formPassword || undefined,
					name: formName || undefined,
				});
			} else {
				if (!formPassword) {
					error = 'Password is required for new admins';
					return;
				}
				await createAdmin({
					email: formEmail,
					password: formPassword,
					name: formName || undefined,
				});
			}
			dialogOpen = false;
			await loadAdmins();
		} catch (err) {
			error = err instanceof Error ? err.message : 'Failed to save admin';
		} finally {
			loading = false;
		}
	}

	onMount(() => {
		loadAdmins();
	});
</script>

<div class="flex flex-col h-full">
	<header class="border-b p-4">
		<div class="flex items-center justify-between">
			<h1 class="text-2xl font-bold">Admin Users</h1>
			<Button onclick={handleAdd}>
				<PlusIcon class="mr-2 size-4" />
				Add Admin
			</Button>
		</div>
	</header>

	<div class="flex-1 overflow-auto p-4">
		{#if error}
			<div class="bg-destructive/10 text-destructive p-4 rounded-lg">
				{error}
			</div>
		{:else if loading && !admins}
			<div class="flex items-center justify-center p-8">
				<p class="text-muted-foreground">Loading...</p>
			</div>
		{:else if admins}
			<div class="rounded-md border">
				<Table.Table>
					<Table.TableHeader>
						<Table.TableRow>
							<Table.TableHead>Email</Table.TableHead>
							<Table.TableHead>Name</Table.TableHead>
							<Table.TableHead>Created</Table.TableHead>
							<Table.TableHead>Actions</Table.TableHead>
						</Table.TableRow>
					</Table.TableHeader>
					<Table.TableBody>
						{#each admins.admins as admin (admin.id)}
							<Table.TableRow>
								<Table.TableCell>{admin.email}</Table.TableCell>
								<Table.TableCell>{admin.name || '-'}</Table.TableCell>
								<Table.TableCell>
									{admin.createdAt
										? new Date(admin.createdAt).toLocaleDateString()
										: '-'}
								</Table.TableCell>
								<Table.TableCell>
									<div class="flex gap-2">
										<Button
											variant="ghost"
											size="sm"
											onclick={() => handleEdit(admin)}
										>
											<EditIcon class="size-4" />
										</Button>
										<Button
											variant="ghost"
											size="sm"
											onclick={() => handleDelete(admin.id)}
										>
											<TrashIcon class="size-4" />
										</Button>
									</div>
								</Table.TableCell>
							</Table.TableRow>
						{/each}
					</Table.TableBody>
				</Table.Table>
			</div>

			<!-- Pagination -->
			<div class="flex items-center justify-between mt-4">
				<div class="text-sm text-muted-foreground">
					Showing {admins.page * admins.pageSize - admins.pageSize + 1} to{' '}
					{Math.min(admins.page * admins.pageSize, admins.total)} of {admins.total}{' '}
					results
				</div>
				<div class="flex items-center gap-2">
					<Button
						variant="outline"
						size="sm"
						disabled={admins.page <= 1}
						onclick={() => {
							currentPage--;
							loadAdmins();
						}}
					>
						Previous
					</Button>
					<span class="text-sm">
						Page {admins.page} of {admins.totalPages}
					</span>
					<Button
						variant="outline"
						size="sm"
						disabled={admins.page >= admins.totalPages}
						onclick={() => {
							currentPage++;
							loadAdmins();
						}}
					>
						Next
					</Button>
				</div>
			</div>
		{/if}
	</div>
</div>

<Dialog.Root bind:open={dialogOpen}>
	<Dialog.DialogContent>
		<Dialog.DialogHeader>
			<Dialog.DialogTitle>
				{editingAdmin ? 'Edit Admin' : 'Create Admin'}
			</Dialog.DialogTitle>
			<Dialog.DialogDescription>
				{editingAdmin
					? 'Update admin user information'
					: 'Create a new admin user account'}
			</Dialog.DialogDescription>
		</Dialog.DialogHeader>

		<form onsubmit={(e) => { e.preventDefault(); handleSave(); }} class="space-y-4">
			{#if error}
				<div class="bg-destructive/10 text-destructive p-4 rounded-lg">
					{error}
				</div>
			{/if}

			<Field.Field>
				<Field.Label for="email">Email</Field.Label>
				<Input
					id="email"
					type="email"
					bind:value={formEmail}
					required
					placeholder="admin@example.com"
				/>
			</Field.Field>

			<Field.Field>
				<Field.Label for="password">
					Password {editingAdmin ? '(leave empty to keep current)' : '*'}
				</Field.Label>
				<Input
					id="password"
					type="password"
					bind:value={formPassword}
					required={!editingAdmin}
					placeholder="••••••••"
				/>
			</Field.Field>

			<Field.Field>
				<Field.Label for="name">Name (optional)</Field.Label>
				<Input id="name" bind:value={formName} placeholder="Admin Name" />
			</Field.Field>

			<Dialog.DialogFooter>
				<Button type="button" variant="outline" onclick={() => (dialogOpen = false)}>
					Cancel
				</Button>
				<Button type="submit" disabled={loading}>
					{loading ? 'Saving...' : editingAdmin ? 'Update' : 'Create'}
				</Button>
			</Dialog.DialogFooter>
		</form>
	</Dialog.DialogContent>
</Dialog.Root>
