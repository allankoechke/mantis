<script lang="ts">
    import * as Sheet from "$lib/components/ui/sheet/index.js";
    import { buttonVariants } from "$lib/components/ui/button/index.js";
    import { Input } from "$lib/components/ui/input/index.js";
    import { Label } from "$lib/components/ui/label/index.js";
    import ListPlusIcon from "@lucide/svelte/icons/list-plus";

    const handleSave = () => {
        // Handle save logic here
    };
</script>

<Sheet.Root>
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
        <div class="grid flex-1 auto-rows-min gap-6 px-4">
            <div class="grid gap-3">
                <Label for="name" class="text-right">Name</Label>
                <Input id="name" value="Pedro Duarte" />
            </div>
            <div class="grid gap-3">
                <Label for="username" class="text-right">Username</Label>
                <Input id="username" value="@peduarte" />
            </div>
        </div>
        <Sheet.Footer>
            <button
                class={buttonVariants({ variant: "default" })}
                on:click={handleSave}
            >
                Save Table
            </button>
        </Sheet.Footer>
    </Sheet.Content>
</Sheet.Root>
