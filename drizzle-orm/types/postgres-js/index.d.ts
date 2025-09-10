type OnlyTables<T> = {
    [K in keyof T as T[K] extends { kind: 'table' } ? K : never]: T[K];
}

export type Db<TSchema extends Record<string, any>> = {
    query: {
        [K in keyof OnlyTables<TSchema>]: {
            findFirst: () => Promise<{ [C in keyof OnlyTables<TSchema>[K]['_columns']]: any}>;
            findMany: () => Promise<{ [C in keyof OnlyTables<TSchema>[K]['_columns']]: any }[]>;
        }
    }
}

export function drizzle<TSchema extends Record<string, any>>(client: any, options: { schema: TSchema }): Db<TSchema>;