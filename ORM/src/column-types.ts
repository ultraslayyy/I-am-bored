export type ColumnOptions = {
    primaryKey?: boolean;
    unique?: boolean;
    notNull?: boolean;
    default?: string | number;
}

export abstract class ColumnType {
    constructor(public name: string, public options: ColumnOptions = {}) {}

    abstract toSQL(columnName: string): string;

    protected applyCommonOptions(sql: string): string {
        if (this.options.primaryKey) sql += ' PRIMARY KEY';
        if (this.options.unique) sql += ' UNIQUE';
        if (this.options.notNull) sql += ' NOT NULL';
        if (this.options.default !== undefined) sql += ` DEFAULT ${this.options.default}`;
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