import { ColumnType } from './column-types.js';

export class Table {
    constructor(public name: string, public columns: Record<string, ColumnType>) {}

    toSQL(): string {
        const colDefs = Object.entries(this.columns).map(([name, col]) => col.toSQL(name));
        return `CREATE TABLE ${this.name} (\n  ${colDefs.join(',\n  ')}\n);`;
    }
}