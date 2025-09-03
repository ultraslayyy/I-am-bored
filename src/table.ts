import { ColumnClass } from './column.js';

export class Table {
    constructor(public name: string, public columns: Record<string, ColumnClass>) {}

    toSQL(): string {
        const colDefs = Object.entries(this.columns).map(([name, col]) => col.toSQL(name));
        return `CREATE TABLE ${this.name} (\n  ${colDefs.join(',\n  ')}\n);`;
    }
}