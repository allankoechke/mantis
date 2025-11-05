<script lang="ts">
    import AppSidebar from "$lib/components/tables-sidebar.svelte";
    import { page } from "$app/state";
    import { derived } from "svelte/store";

    let { children } = $props();
</script>

<div class="flex h-screen">
    <AppSidebar />
    {@render children?.()}
</div>
