import { Column, ColumnClass } from './column.js';

export class Enum {
    constructor(public name: string, public values: string[]) {
        const callable = ((columnName: string, opts: any = {}) => {
            return Column(this, { ...opts });
        }) as Enum & (( columnName: string, opts?: any) => ColumnClass);

        Object.setPrototypeOf(callable, Enum.prototype);
        return callable;
    }

    toSQL(): string {
        return `CREATE TYPE ${this.name} AS ENUM (${this.values.map((v) => `'${v}'`).join(', ')})`;
    }
}