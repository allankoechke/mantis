<script lang="ts" module>
  // This is sample data
  const data = {
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
  import { useSidebar } from "$lib/components/ui/sidebar/context.svelte.js";
  import * as Sidebar from "$lib/components/ui/sidebar/index.js";
  import CreateTableDrawer from "./create-table-drawer.svelte";
  import type { ComponentProps } from "svelte";
  let {
    ref = $bindable(null),
    ...restProps
  }: ComponentProps<typeof Sidebar.Root> = $props();
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
  <!-- This is the second sidebar -->
  <!-- We disable collapsible and let it fill remaining space -->
  <Sidebar.Root collapsible="none" class="hidden flex-1 md:flex">
    <Sidebar.Header class="gap-3.5 border-b p-4">
      <div class="flex w-full items-center justify-between">
        <div class="text-foreground text-base font-medium">
          Database Tables
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
