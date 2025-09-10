export type noConst<T> = {
    -readonly [K in keyof T]: T[K];
}

export type equalValues<A, B> = (<A>() => A extends B ? 1 : 2) extends (<A>() => A extends B ? 1 : 2) ? true : false;

export type PgEnum<Values extends readonly string[]> = {
    kind: 'enum',
    _values: Values
}
export function pgEnum<T extends string, Values extends Readonly<[T, ...T[]]>>(name: string, values: Values | noConst<Values>): PgEnum<noConst<Values>>;

type ColumnType = 'string' | 'number' | 'boolean' | 'date';
export type Table<Columns extends Record<string, ColumnType>> = {
    kind: 'table';
    _columns: Columns;
}
export function pgTable<Columns extends Record<string, ColumnType>>(
    name: string,
    columns: Columns,
    extraConfig?: () => []
): Table<Columns>;


export type CustomTypeValues = {
    data?: unknown;
    driverData?: unknown;
    config?: Record<string, any>;
    configRequired?: boolean;
}
export function customType<T extends CustomTypeValues>(options: {
    dataType: (config: T['config'] | (equalValues<T['configRequired'], true> extends true ? never : undefined)) => string;
    toDriver?: (value: T['data']) => T['driverData'];
    fromDriver?: (value: T['driverData']) => T['data'];
}): void;

//#region COLUMNS



//#endregion