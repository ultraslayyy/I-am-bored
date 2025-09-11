import { ColumnBase } from './pg-core';

export type SQLQuery<T = unknown> = {
    text: string;
    values: T[];
}
export type SQL<T> = {
    (strings: TemplateStringsArray, ...values: unknown[]): SQLQuery<T>;
}

export function sql<T>(strings: TemplateStringsArray, ...params: any[]): SQL<T>;
export function and(...conditions: SQLQuery[]): SQLQuery;
export function or(...conditions: SQLQuery[]): SQLQuery;
export function eq<T>(column: ColumnBase<T, any>, value: T): SQLQuery;