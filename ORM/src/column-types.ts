import { requiredExtensions } from './column.js';

export type ColumnOptions = {
    primaryKey?: boolean;
    unique?: boolean;
    notNull?: boolean;
    default?: string | number | (() => string);
}

export abstract class ColumnType {
    constructor(public name: string, public options: ColumnOptions = {}) {}

    abstract toSQL(columnName: string): string;

    protected applyCommonOptions(sql: string): string {
        if (this.options.primaryKey) sql += ' PRIMARY KEY';
        if (this.options.unique) sql += ' UNIQUE';
        if (this.options.notNull) sql += ' NOT NULL';
        if (this.options.default !== undefined) {
            let defaultVal = this.options.default;
            if (typeof defaultVal === 'function') {
                defaultVal = defaultVal();
            }
            sql += ` DEFAULT ${defaultVal}`;
        };
        return sql;
    }
}

export class TextColumn extends ColumnType {
    toSQL(name: string): string {
        return this.applyCommonOptions(`${name} TEXT`);
    }
}

export class VarcharColumn extends ColumnType {
    constructor(public name: string, public length: number, options?: ColumnOptions) {
        super(name, options);
    }

    toSQL(name: string): string {
        return this.applyCommonOptions(`${name} VARCHAR (${this.length})`);
    }
}

export class SerialColumn extends ColumnType {
    toSQL(name: string): string {
        return this.applyCommonOptions(`${name} SERIAL`);
    }
}

export class TimestampColumn extends ColumnType {
    toSQL(name: string): string {
        return this.applyCommonOptions(`${name} TIMESTAMP`);
    }
}

export class UuidColumn extends ColumnType {
    toSQL(name: string): string {
        return this.applyCommonOptions(`${name} uuid`)
    }

    defaultRandom(): this {
        this.options.default = () => `gen_random_uuid()`;
        return this;
    }
}