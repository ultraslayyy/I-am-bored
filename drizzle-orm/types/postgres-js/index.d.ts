type ColumnType<C> = C extends { $type: infer T } ? T : never;
type Row<T> = T extends { _: { brand: 'Table'; columns: infer C } } ? { [K in keyof C]: ColumnType<C[K]> } : never;

type TableOnly<T> = {
    [K in keyof T as T[K] extends { _: { brand: 'Table' } } ? K : never]: T[K];
}

export type Db<TSchema extends Record<string, any>> = {
    query: {
        [K in keyof TableOnly<TSchema>]: {
            findFirst: () => Promise<Row<TableOnly<TSchema>[K]>>;
            findMany: () => Promise<Row<TableOnly<TSchema>[K]>[]>;
        }
    }
}

export function drizzle<TSchema extends Record<string, any>>(client: any, options: { schema: TSchema }): Db<TSchema>;