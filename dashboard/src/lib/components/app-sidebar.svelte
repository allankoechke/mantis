<script lang="ts" module>
  import DatabaseIcon from "@lucide/svelte/icons/database";
  import LogsIcon from "@lucide/svelte/icons/logs";
  import Settings2Icon from "@lucide/svelte/icons/settings-2";
  import BadgeCheckIcon from "@lucide/svelte/icons/badge-check";
  import SunIcon from "@lucide/svelte/icons/sun";
  import MoonIcon from "@lucide/svelte/icons/moon";
  import ListPlusIcon from "@lucide/svelte/icons/list-plus";

  // This is sample data
  const data = {
    user: {
      name: "shadcn",
      email: "m@example.com",
      avatar: "/avatars/01.png",
    },
    navMain: [
      {
        title: "Database Tables",
        url: "/entities",
        icon: DatabaseIcon,
        isActive: true,
      },
      {
        title: "Admin Users",
        url: "/admins",
        icon: BadgeCheckIcon,
        isActive: false,
      },
      {
        title: "Logs",
        url: "/logs",
        icon: LogsIcon,
        isActive: false,
      },
      {
        title: "Settings",
        url: "/settings",
        icon: Settings2Icon,
        isActive: false,
      },
    ],
    mails: [
      {
        name: "schools",
        type: "base",
        id: "123",
        created: "2025-01-01T00:00:00Z",
        fields: [
          { name: "name", type: "string" },
          { name: "address", type: "string" },
          { name: "phone", type: "string" },
        ],
      },
      {
        name: "students",
        type: "auth",
        id: "1234",
        created: "2025-01-01T00:00:00Z",
        fields: [
          { name: "name", type: "string" },
          { name: "address", type: "string" },
          { name: "phone", type: "string" },
        ],
      },
      {
        name: "classes",
        type: "base",
        id: "1235",
        created: "2025-01-01T00:00:00Z",
        fields: [
          { name: "name", type: "string" },
          { name: "address", type: "string" },
          { name: "phone", type: "string" },
        ],
      },
      {
        name: "courses",
        type: "view",
        id: "1236",
        created: "2025-01-01T00:00:00Z",
        fields: [
          { name: "name", type: "string" },
          { name: "address", type: "string" },
          { name: "phone", type: "string" },
        ],
      },
    ],
  };
</script>

<script lang="ts">
  import NavUser from "$lib/components/nav-user.svelte";
  import MantisLogo from "$lib/assets/mantis.png";
  import { page } from '$app/stores';
  import { goto } from '$app/navigation';
  import { authStore } from '$lib/stores/authStore.js';
  import { toggleMode } from "mode-watcher";
  import { Button } from "$lib/components/ui/button/index.js";
  
  let currentPath = $derived($page.url.pathname);
  let activeItem = $derived(data.navMain.find(item => currentPath.startsWith(item.url)) || data.navMain[0]);

  let user = $derived($authStore.user || data.user);
</script>

<div class="w-20 border-r bg-sidebar flex flex-col h-full flex-shrink-0">
  <!-- Header -->
  <div class="p-3 border-b flex-shrink-0">
    <a href="/entities" class="flex items-center justify-center">
      <div
        class="bg-sidebar-primary text-sidebar-primary-foreground flex aspect-square size-12 items-center justify-center rounded-lg"
      >
        <img src={MantisLogo} alt="Mantis Logo" class="size-8" />
      </div>
    </a>
  </div>

  <!-- Navigation -->
  <div class="flex-1 overflow-y-auto py-2 min-h-0">
    <div class="flex flex-col items-center gap-2 px-2">
      {#each data.navMain as item (item.title)}
        {@const isActive = currentPath.startsWith(item.url)}
        <button
          onclick={() => goto(item.url)}
          class="flex flex-col items-center justify-center w-14 h-14 rounded-lg hover:bg-sidebar-accent hover:text-sidebar-accent-foreground transition-colors {isActive ? 'bg-sidebar-accent text-sidebar-accent-foreground' : ''}"
          title={item.title}
        >
          <item.icon class="size-6" />
        </button>
      {/each}
    </div>
  </div>

  <!-- Footer -->
  <div class="border-t p-2 space-y-2 flex-shrink-0">
    <Button 
      onclick={toggleMode} 
      variant="ghost" 
      size="icon"
      class="w-full h-12 flex flex-col items-center justify-center"
    >
      <SunIcon
        class="h-5 w-5 rotate-0 scale-100 !transition-all dark:-rotate-90 dark:scale-0"
      />
      <MoonIcon
        class="absolute h-5 w-5 rotate-90 scale-0 !transition-all dark:rotate-0 dark:scale-100"
      />
      <span class="sr-only">Toggle theme</span>
    </Button>
    <div class="flex justify-center">
      <NavUser user={user} />
    </div>
  </div>
</div>
