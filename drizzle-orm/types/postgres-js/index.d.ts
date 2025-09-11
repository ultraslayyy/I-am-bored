import { SQL, SQLQuery, and, or, eq } from '..';
import { ColumnBase, TablePublic } from '../pg-core';

type ColumnType<C> = C extends { $type: infer T } ? T : never;
type Row<T> = T extends { _: { brand: 'Table'; columns: infer C } } ? { [K in keyof C]: ColumnType<C[K]> } : never;

type TableOnly<T> = {
    [K in keyof T as T[K] extends { _: { brand: 'Table' } } ? K : never]: T[K];
}

type ColumnSelection<RowType, Selection extends Record<string, boolean | undefined> | undefined> = Selection extends undefined ? RowType : keyof { [K in keyof Selection as Selection[K] extends true ? K : never]: any; } extends never ? { [K in keyof RowType as K extends keyof Selection ? never : K]: RowType[K] } : { [K in keyof RowType as K extends keyof Selection ? (Selection[K] extends true ? K : never) : never]: RowType[K] }

export type WhereHelpers = {
    and: typeof and,
    or: typeof or,
    eq: typeof eq
}

type QueryOptions<TTable> = {
    columns?: Partial<Record<keyof Row<TTable>, boolean>>;
    where?: SQLQuery | ((table: TTable, handlers: WhereHelpers) => SQLQuery);
}

interface ManyQueryOptions<TTable> extends QueryOptions<TTable> {
    limit?: number;
    offset?: number;
}

export type Db<TSchema extends Record<string, any>> = {
    query: {
        [K in keyof TableOnly<TSchema>]: {
            findFirst<Opts extends QueryOptions<TableOnly<TSchema>[K]> = {}>(opts?: Opts): Promise<ColumnSelection<Row<TableOnly<TSchema>[K]>, Opts['columns']>>;
            findMany<Opts extends ManyQueryOptions<TableOnly<TSchema>[K]> = {}>(opts?: Opts): Promise<ColumnSelection<Row<TableOnly<TSchema>[K]>, Opts['columns']>[]>;
        }
    }
    select(): {
        from<Columns extends Record<string, ColumnBase<any, any>>>(source: SQL<unknown> | TablePublic<Columns>): SelectFromBuilder;
    }
    update<Columns extends Record<string, ColumnBase<any, any>>>(table: TablePublic<Columns>): {
        set(): {

        }
    }
}

export interface SelectFromBuilder {
    limit(limit: number): SelectFromBuilder;
}

export function drizzle<TSchema extends Record<string, any>>(client: any, options: { schema: TSchema }): Db<TSchema>;