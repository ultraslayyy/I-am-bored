import { SQL, SQLQuery, and, or, eq, SQLType } from '..';
import { ColumnBase, TablePublic } from '../pg-core';

type ColumnType<C> = C extends { $type: infer T } ? T : never;
type Row<T> = T extends { _: { brand: 'Table'; columns: infer C } } ? { [K in keyof C]: ColumnType<C[K]> } : never;

type ColumnSelectionOptions<TTable> = {
    [K in keyof Row<TTable>]?: boolean;
}

type TableOnly<T> = {
    [K in keyof T as T[K] extends { _: { brand: 'Table' } } ? K : never]: T[K];
}

type Simplify<T> = { [K in keyof T]: T[K] }
type ColumnSelection<RowType, Selection extends Record<string, boolean | undefined> | undefined, Extras extends Record<string, any> = {}> = Simplify<{ [K in keyof (Selection extends undefined ? RowType : keyof Selection extends never ? RowType : keyof { [K in keyof Selection as Selection[K] extends true ? K : never]: any } extends never ? { [K in keyof RowType as K extends keyof Selection ? never : K]: RowType[K] } : { [K in keyof RowType as K extends keyof Selection ? (Selection[K] extends true ? K : never) : never]: RowType[K] }) | keyof Extras]: K extends keyof Extras ? Extras[K] : K extends keyof RowType ? RowType[K] : never; }>;
type ColumnSelectionFromInput<Table extends TablePublic<any>, Input extends Record<string, any> | undefined = undefined> = Simplify<{ [K in keyof (Input extends undefined ? Row<Table> : keyof Input extends never ? Row<Table> : { [K in keyof Input]: K extends keyof Row<Table> ? Row<Table>[K] : Input[K] extends SQL<infer T> ? T : never; })]: K extends keyof Row<Table> ? Row<Table>[K] : Input extends Record<string, any> ? K extends keyof Input ? Input[K] extends SQL<infer T> ? T : never : never : never }>;

type FlattenExtras<E extends Record<string, SQL<any>> | undefined> = E extends Record<string, SQL<any>> ? { [K in keyof E]: SQLType<E[K]> extends Record<string, infer V> ? V : SQLType<E[K]> } : {}

export type WhereHelpers = {
    and: typeof and,
    or: typeof or,
    eq: typeof eq
}

type QueryOptions<TTable> = {
    columns?: ColumnSelectionOptions<TTable>;
    where?: SQLQuery | ((table: TTable, handlers: WhereHelpers) => SQLQuery) | undefined;
    extras?: Record<string, SQL<any>>;
    orderBy?: ColumnBase<any, any> | undefined;
    offset?: number | undefined;
}

interface ManyQueryOptions<TTable> extends QueryOptions<TTable> {
    limit?: number;
}

type SelectInput<Table extends TablePublic<any>> = {
    [K in keyof Table['_']['columns']]?: Table['_']['columns'][K];
} & {
    [key: string]: SQL<any>;
}

export interface SelectFromBuilder<Table extends TablePublic<any>, Input extends Record<string, any> = Row<Table>, Limit extends number | undefined = undefined> extends PromiseLike<Limit extends 1 ? ColumnSelectionFromInput<Table, Input> : ColumnSelectionFromInput<Table, Input>[]> {
    limit<N extends number>(limit: N): SelectFromBuilder<Table, Input, N>;
    offset(offset?: number): SelectFromBuilder<Table, Input, Limit>;
    where(query: SQLQuery | ((table: Table, helpers: WhereHelpers) => SQLQuery)): SelectFromBuilder<Table, Input, Limit>;
    orderBy(column: ColumnBase<any, any>, direction?: 'asc' | 'desc'): SelectFromBuilder<Table, Input, Limit>;
    groupBy(column: ColumnBase<any, any>): SelectFromBuilder<Table, Input, Limit>;
}

export function drizzle<TSchema extends Record<string, any>>(client: any, options: { schema: TSchema }): Db<TSchema>;
export type Db<TSchema extends Record<string, any>> = {
    query: {
        [K in keyof TableOnly<TSchema>]: {
            findFirst<C extends Partial<Record<keyof Row<TableOnly<TSchema>[K]>, boolean>> | undefined = undefined, E extends Record<string, SQL<any>> | undefined = QueryOptions<TableOnly<TSchema>[K]>['extras']>(opts?: {
                columns?: C;
                where?: QueryOptions<TableOnly<TSchema>[K]>['where'];
                extras?: E;
                orderBy?: QueryOptions<TableOnly<TSchema>[K]>['orderBy'];
                offset?: number;
            }): Promise<ColumnSelection<Row<TableOnly<TSchema>[K]>, C, FlattenExtras<E>>>;
            findMany<C extends Partial<Record<keyof Row<TableOnly<TSchema>[K]>, boolean>> | undefined = undefined, E extends Record<string, SQL<any>> | undefined = QueryOptions<TableOnly<TSchema>[K]>['extras']>(opts?: {
                columns?: C;
                where?: QueryOptions<TableOnly<TSchema>[K]>['where'],
                extras?: E;
                orderBy?: QueryOptions<TableOnly<TSchema>[K]>['orderBy'];
                offset?: number;
            }): Promise<ColumnSelection<Row<TableOnly<TSchema>[K]>, C, FlattenExtras<E>>[]>;
        }
    }
    select<Input extends Record<string, any> = Row<any>>(input?: Input): {
        from<Columns extends Record<string, ColumnBase<any, any>>>(source: SQL<unknown> | TablePublic<Columns>): SelectFromBuilder<TablePublic<Columns>, Input>;
    }
    update<Columns extends Record<string, ColumnBase<any, any>>>(table: TablePublic<Columns>): {
        set(): {

        }
    }
}