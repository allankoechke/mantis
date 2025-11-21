import { writable } from 'svelte/store';
import type { Entity } from '../api/entities.js';

interface EntityState {
	entities: Entity[];
	currentEntity: Entity | null;
	loading: boolean;
	error: string | null;
}

function createEntityStore() {
	const { subscribe, set, update } = writable<EntityState>({
		entities: [],
		currentEntity: null,
		loading: false,
		error: null,
	});

	return {
		subscribe,
		setEntities: (entities: Entity[]) => {
			update((state) => ({
				...state,
				entities,
				error: null,
			}));
		},
		setCurrentEntity: (entity: Entity | null) => {
			update((state) => ({
				...state,
				currentEntity: entity,
			}));
		},
		setLoading: (loading: boolean) => {
			update((state) => ({
				...state,
				loading,
			}));
		},
		setError: (error: string | null) => {
			update((state) => ({
				...state,
				error,
			}));
		},
		addEntity: (entity: Entity) => {
			update((state) => ({
				...state,
				entities: [...state.entities, entity],
			}));
		},
		updateEntity: (name: string, entity: Entity) => {
			update((state) => ({
				...state,
				entities: state.entities.map((e) => (e.name === name ? entity : e)),
				currentEntity:
					state.currentEntity?.name === name ? entity : state.currentEntity,
			}));
		},
		removeEntity: (name: string) => {
			update((state) => ({
				...state,
				entities: state.entities.filter((e) => e.name !== name),
				currentEntity:
					state.currentEntity?.name === name ? null : state.currentEntity,
			}));
		},
	};
}

export const entityStore = createEntityStore();
