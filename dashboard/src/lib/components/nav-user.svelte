<script lang="ts">
  import BadgeCheckIcon from "@lucide/svelte/icons/badge-check";
  import ChevronsUpDownIcon from "@lucide/svelte/icons/chevrons-up-down";
  import LogOutIcon from "@lucide/svelte/icons/log-out";
  import KeyIcon from "@lucide/svelte/icons/key";
  import * as Avatar from "$lib/components/ui/avatar/index.js";
  import * as DropdownMenu from "$lib/components/ui/dropdown-menu/index.js";
  import { goto } from '$app/navigation';
  import { authStore } from '$lib/stores/authStore.js';
  import { logout } from '$lib/api/auth.js';
  
  let { user }: { user: { name: string; email: string; avatar: string } } = $props();

  function handleSignOut() {
    logout();
    authStore.clearAuth();
    goto('/login');
  }

  function handleChangePassword() {
    // TODO: Open change password dialog
    console.log('Change password');
  }
</script>

<DropdownMenu.Root>
  <DropdownMenu.Trigger>
    <button class="flex flex-col items-center justify-center w-full p-2 rounded-lg hover:bg-sidebar-accent hover:text-sidebar-accent-foreground transition-colors">
      <Avatar.Root class="size-10 rounded-lg">
        <Avatar.Image src={user.avatar} alt={user.name} />
        <Avatar.Fallback class="rounded-lg text-xs">
          {user.name?.charAt(0)?.toUpperCase() || user.email?.charAt(0)?.toUpperCase() || 'U'}
        </Avatar.Fallback>
      </Avatar.Root>
    </button>
  </DropdownMenu.Trigger>
  <DropdownMenu.Content
    class="min-w-56 rounded-lg"
    side="right"
    align="end"
    sideOffset={4}
  >
    <DropdownMenu.Label class="p-0 font-normal">
      <div class="flex items-center gap-2 px-1 py-1.5 text-left text-sm">
        <Avatar.Root class="size-8 rounded-lg">
          <Avatar.Image src={user.avatar} alt={user.name} />
          <Avatar.Fallback class="rounded-lg">
            {user.name?.charAt(0)?.toUpperCase() || user.email?.charAt(0)?.toUpperCase() || 'U'}
          </Avatar.Fallback>
        </Avatar.Root>
        <div class="grid flex-1 text-left text-sm leading-tight">
          <span class="truncate font-medium">{user.name}</span>
          <span class="truncate text-xs">{user.email}</span>
        </div>
      </div>
    </DropdownMenu.Label>
    <DropdownMenu.Separator />
    <DropdownMenu.Group>
      <DropdownMenu.Item href="/admins">
        <BadgeCheckIcon />
        Admin Accounts
      </DropdownMenu.Item>
      <DropdownMenu.Item onclick={handleChangePassword}>
        <KeyIcon />
        Change Password
      </DropdownMenu.Item>
    </DropdownMenu.Group>
    <DropdownMenu.Separator />
    <DropdownMenu.Item onclick={handleSignOut}>
      <LogOutIcon />
      Log out
    </DropdownMenu.Item>
  </DropdownMenu.Content>
</DropdownMenu.Root>