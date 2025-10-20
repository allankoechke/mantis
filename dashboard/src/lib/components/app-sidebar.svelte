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
        url: "/tables",
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
  import { Label } from "$lib/components/ui/label/index.js";
  import { useSidebar } from "$lib/components/ui/sidebar/context.svelte.js";
  import * as Sidebar from "$lib/components/ui/sidebar/index.js";
  import CreateTableDrawer from "./create-table-drawer.svelte";
  import MantisLogo from "$lib/assets/mantis.png";
  import type { ComponentProps } from "svelte";
  let {
    ref = $bindable(null),
    ...restProps
  }: ComponentProps<typeof Sidebar.Root> = $props();
  let activeItem = $state(data.navMain[0]);
  let dbTables = $state(data.mails);

  const sidebar = useSidebar();

  // Toggle between light and dark mode
  import { toggleMode } from "mode-watcher";
  import { Button } from "$lib/components/ui/button/index.js";

  // Create table drawer state
  let openSheet = $state(false);
</script>

<Sidebar.Root
  bind:ref
  collapsible="icon"
  class="overflow-hidden [&>[data-sidebar=sidebar]]:flex-row"
  {...restProps}
>
  <!-- This is the first sidebar -->
  <!-- We disable collapsible and adjust width to icon. -->
  <!-- This will make the sidebar appear as icons. -->
  <Sidebar.Root
    collapsible="none"
    class="!w-[calc(var(--sidebar-width-icon)_+_1px)] border-r"
  >
    <Sidebar.Header>
      <Sidebar.Menu>
        <Sidebar.MenuItem>
          <Sidebar.MenuButton size="lg" class="md:h-8 md:p-0">
            {#snippet child({ props })}
              <a href="##" {...props}>
                <div
                  class="bg-sidebar-primary text-sidebar-primary-foreground flex aspect-square size-8 items-center justify-center rounded-lg"
                >
                  <img src={MantisLogo} alt="Mantis Logo" />
                </div>
                <div class="grid flex-1 text-left text-sm leading-tight">
                  <span class="truncate font-medium">Mantis Admin</span>
                  <span class="truncate text-xs"
                    >Database, REST APIs, Files, Auth</span
                  >
                </div>
              </a>
            {/snippet}
          </Sidebar.MenuButton>
        </Sidebar.MenuItem>
      </Sidebar.Menu>
    </Sidebar.Header>
    <Sidebar.Content>
      <Sidebar.Group>
        <Sidebar.GroupContent class="px-1.5 md:px-0">
          <Sidebar.Menu>
            {#each data.navMain as item (item.title)}
              <Sidebar.MenuItem>
                <Sidebar.MenuButton
                  tooltipContentProps={{
                    hidden: false,
                  }}
                  onclick={() => {
                    activeItem = item;
                    const mail = data.mails.sort(() => Math.random() - 0.5);
                    mails = mail.slice(
                      0,
                      Math.max(5, Math.floor(Math.random() * 10) + 1),
                    );
                    sidebar.setOpen(true);
                  }}
                  isActive={activeItem.title === item.title}
                  class="px-2.5 md:px-2"
                >
                  {#snippet tooltipContent()}
                    {item.title}
                  {/snippet}
                  <item.icon />
                  <span>{item.title}</span>
                </Sidebar.MenuButton>
              </Sidebar.MenuItem>
            {/each}
          </Sidebar.Menu>
        </Sidebar.GroupContent>
      </Sidebar.Group>
    </Sidebar.Content>
    <Sidebar.Footer>
      <Button onclick={toggleMode} variant="outline" size="icon">
        <SunIcon
          class="h-[1.2rem] w-[1.2rem] rotate-0 scale-100 !transition-all dark:-rotate-90 dark:scale-0"
        />
        <MoonIcon
          class="absolute h-[1.2rem] w-[1.2rem] rotate-90 scale-0 !transition-all dark:rotate-0 dark:scale-100"
        />
        <span class="sr-only">Toggle theme</span>
      </Button>
      <NavUser user={data.user} />
    </Sidebar.Footer>
  </Sidebar.Root>

  <!-- This is the second sidebar -->
  <!-- We disable collapsible and let it fill remaining space -->
  <Sidebar.Root collapsible="none" class="hidden flex-1 md:flex">
    <Sidebar.Header class="gap-3.5 border-b p-4">
      <div class="flex w-full items-center justify-between">
        <div class="text-foreground text-base font-medium">
          {activeItem.title}
        </div>
        
        <!-- Create table drawer -->
        <CreateTableDrawer />
      </div>
      <Sidebar.Input placeholder="Type to search..." />
    </Sidebar.Header>
    <Sidebar.Content>
      <Sidebar.Group class="px-0">
        <Sidebar.GroupContent>
          {#each dbTables as table (table.id)}
            <a
              href={`/tables/${table.name}`}
              class="hover:bg-sidebar-accent hover:text-sidebar-accent-foreground flex flex-col items-start gap-2 whitespace-nowrap border-b p-4 text-sm leading-tight last:border-b-0"
            >
              <div class="flex w-full items-center gap-2">
                <span>{table.name}</span>
                <span class="ml-auto text-xs">{table.type}</span>
              </div>
            </a>
          {/each}
        </Sidebar.GroupContent>
      </Sidebar.Group>
    </Sidebar.Content>
  </Sidebar.Root>
</Sidebar.Root>
