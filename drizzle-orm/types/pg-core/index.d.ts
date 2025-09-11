import { SQL } from '..';

export interface PgSchema {
    table: typeof pgTable,
    enum: typeof pgEnum
}

export const pgSchema: (name?: string) => PgSchema;

export type noConst<T> = {
    -readonly [K in keyof T]: T[K];
}

export type Equal<A, B> = (<T>() => T extends A ? 1 : 2) extends (<T>() => T extends B ? 1 : 2) ? true : false;
export type IsTrue<T> = Equal<T, true>;
export type OptionalIfTrue<A extends boolean, B> = IsTrue<A> extends true ? never : B;

export interface PgEnumColumn<Values extends readonly string[]> extends ColumnBase<Values[number], PgEnumColumn<Values>> {
    _values: Values;
}
export function pgEnum<T extends string, Values extends Readonly<[T, ...T[]]>>(name: string, values: Values | noConst<Values>): (columnName?: string) => PgEnumColumn<noConst<Values>>;

export type Table<Columns extends Record<string, ColumnBase<any, any>>> = {
    brand: 'Table';
    columns: Columns;
}

type TableHelpers<Columns extends Record<string, ColumnBase<any, any>>> = {
    $inferInsert: ColumnsToInsert<Columns>;
    $inferSelect: ColumnsToSelect<Columns>;
    _: Columns;
}

type ColumnsToSelect<Columns extends Record<string, ColumnBase<any, any>>> = {
    [K in keyof Columns]: Columns[K]['$type'];
}

type ColumnsToInsert<Columns extends Record<string, ColumnBase<any, any>>> = {
    [K in keyof Columns]?: Columns[K]['$type'];
}

export type TableInternal<Columns extends Record<string, ColumnBase<any, any>>> = {
    brand: 'Table',
    columns: Columns;
    name: string;
    config?: Record<string, any>;
    schema: undefined;
    inferInsert: ColumnsToInsert<Columns>;
    inferSelect: ColumnsToSelect<Columns>;
}

type TablePublic<Columns extends Record<string, ColumnBase<any, any>>> = { [K in keyof Columns]: Columns[K] } & {
    $inferSelect: ColumnsToSelect<Columns>;
    $inferInsert: ColumnsToInsert<Columns>;
    _: TableInternal<Columns>; 
}

export function pgTable<Columns extends Record<string, ColumnBase<any, any>>>(
    name: string,
    columns: Columns,
    extraConfig?: (table: Columns) => any[]
): TablePublic<Columns>;

export type CustomTypeValues = {
    data?: unknown;
    driverData?: unknown;
    config?: Record<string, any>;
    configRequired?: boolean;
}
export function customType<T extends CustomTypeValues & { configRequired: true}>(options: {
    dataType: (config: T['config']) => string;
    toDriver?: (value: T['data']) => T['driverData'];
    fromDriver?: (value: T['driverData']) => T['data'];
}): void;
export function customType<T extends CustomTypeValues & { configRequired?: false }>(options: {
    dataType: (config?: T['config']) => string;
    toDriver?: (value: T['data']) => T['driverData'];
    fromDriver?: (value: T['driverData']) => T['data'];
}): void;

//#region COLUMNS

type DeleteAction = 'cascade' | 'restrict' | 'no action' | 'set null' | 'set default';

export interface ColumnReferencesConfig {
    onDelete?: DeleteAction
    onUpdate?: DeleteAction
}

export interface ColumnBase<T, Self extends ColumnBase<any, any>> {
    brand: 'Column';
    $type: T;

    array(): Self extends ColumnBase<infer U, any> ? ColumnBase<U[], Self> : never;
    default(value: T | SQL<unknown>): Self;
    $defaultFn(): Self;
    notNull(): Self;
    primaryKey(): Self;
    unique(): Self;
    references<Ref extends ColumnBase<T, any>>(ref: () => Ref, options?: ColumnReferencesConfig): Self;
}

export interface PgBoolean extends ColumnBase<boolean, PgBoolean> {}
export interface PgInteger extends ColumnBase<number, PgInteger> {}
export interface PgText extends ColumnBase<string, PgText> {}
export interface PgTimestamp extends ColumnBase<Date, PgTimestamp> {
    defaultNow(): PgTimestamp;
}
export interface PgUuid extends ColumnBase<string, PgUuid> {
    defaultRandom(): PgUuid;
}
export interface PgVarchar extends ColumnBase<string, PgVarchar> {}
export interface PgVarcharConfig {
    length?: number;
}

export const boolean: (columnName?: string) => PgBoolean;
export const inet: (columnName?: string) => PgText;
export const integer: (columnName?: string) => PgInteger;
export const serial: (columnName?: string) => PgInteger;
export const text: (columnName?: string) => PgText;
export const timestamp: (columnName?: string) => PgTimestamp;
export const uuid: (columnName?: string) => PgUuid;
export const varchar: (columnName?: string, options?: PgVarcharConfig) => PgVarchar;

//#endregion



//#region INDEXES

export type IndexMethod = 'brin' | 'btree' | 'gin' | 'gist' | 'hash' | 'hnsw' | 'ivfflat' | 'spgist' | (string & {});

export interface IndexBase<Columns extends ColumnBase<any, any> = ColumnBase<any, any>> {
    on<Col0 extends Columns, Cols extends Columns[]>(column_0: Col0 | SQL<unknown>, ...columns: Cols | SQL<unknown>[]): IndexExtendedBase<Columns>;
    using<Col0 extends Columns, Cols extends Columns[]>(method: IndexMethod, column_0: Col0 | SQL<unknown>, ...columns: Cols | SQL<unknown>[]): IndexExtendedBase<Columns>;
}

export interface IndexExtendedBase<Columns extends ColumnBase<any, any> = ColumnBase<any, any>> {
    concurrently(): IndexExtendedBase<Columns>;
    where(condition: SQL<unknown>): IndexExtendedBase<Columns>;
    with(obj: Record<string, any>): IndexExtendedBase<Columns>;
}

export const index: <Columns extends ColumnBase<any, any> = ColumnBase<any, any>>(name: string | undefined) => IndexBase<Columns>;
export const uniqueIndex: <Columns extends ColumnBase<any, any> = ColumnBase<any, any>>(name: string | undefined) => IndexBase<Columns>;

export interface Check {
    name: string;
    value: SQL<unknown>;
}

export const check: (name: string, value: SQL<unknown>) => Check;

export interface ForeignKeyConfig<Columns extends ColumnBase<any, any> = ColumnBase<any, any>> {
    columns: Columns[]; // Make local only
    foreignColumns: Columns[];
    name: string;
}

export interface ForeignKeyMethods {
    onDelete(action: DeleteAction): ForeignKeyMethods;
    onUpdate(action: DeleteAction): ForeignKeyMethods;
}

export const foreignKey: <Columns extends ColumnBase<any, any> = ColumnBase<any, any>>(config: ForeignKeyConfig<Columns>) => ForeignKeyMethods;

export interface PrimaryKeyConfig<Columns extends ColumnBase<any, any> = ColumnBase<any, any>> {
    columns: Columns[]; // Make local only
    name: string;
}

export const primaryKey: <Columns extends ColumnBase<any, any> = ColumnBase<any, any>>(config: PrimaryKeyConfig<Columns>) => void;

//#endregion