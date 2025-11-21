import { apiClient } from './client.js';
import type { ApiError } from './client.js';
import { USE_MOCK_DATA, getMockLogs } from './mock-data.js';

export interface LogEntry {
	id: string;
	timestamp: string;
	level: 'info' | 'warn' | 'error' | 'debug';
	message: string;
	source?: string;
	metadata?: Record<string, unknown>;
}

export interface LogListResponse {
	logs: LogEntry[];
	total: number;
	page: number;
	pageSize: number;
	totalPages: number;
}

export interface LogFilters {
	level?: 'info' | 'warn' | 'error' | 'debug';
	source?: string;
	startDate?: string;
	endDate?: string;
	search?: string;
}

export async function getLogs(
	page: number = 1,
	pageSize: number = 10,
	filters?: LogFilters
): Promise<LogListResponse> {
	if (USE_MOCK_DATA) {
		await new Promise((resolve) => setTimeout(resolve, 300));
		let logs = getMockLogs(page, pageSize);
		
		// Apply filters
		if (filters?.search) {
			const searchTerm = String(filters.search).toLowerCase();
			logs = {
				...logs,
				logs: logs.logs.filter(
					(log: LogEntry) =>
						log.message.toLowerCase().includes(searchTerm) ||
						log.source?.toLowerCase().includes(searchTerm)
				),
			};
		}
		if (filters?.level) {
			logs = {
				...logs,
				logs: logs.logs.filter((log: LogEntry) => log.level === filters.level),
			};
		}
		
		return logs;
	}
	try {
		const params = new URLSearchParams({
			page: page.toString(),
			pageSize: pageSize.toString(),
		});

		if (filters) {
			Object.entries(filters).forEach(([key, value]) => {
				if (value !== undefined && value !== null) {
					params.append(key, String(value));
				}
			});
		}

		return await apiClient.get<LogListResponse>(`/logs?${params.toString()}`);
	} catch (error) {
		if (import.meta.env.DEV) {
			return getMockLogs(page, pageSize);
		}
		throw error as ApiError;
	}
}
