import { Enum } from './enum.js';
import type { DataType } from './types.js';

export type ColumnOptions = {
    primaryKey?: boolean;
    unique?: boolean;
    notNull?: boolean;
    default?: string | number;
}

export const requiredExtensions = new Set<string>();

export class ColumnClass {
    constructor(
        public type: DataType | Enum,
        public options: ColumnOptions = {}
    ) {}

    toSQL(name: string): string {
        if ((this.type as any)?.name === 'uuid' && this.options.default === 'random') {
            requiredExtensions.add('pgcrypto');
        }

        let sql = `${name} ${this.type instanceof Enum ? this.type.name : this.type}`;
        if (this.options.primaryKey) sql += ' PRIMARY KEY';
        if (this.options.unique) sql += ' UNIQUE';
        if (this.options.notNull) sql += ' NOT NULL';
        if (this.options.default !== undefined) sql += ` DEFAULT ${this.options.default}`
        return sql;
    }
}

export const Column = (type: DataType | Enum, opts?: ColumnOptions) => new ColumnClass(type, opts);