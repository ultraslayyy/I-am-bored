import {
    type ColumnOptions,
    ColumnType,

    SerialColumn,
    TextColumn,
    TimestampColumn,
    UuidColumn,
    VarcharColumn
} from './column-types.js';

import { TsVectorColumn, type TsVectorOptions } from './custom-columns/ts_vector.js';

export function text(name: string, opts?: ColumnOptions) { return new TextColumn(name, opts); }
export function varchar(name: string, length: number, opts?: ColumnOptions) { return new VarcharColumn(name, length, opts) }
export function serial(name: string, opts?: ColumnOptions) { return new SerialColumn(name, opts) }
export function timestamp(name: string, opts?: ColumnOptions) { return new TimestampColumn(name, opts); }
export function ts_vector(name: string, tsvectorOpts: TsVectorOptions, opts?: ColumnOptions) { return new TsVectorColumn(name, tsvectorOpts, opts )}
export function uuid(name: string, opts?: ColumnOptions) { return new UuidColumn(name, opts) }

export { ColumnType as CustomType }